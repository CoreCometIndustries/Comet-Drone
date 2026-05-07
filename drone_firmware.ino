// ============================================================
//  COMET DRONE - Flight Controller Firmware
//  ESP32-C3 Super Mini | MPU6050 | 0716 Coreless Motors
//  X-Quad Configuration | WiFi WebSocket Control
// ============================================================
//
//  Required Board: ESP32C3 Dev Module
//  Required Library: WebSockets by Links2004
//    Install via: Arduino IDE → Library Manager → "WebSockets"
//
//  Pin Wiring:
//    MPU6050 SDA → Default  MPU6050 SCL → Default
//    Motor FR    → GPIO0    Motor RL    → GPIO1
//    Motor FL    → GPIO3    Motor RR    → GPIO10
//    Battery ADC → GPIO6    LED         → GPIO8
//
// ============================================================

#include "config.h"
#include "imu.h"
#include "pid.h"
#include "motors.h"
// #include "battery.h"  // Battery monitoring removed
#include "wifi_control.h"

// ── Global Objects ──
IMU         imu;
Motors      motors;
// Battery     battery;  // Battery monitoring removed
WifiControl wifiCtrl;

// ── PID Controllers (cascaded: angle → rate) ──
PIDController rollAnglePID,  pitchAnglePID;
PIDController rollRatePID,   pitchRatePID;
PIDController yawRatePID;

// ── State ──
bool     armed = false;
uint8_t  flightMode = 0;     // 0 = angle, 1 = rate
unsigned long loopTimer = 0;
unsigned long lastTelemetry = 0;
// unsigned long lastBattery = 0;  // Battery monitoring removed
int      loopCount = 0;
int      loopHz = 0;
unsigned long hzTimer = 0;

// ── LED Helpers ──
void ledOn()   { digitalWrite(PIN_LED, LOW);  }   // Active LOW
void ledOff()  { digitalWrite(PIN_LED, HIGH); }
void ledBlink(int times, int ms) {
    for (int i = 0; i < times; i++) {
        ledOn(); delay(ms); ledOff(); delay(ms);
    }
}

// ── PID Initialization ──
void initPIDs() {
    // Angle PID (outer loop) - outputs desired angular rate
    rollAnglePID.setGains(ROLL_ANGLE_KP, ROLL_ANGLE_KI, ROLL_ANGLE_KD);
    rollAnglePID.setOutputLimits(-MAX_YAW_RATE, MAX_YAW_RATE);
    rollAnglePID.setIntegralLimit(PID_INTEGRAL_MAX);

    pitchAnglePID.setGains(PITCH_ANGLE_KP, PITCH_ANGLE_KI, PITCH_ANGLE_KD);
    pitchAnglePID.setOutputLimits(-MAX_YAW_RATE, MAX_YAW_RATE);
    pitchAnglePID.setIntegralLimit(PID_INTEGRAL_MAX);

    // Rate PID (inner loop) - outputs motor correction
    rollRatePID.setGains(ROLL_RATE_KP, ROLL_RATE_KI, ROLL_RATE_KD);
    rollRatePID.setOutputLimits(-PID_MAX_OUTPUT, PID_MAX_OUTPUT);
    rollRatePID.setIntegralLimit(PID_INTEGRAL_MAX);

    pitchRatePID.setGains(PITCH_RATE_KP, PITCH_RATE_KI, PITCH_RATE_KD);
    pitchRatePID.setOutputLimits(-PID_MAX_OUTPUT, PID_MAX_OUTPUT);
    pitchRatePID.setIntegralLimit(PID_INTEGRAL_MAX);

    yawRatePID.setGains(YAW_RATE_KP, YAW_RATE_KI, YAW_RATE_KD);
    yawRatePID.setOutputLimits(-PID_MAX_OUTPUT, PID_MAX_OUTPUT);
    yawRatePID.setIntegralLimit(PID_INTEGRAL_MAX);
}

void resetPIDs() {
    rollAnglePID.reset();
    pitchAnglePID.reset();
    rollRatePID.reset();
    pitchRatePID.reset();
    yawRatePID.reset();
}

// ============================================================
//  SETUP
// ============================================================
void setup() {
    // Serial debug
    Serial.begin(115200);
    delay(500);
    Serial.println();
    Serial.println("================================");
    Serial.println("  COMET DRONE - Flight Controller");
    Serial.println("  ESP32-C3 | MPU6050 | 0716 Motors");
    Serial.println("================================");

    // Status LED
    pinMode(PIN_LED, OUTPUT);
    ledOff();

    // Initialize IMU
    Serial.print("[IMU] Initializing MPU6050... ");
    if (imu.begin()) {
        Serial.println("OK");
    } else {
        Serial.println("FAILED! Check wiring.");
        while (1) { ledBlink(3, 100); delay(500); }  // Halt with error blink
    }

    // Calibrate gyro (keep drone perfectly still!)
    Serial.println("[IMU] Calibrating gyro - KEEP STILL!");
    ledOn();  // LED on during calibration
    imu.calibrate();
    ledOff();
    Serial.println("[IMU] Calibration complete");

    // Initialize motors (all off)
    Serial.println("[MOT] Initializing motors...");
    motors.begin();
    motors.stop();
    Serial.println("[MOT] Motors ready (all stopped)");

    // Battery monitoring removed (no voltage divider connected)
    Serial.println("[BAT] Battery monitoring DISABLED");

    // Initialize PIDs
    Serial.println("[PID] Initializing PID controllers...");
    initPIDs();

    // Initialize WiFi + WebSocket
    Serial.println("[NET] Starting WiFi AP...");
    wifiCtrl.begin();

    // Ready!
    Serial.println("================================");
    Serial.println("  READY - Connect to WiFi:");
    Serial.printf("  SSID: %s\n", WIFI_SSID);
    Serial.printf("  Pass: %s\n", WIFI_PASSWORD);
    Serial.println("  Open: http://192.168.4.1");
    Serial.println("================================");
    ledBlink(3, 150);  // 3 blinks = ready

    // Initialize timing
    loopTimer = micros();
    hzTimer = millis();
    lastTelemetry = millis();
    // lastBattery = millis();  // Battery monitoring removed
}

// ============================================================
//  MAIN LOOP - Runs at 250Hz (4ms)
// ============================================================
void loop() {
    // ── Wait for precise loop timing ──
    // CRITICAL: Use yield() instead of pure busy-wait!
    // ESP32-C3 is SINGLE CORE - busy-wait starves WiFi stack → disconnects
    while (micros() - loopTimer < LOOP_TIME_US) {
        yield();  // Give WiFi/TCP stack time to process
    }
    float dt = (micros() - loopTimer) / 1000000.0;  // Delta time in seconds
    loopTimer = micros();

    // ── 1. Update IMU ──
    imu.update(dt);
    IMUData imuData = imu.getData();

    // ── 2. Process WiFi / WebSocket commands ──
    wifiCtrl.update();
    ControlCommand cmd = wifiCtrl.getCommand();

    // ── 3. Failsafe check ──
    bool signalLost = (millis() - wifiCtrl.lastCommandTime() > SIGNAL_TIMEOUT)
                      && wifiCtrl.lastCommandTime() > 0;

    if (signalLost && armed) {
        Serial.println("[SAFE] Signal lost - DISARMING!");
        armed = false;
        motors.stop();
        resetPIDs();
    }

    // ── 4. Battery check ── (DISABLED - no voltage divider)
    // Battery monitoring removed to prevent false critical readings

    // ── 5. Arm / Disarm logic ──
    if (cmd.armed && !armed) {
        // Arm requested - safety check: throttle must be near zero
        if (cmd.throttle < 0.05) {
            armed = true;
            resetPIDs();
            Serial.println("[ARM] >>> ARMED <<<");
        }
    } else if (!cmd.armed && armed) {
        armed = false;
        motors.stop();
        resetPIDs();
        Serial.println("[ARM] >>> DISARMED <<<");
    }

    flightMode = cmd.mode;

    // ── 6. Flight control ──
    if (armed) {
        // Scale command inputs to physical units
        float throttleVal = cmd.throttle * MOTOR_MAX;   // 0-255
        float rollCmd  = cmd.roll  * MAX_ANGLE;          // ±45°
        float pitchCmd = cmd.pitch * MAX_ANGLE;          // ±45°
        float yawCmd   = cmd.yaw   * MAX_YAW_RATE;       // ±180°/s

        float rollOutput, pitchOutput, yawOutput;

        if (flightMode == 0) {
            // ANGLE MODE: Cascaded PID (angle → rate → motor)
            // Outer loop: desired angle → desired rate
            float rollRateCmd  = rollAnglePID.compute(rollCmd, imuData.roll, dt);
            float pitchRateCmd = pitchAnglePID.compute(pitchCmd, imuData.pitch, dt);

            // Inner loop: desired rate → motor correction
            rollOutput  = rollRatePID.compute(rollRateCmd, imuData.rollRate, dt);
            pitchOutput = pitchRatePID.compute(pitchRateCmd, imuData.pitchRate, dt);
        } else {
            // RATE MODE: Direct rate PID (stick = desired rate)
            float rollRateCmd  = cmd.roll  * MAX_YAW_RATE;
            float pitchRateCmd = cmd.pitch * MAX_YAW_RATE;

            rollOutput  = rollRatePID.compute(rollRateCmd, imuData.rollRate, dt);
            pitchOutput = pitchRatePID.compute(pitchRateCmd, imuData.pitchRate, dt);
        }

        // Yaw is always rate-controlled (no absolute reference)
        yawOutput = yawRatePID.compute(yawCmd, imuData.yawRate, dt);

        // Motor mixing
        motors.mix(throttleVal, rollOutput, pitchOutput, yawOutput);

    } else {
        // DISARMED - motors off
        motors.stop();
    }

    // ── 7. Send telemetry ──
    if (millis() - lastTelemetry >= TELEMETRY_INTERVAL) {
        lastTelemetry = millis();
        int m1, m2, m3, m4;
        motors.getValues(&m1, &m2, &m3, &m4);
        wifiCtrl.sendTelemetry(
            imuData.roll, imuData.pitch, imuData.yaw,
            0.0, 0,  // Battery monitoring disabled
            armed, loopHz, m1, m2, m3, m4
        );
    }

    // ── 8. Loop frequency counter ──
    loopCount++;
    if (millis() - hzTimer >= 1000) {
        loopHz = loopCount;
        loopCount = 0;
        hzTimer = millis();
    }

    // ── 9. Status LED ──
    // Fast blink = armed, slow blink = disarmed, solid = no connection
    if (armed) {
        digitalWrite(PIN_LED, (millis() / 100) % 2 == 0 ? LOW : HIGH);
    } else if (false) {  // Battery warning LED disabled
        digitalWrite(PIN_LED, (millis() / 200) % 2 == 0 ? LOW : HIGH);
    } else {
        digitalWrite(PIN_LED, wifiCtrl.isConnected() ? HIGH : ((millis() / 1000) % 2 == 0 ? LOW : HIGH));
    }

    // ── 10. Serial PID tuning interface ──
    handleSerialCommands();
}

// ============================================================
//  Serial Command Interface (for PID tuning)
//  Format: "RP:0.7" sets Roll Rate Kp to 0.7
//    RP/RI/RD = Roll rate Kp/Ki/Kd
//    PP/PI/PD = Pitch rate Kp/Ki/Kd
//    YP/YI/YD = Yaw rate Kp/Ki/Kd
//    AP/AI/AD = Roll angle Kp/Ki/Kd
//    BP/BI/BD = Pitch angle Kp/Ki/Kd
//    MT:x     = Test motor x (0-3) at speed 80
//    MS       = Stop all motors
//    ST       = Print current status
// ============================================================
void handleSerialCommands() {
    if (!Serial.available()) return;

    String input = Serial.readStringUntil('\n');
    input.trim();
    if (input.length() < 2) return;

    char c1 = input[0], c2 = input[1];
    float val = 0;
    if (input.length() > 3 && input[2] == ':') {
        val = input.substring(3).toFloat();
    }

    if (c1 == 'R' && c2 == 'P') { rollRatePID.setKp(val);  Serial.printf("Roll Rate Kp = %.3f\n", val); }
    if (c1 == 'R' && c2 == 'I') { rollRatePID.setKi(val);  Serial.printf("Roll Rate Ki = %.3f\n", val); }
    if (c1 == 'R' && c2 == 'D') { rollRatePID.setKd(val);  Serial.printf("Roll Rate Kd = %.3f\n", val); }
    if (c1 == 'P' && c2 == 'P') { pitchRatePID.setKp(val); Serial.printf("Pitch Rate Kp = %.3f\n", val); }
    if (c1 == 'P' && c2 == 'I') { pitchRatePID.setKi(val); Serial.printf("Pitch Rate Ki = %.3f\n", val); }
    if (c1 == 'P' && c2 == 'D') { pitchRatePID.setKd(val); Serial.printf("Pitch Rate Kd = %.3f\n", val); }
    if (c1 == 'Y' && c2 == 'P') { yawRatePID.setKp(val);   Serial.printf("Yaw Rate Kp = %.3f\n", val); }
    if (c1 == 'Y' && c2 == 'I') { yawRatePID.setKi(val);   Serial.printf("Yaw Rate Ki = %.3f\n", val); }
    if (c1 == 'Y' && c2 == 'D') { yawRatePID.setKd(val);   Serial.printf("Yaw Rate Kd = %.3f\n", val); }
    if (c1 == 'A' && c2 == 'P') { rollAnglePID.setKp(val);  Serial.printf("Roll Angle Kp = %.3f\n", val); }
    if (c1 == 'A' && c2 == 'I') { rollAnglePID.setKi(val);  Serial.printf("Roll Angle Ki = %.3f\n", val); }
    if (c1 == 'A' && c2 == 'D') { rollAnglePID.setKd(val);  Serial.printf("Roll Angle Kd = %.3f\n", val); }
    if (c1 == 'B' && c2 == 'P') { pitchAnglePID.setKp(val); Serial.printf("Pitch Angle Kp = %.3f\n", val); }
    if (c1 == 'B' && c2 == 'I') { pitchAnglePID.setKi(val); Serial.printf("Pitch Angle Ki = %.3f\n", val); }
    if (c1 == 'B' && c2 == 'D') { pitchAnglePID.setKd(val); Serial.printf("Pitch Angle Kd = %.3f\n", val); }

    if (c1 == 'M' && c2 == 'T') {
        int m = (int)val;
        if (m >= 0 && m <= 3) {
            motors.testMotor(m, 80);
            Serial.printf("Testing motor %d at speed 80\n", m);
        }
    }
    if (c1 == 'M' && c2 == 'S') {
        motors.stop();
        Serial.println("All motors stopped");
    }
    if (c1 == 'S' && c2 == 'T') {
        IMUData d = imu.getData();
        Serial.println("──── STATUS ────");
        Serial.printf("Roll:  %.2f° (rate: %.1f°/s)\n", d.roll, d.rollRate);
        Serial.printf("Pitch: %.2f° (rate: %.1f°/s)\n", d.pitch, d.pitchRate);
        Serial.printf("Yaw:   %.2f° (rate: %.1f°/s)\n", d.yaw, d.yawRate);
        Serial.println("Batt:  DISABLED");
        Serial.printf("Armed: %s  Mode: %s\n", armed ? "YES" : "NO", flightMode == 0 ? "ANGLE" : "RATE");
        Serial.printf("Loop:  %d Hz\n", loopHz);
        int m1, m2, m3, m4;
        motors.getValues(&m1, &m2, &m3, &m4);
        Serial.printf("Motors: FR=%d RL=%d FL=%d RR=%d\n", m1, m2, m3, m4);
        Serial.printf("PID Roll Rate:  Kp=%.3f Ki=%.3f Kd=%.3f\n",
                       rollRatePID.getKp(), rollRatePID.getKi(), rollRatePID.getKd());
        Serial.printf("PID Pitch Rate: Kp=%.3f Ki=%.3f Kd=%.3f\n",
                       pitchRatePID.getKp(), pitchRatePID.getKi(), pitchRatePID.getKd());
        Serial.printf("PID Yaw Rate:   Kp=%.3f Ki=%.3f Kd=%.3f\n",
                       yawRatePID.getKp(), yawRatePID.getKi(), yawRatePID.getKd());
        Serial.println("────────────────");
    }
}

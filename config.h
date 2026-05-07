#ifndef CONFIG_H
#define CONFIG_H

// ============================================================
//  COMET DRONE - Configuration
//  ESP32-C3 Super Mini | MPU6050 | 0716 Coreless Motors
//  X-Quad Configuration
// ============================================================

// === Pin Definitions ===

#define PIN_MOTOR_FR    0       // Front-Right motor (CCW)
#define PIN_MOTOR_RL    3       // Rear-Left motor   (CCW)
#define PIN_MOTOR_FL    1      // Front-Left motor  (CW)
#define PIN_MOTOR_RR    4      // Rear-Right motor  (CW)
#define PIN_BATTERY     6       // ADC input (voltage divider)
#define PIN_LED         8       // Onboard LED (active LOW)

// === Motor PWM Settings ===
#define PWM_FREQUENCY   20000   // 20kHz - above audible range
#define PWM_RESOLUTION  8       // 8-bit resolution (0-255)
#define MOTOR_MIN       0       // Minimum PWM value
#define MOTOR_MAX       255     // Maximum PWM value
#define THROTTLE_HEADROOM 80    // PWM headroom reserved for PID corrections
#define MOTOR_IDLE      20      // Idle spin when armed (keeps motors warm)

// === IMU Settings ===
#define MPU6050_ADDR          0x68
#define GYRO_SENSITIVITY      65.5    // LSB/(°/s) at ±500°/s
#define ACCEL_SENSITIVITY     8192.0  // LSB/g at ±4g
#define COMPLEMENTARY_ALPHA   0.98    // Filter coefficient (trust gyro)
#define CALIBRATION_SAMPLES   2000    // Samples for gyro offset calibration

// === PID Gains - Angle Loop (outer) ===
// Tuned for 0716 coreless motors on micro frame
#define ROLL_ANGLE_KP    2.0
#define ROLL_ANGLE_KI    0.5     // Needed to correct steady-state tilt/drift!
#define ROLL_ANGLE_KD    0.1

#define PITCH_ANGLE_KP   2.0
#define PITCH_ANGLE_KI   0.5     // Needed to correct steady-state tilt/drift!
#define PITCH_ANGLE_KD   0.1

// === PID Gains - Rate Loop (inner) ===
#define ROLL_RATE_KP     0.5
#define ROLL_RATE_KI     0.1      // Reduced from 0.4 - was causing integral windup & drift!
#define ROLL_RATE_KD     0.03

#define PITCH_RATE_KP    0.5
#define PITCH_RATE_KI    0.1      // Reduced from 0.4 - was causing integral windup & drift!
#define PITCH_RATE_KD    0.03

#define YAW_RATE_KP      0.8
#define YAW_RATE_KI      0.15
#define YAW_RATE_KD      0.0

// === PID Limits ===
#define PID_MAX_OUTPUT    120.0   // Reduced - 200 was too aggressive for 0716 motors
#define PID_INTEGRAL_MAX  50.0    // Reduced to prevent integral windup
#define MAX_ANGLE         45.0    // Max tilt angle (degrees)
#define MAX_YAW_RATE      180.0   // Max yaw rate (°/s)

// === Control Loop ===
#define LOOP_FREQUENCY    250     // Hz
#define LOOP_TIME_US      4000    // Microseconds per loop (1e6/250)

// === WiFi Settings ===
#define WIFI_SSID         "CometDrone"
#define WIFI_PASSWORD     "drone1234"
#define WIFI_CHANNEL      6
#define WS_PORT           81      // WebSocket port
#define HTTP_PORT         80      // Web server port

// === Battery Monitoring ===
#define BATTERY_DIVIDER   2.0     // Voltage divider ratio
#define BATTERY_FULL      4.20    // 1S LiPo full voltage
#define BATTERY_EMPTY     3.00    // 1S LiPo empty voltage
#define BATTERY_WARNING   3.30    // Warning threshold
#define BATTERY_CRITICAL  3.00    // Critical - auto disarm
#define BATTERY_INTERVAL  500     // Read interval (ms)

// === Safety ===
#define SIGNAL_TIMEOUT    500     // Failsafe timeout (ms)
#define TELEMETRY_INTERVAL 200    // Telemetry send interval (ms) - reduced for WiFi stability

#endif

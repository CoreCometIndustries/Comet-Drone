#include "motors.h"
#include "config.h"

// Motor pin lookup table
static const uint8_t motorPins[4] = {
    PIN_MOTOR_FR,  // M1: Front-Right (CCW)
    PIN_MOTOR_RL,  // M2: Rear-Left   (CCW)
    PIN_MOTOR_FL,  // M3: Front-Left  (CW)
    PIN_MOTOR_RR   // M4: Rear-Right  (CW)
};

void Motors::begin() {
    for (int i = 0; i < 4; i++) {
        // Attach each motor pin to LEDC with 20kHz PWM
        ledcAttach(motorPins[i], PWM_FREQUENCY, PWM_RESOLUTION);
        ledcWrite(motorPins[i], 0);
        _m[i] = 0;
    }
}

// X-Quad motor mixing
//
//    Front
//  M3(CW)  M1(CCW)
//      \  /
//       \/
//       /\
//      /  \
//  M2(CCW) M4(CW)
//     Rear
//
// M1 (FR, CCW) = Throttle - Roll - Pitch + Yaw
// M2 (RL, CCW) = Throttle + Roll + Pitch + Yaw
// M3 (FL, CW)  = Throttle + Roll - Pitch - Yaw
// M4 (RR, CW)  = Throttle - Roll + Pitch - Yaw

void Motors::mix(float throttle, float rollPID, float pitchPID, float yawPID) {
    // Cap throttle to leave headroom for PID corrections
    // Without this, at full throttle PID can't add correction → loss of control
    float maxThrottle = MOTOR_MAX - THROTTLE_HEADROOM;
    float thr = constrain(throttle, 0, maxThrottle);

    _m[0] = (int)(thr - rollPID - pitchPID + yawPID);  // FR CCW
    _m[1] = (int)(thr + rollPID + pitchPID + yawPID);  // RL CCW
    _m[2] = (int)(thr + rollPID - pitchPID - yawPID);  // FL CW
    _m[3] = (int)(thr - rollPID + pitchPID - yawPID);  // RR CW

    // Dynamic scaling: if any motor exceeds max, scale all motors proportionally
    // This preserves PID correction ratios instead of hard clamping
    int maxVal = _m[0];
    int minVal = _m[0];
    for (int i = 1; i < 4; i++) {
        if (_m[i] > maxVal) maxVal = _m[i];
        if (_m[i] < minVal) minVal = _m[i];
    }

    if (maxVal > MOTOR_MAX) {
        int overflow = maxVal - MOTOR_MAX;
        for (int i = 0; i < 4; i++) {
            _m[i] -= overflow;  // Shift all down equally
        }
    }

    for (int i = 0; i < 4; i++) {
        // Enforce minimum idle spin when throttle is above zero
        if (thr > MOTOR_IDLE) {
            _m[i] = constrain(_m[i], MOTOR_IDLE, MOTOR_MAX);
        } else {
            _m[i] = constrain(_m[i], MOTOR_MIN, MOTOR_MAX);
        }
        write(i, _m[i]);
    }
}

void Motors::stop() {
    for (int i = 0; i < 4; i++) {
        _m[i] = 0;
        write(i, 0);
    }
}

void Motors::testMotor(uint8_t motor, uint8_t speed) {
    if (motor < 4) {
        _m[motor] = speed;
        write(motor, speed);
    }
}

void Motors::write(uint8_t motor, int value) {
    if (motor < 4) {
        ledcWrite(motorPins[motor], constrain(value, 0, 255));
    }
}

void Motors::getValues(int* m1, int* m2, int* m3, int* m4) {
    *m1 = _m[0];
    *m2 = _m[1];
    *m3 = _m[2];
    *m4 = _m[3];
}

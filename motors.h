#ifndef MOTORS_H
#define MOTORS_H

#include <Arduino.h>

class Motors {
public:
    void begin();
    void mix(float throttle, float rollPID, float pitchPID, float yawPID);
    void stop();
    void testMotor(uint8_t motor, uint8_t speed);  // 0-3 motor index
    void getValues(int* m1, int* m2, int* m3, int* m4);

private:
    void write(uint8_t motor, int value);
    int _m[4];  // Current motor values
};

#endif

#ifndef IMU_H
#define IMU_H

#include <Arduino.h>
#include <Wire.h>

// IMU data structure
struct IMUData {
    float roll;        // Angle in degrees
    float pitch;       // Angle in degrees
    float yaw;         // Angle in degrees (gyro-integrated)
    float rollRate;    // Angular rate °/s
    float pitchRate;   // Angular rate °/s
    float yawRate;     // Angular rate °/s
};

class IMU {
public:
    bool begin();
    void calibrate();
    void update(float dt);
    IMUData getData();
    bool isCalibrated() { return _calibrated; }

private:
    void readRaw();

    float _accelX, _accelY, _accelZ;
    float _gyroX, _gyroY, _gyroZ;
    float _gyroOffX, _gyroOffY, _gyroOffZ;
    float _accelOffX, _accelOffY, _accelOffZ;  // Accel calibration offsets
    float _roll, _pitch, _yaw;
    float _rollRate, _pitchRate, _yawRate;
    bool  _calibrated;
};

#endif

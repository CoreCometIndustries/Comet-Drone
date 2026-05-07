#include "imu.h"
#include "config.h"

bool IMU::begin() {
    Wire.begin();
    // Use default 100kHz I2C - many MPU6050 clones are unreliable at 400kHz

    // Reset MPU6050 first (like Tockn library does)
    Wire.beginTransmission(MPU6050_ADDR);
    Wire.write(0x6B);  // PWR_MGMT_1
    Wire.write(0x80);  // DEVICE_RESET bit
    uint8_t err = Wire.endTransmission();
    if (err != 0) {
        Serial.printf("[IMU] I2C error %d - device not found at 0x%02X\n", err, MPU6050_ADDR);
        return false;
    }
    delay(100);  // Wait for reset to complete

    // Wake up MPU6050 - use PLL with X-axis gyro reference
    Wire.beginTransmission(MPU6050_ADDR);
    Wire.write(0x6B);  // PWR_MGMT_1
    Wire.write(0x01);  // Clock source = PLL with X gyro
    Wire.endTransmission();
    delay(10);

    // Set DLPF to ~44Hz bandwidth (good noise/latency balance)
    Wire.beginTransmission(MPU6050_ADDR);
    Wire.write(0x1A);  // CONFIG
    Wire.write(0x03);  // DLPF_CFG = 3
    Wire.endTransmission();

    // Set Gyro range to ±500°/s
    Wire.beginTransmission(MPU6050_ADDR);
    Wire.write(0x1B);  // GYRO_CONFIG
    Wire.write(0x08);  // FS_SEL = 1 (±500°/s)
    Wire.endTransmission();

    // Set Accel range to ±4g
    Wire.beginTransmission(MPU6050_ADDR);
    Wire.write(0x1C);  // ACCEL_CONFIG
    Wire.write(0x08);  // AFS_SEL = 1 (±4g)
    Wire.endTransmission();

    // Set sample rate to 1kHz (divider = 0)
    Wire.beginTransmission(MPU6050_ADDR);
    Wire.write(0x19);  // SMPLRT_DIV
    Wire.write(0x00);  // Rate = 1kHz / (1+0) = 1kHz
    Wire.endTransmission();

    // Read WHO_AM_I for diagnostics (don't fail on unexpected values)
    Wire.beginTransmission(MPU6050_ADDR);
    Wire.write(0x75);
    Wire.endTransmission(false);
    Wire.requestFrom((uint8_t)MPU6050_ADDR, (uint8_t)1);
    uint8_t id = Wire.read();
    Serial.printf("[IMU] WHO_AM_I = 0x%02X\n", id);

    _roll = _pitch = _yaw = 0;
    _calibrated = false;

    // Accept any valid I2C response - many clones return non-standard WHO_AM_I
    return true;
}

void IMU::calibrate() {
    _gyroOffX = _gyroOffY = _gyroOffZ = 0;
    _accelOffX = _accelOffY = _accelOffZ = 0;
    float sumAX = 0, sumAY = 0, sumAZ = 0;

    // Collect samples while drone is stationary
    for (int i = 0; i < CALIBRATION_SAMPLES; i++) {
        readRaw();
        _gyroOffX += _gyroX;
        _gyroOffY += _gyroY;
        _gyroOffZ += _gyroZ;
        sumAX += _accelX;
        sumAY += _accelY;
        sumAZ += _accelZ;
        delayMicroseconds(500);
    }

    _gyroOffX /= (float)CALIBRATION_SAMPLES;
    _gyroOffY /= (float)CALIBRATION_SAMPLES;
    _gyroOffZ /= (float)CALIBRATION_SAMPLES;

    // Accelerometer offsets: X,Y should be 0, Z should be 1g
    // Any deviation = sensor bias that causes angle error → drone drifts!
    _accelOffX = sumAX / (float)CALIBRATION_SAMPLES;
    _accelOffY = sumAY / (float)CALIBRATION_SAMPLES;
    _accelOffZ = (sumAZ / (float)CALIBRATION_SAMPLES) - 1.0;  // Remove gravity (1g on Z)

    Serial.printf("[IMU] Gyro offsets:  X=%.3f Y=%.3f Z=%.3f\n", _gyroOffX, _gyroOffY, _gyroOffZ);
    Serial.printf("[IMU] Accel offsets: X=%.4f Y=%.4f Z=%.4f\n", _accelOffX, _accelOffY, _accelOffZ);

    // Initialize angles from calibrated accelerometer
    readRaw();
    float ax = _accelX - _accelOffX;
    float ay = _accelY - _accelOffY;
    float az = _accelZ - _accelOffZ;
    _roll  = atan2(ay, sqrt(ax * ax + az * az)) * 180.0 / PI;
    _pitch = atan2(-ax, sqrt(ay * ay + az * az)) * 180.0 / PI;
    _yaw   = 0;

    _calibrated = true;
}

void IMU::readRaw() {
    Wire.beginTransmission(MPU6050_ADDR);
    Wire.write(0x3B);  // Start at ACCEL_XOUT_H
    Wire.endTransmission(false);
    Wire.requestFrom((uint8_t)MPU6050_ADDR, (uint8_t)14, (uint8_t)true);

    // Read 14 bytes: AccelXYZ(6) + Temp(2) + GyroXYZ(6)
    int16_t rawAX = (Wire.read() << 8) | Wire.read();
    int16_t rawAY = (Wire.read() << 8) | Wire.read();
    int16_t rawAZ = (Wire.read() << 8) | Wire.read();
    int16_t rawTmp = (Wire.read() << 8) | Wire.read();  // Discard temp
    (void)rawTmp;
    int16_t rawGX = (Wire.read() << 8) | Wire.read();
    int16_t rawGY = (Wire.read() << 8) | Wire.read();
    int16_t rawGZ = (Wire.read() << 8) | Wire.read();

    // Convert to physical units
    _accelX = (float)rawAX / ACCEL_SENSITIVITY;
    _accelY = (float)rawAY / ACCEL_SENSITIVITY;
    _accelZ = (float)rawAZ / ACCEL_SENSITIVITY;

    _gyroX = (float)rawGX / GYRO_SENSITIVITY;
    _gyroY = (float)rawGY / GYRO_SENSITIVITY;
    _gyroZ = (float)rawGZ / GYRO_SENSITIVITY;
}

void IMU::update(float dt) {
    readRaw();

    // Apply calibration offsets
    float gx = _gyroX - _gyroOffX;
    float gy = _gyroY - _gyroOffY;
    float gz = _gyroZ - _gyroOffZ;
    float ax = _accelX - _accelOffX;
    float ay = _accelY - _accelOffY;
    float az = _accelZ - _accelOffZ;

    // Store angular rates
    _rollRate  = gx;
    _pitchRate = gy;
    _yawRate   = gz;

    // Accelerometer-based angles (using calibrated values)
    float accelRoll  = atan2(ay, sqrt(ax * ax + az * az)) * 180.0 / PI;
    float accelPitch = atan2(-ax, sqrt(ay * ay + az * az)) * 180.0 / PI;

    // Complementary filter: trust gyro short-term, accel long-term
    _roll  = COMPLEMENTARY_ALPHA * (_roll  + gx * dt) + (1.0 - COMPLEMENTARY_ALPHA) * accelRoll;
    _pitch = COMPLEMENTARY_ALPHA * (_pitch + gy * dt) + (1.0 - COMPLEMENTARY_ALPHA) * accelPitch;

    // Yaw: gyro-only integration (no magnetometer)
    _yaw += gz * dt;
    if (_yaw > 180.0) _yaw -= 360.0;
    if (_yaw < -180.0) _yaw += 360.0;
}

IMUData IMU::getData() {
    IMUData d;
    d.roll      = _roll;
    d.pitch     = _pitch;
    d.yaw       = _yaw;
    d.rollRate  = _rollRate;
    d.pitchRate = _pitchRate;
    d.yawRate   = _yawRate;
    return d;
}

#include "pid.h"
#include <Arduino.h>

PIDController::PIDController() {
    _kp = _ki = _kd = 0;
    _minOut = -200;
    _maxOut = 200;
    _integralLimit = 100;
    _integral = 0;
    _prevMeasurement = 0;
    _pTerm = _iTerm = _dTerm = 0;
    _firstRun = true;
}

void PIDController::setGains(float kp, float ki, float kd) {
    _kp = kp;
    _ki = ki;
    _kd = kd;
}

void PIDController::setOutputLimits(float minOut, float maxOut) {
    _minOut = minOut;
    _maxOut = maxOut;
}

void PIDController::setIntegralLimit(float limit) {
    _integralLimit = limit;
}

float PIDController::compute(float setpoint, float measurement, float dt) {
    if (dt <= 0) return 0;

    float error = setpoint - measurement;

    // P-term
    _pTerm = _kp * error;

    // I-term with anti-windup clamping
    _integral += error * dt;
    _integral = constrain(_integral, -_integralLimit, _integralLimit);
    _iTerm = _ki * _integral;

    // D-term on measurement (not error) to avoid derivative kick on setpoint change
    if (_firstRun) {
        _prevMeasurement = measurement;
        _firstRun = false;
    }
    float derivative = -(measurement - _prevMeasurement) / dt;
    _dTerm = _kd * derivative;
    _prevMeasurement = measurement;

    // Sum and constrain output
    float output = _pTerm + _iTerm + _dTerm;
    output = constrain(output, _minOut, _maxOut);

    return output;
}

void PIDController::reset() {
    _integral = 0;
    _prevMeasurement = 0;
    _pTerm = _iTerm = _dTerm = 0;
    _firstRun = true;
}

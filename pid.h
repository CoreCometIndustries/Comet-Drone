#ifndef PID_H
#define PID_H

class PIDController {
public:
    PIDController();
    void setGains(float kp, float ki, float kd);
    void setOutputLimits(float minOut, float maxOut);
    void setIntegralLimit(float limit);
    float compute(float setpoint, float measurement, float dt);
    void reset();

    // Accessors for debugging/tuning
    float getP() { return _pTerm; }
    float getI() { return _iTerm; }
    float getD() { return _dTerm; }
    float getKp() { return _kp; }
    float getKi() { return _ki; }
    float getKd() { return _kd; }
    void setKp(float v) { _kp = v; }
    void setKi(float v) { _ki = v; }
    void setKd(float v) { _kd = v; }

private:
    float _kp, _ki, _kd;
    float _minOut, _maxOut;
    float _integralLimit;
    float _integral;
    float _prevMeasurement;  // D-term on measurement to avoid derivative kick
    float _pTerm, _iTerm, _dTerm;
    bool  _firstRun;
};

#endif

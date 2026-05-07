#ifndef BATTERY_H
#define BATTERY_H

#include <Arduino.h>

class Battery {
public:
    void begin(uint8_t pin);
    void update();
    float getVoltage()    { return _voltage; }
    int   getPercent()    { return _percent; }
    bool  isWarning()     { return _warning; }
    bool  isCritical()    { return _critical; }

private:
    uint8_t _pin;
    float   _voltage;
    int     _percent;
    bool    _warning;
    bool    _critical;
    float   _samples[10];   // Moving average buffer
    uint8_t _sampleIdx;
};

#endif

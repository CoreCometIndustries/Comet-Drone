#include "battery.h"
#include "config.h"

void Battery::begin(uint8_t pin) {
    _pin = pin;
    pinMode(_pin, INPUT);
    analogReadResolution(12);  // 12-bit ADC (0-4095)

    _voltage = BATTERY_FULL;
    _percent = 100;
    _warning = false;
    _critical = false;
    _sampleIdx = 0;

    // Pre-fill sample buffer
    for (int i = 0; i < 10; i++) {
        _samples[i] = BATTERY_FULL;
    }
}

void Battery::update() {
    // Read ADC and convert to voltage
    int raw = analogRead(_pin);

    // ESP32-C3 ADC: 12-bit (0-4095), reference ~2.5V (with attenuation)
    // With default 11dB attenuation, range is ~0-2.5V
    // Through voltage divider (ratio = BATTERY_DIVIDER), actual = reading * divider
    float reading = (raw / 4095.0) * 2.5 * BATTERY_DIVIDER;

    // Store in moving average buffer
    _samples[_sampleIdx] = reading;
    _sampleIdx = (_sampleIdx + 1) % 10;

    // Calculate average
    float sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += _samples[i];
    }
    _voltage = sum / 10.0;

    // Calculate percentage (linear mapping)
    float range = BATTERY_FULL - BATTERY_EMPTY;
    _percent = (int)((_voltage - BATTERY_EMPTY) / range * 100.0);
    _percent = constrain(_percent, 0, 100);

    // Set warning flags
    _warning  = (_voltage <= BATTERY_WARNING);
    _critical = (_voltage <= BATTERY_CRITICAL);
}

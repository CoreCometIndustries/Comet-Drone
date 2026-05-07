#ifndef WIFI_CONTROL_H
#define WIFI_CONTROL_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>

// Command structure received from controller
struct ControlCommand {
    float throttle;   // 0.0 - 1.0
    float roll;       // -1.0 to 1.0
    float pitch;      // -1.0 to 1.0
    float yaw;        // -1.0 to 1.0
    bool  armed;
    uint8_t mode;     // 0 = angle, 1 = rate
};

class WifiControl {
public:
    void begin();
    void update();
    ControlCommand getCommand()      { return _cmd; }
    bool isConnected()               { return _connected; }
    unsigned long lastCommandTime()  { return _lastCmdTime; }
    void sendTelemetry(float roll, float pitch, float yaw,
                       float battV, int battPct, bool armed,
                       int loopHz, int m1, int m2, int m3, int m4);

private:
    static void onWebSocketEvent(uint8_t num, WStype_t type,
                                  uint8_t* payload, size_t length);
    static void parseCommand(uint8_t* payload, size_t length);
    void handleRoot();

    static ControlCommand _cmd;
    static bool           _connected;
    static unsigned long  _lastCmdTime;
};

#endif

#include "wifi_control.h"
#include "config.h"
#include "web_page.h"

// Static member initialization
ControlCommand WifiControl::_cmd = {0, 0, 0, 0, false, 0};
bool           WifiControl::_connected = false;
unsigned long  WifiControl::_lastCmdTime = 0;

// Global instances (needed for callbacks)
static WebServer    httpServer(HTTP_PORT);
static WebSocketsServer wsServer(WS_PORT);

void WifiControl::begin() {
    // Configure WiFi Access Point
    WiFi.mode(WIFI_AP);
    WiFi.setTxPower(WIFI_POWER_19_5dBm);  // Max TX power for stable connection
    WiFi.softAP(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL);
    WiFi.setSleep(false);  // Disable modem sleep - prevents random disconnects

    Serial.print("[WiFi] AP started: ");
    Serial.println(WIFI_SSID);
    Serial.print("[WiFi] IP: ");
    Serial.println(WiFi.softAPIP());

    // HTTP server - serve the web controller page
    httpServer.on("/", HTTP_GET, [&]() {
        httpServer.send(200, "text/html", FPSTR(WEB_PAGE_HTML));
    });
    httpServer.begin();
    Serial.println("[HTTP] Server started on port 80");

    // WebSocket server
    wsServer.begin();
    wsServer.onEvent(onWebSocketEvent);
    Serial.println("[WS] Server started on port 81");
}

void WifiControl::update() {
    httpServer.handleClient();
    wsServer.loop();
}

void WifiControl::onWebSocketEvent(uint8_t num, WStype_t type,
                                     uint8_t* payload, size_t length) {
    switch (type) {
        case WStype_CONNECTED:
            _connected = true;
            Serial.printf("[WS] Client #%u connected\n", num);
            break;

        case WStype_DISCONNECTED:
            _connected = false;
            // Safety: disarm on disconnect
            _cmd.armed = false;
            _cmd.throttle = 0;
            Serial.printf("[WS] Client #%u disconnected\n", num);
            break;

        case WStype_TEXT:
            parseCommand(payload, length);
            _lastCmdTime = millis();
            break;

        default:
            break;
    }
}

void WifiControl::parseCommand(uint8_t* payload, size_t length) {
    // Expected format: "T:0.50,R:0.00,P:0.00,Y:0.00,A:1,M:0"
    char buf[128];
    size_t len = min(length, (size_t)127);
    memcpy(buf, payload, len);
    buf[len] = '\0';

    float t = 0, r = 0, p = 0, y = 0;
    int a = 0, m = 0;

    // Parse CSV-like format
    char* token = strtok(buf, ",");
    while (token != NULL) {
        if (token[0] == 'T' && token[1] == ':') t = atof(token + 2);
        if (token[0] == 'R' && token[1] == ':') r = atof(token + 2);
        if (token[0] == 'P' && token[1] == ':') p = atof(token + 2);
        if (token[0] == 'Y' && token[1] == ':') y = atof(token + 2);
        if (token[0] == 'A' && token[1] == ':') a = atoi(token + 2);
        if (token[0] == 'M' && token[1] == ':') m = atoi(token + 2);
        token = strtok(NULL, ",");
    }

    _cmd.throttle = constrain(t, 0.0f, 1.0f);
    _cmd.roll     = constrain(r, -1.0f, 1.0f);
    _cmd.pitch    = constrain(p, -1.0f, 1.0f);
    _cmd.yaw      = constrain(y, -1.0f, 1.0f);
    _cmd.armed    = (a == 1);
    _cmd.mode     = (uint8_t)m;
}

void WifiControl::sendTelemetry(float roll, float pitch, float yaw,
                                 float battV, int battPct, bool armed,
                                 int loopHz, int m1, int m2, int m3, int m4) {
    // Send JSON telemetry to all connected WebSocket clients
    char json[256];
    snprintf(json, sizeof(json),
        "{\"r\":%.1f,\"p\":%.1f,\"y\":%.1f,"
        "\"bv\":%.2f,\"bp\":%d,\"a\":%d,"
        "\"hz\":%d,\"m1\":%d,\"m2\":%d,\"m3\":%d,\"m4\":%d}",
        roll, pitch, yaw, battV, battPct, armed ? 1 : 0,
        loopHz, m1, m2, m3, m4);

    wsServer.broadcastTXT(json);
}

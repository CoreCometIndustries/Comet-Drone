# Comet Drone 🚁

A lightweight, high-performance flight controller firmware designed specifically for micro-quadcopters using the **ESP32-C3 Super Mini**. The Comet Drone leverages the ESP32-C3's RISC-V architecture to manage a 250Hz flight control loop, process MPU6050 IMU data, and provide real-time telemetry over WebSockets.

## ✨ Features

* **Cascaded PID Control:** Utilizes an outer Angle loop and an inner Rate loop for highly stable flight characteristics.
* **High-Frequency Control:** Runs a robust 250Hz control loop for responsive handling.
* **X-Quad Motor Mixing:** Optimized for standard X-configuration frames.
* **Real-time WiFi Telemetry:** Transmits live flight data and status via WebSocket over an ESP32-hosted Access Point.
* **Live Serial Tuning:** Adjust PID gains, monitor motor outputs, and view IMU data in real-time via the Serial Monitor without reflashing.
* **Smart Failsafes:** Built-in dynamic motor scaling (anti-windup) and automatic motor kill if WiFi telemetry is lost for more than 500ms.
* **Battery Monitoring:** Reads and filters 1S LiPo voltage via an ADC divider to monitor battery health (configurable).

---

## 🛠️ Hardware Requirements

* **Microcontroller:** ESP32-C3 Super Mini
* **IMU:** MPU6050 (I2C interface)
* **Motors:** 4x 0716 Coreless Motors (or similar DC brushed motors)
* **Motor Drivers:** 4x N-Channel Logic-Level MOSFETs (e.g., SI2302) with flyback diodes.
* **Battery:** 1S (3.7V) LiPo Battery

### Pin Configuration (`config.h`)

| Component | ESP32-C3 Pin | Function |
| :--- | :--- | :--- |
| **Motor FR (M1)** | `GPIO 0` | Front-Right (Counter-Clockwise) |
| **Motor RL (M2)** | `GPIO 3` | Rear-Left (Counter-Clockwise) |
| **Motor FL (M3)** | `GPIO 1` | Front-Left (Clockwise) |
| **Motor RR (M4)** | `GPIO 4` | Rear-Right (Clockwise) |
| **IMU SDA** | `GPIO 8` | I2C Data Line |
| **IMU SCL** | `GPIO 9` | I2C Clock Line |
| **Battery ADC** | `GPIO 6` | Voltage Divider Input (Ratio 2.0) |
| **Status LED** | `GPIO 8` | Onboard LED (Active LOW) |

---

## 💻 Software Setup

### Dependencies
This project is built using the Arduino framework. You will need the following installed in your Arduino IDE:
1. **ESP32 Board Support Package** (Search for `esp32` by Espressif Systems in the Boards Manager).
2. **WebSockets Library** by Links2004 (Available in the Arduino Library Manager).

### Flashing the Firmware
1. Open `drone_firmware.ino` in the Arduino IDE.
2. Select **ESP32C3 Dev Module** from the boards menu.
3. Ensure the following settings:
   * **USB CDC On Boot:** Enabled (required for serial monitoring)
   * **Flash Mode:** QIO or DIO
4. Connect the ESP32-C3 via USB and click **Upload**.

---

## ⚙️ Tuning & Control Interface

### Serial Monitor Commands
Connect your drone via USB and open the Serial Monitor at **115200 baud**. You can send the following commands to tune the drone on the fly:

* **View Status:** `ST` (Prints current angles, rates, PID values, and motor speeds)
* **Emergency Stop:** `MS` (Instantly stops all motors)
* **Motor Test:** `T[0-3]` (Spins the specified motor at an idle speed of 80 to verify wiring. e.g., `T0`)
* **Tune PID Gains:** Send the axis and parameter followed by the value. 
  * *Format:* `[Axis][Param]:[Value]`
  * *Axes:* `R` (Roll), `P` (Pitch), `Y` (Yaw)
  * *Params:* `P` (Proportional rate), `I` (Integral rate), `D` (Derivative rate), `A` (Angle Proportional)
  * *Examples:*
    * `RP:0.5` -> Sets Roll Rate Kp to 0.5
    * `AI:0.2` -> Sets Pitch Angle Kp to 0.2

### WiFi Access Point
Upon boot, the drone creates a WiFi network:
* **SSID:** `CometDrone`
* **Password:** `drone1234`
* **IP Address:** `192.168.4.1` (Connect over port `81` for WebSocket telemetry data).

---

## ⚠️ Safety Warning

* Always remove propellers when testing motors or flashing new firmware on the bench.
* Coreless motors can draw significant current; ensure your MOSFETs are appropriately rated and you are using proper flyback diodes to protect the ESP32.
* Ensure the drone is perfectly flat and stationary when powering on, as the IMU calibrates its gyro offsets during the first few seconds of boot.

## 📄 License
Distributed under the MIT License. See `LICENSE` for more information.

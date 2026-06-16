# 📡 Dual-MCU Ultrasonic Radar & Web Visualization

An IoT embedded systems project utilizing a dual-microcontroller architecture. A PIC16F877A handles low-level hardware interfacing and sensor data acquisition, while an ESP32 manages Wi-Fi telemetry and serves a real-time web dashboard.

## 📌 Project Overview
This system acts as a real-time proximity monitor. The PIC16 continuously polls an ultrasonic sensor, calculates the distance, triggers local hardware alarms (LCD, LEDs, Buzzer), and streams the formatted data via UART to the ESP32. The ESP32 hosts a SoftAP Web Server, dynamically graphing the incoming telemetry data on a connected client's browser.

### ⚙️ System Architecture & Hardware
* **Sensor Node (PIC16F877A @ 4MHz):** * Interfaces with an HC-SR04 Ultrasonic Sensor using hardware timers (Timer1) for accurate pulse-width measurement.
  * Drives a 16x2 Character LCD in 4-bit mode for local status display.
  * Controls physical alert mechanisms (LEDs/Buzzer) based on distance thresholds.
* **Telemetry & UI Node (ESP32):**
  * Receives data from the PIC via UART (9600 baud).
  * Hosts a Wi-Fi Access Point and an asynchronous HTTP web server.
* **Communication Protocol:** Unidirectional UART (TX from PIC $\rightarrow$ RX on ESP32).


## 💻 Software & Firmware Details
* **PIC Firmware (Bare-metal C):** Built in MPLAB X / XC8. Features direct hardware manipulation for timers and GPIO, alongside custom UART and LCD driver implementations.
* **ESP32 Firmware (C++):** Utilizes `WiFi.h` and `WebServer.h` to host the network. Maintains a rolling buffer of the last 60 samples for min/max calculations.
* **Frontend UI (HTML/JS/CSS):** Embedded directly into the ESP32 memory. Uses the HTML5 `<canvas>` element and a REST API endpoint (`/data`) fetched via JavaScript to render a live, color-coded line graph.

### 🚨 Alert Thresholds
* `Safe (> 50cm):` Green UI, clear LCD.
* `Medium (< 50cm):` Green UI, LED indicator.
* `Warning (< 30cm):` Orange UI, escalated LED indicator.
* `Critical (< 15cm):` Red UI, LCD warning, Active Buzzer alarm.

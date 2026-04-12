# RadarManager-ESP32

[![Build Status](https://github.com/CSNistor/esp32-udp-radar-simulator/actions/workflows/build.yml/badge.svg)](https://github.com/CSNistor/esp32-udp-radar-simulator/actions)
[![ESP-IDF](https://img.shields.io/badge/ESP--IDF-v6.0-blue)](https://docs.espressif.com/projects/esp-idf/en/v6.0/)

A C++ learning project built on ESP32 using ESP-IDF, focused on embedded networking and AUTOSAR Adaptive-style design concepts.

---

## 🚀 Overview

This project demonstrates a basic but structured embedded system that:

- connects to Wi-Fi (station mode)
- sends UDP packets periodically
- runs concurrent tasks using FreeRTOS
- uses modern C++ class-based design

It is intended as a stepping stone towards more advanced embedded networking concepts such as SOME/IP, service-oriented architecture, and radar-like data streaming.

---

## 🧠 Concepts Covered

- ESP-IDF development
- FreeRTOS tasks
- Wi-Fi networking (STA mode)
- UDP sockets (lwIP)
- C++ encapsulation and modular design
- basic concurrency patterns

---

## 📁 Project Structure

```text
RadarManager/
├── include/
│   ├── WiFi.hpp
│   └── UdpSender.hpp
├── src/
│   ├── WiFi.cpp
│   └── UdpSender.cpp
├── main/
│   ├── RadarManager.cpp
│   └── CMakeLists.txt
├── CMakeLists.txt
├── sdkconfig
├── README.md
```

---

## ⚙️ Requirements

- ESP32 development board
- ESP-IDF v6.x
- Visual Studio Code (recommended) with ESP-IDF extension
- Python 3.x (for UDP receiver)

---

## 🔧 Configuration

Before building, update the following constants in:

`main/RadarManager.cpp`

```cpp
constexpr const char* WIFI_SSID = "YOUR_WIFI_SSID";
constexpr const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";
constexpr const char* DEST_IP   = "DESTINATION_IP";
constexpr uint16_t DEST_PORT    = 5000;
```

---

## 🛠️ Build

```bash
idf.py build
```

---

## ⚡ Flash & Monitor

```bash
idf.py -p COM3 flash monitor
```

Replace `COM3` with your actual serial port.

---

## 🖥️ UDP Receiver (PC Side)

Example Python script to receive data:

```python
import socket

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(("0.0.0.0", 5000))

print("Listening on UDP port 5000...")

while True:
    data, addr = sock.recvfrom(1024)
    print(f"{addr}: {data.decode(errors='replace')}")
```

Run with:

```bash
python udp_receiver.py
```

---

## 📡 Example Output

```text
('192.168.1.28', 53771): hello from esp32, counter=0
('192.168.1.28', 53771): hello from esp32, counter=1
('192.168.1.28', 53771): hello from esp32, counter=2
```

---

## 🏗️ Architecture

- **WiFiStation**
  - handles Wi-Fi initialization and connection

- **UdpSender**
  - encapsulates socket logic
  - runs as a FreeRTOS task

- **app_main**
  - orchestrates system startup

---

## 🔮 Roadmap

Planned improvements:

- [ ] binary payload instead of plain text
- [ ] radar-like data structure
- [ ] producer/consumer architecture (FreeRTOS queue)
- [ ] Wireshark packet inspection
- [ ] error handling and reconnection logic

---

## ⚠️ Notes

- This project is for learning purposes and will evolve over time

---

## 📜 License

This project is licensed under the MIT License.
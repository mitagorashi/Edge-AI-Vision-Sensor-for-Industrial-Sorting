# 🤖 ESP32-CAM Industrial Vision Sensor: AI-Powered Sorting Architecture

![Hardware](https://img.shields.io/badge/Hardware-ESP32--CAM-blue)
![AI](https://img.shields.io/badge/AI-Edge_Impulse-orange)
![Status](https://img.shields.io/badge/Status-Functional-brightgreen)
![Author](https://img.shields.io/badge/Author-Mohammed_Gorashi-blue)

![AI Vision Dashboard](media/pic1.png)
## 📌 Project Overview
This project transforms a standard $5 ESP32-CAM into a standalone, wireless, AI-powered industrial vision sensor. Built as the visual cortex for a PLC-supervised robotic sorting arm, this system utilizes a custom neural network to detect, classify, and locate objects on a conveyor belt in real-time.

Instead of relying on an external computer or cloud processing, the AI model runs entirely on the bare-metal microcontroller (Edge AI). Furthermore, the device bypasses restrictive enterprise networks by generating its own independent Wi-Fi Access Point, serving a live HTML dashboard with dynamic bounding boxes drawn directly over the video feed.

## 🎥 Live Demonstration
<video src="media/20260408-1632-52.1919819.mp4" controls="controls" style="max-width: 100%;">
  Your browser does not support the video tag.
</video>

## ✨ Core Features
* **Bare-Metal Edge AI:** Runs a custom object detection neural network directly on the ESP32 silicon.
* **Independent Wi-Fi Access Point:** Operates as its own router, allowing secure, offline connections in hostile industrial environments.
* **Live Video Dashboard:** Embeds an onboard web server that streams Base64-encoded JPEGs to a responsive HTML dashboard.
* **Real-Time Bounding Boxes:** Uses dynamic CSS and HTML injection to draw bounding boxes and confidence scores over detected objects.
* **PLC Integration Ready:** Outputs raw X, Y, Width, and Height coordinates suitable for Modbus or serial transmission to a PLC.

## 🧠 The Edge Impulse Machine Learning Pipeline
This system mathematically analyzes video using a custom Edge Impulse pipeline optimized for highly constrained microcontrollers.

* **Data Engineering:** A custom dataset was collected, downsampled to 96x96 pixel resolution, and manually labeled with bounding boxes.
* **The FOMO Architecture:** Unlike YOLO models, FOMO (Faster Objects, More Objects) predicts object centroids, reducing the RAM footprint by 30x.
* **C++ Bare-Metal Deployment:** The neural network was exported as a raw C++ library and compiled directly into the firmware for zero-latency decision-making.

## 🛠️ Hardware Requirements
* **Microcontroller:** ESP32-CAM (AI Thinker Model) with OV2640 Module.
* **Power Supply:** Stable 5V source (500mA+ current spikes are common during Wi-Fi + AI processing).
* **Programmer:** USB-to-TTL Serial Adapter (CP2102 or CH340).

## 🚀 Installation & Deployment

### 1. Hardware Wiring (Flash Mode)
| USB-to-TTL | ESP32-CAM |
| :--- | :--- |
| 5V | 5V |
| GND | GND |
| TX | U0R (RX) |
| RX | U0T (TX) |
| **GND** | **IO0 (GPIO 0)** *<- Required for flashing!* |

### 2. Flashing the Firmware
1. Open the `.ino` sketch in Arduino IDE.
2. Select Board: **AI Thinker ESP32-CAM** and enable **PSRAM**.
3. When the console displays `Connecting........`, press the physical **RST** button on the ESP32.
4. Once verified, **disconnect IO0 from GND** and press **RST** again to boot.

## 📡 Accessing the Live Dashboard
1. Connect your PC/Smartphone to the Wi-Fi network: **RobotVision**.
2. Open a browser and navigate to: `http://192.168.4.1`.
3. View the live camera feed with real-time AI predictions.

## ⚠️ Known Hardware Limitations
* **The Prolific Driver Blockade:** PL2303HXA modules on Windows 11 may require a rollback to the 2008 legacy driver.
* **Brownout Detector:** If the ESP32 crashes during boot, ensure a strict 5V input and use a short USB cable to handle current spikes.
* **Compute Bottleneck:** Running AI, encoding Base64, and serving Wi-Fi simultaneously limits the system to approximately 1 FPS.

## 🔮 Future Architecture Upgrades
For the final production phase at Kırşehir Ahi Evran University, the system will move to:
1. **ESP32-S3:** For hardware-accelerated vector AI processing (~5 FPS).
2. **Raspberry Pi 4 + ROS:** For high-speed (30+ FPS) YOLOv8 processing and full robotic integration.

---
*Engineered by Mohammed Gorashi | Master's Program in Electrical and Electronics Engineering*

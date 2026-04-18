# Edge AI Vision Sensor for Real-Time Industrial Sorting

![AI Vision Dashboard](pic1.jpg)

 A low-cost standalone vision sensor utilizing the FOMO algorithm for real-time object detection and classification on the ESP32-CAM.

## 📌 Project Overview
This project demonstrates the deployment of Edge AI on constrained hardware. Using an ESP32-CAM, I developed a smart vision sensor capable of identifying and locating specific objects (Erasers and Pencil Sharpeners) in real-time. Unlike traditional computer vision, all processing happens locally on the chip, making it ideal for high-speed industrial sorting applications where cloud latency is not an option.

## 🔗 Edge Impulse Project
You can view the full dataset, impulse design, and training performance here
[httpsstudio.edgeimpulse.comstudio953248](httpsstudio.edgeimpulse.comstudio953248)

## 🚀 Key Features
 Bare-Metal Inference Executes deep learning models directly on the ESP32-CAM without external cloud dependencies.
 FOMO (Faster Objects, More Objects) Implements a spatial-centroid detection algorithm that is 30x faster than standard SSD or YOLO models on mobile hardware.
 Live HTML Dashboard Operates in Access Point (AP) Mode, broadcasting a local Wi-Fi signal to serve a real-time visual dashboard with object bounding boxes.
 Optimized Power Management Custom firmware designed to handle the high current demands of the camera and Wi-Fi radio simultaneously, preventing brownout resets.
 Industrial Integration Ready Engineered to provide spatial coordinates (X, Y) which can be sent to a PLC or Robotic Arm for automated picking.

## 🛠️ Tech Stack
 Hardware ESP32-CAM (AI-Thinker), PL2303 TTL Programmer.
 AI Framework Edge Impulse (FOMO Architecture).
 Languages C++, Embedded HTMLCSS.
 Tools Roboflow (Dataset Annotation), Arduino IDE.

## 🧠 Machine Learning Pipeline
1. Data Acquisition Created a custom dataset of office objects under various industrial lighting conditions.
2. Preprocessing Images were downsampled to a 96x96 grayscale matrix to optimize inference speed and memory footprint.
3. Training Used spatial centroid detection to distinguish between Silgi (Eraser) and Kalemtıraş (Pencil Sharpener).
4. Deployment Exported the model as a standalone C++ library for integration into the Arduino environment.

## 📂 Repository Structure
 `firmware` Full C++ source code and web server implementation.
 `model` Exported Edge Impulse C++ library.
 `media` Screenshots of the live AI dashboard and dataset samples.

---
Developed as part of an Electrical and Electronics Engineering Master’s project focusing on Artificial Intelligence and Industrial Automation.

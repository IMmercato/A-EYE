# A-EYE (Artificial EYE) 🧠👓

Personal A-EYE built with the ESP32-S3-EYE

<p align="center">
  <a href="https://shipwrecked.hackclub.com/?t=ghrm" target="_blank">
    <img
      src="https://hc-cdn.hel1.your-objectstorage.com/s/v3/739361f1d440b17fc9e2f74e49fc185d86cbec14_badge.png"
      alt="This project is part of Shipwrecked, the world's first hackathon on an island!"
      width="35%"
    />
  </a>
</p>

---

## 🔍 Overview

Why not build your own AI-powered glasses?

**A-EYE** is a DIY wearable vision device built with the **ESP32-S3-EYE**, housed inside a pair of safety goggles. It’s lightweight, portable, and capable of running edge AI tasks—ideal for face detection, object tracking, or any creative application you dream up.

This project uses the **ESP32-S3-EYE** module, which I received from [HackClub](https://hackclub.com)'s [Visioneer](https://visioneer.hackclub.com/) event. I built and showcased this at [Shipwrecked](https://shipwrecked.hackclub.com), the world's first hackathon on an island! 🏝️

---

## 📦 Installation & Usage

### 1. Clone this repository
```bash
git clone https://github.com/IMmercato/A-EYE.git
cd A-EYE
```

### 2. Set up ESP-IDF development environment

Follow the official ESP-IDF guide for your OS to install dependencies:  
🔗 https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/get-started/index.html

### 3. Build and flash the firmware to ESP32-S3-EYE
```bash
idf.py set-target esp32s3
idf.py menuconfig       # Optional: customize settings
idf.py build            # Build the project
idf.py flash            # Flash firmware to device
idf.py monitor          # View live output
```

> Make sure your ESP32-S3-EYE is connected via USB and properly recognized before running these commands.

### 4. Mount the device

Use zip ties, Velcro strips, or a custom 3D-printed case to attach the ESP32-S3-EYE securely to your safety goggles. Keep the wiring tidy and plug into a portable USB battery or computer.

### 5. Power on and observe

Put on your AI glasses, connect to power, and watch it come alive! You can monitor logs via serial or connect wirelessly depending on your configuration.

---

## 🔧 Features

- 🎯 Real-time face/object detection
- 📡 Wi-Fi & Bluetooth capabilities
- 🎙️ Voice input via built-in microphone
- 💡 Compact and wearable form factor

---

## 🗂️ Repo Structure

```
A-EYE/
├── main/               # Core application code
├── components/         # Additional ESP modules
├── sdkconfig.defaults  # Default config
└── README.md           # You're reading it!
```

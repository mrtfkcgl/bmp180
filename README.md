# BMP180 Linux I2C Driver

This repository contains a robust, C-based I2C driver for the **BMP180** pressure and temperature sensor, specifically developed for **Linux-based systems** like the Raspberry Pi.

## 🚀 Purpose
I developed this driver as a personal learning project to master **Linux I2C subsystem communication**. As a Senior Embedded Software Engineer with a background in vehicle cybersecurity and artificial neural networks, I focused on implementing this driver with industry-standard practices:
* **Defensive Programming:** Strict pointer validation and boundary checks.
* **Atomic Transactions:** Leveraging `I2C_RDWR` with Repeated Start for high reliability.
* **Modular Architecture:** A clean separation between the Hardware Abstraction Layer (I2C Bus) and the Application Logic (BMP180 Driver).

## 🛠 Features
* Full support for BMP180 **Temperature** and **Pressure** readings.
* Compatibility with all **Pressure Over-sampling Settings (OSS)** (Ultra Low Power to Ultra High Resolution).
* Robust error handling and system-level logging.
* Memory-safe fixed-buffer management (avoiding VLA risks).

## 📂 Project Structure
* `inc/`: Professional header files (`bmp180.h`, `i2c_bus.h`).
* `src/`: Driver implementation logic (`bmp180.c`, `i2c_bus.c`).
* `main.c`: A clean application loop for testing and demonstration.
* `Makefile`: Fully automated build system.

## ⚡ Quick Start

### 1. Enable I2C
Ensure the I2C interface is enabled on your Raspberry Pi:
```bash
sudo raspi-config
# Navigate to Interface Options -> I2C -> Enable
```

### 2. Build the Project
Compile the driver and test application using the provided Makefile:
```bash
make
```

### 3. Run the Driver
Execute the test binary to see real-time sensor data:
```bash
./bmp180_test
```

## 📖 Example Usage Scenario
This driver is ideal for integration into larger projects like **DIY Weather Stations**, **Drone Altimeters**, or **IoT environmental monitoring systems**. 

**Basic Integration Example:**
```c
bmp180_t sensor;
float temp;
int32_t pressure;

// Initialize the I2C handle
i2c_init(&sensor.i2c, "/dev/i2c-1", 0x77);

// Load factory-set calibration data
bmp180_load_calibration(&sensor);

// Fetch measurements
bmp180_read_temperature(&sensor, &temp);
bmp180_read_pressure(&sensor, BMP180_OSS_ULTRA_RES, &pressure);

printf("Environment: %.2f C | %.2f hPa\n", temp, (float)pressure/100.0f);
```

## 🤝 Open for Use & Learning
This project is open-source! I wrote this primarily for learning purposes, and it is open for anyone to use, modify, or study. Whether you are a student or a fellow engineer, feel free to dive into the code.

## ❤️ Acknowledgments
Thank you for visiting this repository! Exploring the depths of the Linux kernel and hardware communication has been a great journey. I hope this code helps you in your own learning path.

---
Developed with ❤️ by **Emre TUFEKCIOGLU**

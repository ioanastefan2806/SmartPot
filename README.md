# SmartPot - Autonomous Smart Plant Pot

## Overview
SmartPot is a fully autonomous plant pot system designed to care for indoor plants using embedded systems. It uses an ESP32 microcontroller to monitor soil moisture and light intensity, rotate the plant toward sunlight, and activate a water pump when the soil is dry. The system features LCD feedback, audible and visual alarms, and a web interface for manual control.

## Features
- Automatic watering based on soil humidity
- Automatic rotation toward the light using LDRs and a stepper motor
- LCD I2C display for system status
- WiFi Web interface for manual control
- Water level detection with buzzer and LED alerts
- Optional FM transmission and interrupt-based control logic

## Hardware Components
| Component                | Quantity | Notes                              |
|--------------------------|----------|------------------------------------|
| ESP32 DevKit v1         | 1        | Main microcontroller                |
| LCD 1602 I2C Display    | 1        | I2C connected screen                |
| Capacitive Soil Sensor  | 1        | Connected to analog pin             |
| Water Level Sensor      | 1        | Digital signal                      |
| LDR + Resistors (10kΩ)   | 2 + 2    | For light tracking                  |
| 5V Relay Module         | 1        | Controls pump power                 |
| Water Pump 5V + Hose    | 1        | Powered by 4xAA battery pack        |
| Stepper Motor 28BYJ-48  | 1        | Motor for pot rotation              |
| ULN2003 Driver Board    | 1        | Controls stepper motor              |
| Buzzer                  | 1        | For audible alerts                  |
| LEDs (Red, Green)       | 1 + 1    | Visual water level indicators       |

## Pin Configuration (ESP32)
- **GPIO 32** – Soil Sensor (Analog)
- **GPIO 33** – Water Level Sensor (Digital)
- **GPIO 4** – Relay for Water Pump
- **GPIO 16** – Buzzer
- **GPIO 15** – Red LED
- **GPIO 2**  – Green LED
- **GPIO 34, 35** – LDR Left/Right (Analog)
- **GPIO 14, 27, 26, 25** – Stepper Motor IN1–IN4
- **GPIO 21, 22** – SDA/SCL for LCD

## Software Architecture
Written in Arduino C++, the system includes two operating modes:

### Automatic Mode
- Monitors moisture and rotates plant automatically
- Pumps water when dry, if water is available

### Manual Mode
- Triggered via web interface (`/water`, `/rotate` routes)
- Allows manual control of pump and rotation
- Times pump to prevent overwatering (max 10s)

## Display Feedback
The LCD shows:
- Moisture level percentage
- System mode (AUTO / MANUAL)
- Water level status (OK / LOW)

## Web Interface
The ESP32 serves a simple HTML page accessible over local WiFi, allowing:
- Manual watering (`/water?enable=1`)
- Rotation left/right/stop (`/rotate?dir=1`, `-1`, `0`)

## Advanced Features
- Uses `ISR(ADC_vect)` and `INT0_vect` for sensor precision and mode switching (C++ test version)
- Supports FM broadcast using `FMTX.h` (experimental)

## Results
- Real-time LCD update, reliable WiFi control
- Responsive automatic rotation and watering
- Alerts user with LED + buzzer when reservoir is empty

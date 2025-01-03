# esp32-project-ai
Resources Repository For Artificial Intelligence Final Projects.<br>

<img src=https://github.com/user-attachments/assets/cdb347d6-ae17-41e0-9245-11b409b0bb05 width="512"/>

In this projects, our group are using PCB ESP32 IoT Starter Kit by PCBWAY. We use this kit because it contains all reqired sensors. 

## Goal:
Implementing fuzzy logic on ESP32 for advance plant monitoring system.<br>
We use fuzzy logic to determine the required amount of water by the monitored plants.

## Tools:
1. ESP32
2. DHT11
3. Light Dependent Resistor (LDR)
4. OLED 128x64
5. ARDUINO IDE

## Workflow Diagram
<img src=image/FuzzyPlantMonitoringSystem.jpg width="512"/>

## Schematics
1. Connect ADC pin 33 ESP32 to DHT11 pin
2. Connect I2C_SDA pin 21 ESP32 to SDA OLED 128x64 pin
3. Connect I2C_SCL pin 22 ESP32 to SCL OLED 128x64 pin
4. Connect ADC pin 4 ESP32 to LDR pin

## How To Install
1. Download eFLL library on Arduino IDE
2. Choose ESP32 Dev Module Board
3. Upload kodingan_esp.ino
4. Your ESP is ready to use!

## Results:
<img src=image/runningtools.jpg width="512"/>

This projects is for Artificial Intelligence Final Projects<br>
Groupu 2 Haikal, Bondan, Darryl.<br>

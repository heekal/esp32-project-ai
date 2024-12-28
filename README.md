# esp32-project-ai
Repository untuk resouces kecerdasan buatan.<br>

<img src=https://github.com/user-attachments/assets/cdb347d6-ae17-41e0-9245-11b409b0bb05 width="512"/>

Pada project kali ini, kelompok kita menggunakan PCB ESP32 IoT Starter Kit (PCBWay), yang mana pada SHIELD ini telah tertanam berbagai sensor yang bisa digunakan. 

## Tujuan:
Mengimplementasi logika fuzzy pada ESP32 untuk advance plant monitoring system.<br>
Untuk mendapatkan output yang diharapkan, pada project kali ini kami menggunakan defuzzifikasi sugeno.

## Alat dan Bahan:
1. ESP32
2. DHT11
3. LDR
4. OLED 128x64
5. ARDUINO IDE

## Workflow Diagram
<img src=image/FuzzyPlantMonitoringSystem.jpg width="512"/>

## Skematik
1. Hubungkan pin ADC 33 ESP32 pada pin DHT11 dengan jumper
2. Hubungkan pin I2C_SDA 21 ESP32 pada pin SDA OLED 128x64
3. Hubungkan pin I2C_SCL 22 ESP32 pada pin SCL OLED 128x64
4. Hubungkan pin ADC 4 ESP32 pada pin LDR

## Cara kerja
1. Download library eFLL di Arduino IDE
2. Pilih board > ESP32 Dev Module
3. Upload kodingan_esp.ino
4. Alat siap digunakan!

tujuan dari project ini sebagai tugas besar mata kuliah kecerdasan buatan s1 telkom university<br>
kelompok 2 Haikal, Bondan, Darryl @rryealy

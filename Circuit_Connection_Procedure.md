## Circuit Connection Procedure

1. Power the ESP32 using a USB cable or a regulated 5V supply.

2. Connect the ZMPT101B Voltage Sensor:
   - VCC → 3.3V/5V
   - GND → GND
   - OUT → GPIO 34

3. Connect the ACS712 Current Sensor:
   - VCC → 5V
   - GND → GND
   - OUT → GPIO 35

4. Connect the DHT11 Sensor:
   - VCC → 3.3V
   - GND → GND
   - DATA → GPIO 4

5. Connect the I2C LCD:
   - VCC → 5V
   - GND → GND
   - SDA → GPIO 21
   - SCL → GPIO 22

6. Connect the Relay Module:
   - IN → GPIO 5
   - VCC → 5V
   - GND → GND

7. Connect the Buzzer:
   - Positive → GPIO 18
   - Negative → GND

8. Connect the Push Button:
   - One terminal → GPIO 19
   - Other terminal → GND

9. Upload the code to ESP32.

10. Power ON the circuit and monitor the readings on the LCD, Blynk app, and Telegram alerts.
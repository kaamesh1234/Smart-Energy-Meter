/************ BLYNK DETAILS ************/
#define BLYNK_TEMPLATE_ID "Your Blynk code"
#define BLYNK_TEMPLATE_NAME "IoT Smart Energy Meter"
#define BLYNK_PRINT Serial

/************ LIBRARIES ************/
#include "EmonLib.h"
#include <EEPROM.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

/************ LCD ************/
LiquidCrystal_I2C lcd(0x27, 16, 2);

/************ TELEGRAM ************/
const char* telegramBotToken = "YOUR_BOT_TOKEN";
const char* telegramChatID   = "YOUR_CHAT_ID";

/************ CALIBRATION ************/
const float vCalibration = 42.7;
const float currCalibration = 1.80;

/************ WIFI + BLYNK ************/
char auth[] = "YOUR_BLYNK_AUTH";
char ssid[] = "YOUR_WIFI_NAME";
char pass[] = "YOUR_WIFI_PASSWORD";

/************ ENERGY MONITOR ************/
EnergyMonitor emon;
BlynkTimer timer;

/************ VARIABLES ************/
float voltage;
float current;
float power;
float energy = 0;
float cost = 0;
float ratePerkWh = 6.5;

/************ TELEGRAM FUNCTION ************/
void sendTelegram(String message)
{
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;

    String url = "https://api.telegram.org/bot";
    url += telegramBotToken;
    url += "/sendMessage?chat_id=";
    url += telegramChatID;
    url += "&text=" + message;

    http.begin(url);
    http.GET();
    http.end();
  }
}

/************ ENERGY READ ************/
void readEnergy()
{
  emon.calcVI(20,2000);

  voltage = emon.Vrms * vCalibration;
  current = emon.Irms * currCalibration;
  power   = voltage * current;

  energy += power / 3600000.0;
  cost = energy * ratePerkWh;

  Serial.print("Voltage: ");
  Serial.println(voltage);
  Serial.print("Current: ");
  Serial.println(current);

  /******** LCD DISPLAY ********/
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("V:");
  lcd.print(voltage,1);
  lcd.print(" I:");
  lcd.print(current,1);

  lcd.setCursor(0,1);
  lcd.print("P:");
  lcd.print(power,1);

  /******** BLYNK ********/
  Blynk.virtualWrite(V0, voltage);
  Blynk.virtualWrite(V1, current);
  Blynk.virtualWrite(V2, power);
  Blynk.virtualWrite(V3, energy);
  Blynk.virtualWrite(V4, cost);

  /******** ALERT ********/
  if(power > 1000)   // change limit
  {
    sendTelegram("⚠ High Power Usage Detected!");
  }
}

/************ SETUP ************/
void setup()
{
  Serial.begin(115200);

  lcd.init();
  lcd.backlight();

  emon.voltage(34, vCalibration, 1.7);
  emon.current(35, currCalibration);

  Blynk.begin(auth, ssid, pass);

  timer.setInterval(2000L, readEnergy);
}

/************ LOOP ************/
void loop()
{
  Blynk.run();
  timer.run();
}
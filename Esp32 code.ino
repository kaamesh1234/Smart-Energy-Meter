/************ BLYNK DETAILS ************/
#define BLYNK_TEMPLATE_ID "Your Blynk ID"
#define BLYNK_TEMPLATE_NAME "IoT Smart Energy Meter"
#define BLYNK_PRINT Serial

#include "EmonLib.h"
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <HTTPClient.h>
#include "DHT.h"

/************ LCD ************/
LiquidCrystal_I2C lcd(0x27, 16, 2);

/************ DHT ************/
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

/************ PINS ************/
#define RELAY_PIN 5
#define BUZZER_PIN 18
#define BUTTON_PIN 19

/************ LIMITS ************/
float maxVoltage = 245.0;
float maxTemp = 45.0;
float maxCurrent = 5.0;

/************ TELEGRAM ************/
const char* botToken = "YOUR_TELEGRAM_BOT_TOKEN";
const char* chatID = "YOUR_CHAT_ID";

/************ WIFI ************/
char auth[] = "YOUR_BLYNK_AUTH_TOKEN";
char ssid[] = "YOUR_WIFI_NAME";
char pass[] = "YOUR_WIFI_PASSWORD";

/************ ENERGY ************/
EnergyMonitor emon;
BlynkTimer timer;

float voltage, current, power;
float energy = 0, cost = 0;
float ratePerkWh = 6.5;
float temp = 0;

/************ FLAGS ************/
bool voltageAlertSent = false;
bool currentAlertSent = false;
bool tempAlertSent = false;

/************ DISPLAY ************/
int page = 0;
int lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;

/************ TELEGRAM FUNCTION ************/
void sendTelegram(String msg)
{
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    String url = "https://api.telegram.org/bot" + String(botToken) +
                 "/sendMessage?chat_id=" + String(chatID) +
                 "&text=" + msg;

    http.begin(url);
    http.GET();
    http.end();
  }
}

/************ MAIN FUNCTION ************/
void readEnergy()
{
  emon.calcVI(20,2000);

  float rawVoltage = emon.Vrms * 42.7;

  // Actual Voltage Reading
if (rawVoltage < 50)
{
    voltage = 0;
}
else
{
    voltage = rawVoltage;
}

  // ✅ REAL CURRENT
  current = emon.Irms * 1.80;

  // ✅ POWER
  power = voltage * current;

  // ✅ REAL TEMPERATURE
  temp = dht.readTemperature();

  energy += power / 3600000.0;
  cost = energy * ratePerkWh;

  bool fault = false;

  // Voltage protection
  if(voltage > maxVoltage)
  {
    fault = true;
    if(!voltageAlertSent)
    {
      sendTelegram("⚠ Over Voltage!");
      voltageAlertSent = true;
    }
  }
  else voltageAlertSent = false;

  // Current protection
  if(current > maxCurrent)
  {
    fault = true;
    if(!currentAlertSent)
    {
      sendTelegram("⚠ Over Current!");
      currentAlertSent = true;
    }
  }
  else currentAlertSent = false;

  // Temperature protection
  if(temp > maxTemp)
  {
    fault = true;
    if(!tempAlertSent)
    {
      sendTelegram("🔥 High Temperature!");
      tempAlertSent = true;
    }
  }
  else tempAlertSent = false;

  // 🔴 RELAY CONTROL
  if(fault)
  {
    digitalWrite(RELAY_PIN, HIGH); // OFF
    digitalWrite(BUZZER_PIN, HIGH);
  }
  else
  {
    digitalWrite(RELAY_PIN, LOW);  // ON
    digitalWrite(BUZZER_PIN, LOW);
  }

  // Blynk
  Blynk.virtualWrite(V0, voltage);
  Blynk.virtualWrite(V1, current);
  Blynk.virtualWrite(V2, power);
  Blynk.virtualWrite(V3, energy);
  Blynk.virtualWrite(V4, cost);
  Blynk.virtualWrite(V5, temp);
}

/************ DISPLAY ************/
void displayData()
{
  lcd.setCursor(0,0);
  lcd.print("Smart Energy Meter");
  lcd.setCursor(0,1);
  lcd.print("Initializing");

  if(page == 0)
  {
    lcd.setCursor(0,0);
    lcd.print("V:");
    lcd.print(voltage,1);
    lcd.print(" I:");
    lcd.print(current,2);

    lcd.setCursor(0,1);
    lcd.print("P:");
    lcd.print(power,1);
    lcd.print("W");
  }
  else
  {
    lcd.setCursor(0,0);
    lcd.print("Temp:");
    lcd.print(temp,1);

    lcd.setCursor(0,1);
    lcd.print("Cost:");
    lcd.print(cost,1);
  }
}

/************ BUTTON ************/
void checkButton()
{
  int reading = digitalRead(BUTTON_PIN);

  if (reading == LOW && lastButtonState == HIGH &&
      (millis() - lastDebounceTime) > 200)
  {
    page = !page;
    lastDebounceTime = millis();
  }

  lastButtonState = reading;
}

/************ SETUP ************/
void setup()
{
  Serial.begin(115200);

  Wire.begin(21,22);
  lcd.init();
  lcd.backlight();

  lcd.setCursor(0,0);
  lcd.print("Smart Energy");
  lcd.setCursor(0,1);
  lcd.print(" KAAMESH ");
  delay(3000);
  lcd.clear();

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  dht.begin();

  emon.voltage(34, 42.7, 1.7);
  emon.current(35, 1.80);

  Blynk.begin(auth, ssid, pass);

  timer.setInterval(500L, readEnergy);
  timer.setInterval(300L, displayData);
}

/************ LOOP ************/
void loop()
{
  Blynk.run();
  timer.run();
  checkButton();
}
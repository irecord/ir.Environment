/******************************************************************************
 * M5ATOMS3 ENVIII monitor
 * Environment monitor that will log to EmonPi (Open Energy Monitor)
 * 
 * irecord
 * v0.1 | 10 April 2023
 * https://github.com/irecord/Environment
 * 
 * 
 * used Resources:
 * https://github.com/electricidea/M5ATOM/tree/master/ATOM-Web-Monitor
 * https://community.m5stack.com/topic/2564/atom-lite-and-env-ii
 * https://community.m5stack.com/topic/4900/issue-with-env-iii-hat
 * https://community.home-assistant.io/t/got-the-lilygo-t-qt-esp32-s3-display-working/537328
 * https://admin.osptek.com/uploads/GC_9107_Data_Sheet_V1_2_5897aaab18.pdf
 * + Other Question/Answers around
 * 
 * Distributed as-is; no warranty is given.
 ******************************************************************************/

#include <Arduino.h>
#include <M5AtomS3.h>
#include <M5_ENV.h>
#include <WiFi.h>
#include <HTTPClient.h>

#include "local.h"

int font_height;
int32_t lineHeight = 43;

/// @brief Holds the current millis value at the time of the loop start
unsigned long current_millis;

/// @brief Millis until next sensor check
unsigned long sensor_millis;
/// @brief Seconds until next sensor check
unsigned int sensor_seconds = 3;

/// @brief Millis until next emon post
unsigned long emon_millis;
/// @brief Seconds until next emon post
unsigned int emon_seconds = 30;

/// @brief Millis the screen has been active;
unsigned long lcd_millis = 0;
/// @brief Seconds until screen turns off
unsigned int lcd_seconds = 10;
/// @brief Current state of the LCD
bool isLcdOn = false;

// ENVIII:
// SHT30:   temperature and humidity sensor  I2C: 0x44
// QMP6988: absolute air pressure sensor     I2C: 0x70
SHT3X sht30;
QMP6988 qmp6988;

// Latest sensor readings
float latestPressure = 0.0;
float latestTemperature = 0.0;
float latestHumidity = 0.0;

bool connectWifi();
bool initSensors();
void writeTemperature();
void writeHumidity();
void writePressure();
void writeSensorsToLcd();
void setLcdOn();
void setLcdOff();
void sendDataToEmon();

void setup() {
  M5.begin(true, true, false, false);

  // Set font
  M5.lcd.setTextSize(2);
  font_height = M5.lcd.fontHeight();
  M5.Lcd.println("Starting");
  
  if(!connectWifi() || !initSensors())
  {
    delay(1000);
    esp_restart();
  }
  
  sensor_millis = millis();
  setLcdOn();
}

void loop() 
{ 
  current_millis = millis();

  // If button pushed, show LCD, otherwise turn it back off again
  M5.Btn.read();
  if(M5.Btn.wasPressed())
    setLcdOn();
  else if(current_millis > lcd_millis)
    setLcdOff();
  
  // Ready for next sensor read?
  if(current_millis > sensor_millis)
  {
    sensor_millis = current_millis + sensor_seconds*1000;
    if (sht30.get() != 0) {
      return;
    }
    
    latestPressure = qmp6988.calcPressure()/100.0F;
    latestTemperature = sht30.cTemp;
    latestHumidity = sht30.humidity;

    writeSensorsToLcd();
  }  

  // Check wifi still connected
  wl_status_t wifi_Status = WiFi.status();
  if(wifi_Status != WL_CONNECTED){
      esp_restart();
  }
  
  if(current_millis > emon_millis && latestPressure > 0)
  {
    emon_millis = current_millis + emon_seconds*1000;
    sendDataToEmon();
  }
}

bool connectWifi()
{
  M5.Lcd.printf(": %s\n", WifiSSID);
  M5.Lcd.printf(": %s\n", WifiHostName);
  WiFi.setHostname(WifiHostName);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WifiSSID, WifiPassword);
  delay(1000);

  wl_status_t status = WiFi.status();
  if(status == WL_CONNECTED)
  {
    M5.Lcd.println("WiFi: OK!");
    return true;
  }

  M5.Lcd.println("WIFI: NOK!");
  return false;
}

///
/// Init the Sensors, if required
///
bool initSensors()
{ 
  // Start the sensor comms
  Wire.end();
  Wire.begin(2, 1); 
  delay(50);

  if(qmp6988.init() == 1)
  {
    qmp6988.setFilter(QMP6988_FILTERCOEFF_32);
    qmp6988.setOversamplingP(QMP6988_OVERSAMPLING_32X);
    qmp6988.setOversamplingT(QMP6988_OVERSAMPLING_4X);

    M5.lcd.println("Sensor: OK!");
    return true;
  }
  
  M5.lcd.println("Sensor: NOK!");
  return false;
}

void sendDataToEmon()
{
  char url[255];
  sprintf(url, EmonUriFormat, EmonNode, EmonApiKey, latestTemperature, latestHumidity, latestPressure);
  USBSerial.println(url);

  HTTPClient httpClient;
  httpClient.setReuse(false);
  httpClient.begin(url);
  int httpResponseCode = httpClient.POST("");
  if (httpResponseCode > 0) {
    USBSerial.print("HTTP ");
    USBSerial.println(httpResponseCode);
  }
  else {
    USBSerial.print("Error code: ");
    USBSerial.println(httpResponseCode);
    USBSerial.println(":-(");
  }
  //httpClient.end();
}

///
/// Write the sensor information to the screen
///
void writeSensorsToLcd()
{
  if(!isLcdOn)
    return;

  writeTemperature();
  writeHumidity();
  writePressure();
}

void writeTemperature()
{
  M5.lcd.fillRect(0, 0, 128, lineHeight, RED);
  M5.lcd.setTextColor(BLACK, RED);
  M5.lcd.setCursor(4, 12);
  M5.lcd.printf("%3.2f C\n", latestTemperature);
}

void writeHumidity()
{
  M5.lcd.fillRect(0, lineHeight, 128, lineHeight, YELLOW);
  M5.lcd.setTextColor(BLACK, YELLOW);
  M5.lcd.setCursor(4, lineHeight+12);
  M5.lcd.printf("%3.2f %%\n", latestHumidity);
}

void writePressure()
{
  M5.lcd.fillRect(0, lineHeight*2, 128, lineHeight, BLUE);
  M5.lcd.setTextColor(BLACK, BLUE);
  M5.lcd.setCursor(4, lineHeight*2+12);
  M5.lcd.printf("%3.2f mb\n", latestPressure);
}

//
// M5AtomS3 doesn't have working sleep() command atm
//
#define GC9107_SLPIN 0x10
#define GC9107_SLPOUT 0x11

void setLcdOn()
{
  lcd_millis = current_millis + 1000*lcd_seconds;

  if(isLcdOn)
    return;
  
  isLcdOn = true;

  M5.Lcd.startWrite();
  M5.Lcd.writecommand(GC9107_SLPOUT);
  M5.Lcd.endWrite();
  digitalWrite(16, HIGH);

  writeSensorsToLcd();
}

void setLcdOff()
{
  if(!isLcdOn)
    return;

  M5.Lcd.startWrite();
  M5.Lcd.writecommand(GC9107_SLPIN);
  M5.Lcd.endWrite();
  digitalWrite(16, LOW);

  isLcdOn = false;
}

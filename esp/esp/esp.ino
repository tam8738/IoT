#include <WiFi.h>
#include "time.h"
#include "esp_sntp.h"
#include <ArduinoJson.h>

HardwareSerial TFTSerial(1);

const char *ssid = "Rain74";
const char *password = "12345678";

const char *ntpServer1 = "pool.ntp.org";
const char *ntpServer2 = "time.nist.gov";
const long gmtOffset_sec = 7*3600;
const int daylightOffset_sec = 3600;
const char *time_zone = "CET-1CEST,M3.5.0,M10.5.0/3";  // TimeZone rule for Europe/Rome including daylight adjustment rules (optional)

 
int soilMoistureValue = 0; 
int soilMoisturePercentage = 0;
int LDRValue = 0; 
int LDRPercentage = 0;

JsonDocument doc;
void setup() {
  Serial.begin(115200);
  TFTSerial.begin(115200,SERIAL_8N1,13,14); // serial send data to TFT
  //Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  esp_sntp_servermode_dhcp(1);  // (optional)

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" CONNECTED");
  //JsonDocument doc;
  // set notification call-back function
  sntp_set_time_sync_notification_cb(timeavailable);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);
}

void loop() {
  printLocalTime();
  readSoilMoisteur();
  readLDR();
  serializeJson(doc, TFTSerial);
  TFTSerial.println();
  serializeJson(doc, Serial);
  Serial.println();
  delay(1000);
}
void readSoilMoisteur(){
  soilMoistureValue = analogRead(15);  //put Sensor insert into soil 
  //Serial.print("Soil Moisture value:");
  //Serial.println(soilMoistureValue);
  soilMoisturePercentage = map(soilMoistureValue,2730,1215,0,100);
  //Serial.print("Soil Moisture Percentage:");
  //Serial.println(soilMoisturePercentage);
  char soilMoisturePercentagetxt[10];
  sprintf(soilMoisturePercentagetxt,"%d",soilMoisturePercentage);
  doc["soil"] = soilMoisturePercentagetxt;
}
void readLDR(){
  LDRValue = analogRead(16);  //put Sensor insert into soil 
  //Serial.print("LDR value:");
  //Serial.println(LDRValue);
  LDRPercentage = map(LDRValue,4095,255,0,100);
  //Serial.print("LDR Percentage:");
  //Serial.println(LDRPercentage);
  char LDRPercentagetxt[10];
  sprintf(LDRPercentagetxt,"%d",LDRPercentage);
  doc["ldr"] = LDRPercentagetxt;
}
void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("No time available (yet)");
    return;
  }
  char hour[3];
  strftime(hour,3,"%H",&timeinfo);
  //TFTSerial.print(hour);
  doc["hour"] = hour;
  //TFTSerial.print(":");
  char minutes[3];
  strftime(minutes,3,"%M",&timeinfo);
  //TFTSerial.println(minutes);
  doc["minute"] = minutes;

}

// Callback function (gets called when time adjusts via NTP)
void timeavailable(struct timeval *t) {
  Serial.println("Got time adjustment from NTP!");
  printLocalTime();
}

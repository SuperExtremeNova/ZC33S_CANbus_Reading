#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>

#include "../lib/otaUpdate/src/otaUpdate.h"

void setup() {
  // initialize the baud rate for the serial and start it
  Serial.begin(115200);
  delay(1000);

  // connection of the WiFi and checking for update from the GitHub
  Serial.println("System initiallizing...OTA Updated");
  connectToWiFi();
  Serial.println("Device is ready.");
  Serial.println("Current Firmware Version: " + String(currentFirmwareVersion));
  checkForFirmwareUpdate();



}

void loop() {
  // put your main code here, to run repeatedly:
  //Serial.println(currentFirmwareVersion);
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}

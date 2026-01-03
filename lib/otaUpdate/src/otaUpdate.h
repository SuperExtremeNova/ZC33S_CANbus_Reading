#pragma once

#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>

extern const char* currentFirmwareVersion;
extern unsigned long previous;
extern const unsigned long updateCheckInterval;

// put function declarations here:
int myFunction(int, int);
void connectToWiFi();
void checkForFirmwareUpdate();
String fetchLatestVersion();
void downloadAndApplyFirmware();
boolean startOTAUpdate(WiFiClient*,int);



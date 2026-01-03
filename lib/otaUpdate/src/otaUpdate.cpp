#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>
#include "otaUpdate.h"

//WiFi credentials
const char* ssid = "Digicel_WiFi_nvQ6";
const char* password = "TA4UauCR";

//URL for the firmware and the raw text file for the version number of the latest firmware [url for the version file need to be a raw url]
const char* versionUrl = "https://raw.githubusercontent.com/SuperExtremeNova/ZC33S_CANbus_Reading/main/lib/otaUpdate/version.txt";
const char* firmwareUrl = "https://github.com/SuperExtremeNova/ZC33S_CANbus_Reading/releases/download/v0.0.1/firmware.bin";

//Current firmware version control
const char* currentFirmwareVersion = "0.0.4";
const unsigned long updateCheckInterval = 5 * 60 * 1000; //calculation for 5 minutes in milliseconds [change based on need]
unsigned long previous = 0;
unsigned long lastUpdateCheck = 0;


void connectToWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.println("IP address: " + WiFi.localIP().toString());
}

void checkForFirmwareUpdate() {
  Serial.println("Checking for firmware update...");
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected");
    return;
  }

  // Step 1: Fetch the latest version from GitHub
  String latestVersion = fetchLatestVersion();
  if (latestVersion == "") {
    Serial.println("Failed to fetch latest version");
    return;
  }

  Serial.println("Current Firmware Version: " + String(currentFirmwareVersion));
  Serial.println("Latest Firmware Version: " + latestVersion);

  // Step 2: Compare versions
  if (latestVersion != currentFirmwareVersion) {
    Serial.println("New firmware available. Starting OTA update...");
    downloadAndApplyFirmware();
  } else {
    Serial.println("Device is up to date.");
  }
}

String fetchLatestVersion() {

  //adding code for setting a secure client to test why the http protocal is failing
  WiFiClientSecure client;
  client.setInsecure(); //for testing purposes, update if and when the project is completed

  HTTPClient http;
  http.begin(client, versionUrl); //updates made to the call function [begin] to capture the client

  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {
    String latestVersion = http.getString();
    latestVersion.trim();  // Remove any extra whitespace
    http.end();
    return latestVersion;
  } else {
    Serial.printf("Failed to fetch version. HTTP code: %d\n", httpCode);
    http.end();
    return "";
  }
}

void downloadAndApplyFirmware() {
  HTTPClient http;
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  http.begin(firmwareUrl);

  int httpCode = http.GET();
  Serial.printf("HTTP GET code: %d\n", httpCode);

  if (httpCode == HTTP_CODE_OK) {
    int contentLength = http.getSize();
    Serial.printf("Firmware size: %d bytes\n", contentLength);

    if (contentLength > 0) {
      WiFiClient* stream = http.getStreamPtr();
      if (startOTAUpdate(stream, contentLength)) {
        Serial.println("OTA update successful, restarting...");
        delay(2000);
        ESP.restart();
      } else {
        Serial.println("OTA update failed");
      }
    } else {
      Serial.println("Invalid firmware size");
    }
  } else {
    Serial.printf("Failed to fetch firmware. HTTP code: %d\n", httpCode);
  }
  http.end();
}


bool startOTAUpdate(WiFiClient* client, int contentLength) {
  Serial.println("Initializing update...");
  if (!Update.begin(contentLength)) {
    Serial.printf("Update begin failed: %s\n", Update.errorString());
    return false;
  }

  Serial.println("Writing firmware...");
  size_t written = 0;
  int progress = 0;
  int lastProgress = 0;

  // Timeout variables
  const unsigned long timeoutDuration = 120*1000;  // 10 seconds timeout
  unsigned long lastDataTime = millis();

  while (written < contentLength) {
    if (client->available()) {
      uint8_t buffer[128];
      size_t len = client->read(buffer, sizeof(buffer));
      if (len > 0) {
        Update.write(buffer, len);
        written += len;

        // Calculate and print progress
        progress = (written * 100) / contentLength;
        if (progress != lastProgress) {
          Serial.printf("Writing Progress: %d%%\n", progress);
          lastProgress = progress;
        }
      }
    }
    // Check for timeout
    if (millis() - lastDataTime > timeoutDuration) {
      Serial.println("Timeout: No data received for too long. Aborting update...");
      Update.abort();
      return false;
    }

    yield();
  }
  Serial.println("\nWriting complete");

  if (written != contentLength) {
    Serial.printf("Error: Write incomplete. Expected %d but got %d bytes\n", contentLength, written);
    Update.abort();
    return false;
  }

  if (!Update.end()) {
    Serial.printf("Error: Update end failed: %s\n", Update.errorString());
    return false;
  }

  Serial.println("Update successfully completed");
  return true;
}
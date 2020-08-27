#include "sdcard.hpp"

void sdInitialize() {
  if (SD.begin(SDCARD_SS_PIN, SDCARD_SPI)) {
    return;
  }

  while (true) {
    Serial.println("Failed to initialize SD card!");
    delay(1000);
  }
}

void sdListFiles(void (*callback)(File*)) {
  File root = SD.open("/");

  File currentFile = root.openNextFile();
  while (currentFile) {
    callback(&currentFile);
    currentFile = root.openNextFile();
  }
}

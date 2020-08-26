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

void sdListFiles() {
  File root = SD.open("/");

  File currentFile = root.openNextFile();
  while (currentFile) {
    Serial.printf("%s%s\n", currentFile.name(),
                  currentFile.isDirectory() ? "/" : "");
    currentFile = root.openNextFile();
  }
}

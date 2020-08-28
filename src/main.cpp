#include "main.hpp"

static void onListFile(File *file);

TFT_eSPI display;
ControlFace controlFace(&display);
bool foundScript = false;

void setup() {
  Serial.begin(115200);
  display.begin();
  display.setRotation(3);

  controlFace.reset();
  sdInitialize();
  sdListFiles(onListFile);

  controlFace.dumpMenuItems();
  controlFace.redrawAll();
}

void loop() {
  controlFace.tick();
  delay(10);
}

static void onListFile(File *file) {
  if (foundScript) {
    return;
  }

  String filename(file->name());
  if (!filename.endsWith(".luo") && !filename.endsWith(".lua")) {
    return;
  }

  controlFace.loadItemsFromLua(file);
  foundScript = true;
}

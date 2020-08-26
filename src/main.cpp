#include "main.hpp"

void setup() {
  Serial.begin(115200);
  sdInitialize();
}

void loop() {
  sdListFiles();
  delay(1000);
}

#include "main.hpp"

void setup() {
  Serial.begin(115200);

  sdInitialize();
  luaReset();
}

void loop() {
  sdListFiles([](File *file) {
    String filename(file->name());
    if (filename.endsWith(".luo") || filename.endsWith(".lua")) {
      Serial.printf("Found Lua file: %s\n", filename.c_str());
      int status = luaLoadFile(file);
      Serial.printf("Status: %d\n", status);
    }
  });
  delay(1000);
}

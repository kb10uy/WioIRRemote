#include "main.hpp"

static void onListFile(File *file);

TFT_eSPI display;
ControlFace controlFace(&display);

void setup() {
  Serial.begin(115200);
  display.begin();
  display.setRotation(3);

  sdInitialize();
  luaReset();
}

void loop() {
  sdListFiles(onListFile);
  delay(1000);
}

static void onListFile(File *file) {
  String filename(file->name());
  if (!filename.endsWith(".luo") && !filename.endsWith(".lua")) {
    return;
  }

  int status = luaLoadFile(file);
  if (status != LUA_OK) {
    return;
  }

  lua_State *lua = luaGetLuaState();

  status = lua_getglobal(lua, "getMenu");
  if (status != LUA_TFUNCTION) {
    lua_pop(lua, -1);
    return;
  }
  lua_call(lua, 0, 1);

  controlFace.loadItemsFromLuaTable(lua);
  controlFace.dumpMenuItems();
}

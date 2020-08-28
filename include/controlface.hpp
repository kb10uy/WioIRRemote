#pragma once

#include <cstddef>
#include <cstdint>

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <Seeed_FS.h>

#include <lua.hpp>

#include "ControlFaceItem.hpp"
#include "IRSender.hpp"

struct FileBuffer {
  File *file;
  size_t readBytes;
  size_t size;
  uint8_t buffer[64];
};

class ControlFace {
 private:
  lua_State *lua = nullptr;
  TFT_eSPI *display = nullptr;
  ControlFaceItem items[8];
  size_t itemCount = 0;
  size_t selectedItem = 0;
  bool previousButtons[8];
  bool triggerButtons[8];

  void redrawItem(size_t index, bool enabled);

 public:
  ControlFace(TFT_eSPI *display);
  void reset();
  void tick();
  void send();
  void redrawAll();
  void loadItemsFromLua(File *lua);
  void dumpMenuItems();
};

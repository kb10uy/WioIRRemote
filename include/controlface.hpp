#pragma once

#include <Arduino.h>

#include <cstdio>
#include <cstring>
#include <lua.hpp>

#include "TFT_eSPI.h"

enum class ControlType : uint8_t {
  Select,
  Integer,
  Button,
};

class ControlFaceItem {
 private:
  ControlType type = ControlType::Button;
  uint8_t name[16] = {0};
  uint16_t currentValue = 0;
  union {
    struct {
      uint16_t minimum;
      uint16_t maximum;
      uint16_t step;
    } range;
    struct {
      uint8_t buffer[128];
      uint8_t headIndices[8];
      uint8_t count;
    } items;
  };

 public:
  void loadSettingFromLuaTable(lua_State *lua);
  void dump();
  void moveNext();
  void movePrevious();
  const ControlType getType() { return this->type; }
  const uint8_t *getName() { return this->name; }
  const uint16_t getValue() { return this->currentValue; }
  const uint8_t *getStringValue() {
    return this->items.buffer + this->items.headIndices[this->currentValue];
  }
};

class ControlFace {
 private:
  TFT_eSPI *display;
  ControlFaceItem items[8];
  size_t itemCount = 0;
  size_t selectedItem = 0;
  bool previousButtons[8];
  bool triggerButtons[8];

 public:
  ControlFace(TFT_eSPI *display);
  void loadItemsFromLuaTable(lua_State *lua);
  void dumpMenuItems();
  void redrawAll();
  void redrawItem(int index, bool enabled);
  void tick();
};

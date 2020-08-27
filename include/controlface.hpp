#pragma once

#include <cstdio>
#include <cstring>

#include <Arduino.h>
#include "TFT_eSPI.h"

#include <lua.hpp>

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
};

class ControlFace {
 private:
  TFT_eSPI *display;
  ControlFaceItem items[8];
  size_t itemCount = 0;

 public:
  ControlFace(TFT_eSPI *display);
  void loadItemsFromLuaTable(lua_State *lua);
  void dumpMenuItems();
  void redrawAll();
};

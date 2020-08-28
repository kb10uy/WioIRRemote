#pragma once

#include <cstddef>
#include <cstdint>

#include <lua.hpp>

#define IRSENDER_METATABLE "irsender"

enum class IRType {
  NEC = 1,
  AEHA = 2,
  Sony = 3,
};

struct IRSender {
  uint8_t data[256];
  size_t written = 0;
  uint16_t customerCode = 0;
  IRType type = IRType::NEC;
};

void irRegister(lua_State *lua);
void irPushIRSender(lua_State *lua);

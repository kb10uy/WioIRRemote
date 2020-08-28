#pragma once

#include <Arduino.h>

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <lua.hpp>

#define IRSENDER_METATABLE "irsender"

/**
 * 送信する信号のタイプ
 */
enum class IRType {
  NEC = 1,
  AEHA = 2,
  Sony = 3,
};

/**
 * Lua 側に irsender として公開する userdata
 */
class IRSender {
 private:
  uint32_t pinIR = 0;
  uint8_t data[256];
  size_t written = 0;
  uint16_t customerCode = 0;
  IRType type = IRType::NEC;

  void sendLeader();
  void sendCustomerCode();
  void sendData();
  void sendZero();
  void sendOne();

 public:
  void reset(uint32_t pinIR);
  void setType(IRType type);
  void setCustomerCode(uint16_t customerCode);
  size_t pushData(uint8_t data);
  size_t pushData(uint8_t *data, size_t length);
  void send();
};

void irRegister(lua_State *lua);
void irPushIRSender(lua_State *lua, uint32_t pinIR);

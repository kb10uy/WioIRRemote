#include "IRSender.hpp"

static int irsenderSetType(lua_State *lua);
static int irsenderSetCustomerCode(lua_State *lua);
static int irsenderPushData(lua_State *lua);
static int irsenderSend(lua_State *lua);

static const luaL_Reg irsenderMethods[] = {
    {"setType", irsenderSetType},
    {"setCustomerCode", irsenderSetCustomerCode},
    {"pushData", irsenderPushData},
    {"send", irsenderSend},
    {nullptr, nullptr},
};

/**
 * irsender を登録する。
 */
void irRegister(lua_State *lua) {
  luaL_newmetatable(lua, IRSENDER_METATABLE);
  lua_pushliteral(lua, "__index");
  luaL_newlib(lua, irsenderMethods);
  lua_settable(lua, -3);
  lua_pop(lua, 1);
}

/**
 * スタックに irsender をプッシュする。
 */
void irPushIRSender(lua_State *lua, uint32_t pinIR) {
  IRSender *irsender =
      static_cast<IRSender *>(lua_newuserdata(lua, sizeof(IRSender)));
  luaL_setmetatable(lua, IRSENDER_METATABLE);

  irsender->reset(pinIR);
}

/**
 * ir:reset(type)
 * - type: 信号タイプ。 1, 2, 3 でそれぞれ NEC, AEHA, Sony
 */
static int irsenderSetType(lua_State *lua) {
  Serial.printf("irsender:setType()\n");
  IRSender *irsender = static_cast<IRSender *>(lua_touserdata(lua, 1));
  if (!irsender) {
    return luaL_error(lua, "First argument is not IRSender");
  }

  irsender->setType(static_cast<IRType>(lua_tointeger(lua, 2)));
  return 0;
}

/**
 * ir:setCustomerCode(code)
 * - code: カスタマーコード。 16bit 幅
 */
static int irsenderSetCustomerCode(lua_State *lua) {
  Serial.printf("irsender:setCustomerCode()\n");
  IRSender *irsender = static_cast<IRSender *>(lua_touserdata(lua, 1));
  if (!irsender) {
    return luaL_error(lua, "First argument is not IRSender");
  }

  irsender->setCustomerCode(static_cast<uint16_t>(lua_tointeger(lua, 2)));
  return 0;
}

/**
 * ir:pushData(data)
 * - data: データ。整数か配列(テーブル)のいずれかを指定できる
 * - 返り値: 追加されたデータ数
 */
static int irsenderPushData(lua_State *lua) {
  Serial.printf("irsender:pushData()\n");
  IRSender *irsender = static_cast<IRSender *>(lua_touserdata(lua, 1));
  if (!irsender) {
    return luaL_error(lua, "First argument is not IRSender");
  }

  size_t pushed = 0;
  uint8_t buffer[64];

  switch (lua_type(lua, 2)) {
    case LUA_TNUMBER: {
      buffer[0] = static_cast<uint8_t>(lua_tointeger(lua, 2));
      pushed = 1;
      break;
    }
    case LUA_TTABLE: {
      // スタックトップにテーブルをコピーして操作する
      lua_pushvalue(lua, 2);
      lua_pushnil(lua);
      while (lua_next(lua, -2) != 0) {
        if (pushed < 64) {
          buffer[pushed++] = static_cast<uint8_t>(lua_tointeger(lua, -1));
        }
        lua_pop(lua, 1);
      }
      lua_pop(lua, 1);

      break;
    }
    default:
      return luaL_error(lua, "Second argument is not integer nor table");
  }

  pushed = irsender->pushData(buffer, pushed);
  lua_pushinteger(lua, pushed);
  return 1;
}

/**
 * ir:send()
 */
static int irsenderSend(lua_State *lua) {
  Serial.printf("irsender:send()\n");
  IRSender *irsender = static_cast<IRSender *>(lua_touserdata(lua, 1));
  if (!irsender) {
    return luaL_error(lua, "First argument is not IRSender");
  }

  irsender->send();
  return 0;
}

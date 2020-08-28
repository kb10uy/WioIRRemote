#include <irsender.hpp>

static int irsenderReset(lua_State *lua);
static int irsenderSetCustomerCode(lua_State *lua);
static int irsenderPushData(lua_State *lua);

static const luaL_Reg irsenderMethods[] = {
    {"reset", irsenderReset},
    {"setCustomerCode", irsenderSetCustomerCode},
    {"pushData", irsenderPushData},
    {nullptr, nullptr},
};

void irRegister(lua_State *lua) {
  luaL_newmetatable(lua, IRSENDER_METATABLE);
  lua_pushliteral(lua, "__index");
  luaL_newlib(lua, irsenderMethods);
  lua_settable(lua, -3);
  lua_pop(lua, 1);
}

void irPushIRSender(lua_State *lua) {
  IRSender *irsender =
      static_cast<IRSender *>(lua_newuserdata(lua, sizeof(IRSender)));
  luaL_setmetatable(lua, IRSENDER_METATABLE);

  irsender->type = IRType::NEC;
  irsender->customerCode = 0x0000;
  irsender->written = 0;
}

/**
 * ir:reset(type)
 * - type: 信号タイプ。 1, 2, 3 でそれぞれ NEC, AEHA, Sony
 */
static int irsenderReset(lua_State *lua) {
  IRSender *irsender = static_cast<IRSender *>(lua_touserdata(lua, 1));
  if (!irsender) {
    return luaL_error(lua, "First argument is not IRSender");
  }

  IRType type = static_cast<IRType>(lua_tointeger(lua, 2));
  irsender->type = type;
  irsender->written = 0;
  return 0;
}

/**
 * ir:setCustomerCode(code)
 * - code: カスタマーコード。 16bit 幅
 */
static int irsenderSetCustomerCode(lua_State *lua) {
  IRSender *irsender = static_cast<IRSender *>(lua_touserdata(lua, 1));
  if (!irsender) {
    return luaL_error(lua, "First argument is not IRSender");
  }

  uint16_t customerCode = static_cast<uint16_t>(lua_tointeger(lua, 2));
  irsender->customerCode = customerCode;
  return 0;
}

/**
 * ir:pushData(data)
 * - data: データ。整数か配列(テーブル)のいずれかを指定できる
 * - 返り値: 追加されたデータ数
 */
static int irsenderPushData(lua_State *lua) {
  IRSender *irsender = static_cast<IRSender *>(lua_touserdata(lua, 1));
  if (!irsender) {
    return luaL_error(lua, "First argument is not IRSender");
  }
  if (irsender->written >= 256) {
    return luaL_error(lua, "Too much data payload");
  }

  if (lua_isinteger(lua, 2)) {
    uint8_t data = static_cast<uint8_t>(lua_tointeger(lua, 2));
    irsender->data[irsender->written++] = data;

    lua_pushinteger(lua, 1);
    return 1;
  } else if (lua_istable(lua, 2)) {
    size_t pushed = 0;

    // 念のためスタックトップにコピーする
    lua_pushvalue(lua, 2);
    lua_pushnil(lua);
    while (lua_next(lua, -2) != 0) {
      if (irsender->written < 256) {
        uint8_t value = static_cast<uint8_t>(lua_tointeger(lua, -1));
        irsender->data[irsender->written++] = value;
        ++pushed;
      }
      lua_pop(lua, -1);
    }
    // スタックトップのテーブルを飛ばす
    lua_pop(lua, 1);

    lua_pushinteger(lua, pushed);
    return 1;
  } else {
    return luaL_error(lua, "Second argument is not integer nor table");
  }
}

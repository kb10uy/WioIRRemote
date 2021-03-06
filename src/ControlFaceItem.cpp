#include "ControlFaceItem.hpp"

/**
 * 次に移動する。
 */
void ControlFaceItem::moveNext() {
  switch (this->type) {
    case ControlType::Select: {
      if (this->items.count == 0) {
        return;
      }
      this->currentValue = (this->currentValue + 1) % this->items.count;
      break;
    }
    case ControlType::Integer: {
      this->currentValue =
          min(this->currentValue + this->range.step, this->range.maximum);
      break;
    }
    default:
      break;
  }
}

/**
 * 前に移動する。
 */
void ControlFaceItem::movePrevious() {
  switch (this->type) {
    case ControlType::Select: {
      if (this->items.count == 0) {
        return;
      }
      this->currentValue =
          (this->currentValue + this->items.count - 1) % this->items.count;
      break;
    }
    case ControlType::Integer: {
      this->currentValue =
          max(this->currentValue - this->range.step, this->range.minimum);
      break;
    }
    default:
      break;
  }
}

/**
 * スタックトップのテーブルからメニュー項目の詳細を読み込む。
 */
void ControlFaceItem::loadSettingFromLuaTable(lua_State *lua) {
  this->range.minimum = 1;
  this->range.maximum = 10;
  this->range.step = 1;

  lua_pushnil(lua);
  while (lua_next(lua, -2) != 0) {
    if (lua_type(lua, -2) != LUA_TSTRING) {
      lua_pop(lua, 1);
      continue;
    }
    const char *key = lua_tostring(lua, -2);
    if (strcmp(key, "name") == 0) {
      if (lua_type(lua, -1) == LUA_TSTRING) {
        snprintf(reinterpret_cast<char *>(this->name), 16, "%s",
                 lua_tostring(lua, -1));
      } else {
        snprintf(reinterpret_cast<char *>(this->name), 16, "%d",
                 lua_tointeger(lua, -1));
      }
    } else if (strcmp(key, "kind") == 0) {
      const char *kindString = luaL_checkstring(lua, -1);
      if (strcmp(kindString, "button") == 0) {
        this->type = ControlType::Button;
      } else if (strcmp(kindString, "select") == 0) {
        this->type = ControlType::Select;
      } else if (strcmp(kindString, "integer") == 0) {
        this->type = ControlType::Integer;
      }
    } else if (strcmp(key, "minimum") == 0) {
      this->range.minimum = lua_tointeger(lua, -1);
      this->currentValue = this->range.minimum;
    } else if (strcmp(key, "maximum") == 0) {
      this->range.maximum = lua_tointeger(lua, -1);
    } else if (strcmp(key, "step") == 0) {
      this->range.step = lua_tointeger(lua, -1);
    } else if (strcmp(key, "items") == 0) {
      this->items.count = 0;
      this->currentValue = 0;

      // さらにテーブルを読む
      lua_pushnil(lua);
      uint8_t readItems = 0;
      uint8_t fulfilled = 0;
      while (lua_next(lua, -2) != 0) {
        const char *itemString = lua_tostring(lua, -1);
        size_t itemLength = strlen(itemString) + 1;

        if (readItems >= 8 ||
            itemLength > static_cast<size_t>(128 - fulfilled)) {
          // 入り切らないので中断
          // まだ残っているので key, value 両方ポップしておく
          lua_pop(lua, 2);
          break;
        }

        this->items.headIndices[readItems++] = fulfilled;
        memcpy(this->items.buffer + fulfilled, itemString, itemLength);
        fulfilled += itemLength;

        lua_pop(lua, 1);
      }

      this->items.count = readItems;
    }
    lua_pop(lua, 1);
  }
}

/**
 * シリアルポートにメニュー項目の情報を表示する。
 */
void ControlFaceItem::dump() {
  Serial.printf("Setting Item -------------------\n");
  Serial.printf("Name: %s\n", this->name);
  switch (this->type) {
    case ControlType::Button:
      Serial.printf("Type: Button\n");
      break;
    case ControlType::Select: {
      Serial.printf("Type: Select\n");
      Serial.printf("Items: ");
      for (int i = 0; i < this->items.count; ++i) {
        Serial.printf("%s ", this->items.buffer + this->items.headIndices[i]);
      }
      Serial.printf("\n");
      break;
    }
    case ControlType::Integer: {
      Serial.printf("Type: Integer\n");
      Serial.printf("Range: %d to %d\n", this->range.minimum,
                    this->range.maximum);
      Serial.printf("Step: %d\n", this->range.step);
      break;
    }
  }
}

#include "controlface.hpp"

static const char *luaReader(lua_State *lua, void *data, size_t *size);

const int WIO_KEYS[8] = {
    WIO_5S_UP,    WIO_5S_DOWN, WIO_5S_LEFT, WIO_5S_RIGHT,
    WIO_5S_PRESS, WIO_KEY_A,   WIO_KEY_B,   WIO_KEY_C,
};

// ControlFaceItem ------------------------------------------------------------

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

// ControlFace ----------------------------------------------------------------

ControlFace::ControlFace(TFT_eSPI *display) : display{display} {}

/**
 * 全ての状態をリセットする。
 */
void ControlFace::reset() {
  for (int i = 0; i < 8; ++i) {
    pinMode(WIO_KEYS[i], INPUT_PULLUP);
  }

  if (this->lua) {
    lua_close(this->lua);
  }
  this->lua = luaL_newstate();
  luaL_openlibs(this->lua);
  irRegister(this->lua);

  this->selectedItem = 0;
  this->itemCount = 0;
}

/**
 * ループごとに毎回呼び出し、キーなどの処理をする。
 */
void ControlFace::tick() {
  // キー情報
  bool currentButtons[8];
  for (int i = 0; i < 8; ++i) {
    currentButtons[i] = digitalRead(WIO_KEYS[i]);
    triggerButtons[i] = this->previousButtons[i] && !currentButtons[i];
    this->previousButtons[i] = currentButtons[i];
  }

  if (triggerButtons[0]) {
    redrawItem(this->selectedItem, false);
    this->selectedItem =
        (this->selectedItem + this->itemCount - 1) % this->itemCount;
    redrawItem(this->selectedItem, true);
  }
  if (triggerButtons[1]) {
    redrawItem(this->selectedItem, false);
    this->selectedItem = (this->selectedItem + 1) % this->itemCount;
    redrawItem(this->selectedItem, true);
  }
  if (triggerButtons[2]) {
    this->items[this->selectedItem].movePrevious();
    redrawItem(this->selectedItem, true);
  }
  if (triggerButtons[3]) {
    this->items[this->selectedItem].moveNext();
    redrawItem(this->selectedItem, true);
  }
  if (triggerButtons[4]) {
    this->display->fillRect(288, 0, 32, 32, TFT_BLUE);
    this->send();
    delay(100);
    this->display->fillRect(288, 0, 32, 32, TFT_WHITE);
  }
}

void ControlFace::send() {
  if (lua_getglobal(this->lua, "onSend") != LUA_TFUNCTION) {
    Serial.printf("Lua Error: onSend is not a function\n");
    lua_pop(this->lua, 1);
    return;
  }

  // IRSender
  irPushIRSender(this->lua);

  // values
  lua_createtable(this->lua, 0, this->itemCount);
  for (size_t i = 0; i < this->itemCount; ++i) {
    lua_pushstring(this->lua,
                   reinterpret_cast<const char *>(this->items[i].getName()));
    switch (this->items[i].getType()) {
      case ControlType::Button:
        lua_pushinteger(this->lua, i == this->selectedItem ? 1 : 0);
        break;
      case ControlType::Select:
        // Lua は 1-based なのでそれを考慮して +1
        lua_pushinteger(this->lua, this->items[i].getValue() + 1);
        break;
      case ControlType::Integer:
        lua_pushinteger(this->lua, this->items[i].getValue());
        break;
    }
    lua_settable(this->lua, -3);
  }

  lua_call(this->lua, 2, 0);
}

/**
 * 画面全体を再描画する。
 */
void ControlFace::redrawAll() {
  this->display->fillScreen(TFT_WHITE);

  // 項目
  for (size_t i = 0; i < 8; ++i) {
    this->redrawItem(i, i == this->selectedItem);
  }
}

/**
 * 指定した位置のメニュー項目を再描画する。
 */
void ControlFace::redrawItem(size_t index, bool enabled) {
  this->display->fillRect(0, index * 24 + 48, 320, 24,
                          enabled ? TFT_BLACK : TFT_WHITE);
  this->display->drawLine(0, index * 24 + 48, 320, index * 24 + 48,
                          enabled ? TFT_WHITE : TFT_BLACK);
  this->display->drawLine(128, index * 24 + 48, 128, (index + 1) * 24 + 48,
                          enabled ? TFT_WHITE : TFT_BLACK);

  if (index >= this->itemCount) {
    return;
  }

  this->display->setFreeFont(&FreeMono9pt7b);
  this->display->setTextColor(TFT_RED);
  this->display->drawString(
      reinterpret_cast<const char *>(this->items[index].getName()), 0,
      index * 24 + 52);

  switch (this->items[index].getType()) {
    case ControlType::Button:
      this->display->drawString("Execute", 192, index * 24 + 52);
      break;
    case ControlType::Select:
      this->display->drawString(
          reinterpret_cast<const char *>(this->items[index].getStringValue()),
          192, index * 24 + 52);
      break;
    case ControlType::Integer:
      String number{this->items[index].getValue()};
      this->display->drawString(number, 192, index * 24 + 52);
      break;
  }
}

/**
 * スタックトップのテーブルからメニュー項目を読み込む。
 */
void ControlFace::loadItemsFromLua(File *file) {
  // Lua スクリプトを読み込む
  FileBuffer fileBuffer;
  fileBuffer.file = file;
  fileBuffer.readBytes = 0;
  fileBuffer.size = file->size();

  if (lua_load(this->lua, luaReader, &fileBuffer, file->name(), nullptr) ==
      LUA_OK) {
    lua_pcall(this->lua, 0, LUA_MULTRET, 0);
  } else {
    Serial.printf("Lua Error: %s\n", lua_tostring(this->lua, -1));
    lua_pop(this->lua, 1);
    return;
  }

  // getMenu 呼び出す
  if (lua_getglobal(this->lua, "getMenu") != LUA_TFUNCTION) {
    Serial.printf("Lua Error: getMenu is not a function\n");
    lua_pop(this->lua, 1);
    return;
  }
  lua_call(this->lua, 0, 1);

  // テーブルから読み込む
  size_t readItems = 0;
  lua_pushnil(this->lua);
  while (lua_next(this->lua, -2) != 0) {
    if (readItems >= 8) {
      // まだ残っているので key, value 両方ポップしておく
      lua_pop(this->lua, 2);
      break;
    }

    // 項目名もテーブル内に含めてある
    this->items[readItems++].loadSettingFromLuaTable(lua);
    lua_pop(this->lua, 1);
  }

  this->itemCount = readItems;
}

/**
 * シリアルポートに全てのメニュー項目の情報を表示する。
 */
void ControlFace::dumpMenuItems() {
  for (size_t i = 0; i < this->itemCount; ++i) {
    this->items[i].dump();
  }
}

/**
 * lua_Reader 実装
 */
static const char *luaReader(lua_State *lua, void *data, size_t *size) {
  FileBuffer *buffer = reinterpret_cast<FileBuffer *>(data);
  if (buffer->readBytes >= buffer->size) {
    *size = 0;
    return nullptr;
  } else {
    size_t fulfilled = buffer->file->readBytes(buffer->buffer, 64);
    buffer->readBytes += fulfilled;
    *size = fulfilled;
    return reinterpret_cast<const char *>(buffer->buffer);
  }
}

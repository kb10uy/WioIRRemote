#pragma once

#include <Seeed_FS.h>
#include <lua.hpp>

struct FileBuffer {
  File *file;
  size_t readBytes;
  size_t size;
  uint8_t buffer[64];
};

void luaReset();
int luaLoadFile(File *file);
const char *luaOnRead(lua_State *lua, void *data, size_t *size);

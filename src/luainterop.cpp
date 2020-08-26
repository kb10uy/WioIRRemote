#include "luainterop.hpp"

lua_State *lua = nullptr;

void luaReset() {
  if (lua != nullptr) {
    lua_close(lua);
  }

  lua = luaL_newstate();
  luaL_openlibs(lua);
}

int luaLoadFile(File *file) {
  FileBuffer fileBuffer;
  fileBuffer.file = file;
  fileBuffer.readBytes = 0;
  fileBuffer.size = 0;

  int status = lua_load(lua, luaOnRead, &fileBuffer, file->name(), nullptr);
  return status == LUA_OK ? lua_pcall(lua, 0, LUA_MULTRET, 0) : status;
}

const char *luaOnRead(lua_State *lua, void *data, size_t *size) {
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

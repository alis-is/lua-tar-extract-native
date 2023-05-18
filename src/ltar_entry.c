#include "lauxlib.h"
#include "ltar_archive.h"
#include "lua.h"
#include "lutil.h"
#include "stdio.h"
#include "string.h"
#include <stdlib.h>

int ltar_entry_header(lua_State *L) {
  TAR_ARCHIVE_ENTRY *entry = (TAR_ARCHIVE_ENTRY *)lua_touserdata(L, 1);
  lua_getiuservalue(L, 1, 1); // entry, archive
  TAR_ARCHIVE *tar = (TAR_ARCHIVE *)lua_touserdata(L, -1);
  if (tar->closed) {
    return push_error(L, "Cannot read closed file!");
  }
  fseek(tar->f, entry->headerStart, SEEK_SET);
  char header[512];
  fread(header, 512, 1, tar->f);
  lua_pushlstring(L, header, 512);
  return 1;
}

int ltar_entry_path(lua_State *L) {
  TAR_ARCHIVE_ENTRY *entry = (TAR_ARCHIVE_ENTRY *)lua_touserdata(L, 1);
  if (entry->path == NULL) {
    lua_pushnil(L);
    return 1;
  }
  lua_pushstring(L, entry->path);
  return 1;
}

int ltar_entry_linkpath(lua_State *L) {
  TAR_ARCHIVE_ENTRY *entry = (TAR_ARCHIVE_ENTRY *)lua_touserdata(L, 1);
  if (entry->linkpath == NULL) {
    lua_pushnil(L);
    return 1;
  }
  lua_pushstring(L, entry->linkpath);
  return 1;
}

int ltar_entry_type(lua_State *L) {
  TAR_ARCHIVE_ENTRY *entry = (TAR_ARCHIVE_ENTRY *)lua_touserdata(L, 1);
  lua_pushlstring(L, &entry->type, 1);
  return 1;
}

int ltar_entry_kind(lua_State *L) {
  TAR_ARCHIVE_ENTRY *entry = (TAR_ARCHIVE_ENTRY *)lua_touserdata(L, 1);
  switch (entry->type) {
  case TAR_FILE:
  case TAR_AFILE:
    lua_pushstring(L, "file");
    break;
  case TAR_DIR:
    lua_pushstring(L, "directory");
    break;
  case TAR_HARDLINK:
    lua_pushstring(L, "hardlink");
    break;
  case TAR_SYMLINK:
    lua_pushstring(L, "symlink");
    break;
  case TAR_CHARDEV:
    lua_pushstring(L, "chardev");
    break;
  case TAR_BLOCKDEV:
    lua_pushstring(L, "blockdev");
    break;
  case TAR_FIFO:
    lua_pushstring(L, "fifo");
    break;
  default:
    lua_pushstring(L, "other");
    break;
  }
  return 1;
}

int ltar_entry_size(lua_State *L) {
  TAR_ARCHIVE_ENTRY *entry = (TAR_ARCHIVE_ENTRY *)lua_touserdata(L, 1);
  lua_pushinteger(L, entry->size);
  return 1;
}

int ltar_entry_mode(lua_State *L) {
  TAR_ARCHIVE_ENTRY *entry = (TAR_ARCHIVE_ENTRY *)lua_touserdata(L, 1);
  lua_pushinteger(L, entry->mode);
  return 1;
}

int ltar_entry_seek(lua_State *L) {
  TAR_ARCHIVE_ENTRY *entry = (TAR_ARCHIVE_ENTRY *)lua_touserdata(L, 1);
  size_t whencel;
  const char *whence = luaL_checklstring(L, 2, &whencel);
  if (whencel == 0) {
    return push_error(L, "Invalid whence parameter");
  }
  ssize_t pos = luaL_optinteger(L, 3, 0);
  switch (whence[0]) {
  case 's':
    entry->readPosition = pos;
    break;
  case 'c':
    entry->readPosition += pos;
    break;
    ;
  case 'e':
    entry->readPosition = entry->size;
    break;
  }
  return 0;
}

int ltar_entry_read(lua_State *L) {
  TAR_ARCHIVE_ENTRY *entry = (TAR_ARCHIVE_ENTRY *)lua_touserdata(L, 1);
  size_t len = luaL_optinteger(L, 2, entry->size);
  if (entry->readPosition >= entry->size) {
    lua_pushnil(L);
    return 1;
  }

  lua_getiuservalue(L, 1, 1); // entry, archive

  TAR_ARCHIVE *tar = (TAR_ARCHIVE *)lua_touserdata(L, -1);
  if (tar->closed) {
    return push_error(L, "Cannot read closed file!");
  }

  size_t curPos = ftell(tar->f);
  size_t dataStart = entry->headerStart + 512;
  size_t targetPos = dataStart + entry->readPosition;
  if (curPos > targetPos) {
    fseek(tar->f, targetPos, SEEK_SET);
  } else {
    fseek(tar->f, targetPos - curPos, SEEK_CUR);
  }

  size_t dataL = entry->size - (targetPos - dataStart);
  if (dataL > len)
    dataL = len;

  luaL_Buffer b;
  luaL_buffinit(L, &b);

  do {
    size_t nextChunkSize = dataL > 512 ? 512 : dataL;
    char *p = luaL_prepbuffsize(&b, nextChunkSize);
    size_t res = fread(p, 1, nextChunkSize, tar->f);
    luaL_addsize(&b, res);
    entry->readPosition += res;
    dataL -= nextChunkSize;
  } while (dataL > 0);

  luaL_pushresult(&b);
  return 1;
}

int ltar_entry_close(lua_State *L) {
  TAR_ARCHIVE_ENTRY *entry = (TAR_ARCHIVE_ENTRY *)lua_touserdata(L, 1);
  free(entry->path);
}

int create_tar_entry_meta(lua_State *L) {
  luaL_newmetatable(L, ELI_TAR_ENTRY_METATABLE);

  /* Method table */
  lua_newtable(L);
  lua_pushcfunction(L, ltar_entry_read);
  lua_setfield(L, -2, "read");
  lua_pushcfunction(L, ltar_entry_size);
  lua_setfield(L, -2, "size");
  lua_pushcfunction(L, ltar_entry_kind);
  lua_setfield(L, -2, "kind");
  lua_pushcfunction(L, ltar_entry_type);
  lua_setfield(L, -2, "type");
  lua_pushcfunction(L, ltar_entry_header);
  lua_setfield(L, -2, "header");
  lua_pushcfunction(L, ltar_entry_seek);
  lua_setfield(L, -2, "seek");
  lua_pushcfunction(L, ltar_entry_path);
  lua_setfield(L, -2, "path");
  lua_pushcfunction(L, ltar_entry_linkpath);
  lua_setfield(L, -2, "linkpath");
  lua_pushcfunction(L, ltar_entry_mode);
  lua_setfield(L, -2, "mode");

  lua_pushstring(L, ELI_TAR_ENTRY_METATABLE);
  lua_setfield(L, -2, "__type");

  /* Metamethods */
  lua_setfield(L, -2, "__index");
  lua_pushcfunction(L, ltar_entry_close);
  lua_setfield(L, -2, "__gc");

  return 1;
}
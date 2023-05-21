#include "ltar_archive.h"
#include "lauxlib.h"
#include "ltar.h"
#include "lua.h"
#include "lutil.h"
#include "stdio.h"
#include "string.h"
#include <stdlib.h>

/* Parse an octal number, ignoring leading and trailing nonsense. */
static int parseoct(const char *p, size_t n) {
  int i = 0;
  while ((*p < '0' || *p > '7') && n > 0) {
    ++p;
    --n;
  }
  while (*p >= '0' && *p <= '7' && n > 0) {
    i *= 8;
    i += *p - '0';
    ++p;
    --n;
  }
  return (i);
}

/* Returns true if this is 512 zero bytes. */
static int is_end_of_archive(const char *p) {
  int n;
  for (n = 511; n >= 0; --n)
    if (p[n] != '\0')
      return (0);
  return (1);
}

static int verify_checksum(const char *p) {
  int n, u = 0;
  for (n = 0; n < 512; ++n) {
    if (n < 148 || n > 155)
      /* Standard tar checksum adds unsigned bytes. */
      u += ((unsigned char *)p)[n];
    else
      u += 0x20;
  }
  return (u == parseoct(p + 148, 8));
}

int ltar_open(lua_State *L) {
  char buff[512];
  char error[130]; // 100 for path + 30 for error
  size_t bytes_read;
  const char *path = luaL_checkstring(L, 1);
  lua_remove(L, 1);

  FILE *f = fopen(path, "rb");
  if (f == NULL) {
    sprintf(error, "Failed to open tar file - %s!", path);
    return push_error(L, error);
  }
  lua_newtable(L); // entries
  TAR_ARCHIVE *archive =
      lua_newuserdata(L, sizeof(TAR_ARCHIVE)); // entries, archive
  archive->f = f;
  archive->closed = 0;
  luaL_getmetatable(L, TAR_ARCHIVE_METATABLE); // entries, archive, archive meta
  lua_setmetatable(L, -2);                     // entries, archive

  size_t fileIndex = 1; // lua indexing
  size_t position = 0;

  for (;;) {
    bytes_read = fread(buff, 1, 512, f);
    if (bytes_read < 512) {
      sprintf(error, "Short read on %s: expected 512, got %d\n", path,
              (int)bytes_read);
      return push_error(L, error);
    }
    if (is_end_of_archive(buff)) {
      break;
    }
    if (!verify_checksum(buff)) {
      return push_error(L, "Checksum failure");
    }

    size_t filesize = parseoct(buff + 124, 12);

    // entries, archive
    TAR_ARCHIVE_ENTRY *entry = lua_newuserdatauv(L, sizeof(TAR_ARCHIVE_ENTRY),
                                                 1); // entries, archive, entry

    // store archive in entry's user value
    lua_pushvalue(L, 2);         // entries, archive, entry, archive
    lua_setiuservalue(L, -2, 1); // entries, archive, entry

    entry->path = strdup(buff);
    entry->type = buff[156];
    entry->linkpath =
        (entry->type == TAR_HARDLINK || entry->type == TAR_SYMLINK)
            ? strdup(buff + 157)
            : NULL;
    entry->headerStart = position;
    entry->readPosition = 0;
    entry->size = filesize;
    entry->mode = parseoct(buff + 100, 8);
    luaL_getmetatable(
        L, TAR_ARCHIVE_ENTRY_METATABLE); // entries, archive, entry, entry meta
    lua_setmetatable(L, -2);             // entries, archive, entry
    lua_seti(L, -3, fileIndex++);        // entries, archive
    filesize = filesize % 512 == 0 ? filesize
                                   : (filesize / 512 + 1) *
                                         512; // align file size to 512B block
    position += filesize + 512;               // filesize + header
    fseek(f, filesize, SEEK_CUR);
  }
  // entries, archive
  lua_setfield(L, 1, "archive"); // entries
  return 1;
}

int ltar_close(lua_State *L) {
  TAR_ARCHIVE *archive = (TAR_ARCHIVE *)luaL_checkudata(L, 1, TAR_ARCHIVE_METATABLE);
  if (!archive->closed) {
    fclose(archive->f);
    archive->closed = 1;
  }
}

int create_tar_meta(lua_State *L) {
  luaL_newmetatable(L, TAR_ARCHIVE_METATABLE);

  /* Method table */
  lua_newtable(L);
  lua_pushcfunction(L, ltar_close);
  lua_setfield(L, -2, "close");

  lua_pushstring(L, TAR_ARCHIVE_METATABLE);
  lua_setfield(L, -2, "__type");

  /* Metamethods */
  lua_setfield(L, -2, "__index");
  lua_pushcfunction(L, ltar_close);
  lua_setfield(L, -2, "__gc");

  return 1;
}

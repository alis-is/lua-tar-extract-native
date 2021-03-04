#ifndef LTAR_ARCHIVE_H_
#define LTAR_ARCHIVE_H_

#include "lua.h"
#include <stdio.h>
#include "ltar_entry.h"

#define ELI_TAR_METATABLE "TAR_ARCHIVE_METATABLE"

typedef struct TAR_ARCHIVE {
  FILE *f;
  int closed;
} TAR_ARCHIVE;

int create_tar_meta(lua_State *L);
int ltar_open(lua_State *L);

#endif
#ifndef LTAR_ARCHIVE_H_
#define LTAR_ARCHIVE_H_

#include "lua.h"
#include <stdio.h>
#include "ltar_entry.h"

#define TAR_ARCHIVE_METATABLE "TAR_ARCHIVE"

typedef struct TAR_ARCHIVE {
  FILE *f;
  int closed;
} TAR_ARCHIVE;

int create_tar_meta(lua_State *L);
int ltar_open(lua_State *L);

#define TAR_FILE '0'
#define TAR_AFILE '\0'
#define TAR_HARDLINK '1'
#define TAR_SYMLINK '2'
#define TAR_CHARDEV '3'
#define TAR_BLOCKDEV '4'
#define TAR_DIR '5'
#define TAR_FIFO '6'
#define TAR_CONTIGUOUS '7'
#define TAR_XHDTYPE 'x'
#define TAR_XGLTYPE 'g'
#define TAR_GNU_LONGNAME 'L'

#endif
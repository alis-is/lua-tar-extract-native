#ifndef LTAR_ARCHIVE_ENTRY_H_
#define LTAR_ARCHIVE_ENTRY_H_

#include "lua.h"

#define ELI_TAR_ENTRY_METATABLE "TAR_ARCHIVE_ENTRY_METATABLE"

typedef enum TAR_ARCHIVE_ENTRY_KIND {
  ELI_TAR_ENTRY_FILE_KIND,
  ELI_TAR_ENTRY_DIR_KIND,
  ELI_TAR_ENTRY_OTHER_KIND
} TAR_ARCHIVE_ENTRY_KIND;

typedef struct TAR_ARCHIVE_ENTRY {
  char *path;
  size_t size;
  int mode;
  size_t readPosition;
  size_t headerStart; // data starts headerStart + 512
  TAR_ARCHIVE_ENTRY_KIND kind;
} TAR_ARCHIVE_ENTRY;

int create_tar_entry_meta(lua_State *L);
#endif
#ifndef LTAR_ARCHIVE_ENTRY_H_
#define LTAR_ARCHIVE_ENTRY_H_

#include "lua.h"

#define TAR_ARCHIVE_ENTRY_METATABLE "TAR_ARCHIVE_ENTRY"

typedef struct TAR_ARCHIVE_ENTRY {
	char *path;
	char *linkpath;
	size_t size;
	int mode;
	size_t readPosition;
	size_t headerStart;
	char type;
} TAR_ARCHIVE_ENTRY;

int create_tar_entry_meta(lua_State *L);
#endif
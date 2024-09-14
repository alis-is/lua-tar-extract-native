#include "lauxlib.h"
#include "ltar_archive.h"
#include "ltar_entry.h"
#include "lua.h"

typedef struct {
	const char *name;
	char value;
} TarTypeConst;

static const struct luaL_Reg ltarFuncs[] = { { "open", ltar_open },
					     { NULL, NULL } };

static const TarTypeConst tarTypes[] = { { "FILE", TAR_FILE },
					 { "AFILE", TAR_AFILE },
					 { "HARDLINK", TAR_HARDLINK },
					 { "SYMLINK", TAR_SYMLINK },
					 { "CHARDEV", TAR_CHARDEV },
					 { "BLOCKDEV", TAR_BLOCKDEV },
					 { "DIR", TAR_DIR },
					 { "FIFO", TAR_FIFO },
					 { "CONTIGUOUS", TAR_CONTIGUOUS },
					 { "XHDTYPE", TAR_XHDTYPE },
					 { "XGLTYPE", TAR_XGLTYPE },
					 { "GNU_LONGNAME", TAR_GNU_LONGNAME },
					 { "GNU_LONGLINK", TAR_GNU_LONGLINK } };

int luaopen_ltar(lua_State *L)
{
	create_tar_meta(L);
	create_tar_entry_meta(L);

	lua_newtable(L);

	char type = { 0 }; // Used to convert char to string
	for (size_t i = 0; i < sizeof(tarTypes) / sizeof(TarTypeConst); ++i) {
		type = tarTypes[i].value;
		lua_pushstring(L, tarTypes[i].name);
		lua_pushlstring(L, &type, 1);
		lua_settable(L, -3);
	}

	luaL_setfuncs(L, ltarFuncs, 0);
	return 1;
}

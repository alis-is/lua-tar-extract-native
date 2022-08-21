#include "ltar_entry.h"
#include "ltar_archive.h"
#include "lua.h"
#include "lauxlib.h"

static const struct luaL_Reg ltarFuncs[] = {
    {"open", ltar_open},
    {NULL, NULL}};

int luaopen_ltar(lua_State *L) {
  create_tar_meta(L);
  create_tar_entry_meta(L);

  lua_newtable(L);
  luaL_setfuncs(L, ltarFuncs, 0);
  return 1;
}

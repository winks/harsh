extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

std::string readfile(std::string filename, bool skipFirst = false);
std::vector<std::string> split(std::string s);
std::string trim(std::string s);

int luax_argv(lua_State *L);
int luax_dirname(lua_State *L);
int luax_basename(lua_State *L);
int luax_abspath(lua_State *L);
int luax_execv(lua_State *L);
int luax_mkdir(lua_State *L);
int luax_slurp(lua_State *L);
int luax_spit(lua_State *L);

#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <streambuf>
#include <string>
#include <unistd.h>
#include <vector>
#include "main.h"

#define HARSHLIB_PATH "/usr/local/share/harshlib.lua"
#define ERR_PREFIX "#o# "

std::string readfile(std::string filename, bool skipFirstLine) {
    std::ifstream t(filename);
    std::string str((std::istreambuf_iterator<char>(t)), 
                     std::istreambuf_iterator<char>());

    if (skipFirstLine) {
        auto first = str.find("\n");
        str.erase(0, first);
    }
    return str;
}

std::vector<std::string> split(std::string s) {
    std::vector<std::string> rv;
    std::string ws = " \n\r\t";
    int i = 0;
    size_t left  = 0;
    while ((i = s.substr(left).find_first_of(ws)) != -1) {
        std::string r = s.substr(left, static_cast<size_t>(i));
        rv.push_back(r);
        left = left + 1 + static_cast<size_t>(i);
    }
    rv.push_back(s.substr(left));
    return rv;
}

std::string trim(std::string s) {
    return std::regex_replace(s, std::regex{R"(^\s+|\s+$)"}, "");
}

int luax_argv(lua_State *L) {
    int idx = luaL_checkinteger(L, 1);
    auto idx2 = static_cast<long unsigned int>(idx);
    const std::vector<std::string> *pVec = static_cast<const std::vector<std::string> *>(lua_topointer(L, lua_upvalueindex(1)));
    
    if (idx2 >= pVec->size()) {
        lua_pushnil(L);
    } else {
        lua_pushstring(L, pVec->at(idx2).c_str());
    }

    return 1;
}

int luax_abspath(lua_State *L) {
    size_t * len = 0;
    const char* v = luaL_checklstring(L, 1, len);
    lua_pushstring(L, std::filesystem::canonical(std::filesystem::absolute(v)).lexically_normal().c_str());
    return 1;
}

int luax_dirname(lua_State *L) {
    size_t * len = 0;
    const char* v = luaL_checklstring(L, 1, len);
    lua_pushstring(L, std::filesystem::canonical(std::filesystem::absolute(v).remove_filename()).lexically_normal().c_str());
    return 1;
}

int luax_basename(lua_State *L) {
    size_t * len = 0;
    const char * v = luaL_checklstring(L, 1, len);
    lua_pushstring(L, std::filesystem::absolute(v).filename().c_str());
    return 1;
}

int luax_execv(lua_State *L) {
    size_t * len = 0;
    const char * v = luaL_checklstring(L, 1, len);
    std::cerr << ERR_PREFIX << "val : " << v << std::endl;
    std::string val(v);
    std::vector parts(split(trim(val)));
    //static char * newenviron[] = { NULL };
    static char * newargv[] = { parts[0].data() };
    std::vector<const char *> all;
    std::string envbin("/usr/bin/env");
    all.push_back(envbin.data());
    for (auto i = parts.begin(); i != parts.end(); ++i) {
        all.push_back((*i).data());
    }
    all.push_back(NULL);
    execv(envbin.data(), const_cast<char * const *>(all.data()));

    return 0;
}

int luax_mkdir(lua_State *L) {
    size_t * len = 0;
    const char * v = luaL_checklstring(L, 1, len);
    std::string dir(v);
    std::filesystem::create_directories(dir);
    lua_pushboolean(L, 1);
    return 1;
}

int luax_slurp(lua_State *L) {
    size_t * len = 0;
    const char * v = luaL_checklstring(L, 1, len);
    std::string fileName(v);
    std::string rv(readfile(fileName));
    lua_pushstring(L, rv.c_str());
    return 1;
}

int luax_spit(lua_State *L) {
    size_t * len = 0;
    const char * v = luaL_checklstring(L, 1, len);
    size_t * len2 = 0;
    const char * v2 = luaL_checklstring(L, 2, len2);
    std::string fileName(v);
    std::string data(v2);
    std::ofstream ostream;
    ostream.open(fileName);
    ostream << data;
    ostream.close();
    lua_pushboolean(L, 1);
    return 1;
}

static int luax_sleep(lua_State *L) {
	long secs = lua_tointeger(L, -1);
	sleep(secs);
	return 0;
}

static int luax_msleep(lua_State *L) {
	long msecs = lua_tointeger(L, -1);
	usleep(1000*msecs);
	return 0;
}


int main(int argc, char ** argv) {
    if (argc == 1) return 0;
    std::cerr << ERR_PREFIX << "argc  : " << argc << std::endl;
    std::vector<std::string> vargv;
    for (int i = 0; i < argc; i++) {
        std::cerr << ERR_PREFIX << "" << argv[i] << std::endl;
        if (i > 0) {
            vargv.push_back(argv[i]);
        }
    }
    std::cerr << ERR_PREFIX << "vargv : " << vargv.size() << std::endl;

    auto pwd = std::filesystem::current_path();
    std::cerr << ERR_PREFIX << "pwd   : " << pwd << std::endl;

    std::string all_code;
    std::string file_code;
    all_code = readfile(HARSHLIB_PATH);
    std::cerr << ERR_PREFIX << all_code.length() << " harshlib" << std::endl;
    file_code = readfile(argv[1], true);
    std::cerr << ERR_PREFIX << file_code.length() << " " << argv[1] << std::endl;
    //std::cout << "[[" << mycode << "]]" << std::endl;
    all_code.append(file_code);
    //std::cerr << "[[" << mylib << "]]" << std::endl;
    std::cerr << ERR_PREFIX << "---------------------" << std::endl;


    // lua init starts here
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    
    lua_pushlightuserdata(L, static_cast<void*>(&vargv));
    lua_pushcclosure(L, luax_argv, 1);
    lua_setglobal(L, "argv");

    lua_pushcfunction(L, luax_abspath);
    lua_setglobal(L, "abspath");
    lua_pushcfunction(L, luax_basename);
    lua_setglobal(L, "basename");
    lua_pushcfunction(L, luax_dirname);
    lua_setglobal(L, "dirname");
    lua_pushcfunction(L, luax_execv);
    lua_setglobal(L, "execv");

    lua_register(L, "sleep", luax_sleep);
    lua_register(L, "msleep", luax_msleep);
    lua_register(L, "mkdir", luax_mkdir);
    lua_register(L, "slurp", luax_slurp);
    lua_register(L, "spit", luax_spit);

    if (luaL_loadstring(L, all_code.c_str()) == LUA_OK) {
        if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
            std::cerr << lua_tostring(L, lua_gettop(L)) << std::endl;
        }
        lua_pop(L, lua_gettop(L));
    }

    lua_close(L);
    return 0;
}

/*
** $Id: linit.c $
** Initialization of libraries for lua.c and other clients
** Lua的初始化库和其他客户端
** See Copyright Notice in lua.h
*/


#define linit_c
#define LUA_LIB

/*
** If you embed Lua in your program and need to open the standard
** libraries, call luaL_openlibs in your program. If you need a
** different set of libraries, copy this file to your project and edit
** it to suit your needs.
** 如果在程序中嵌入Lua并需要打开标准库，请在程序中调用luaL_openlibs。
** 如果您需要一组不同的库，请将此文件复制到项目中并进行编辑以满足您的需求。
**
** You can also *preload* libraries, so that a later 'require' can
** open the library, which is already linked to the application.
** For that, do the following code:
** 您还是可以 *preload* 库，以便稍后的 'require' 可以打开已链接到应用程序的库。
** 为此，请执行以下代码：
**
**  luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_PRELOAD_TABLE);
**  lua_pushcfunction(L, luaopen_modname);
**  lua_setfield(L, -2, modname);
**  lua_pop(L, 1);  // remove PRELOAD table
*/

#include "src/lprefix.h"


#include <stddef.h>

#include "src/lua.h"

#include "src/lualib.h"
#include "src/lauxlib.h"


/*
** these libs are loaded by lua.c and are readily available to any Lua
** program
** 这些库由 lua.c 并且可随时用于任何Lua程序
*/
static const luaL_Reg loadedlibs[] = {
  {LUA_GNAME, luaopen_base}, // 基础库
  {LUA_STRLIBNAME, luaopen_string}, // 字符串库
  {LUA_TABLIBNAME, luaopen_table}, // 表库
  {LUA_MATHLIBNAME, luaopen_math}, // 数学库
  {NULL, NULL}
};


LUALIB_API void luaL_openlibs (lua_State *L) {
  const luaL_Reg *lib;
  /* 
     "require" functions from 'loadedlibs' and set results to global table 
     从 'loadedlibs' 中 "require"函数并将结果设置到全局表
  */
  for (lib = loadedlibs; lib->func; lib++) {
    luaL_requiref(L, lib->name, lib->func, 1);
    lua_pop(L, 1);  /* remove lib */
  }
}


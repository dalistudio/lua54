/*
** $Id: lualib.h $
** Lua standard libraries
** Lua 标准库
** See Copyright Notice in lua.h
*/


#ifndef lualib_h
#define lualib_h

#include "lua.h"


/* 
   version suffix for environment variable names 
   环境变量名称的版本后缀
*/
#define LUA_VERSUFFIX          "_" LUA_VERSION_MAJOR "_" LUA_VERSION_MINOR


LUAMOD_API int (luaopen_base) (lua_State *L);

#define LUA_COLIBNAME	"coroutine" // 协程库
LUAMOD_API int (luaopen_coroutine) (lua_State *L);

#define LUA_TABLIBNAME	"table" // 表库
LUAMOD_API int (luaopen_table) (lua_State *L);

#define LUA_IOLIBNAME	"io" // 输入输出库
LUAMOD_API int (luaopen_io) (lua_State *L);

#define LUA_OSLIBNAME	"os" // 操作系统库
LUAMOD_API int (luaopen_os) (lua_State *L);

#define LUA_STRLIBNAME	"string" // 字符串库
LUAMOD_API int (luaopen_string) (lua_State *L);

#define LUA_UTF8LIBNAME	"utf8" // UTF8库
LUAMOD_API int (luaopen_utf8) (lua_State *L);

#define LUA_MATHLIBNAME	"math" // 数学库
LUAMOD_API int (luaopen_math) (lua_State *L);

#define LUA_DBLIBNAME	"debug" // 调试库
LUAMOD_API int (luaopen_debug) (lua_State *L);

#define LUA_LOADLIBNAME	"package" // 封包库
LUAMOD_API int (luaopen_package) (lua_State *L);


/* open all previous libraries 打开所有以前的库 */
LUALIB_API void (luaL_openlibs) (lua_State *L);


#endif

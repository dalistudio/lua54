/*
** $Id: lstring.h $
** String table (keep all strings handled by Lua)
** 字符串表（保留Lua处理的所有字符串）
** See Copyright Notice in lua.h
*/

#ifndef lstring_h
#define lstring_h

#include "lgc.h"
#include "lobject.h"
#include "lstate.h"


/*
** Memory-allocation error message must be preallocated (it cannot
** be created after memory is exhausted)
** 内存分配错误消息必须预先分配（内存耗尽后无法创建）
*/
#define MEMERRMSG       "not enough memory" // 内存不足


/*
** Size of a TString: Size of the header plus space for the string
** itself (including final '\0').
** TString的大小：标头的大小加上字符串本身的空间（包括最后的'\0'）
*/
#define sizelstring(l)  (offsetof(TString, contents) + ((l) + 1) * sizeof(char))

#define luaS_newliteral(L, s)	(luaS_newlstr(L, "" s, \
                                 (sizeof(s)/sizeof(char))-1))


/*
** test whether a string is a reserved word
** 测试字符串是否为保留字
*/
#define isreserved(s)	((s)->tt == LUA_VSHRSTR && (s)->extra > 0)


/*
** equality for short strings, which are always internalized
** 短字符串的相等性，这些字符串总是内部化的
*/
#define eqshrstr(a,b)	check_exp((a)->tt == LUA_VSHRSTR, (a) == (b))


LUAI_FUNC unsigned int luaS_hash (const char *str, size_t l, unsigned int seed);
LUAI_FUNC unsigned int luaS_hashlongstr (TString *ts);
LUAI_FUNC int luaS_eqlngstr (TString *a, TString *b);
LUAI_FUNC void luaS_resize (lua_State *L, int newsize);
LUAI_FUNC void luaS_clearcache (global_State *g);
LUAI_FUNC void luaS_init (lua_State *L);
LUAI_FUNC void luaS_remove (lua_State *L, TString *ts);
LUAI_FUNC Udata *luaS_newudata (lua_State *L, size_t s, int nuvalue);
LUAI_FUNC TString *luaS_newlstr (lua_State *L, const char *str, size_t l);
LUAI_FUNC TString *luaS_new (lua_State *L, const char *str);
LUAI_FUNC TString *luaS_createlngstrobj (lua_State *L, size_t l);


#endif

/*
** $Id: lundump.h $
** load precompiled Lua chunks
** 加载预编译的Lua块
** See Copyright Notice in lua.h
*/

#ifndef lundump_h
#define lundump_h

#include "llimits.h"
#include "lobject.h"
#include "lzio.h"


/* data to catch conversion errors 要捕获转换错误的数据 */
#define LUAC_DATA	"\x19\x93\r\n\x1a\n"

#define LUAC_INT	0x5678
#define LUAC_NUM	cast_num(370.5)

/*
** Encode major-minor version in one byte, one nibble for each
** 用一个字节对主要次要版本进行编码，每个字节一个半字节
*/
#define MYINT(s)	(s[0]-'0')  /* assume one-digit numerals 假设一位数 */
#define LUAC_VERSION	(MYINT(LUA_VERSION_MAJOR)*16+MYINT(LUA_VERSION_MINOR))

#define LUAC_FORMAT	0	/* this is the official format 这是正式格式 */

/* load one chunk; from lundump.c 加载一个块；来自lundump.c */
LUAI_FUNC LClosure* luaU_undump (lua_State* L, ZIO* Z, const char* name);

/* dump one chunk; from ldump.c 转储一个块；来自ldump.c */
LUAI_FUNC int luaU_dump (lua_State* L, const Proto* f, lua_Writer w,
                         void* data, int strip);

#endif

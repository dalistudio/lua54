/*
** $Id: lauxlib.h $
** Auxiliary functions for building Lua libraries
** 构建Lua库的辅助功能
** See Copyright Notice in lua.h
*/


#ifndef lauxlib_h
#define lauxlib_h


#include <stddef.h>
#include <stdio.h>

#include "luaconf.h"
#include "lua.h"


/* global table 全局表 */
#define LUA_GNAME	"_G"


typedef struct luaL_Buffer luaL_Buffer;


/* extra error code for 'luaL_loadfilex'
   'luaL_loadfilex' 的额外错误代码
*/
#define LUA_ERRFILE     (LUA_ERRERR+1)


/* 
   key, in the registry, for table of loaded modules 
   注册表中已加载模块表的键
*/
#define LUA_LOADED_TABLE	"_LOADED"


/* 
   key, in the registry, for table of preloaded loaders 
   注册表中的键，用于预加载的加载程序表
*/
#define LUA_PRELOAD_TABLE	"_PRELOAD"


typedef struct luaL_Reg {
  const char *name;
  lua_CFunction func;
} luaL_Reg;


#define LUAL_NUMSIZES	(sizeof(lua_Integer)*16 + sizeof(lua_Number))

LUALIB_API void (luaL_checkversion_) (lua_State *L, lua_Number ver, size_t sz);
#define luaL_checkversion(L)  \
	  luaL_checkversion_(L, LUA_VERSION_NUM, LUAL_NUMSIZES)

LUALIB_API int (luaL_getmetafield) (lua_State *L, int obj, const char *e);
LUALIB_API int (luaL_callmeta) (lua_State *L, int obj, const char *e);
LUALIB_API const char *(luaL_tolstring) (lua_State *L, int idx, size_t *len);
LUALIB_API int (luaL_argerror) (lua_State *L, int arg, const char *extramsg);
LUALIB_API int (luaL_typeerror) (lua_State *L, int arg, const char *tname);
LUALIB_API const char *(luaL_checklstring) (lua_State *L, int arg,
                                                          size_t *l);
LUALIB_API const char *(luaL_optlstring) (lua_State *L, int arg,
                                          const char *def, size_t *l);
LUALIB_API lua_Number (luaL_checknumber) (lua_State *L, int arg);
LUALIB_API lua_Number (luaL_optnumber) (lua_State *L, int arg, lua_Number def);

LUALIB_API lua_Integer (luaL_checkinteger) (lua_State *L, int arg);
LUALIB_API lua_Integer (luaL_optinteger) (lua_State *L, int arg,
                                          lua_Integer def);

LUALIB_API void (luaL_checkstack) (lua_State *L, int sz, const char *msg);
LUALIB_API void (luaL_checktype) (lua_State *L, int arg, int t);
LUALIB_API void (luaL_checkany) (lua_State *L, int arg);

LUALIB_API int   (luaL_newmetatable) (lua_State *L, const char *tname);
LUALIB_API void  (luaL_setmetatable) (lua_State *L, const char *tname);
LUALIB_API void *(luaL_testudata) (lua_State *L, int ud, const char *tname);
LUALIB_API void *(luaL_checkudata) (lua_State *L, int ud, const char *tname);

LUALIB_API void (luaL_where) (lua_State *L, int lvl);
LUALIB_API int (luaL_error) (lua_State *L, const char *fmt, ...);

LUALIB_API int (luaL_checkoption) (lua_State *L, int arg, const char *def,
                                   const char *const lst[]);

LUALIB_API int (luaL_fileresult) (lua_State *L, int stat, const char *fname);
LUALIB_API int (luaL_execresult) (lua_State *L, int stat);


/* predefined references 预定义的引用 */
#define LUA_NOREF       (-2)
#define LUA_REFNIL      (-1)

LUALIB_API int (luaL_ref) (lua_State *L, int t);
LUALIB_API void (luaL_unref) (lua_State *L, int t, int ref);

LUALIB_API int (luaL_loadfilex) (lua_State *L, const char *filename,
                                               const char *mode);

#define luaL_loadfile(L,f)	luaL_loadfilex(L,f,NULL)

LUALIB_API int (luaL_loadbufferx) (lua_State *L, const char *buff, size_t sz,
                                   const char *name, const char *mode);
LUALIB_API int (luaL_loadstring) (lua_State *L, const char *s);

LUALIB_API lua_State *(luaL_newstate) (void);

LUALIB_API lua_Integer (luaL_len) (lua_State *L, int idx);

LUALIB_API void (luaL_addgsub) (luaL_Buffer *b, const char *s,
                                     const char *p, const char *r);
LUALIB_API const char *(luaL_gsub) (lua_State *L, const char *s,
                                    const char *p, const char *r);

LUALIB_API void (luaL_setfuncs) (lua_State *L, const luaL_Reg *l, int nup);

LUALIB_API int (luaL_getsubtable) (lua_State *L, int idx, const char *fname);

LUALIB_API void (luaL_traceback) (lua_State *L, lua_State *L1,
                                  const char *msg, int level);

LUALIB_API void (luaL_requiref) (lua_State *L, const char *modname,
                                 lua_CFunction openf, int glb);

/*
** ===============================================================
** some useful macros 一些有用的宏
** ===============================================================
*/


#define luaL_newlibtable(L,l)	\
  lua_createtable(L, 0, sizeof(l)/sizeof((l)[0]) - 1)

#define luaL_newlib(L,l)  \
  (luaL_checkversion(L), luaL_newlibtable(L,l), luaL_setfuncs(L,l,0))

#define luaL_argcheck(L, cond,arg,extramsg)	\
	((void)(luai_likely(cond) || luaL_argerror(L, (arg), (extramsg))))

#define luaL_argexpected(L,cond,arg,tname)	\
	((void)(luai_likely(cond) || luaL_typeerror(L, (arg), (tname))))

#define luaL_checkstring(L,n)	(luaL_checklstring(L, (n), NULL))
#define luaL_optstring(L,n,d)	(luaL_optlstring(L, (n), (d), NULL))

#define luaL_typename(L,i)	lua_typename(L, lua_type(L,(i)))

#define luaL_dofile(L, fn) \
	(luaL_loadfile(L, fn) || lua_pcall(L, 0, LUA_MULTRET, 0))

#define luaL_dostring(L, s) \
	(luaL_loadstring(L, s) || lua_pcall(L, 0, LUA_MULTRET, 0))

#define luaL_getmetatable(L,n)	(lua_getfield(L, LUA_REGISTRYINDEX, (n)))

#define luaL_opt(L,f,n,d)	(lua_isnoneornil(L,(n)) ? (d) : f(L,(n)))

#define luaL_loadbuffer(L,s,sz,n)	luaL_loadbufferx(L,s,sz,n,NULL)


/*
** Perform arithmetic operations on lua_Integer values with wrap-around
** semantics, as the Lua core does.
** 像Lua核心一样，使用环绕语义对 lua_Integer 值执行算术运算。
*/
#define luaL_intop(op,v1,v2)  \
	((lua_Integer)((lua_Unsigned)(v1) op (lua_Unsigned)(v2)))


/* 
   push the value used to represent failure/error 
   推送用于表示故障/错误的值
*/
#define luaL_pushfail(L)	lua_pushnil(L)


/*
** Internal assertions for in-house debugging
** 内部调试的内部断言
*/
#if !defined(lua_assert)

#if defined LUAI_ASSERT
  #include <assert.h>
  #define lua_assert(c)		assert(c)
#else
  #define lua_assert(c)		((void)0)
#endif

#endif



/*
** {======================================================
** Generic Buffer manipulation
** 通用缓冲区操作
** =======================================================
*/

struct luaL_Buffer {
  char *b;  /* buffer address 缓冲地址 */
  size_t size;  /* buffer size 缓冲大小 */
  size_t n;  /* number of characters in buffer 缓冲区中的字节数 */
  lua_State *L;
  union {
    LUAI_MAXALIGN;  /* ensure maximum alignment for buffer 确保缓冲区的最大对齐 */
    char b[LUAL_BUFFERSIZE];  /* initial buffer 初始化缓冲区 */
  } init;
};


#define luaL_bufflen(bf)	((bf)->n)
#define luaL_buffaddr(bf)	((bf)->b)


#define luaL_addchar(B,c) \
  ((void)((B)->n < (B)->size || luaL_prepbuffsize((B), 1)), \
   ((B)->b[(B)->n++] = (c)))

#define luaL_addsize(B,s)	((B)->n += (s))

#define luaL_buffsub(B,s)	((B)->n -= (s))

LUALIB_API void (luaL_buffinit) (lua_State *L, luaL_Buffer *B);
LUALIB_API char *(luaL_prepbuffsize) (luaL_Buffer *B, size_t sz);
LUALIB_API void (luaL_addlstring) (luaL_Buffer *B, const char *s, size_t l);
LUALIB_API void (luaL_addstring) (luaL_Buffer *B, const char *s);
LUALIB_API void (luaL_addvalue) (luaL_Buffer *B);
LUALIB_API void (luaL_pushresult) (luaL_Buffer *B);
LUALIB_API void (luaL_pushresultsize) (luaL_Buffer *B, size_t sz);
LUALIB_API char *(luaL_buffinitsize) (lua_State *L, luaL_Buffer *B, size_t sz);

#define luaL_prepbuffer(B)	luaL_prepbuffsize(B, LUAL_BUFFERSIZE)

/* }====================================================== */



/*
** {======================================================
** File handles for IO library 
** IO库的文件句柄
** =======================================================
*/

/*
** A file handle is a userdata with metatable 'LUA_FILEHANDLE' and
** initial structure 'luaL_Stream' (it may contain other fields
** after that initial structure).
** 文件句柄是具有元表 'LUA_FILEHANDLE' 和初始化'luaL_Stream'结构 的用户数据，
** （它可能包含该初始结构之后的其他字段）
*/

#define LUA_FILEHANDLE          "FILE*"


typedef struct luaL_Stream {
  FILE *f;  /* stream (NULL for incompletely created streams) 流（对于未完全创建的流为NULL）*/
  lua_CFunction closef;  /* to close stream (NULL for closed streams) 关闭流（关闭流为NULL）*/
} luaL_Stream;

/* }====================================================== */

/*
** {==================================================================
** "Abstraction Layer" for basic report of messages and errors
** 消息和错误基本报告的“抽象层”
** ===================================================================
*/

/* print a string 打印字符串 */
#if !defined(lua_writestring)
#define lua_writestring(s,l)   fwrite((s), sizeof(char), (l), stdout)
#endif

/* print a newline and flush the output 打印新行并刷新输出 */
#if !defined(lua_writeline)
#define lua_writeline()        (lua_writestring("\n", 1), fflush(stdout))
#endif

/* print an error message 打印错误消息 */
#if !defined(lua_writestringerror)
#define lua_writestringerror(s,p) \
        (fprintf(stderr, (s), (p)), fflush(stderr))
#endif

/* }================================================================== */


/*
** {============================================================
** Compatibility with deprecated conversions
** 兼容不推荐的转换
** =============================================================
*/
#if defined(LUA_COMPAT_APIINTCASTS)

#define luaL_checkunsigned(L,a)	((lua_Unsigned)luaL_checkinteger(L,a))
#define luaL_optunsigned(L,a,d)	\
	((lua_Unsigned)luaL_optinteger(L,a,(lua_Integer)(d)))

#define luaL_checkint(L,n)	((int)luaL_checkinteger(L, (n)))
#define luaL_optint(L,n,d)	((int)luaL_optinteger(L, (n), (d)))

#define luaL_checklong(L,n)	((long)luaL_checkinteger(L, (n)))
#define luaL_optlong(L,n,d)	((long)luaL_optinteger(L, (n), (d)))

#endif
/* }============================================================ */



#endif



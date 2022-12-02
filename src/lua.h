/*
** $Id: lua.h $
** Lua - A Scripting Language
** Lua - 一个脚本语言
** Lua.org, PUC-Rio, Brazil (http://www.lua.org)
** See Copyright Notice at the end of this file
*/


#ifndef lua_h
#define lua_h

#include <stdarg.h>
#include <stddef.h>


#include "luaconf.h"


#define LUA_VERSION_MAJOR	"5" // 主版本号
#define LUA_VERSION_MINOR	"4" // 次版本号
#define LUA_VERSION_RELEASE	"4" // 发行版本号

#define LUA_VERSION_NUM			504
#define LUA_VERSION_RELEASE_NUM		(LUA_VERSION_NUM * 100 + 4)

#define LUA_VERSION	"Lua " LUA_VERSION_MAJOR "." LUA_VERSION_MINOR // 版本信息
#define LUA_RELEASE	LUA_VERSION "." LUA_VERSION_RELEASE
#define LUA_COPYRIGHT	LUA_RELEASE "  Copyright (C) 1994-2022 Lua.org, PUC-Rio" // 版权信息
#define LUA_AUTHORS	"R. Ierusalimschy, L. H. de Figueiredo, W. Celes" // 作者信息


/* 
   mark for precompiled code ('<esc>Lua') 
   预编译代码标记
*/
#define LUA_SIGNATURE	"\x1bLua"

/* 
   option for multiple returns in 'lua_pcall' and 'lua_call' 
   'lua_pcall' 和 'lua_call' 中的多个返回选项
*/
#define LUA_MULTRET	(-1)


/*
** Pseudo-indices 伪索引
** (-LUAI_MAXSTACK is the minimum valid index; we keep some free empty
** space after that to help overflow detection)
** （-LUAI_MAXSTACK是最小有效索引；在此之后，我们保留一些空闲的空间以帮助检测溢出）
*/
#define LUA_REGISTRYINDEX	(-LUAI_MAXSTACK - 1000)
#define lua_upvalueindex(i)	(LUA_REGISTRYINDEX - (i))


/* thread status 线程状态 */
#define LUA_OK		0 // 正常
#define LUA_YIELD	1 // 让出
#define LUA_ERRRUN	2 // 运行错误
#define LUA_ERRSYNTAX	3 // 错误语法
#define LUA_ERRMEM	4 // 错误内存
#define LUA_ERRERR	5 // 错误


typedef struct lua_State lua_State;


/*
** basic types 基础类型
*/
#define LUA_TNONE		(-1) // 无

#define LUA_TNIL		0 // 空值
#define LUA_TBOOLEAN		1 // 布尔值
#define LUA_TLIGHTUSERDATA	2 // 轻量级用户数据
#define LUA_TNUMBER		3 // 数值
#define LUA_TSTRING		4 // 字符串
#define LUA_TTABLE		5 // 表
#define LUA_TFUNCTION		6 // 函数
#define LUA_TUSERDATA		7 // 用户数据
#define LUA_TTHREAD		8 // 线程

#define LUA_NUMTYPES		9 // 数值类型



/* 
   minimum Lua stack available to a C function 
   C函数可用的最小Lua堆栈
*/
#define LUA_MINSTACK	20 // 最小堆栈


/* 
   predefined values in the registry 
   注册表中的预定义值
*/
#define LUA_RIDX_MAINTHREAD	1
#define LUA_RIDX_GLOBALS	2
#define LUA_RIDX_LAST		LUA_RIDX_GLOBALS


/* 
   type of numbers in Lua 
   Lua数值类型
*/
typedef LUA_NUMBER lua_Number;


/* 
   type for integer functions 
   整数函数的类型
*/
typedef LUA_INTEGER lua_Integer;

/* 
   unsigned integer type 
   无符号整数的类型
*/
typedef LUA_UNSIGNED lua_Unsigned;

/* 
   type for continuation-function contexts 
   延续函数上下文的类型
*/
typedef LUA_KCONTEXT lua_KContext;


/*
** Type for C functions registered with Lua
** 向Lua注册的C函数的类型
*/
typedef int (*lua_CFunction) (lua_State *L);

/*
** Type for continuation functions
** 延续函数的类型
*/
typedef int (*lua_KFunction) (lua_State *L, int status, lua_KContext ctx);


/*
** Type for functions that read/write blocks when loading/dumping Lua chunks
** 加载/转储Lua块时，读取/写入块的函数的类型
*/
typedef const char * (*lua_Reader) (lua_State *L, void *ud, size_t *sz);

typedef int (*lua_Writer) (lua_State *L, const void *p, size_t sz, void *ud);


/*
** Type for memory-allocation functions
** 内存分配函数的类型
*/
typedef void * (*lua_Alloc) (void *ud, void *ptr, size_t osize, size_t nsize);


/*
** Type for warning functions
** 警告功能类型
*/
typedef void (*lua_WarnFunction) (void *ud, const char *msg, int tocont);




/*
** generic extra include file
** 通用额外 include 文件
*/
#if defined(LUA_USER_H)
#include LUA_USER_H
#endif


/*
** RCS ident string 标识字符串
*/
extern const char lua_ident[];


/*
** state manipulation 状态操作
*/
LUA_API lua_State *(lua_newstate) (lua_Alloc f, void *ud);
LUA_API void       (lua_close) (lua_State *L);
LUA_API lua_State *(lua_newthread) (lua_State *L);
LUA_API int        (lua_resetthread) (lua_State *L);

LUA_API lua_CFunction (lua_atpanic) (lua_State *L, lua_CFunction panicf);


LUA_API lua_Number (lua_version) (lua_State *L);


/*
** basic stack manipulation 基础堆栈操作
*/
LUA_API int   (lua_absindex) (lua_State *L, int idx);
LUA_API int   (lua_gettop) (lua_State *L);
LUA_API void  (lua_settop) (lua_State *L, int idx);
LUA_API void  (lua_pushvalue) (lua_State *L, int idx);
LUA_API void  (lua_rotate) (lua_State *L, int idx, int n);
LUA_API void  (lua_copy) (lua_State *L, int fromidx, int toidx);
LUA_API int   (lua_checkstack) (lua_State *L, int n);

LUA_API void  (lua_xmove) (lua_State *from, lua_State *to, int n);


/*
** access functions (stack -> C) 。访问函数(stack -> C)
*/

LUA_API int             (lua_isnumber) (lua_State *L, int idx);
LUA_API int             (lua_isstring) (lua_State *L, int idx);
LUA_API int             (lua_iscfunction) (lua_State *L, int idx);
LUA_API int             (lua_isinteger) (lua_State *L, int idx);
LUA_API int             (lua_isuserdata) (lua_State *L, int idx);
LUA_API int             (lua_type) (lua_State *L, int idx);
LUA_API const char     *(lua_typename) (lua_State *L, int tp);

LUA_API lua_Number      (lua_tonumberx) (lua_State *L, int idx, int *isnum);
LUA_API lua_Integer     (lua_tointegerx) (lua_State *L, int idx, int *isnum);
LUA_API int             (lua_toboolean) (lua_State *L, int idx);
LUA_API const char     *(lua_tolstring) (lua_State *L, int idx, size_t *len);
LUA_API lua_Unsigned    (lua_rawlen) (lua_State *L, int idx);
LUA_API lua_CFunction   (lua_tocfunction) (lua_State *L, int idx);
LUA_API void	       *(lua_touserdata) (lua_State *L, int idx);
LUA_API lua_State      *(lua_tothread) (lua_State *L, int idx);
LUA_API const void     *(lua_topointer) (lua_State *L, int idx);


/*
** Comparison and arithmetic functions
** 比较和算术函数
*/

#define LUA_OPADD	0	/* ORDER TM, ORDER OP */
#define LUA_OPSUB	1
#define LUA_OPMUL	2
#define LUA_OPMOD	3
#define LUA_OPPOW	4
#define LUA_OPDIV	5
#define LUA_OPIDIV	6
#define LUA_OPBAND	7
#define LUA_OPBOR	8
#define LUA_OPBXOR	9
#define LUA_OPSHL	10
#define LUA_OPSHR	11
#define LUA_OPUNM	12
#define LUA_OPBNOT	13

LUA_API void  (lua_arith) (lua_State *L, int op);

#define LUA_OPEQ	0
#define LUA_OPLT	1
#define LUA_OPLE	2

LUA_API int   (lua_rawequal) (lua_State *L, int idx1, int idx2);
LUA_API int   (lua_compare) (lua_State *L, int idx1, int idx2, int op);


/*
** push functions (C -> stack) 压入函数(C -> stack)
*/
LUA_API void        (lua_pushnil) (lua_State *L);
LUA_API void        (lua_pushnumber) (lua_State *L, lua_Number n);
LUA_API void        (lua_pushinteger) (lua_State *L, lua_Integer n);
LUA_API const char *(lua_pushlstring) (lua_State *L, const char *s, size_t len);
LUA_API const char *(lua_pushstring) (lua_State *L, const char *s);
LUA_API const char *(lua_pushvfstring) (lua_State *L, const char *fmt,
                                                      va_list argp);
LUA_API const char *(lua_pushfstring) (lua_State *L, const char *fmt, ...);
LUA_API void  (lua_pushcclosure) (lua_State *L, lua_CFunction fn, int n);
LUA_API void  (lua_pushboolean) (lua_State *L, int b);
LUA_API void  (lua_pushlightuserdata) (lua_State *L, void *p);
LUA_API int   (lua_pushthread) (lua_State *L);


/*
** get functions (Lua -> stack) 获取函数(Lua -> stack)
*/
LUA_API int (lua_getglobal) (lua_State *L, const char *name);
LUA_API int (lua_gettable) (lua_State *L, int idx);
LUA_API int (lua_getfield) (lua_State *L, int idx, const char *k);
LUA_API int (lua_geti) (lua_State *L, int idx, lua_Integer n);
LUA_API int (lua_rawget) (lua_State *L, int idx);
LUA_API int (lua_rawgeti) (lua_State *L, int idx, lua_Integer n);
LUA_API int (lua_rawgetp) (lua_State *L, int idx, const void *p);

LUA_API void  (lua_createtable) (lua_State *L, int narr, int nrec);
LUA_API void *(lua_newuserdatauv) (lua_State *L, size_t sz, int nuvalue);
LUA_API int   (lua_getmetatable) (lua_State *L, int objindex);
LUA_API int  (lua_getiuservalue) (lua_State *L, int idx, int n);


/*
** set functions (stack -> Lua) 设置函数(stack -> Lua)
*/
LUA_API void  (lua_setglobal) (lua_State *L, const char *name);
LUA_API void  (lua_settable) (lua_State *L, int idx);
LUA_API void  (lua_setfield) (lua_State *L, int idx, const char *k);
LUA_API void  (lua_seti) (lua_State *L, int idx, lua_Integer n);
LUA_API void  (lua_rawset) (lua_State *L, int idx);
LUA_API void  (lua_rawseti) (lua_State *L, int idx, lua_Integer n);
LUA_API void  (lua_rawsetp) (lua_State *L, int idx, const void *p);
LUA_API int   (lua_setmetatable) (lua_State *L, int objindex);
LUA_API int   (lua_setiuservalue) (lua_State *L, int idx, int n);


/*
** 'load' and 'call' functions (load and run Lua code)
** 'load' 和 'call' 函数(加载和运行 Lua 代码)
*/
LUA_API void  (lua_callk) (lua_State *L, int nargs, int nresults,
                           lua_KContext ctx, lua_KFunction k);
#define lua_call(L,n,r)		lua_callk(L, (n), (r), 0, NULL)

LUA_API int   (lua_pcallk) (lua_State *L, int nargs, int nresults, int errfunc,
                            lua_KContext ctx, lua_KFunction k);
#define lua_pcall(L,n,r,f)	lua_pcallk(L, (n), (r), (f), 0, NULL)

LUA_API int   (lua_load) (lua_State *L, lua_Reader reader, void *dt,
                          const char *chunkname, const char *mode);

LUA_API int (lua_dump) (lua_State *L, lua_Writer writer, void *data, int strip);


/*
** coroutine functions 协程函数
*/
LUA_API int  (lua_yieldk)     (lua_State *L, int nresults, lua_KContext ctx,
                               lua_KFunction k);
LUA_API int  (lua_resume)     (lua_State *L, lua_State *from, int narg,
                               int *nres);
LUA_API int  (lua_status)     (lua_State *L);
LUA_API int (lua_isyieldable) (lua_State *L);

#define lua_yield(L,n)		lua_yieldk(L, (n), 0, NULL)


/*
** Warning-related functions 警告相关函数
*/
LUA_API void (lua_setwarnf) (lua_State *L, lua_WarnFunction f, void *ud);
LUA_API void (lua_warning)  (lua_State *L, const char *msg, int tocont);


/*
** garbage-collection function and options
** 垃圾回收函数和选项
*/

#define LUA_GCSTOP		0 // 停止
#define LUA_GCRESTART		1 // 重启
#define LUA_GCCOLLECT		2 // 收集
#define LUA_GCCOUNT		3 // 计数
#define LUA_GCCOUNTB		4 // 计数B
#define LUA_GCSTEP		5 // 步长
#define LUA_GCSETPAUSE		6 // 设置暂停
#define LUA_GCSETSTEPMUL	7 // 设置步长
#define LUA_GCISRUNNING		9 // 正在运行
#define LUA_GCGEN		10 // 生成
#define LUA_GCINC		11 // 加一

LUA_API int (lua_gc) (lua_State *L, int what, ...);


/*
** miscellaneous functions 杂项函数
*/

LUA_API int   (lua_error) (lua_State *L);

LUA_API int   (lua_next) (lua_State *L, int idx);

LUA_API void  (lua_concat) (lua_State *L, int n);
LUA_API void  (lua_len)    (lua_State *L, int idx);

LUA_API size_t   (lua_stringtonumber) (lua_State *L, const char *s);

LUA_API lua_Alloc (lua_getallocf) (lua_State *L, void **ud);
LUA_API void      (lua_setallocf) (lua_State *L, lua_Alloc f, void *ud);

LUA_API void (lua_toclose) (lua_State *L, int idx);
LUA_API void (lua_closeslot) (lua_State *L, int idx);


/*
** {==============================================================
** some useful macros 一些有用的宏
** ===============================================================
*/

#define lua_getextraspace(L)	((void *)((char *)(L) - LUA_EXTRASPACE))

#define lua_tonumber(L,i)	lua_tonumberx(L,(i),NULL)
#define lua_tointeger(L,i)	lua_tointegerx(L,(i),NULL)

#define lua_pop(L,n)		lua_settop(L, -(n)-1)

#define lua_newtable(L)		lua_createtable(L, 0, 0)

#define lua_register(L,n,f) (lua_pushcfunction(L, (f)), lua_setglobal(L, (n)))

#define lua_pushcfunction(L,f)	lua_pushcclosure(L, (f), 0)

#define lua_isfunction(L,n)	(lua_type(L, (n)) == LUA_TFUNCTION)
#define lua_istable(L,n)	(lua_type(L, (n)) == LUA_TTABLE)
#define lua_islightuserdata(L,n)	(lua_type(L, (n)) == LUA_TLIGHTUSERDATA)
#define lua_isnil(L,n)		(lua_type(L, (n)) == LUA_TNIL)
#define lua_isboolean(L,n)	(lua_type(L, (n)) == LUA_TBOOLEAN)
#define lua_isthread(L,n)	(lua_type(L, (n)) == LUA_TTHREAD)
#define lua_isnone(L,n)		(lua_type(L, (n)) == LUA_TNONE)
#define lua_isnoneornil(L, n)	(lua_type(L, (n)) <= 0)

#define lua_pushliteral(L, s)	lua_pushstring(L, "" s)

#define lua_pushglobaltable(L)  \
	((void)lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS))

#define lua_tostring(L,i)	lua_tolstring(L, (i), NULL)


#define lua_insert(L,idx)	lua_rotate(L, (idx), 1)

#define lua_remove(L,idx)	(lua_rotate(L, (idx), -1), lua_pop(L, 1))

#define lua_replace(L,idx)	(lua_copy(L, -1, (idx)), lua_pop(L, 1))

/* }============================================================== */


/*
** {==============================================================
** compatibility macros 兼容性宏
** ===============================================================
*/
#if defined(LUA_COMPAT_APIINTCASTS)

#define lua_pushunsigned(L,n)	lua_pushinteger(L, (lua_Integer)(n))
#define lua_tounsignedx(L,i,is)	((lua_Unsigned)lua_tointegerx(L,i,is))
#define lua_tounsigned(L,i)	lua_tounsignedx(L,(i),NULL)

#endif

#define lua_newuserdata(L,s)	lua_newuserdatauv(L,s,1)
#define lua_getuservalue(L,idx)	lua_getiuservalue(L,idx,1)
#define lua_setuservalue(L,idx)	lua_setiuservalue(L,idx,1)

#define LUA_NUMTAGS		LUA_NUMTYPES

/* }============================================================== */

/*
** {======================================================================
** Debug API 调试API
** =======================================================================
*/


/*
** Event codes 事件代码
*/
#define LUA_HOOKCALL	0 // 钩子调用
#define LUA_HOOKRET	1
#define LUA_HOOKLINE	2 // 钩子行
#define LUA_HOOKCOUNT	3 // 钩子总数
#define LUA_HOOKTAILCALL 4 // 钩子尾调


/*
** Event masks 事件掩码
*/
#define LUA_MASKCALL	(1 << LUA_HOOKCALL)
#define LUA_MASKRET	(1 << LUA_HOOKRET)
#define LUA_MASKLINE	(1 << LUA_HOOKLINE)
#define LUA_MASKCOUNT	(1 << LUA_HOOKCOUNT)

typedef struct lua_Debug lua_Debug;  /* activation record 活动的记录 */


/* 
   Functions to be called by the debugger in specific events 
   调试器在特定事件中调用的函数
*/
typedef void (*lua_Hook) (lua_State *L, lua_Debug *ar);


LUA_API int (lua_getstack) (lua_State *L, int level, lua_Debug *ar);
LUA_API int (lua_getinfo) (lua_State *L, const char *what, lua_Debug *ar);
LUA_API const char *(lua_getlocal) (lua_State *L, const lua_Debug *ar, int n);
LUA_API const char *(lua_setlocal) (lua_State *L, const lua_Debug *ar, int n);
LUA_API const char *(lua_getupvalue) (lua_State *L, int funcindex, int n);
LUA_API const char *(lua_setupvalue) (lua_State *L, int funcindex, int n);

LUA_API void *(lua_upvalueid) (lua_State *L, int fidx, int n);
LUA_API void  (lua_upvaluejoin) (lua_State *L, int fidx1, int n1,
                                               int fidx2, int n2);

LUA_API void (lua_sethook) (lua_State *L, lua_Hook func, int mask, int count);
LUA_API lua_Hook (lua_gethook) (lua_State *L);
LUA_API int (lua_gethookmask) (lua_State *L);
LUA_API int (lua_gethookcount) (lua_State *L);

LUA_API int (lua_setcstacklimit) (lua_State *L, unsigned int limit);

struct lua_Debug {
  int event;
  const char *name;	/* (n) */
  const char *namewhat;	/* (n) 'global', 'local', 'field', 'method' */
  const char *what;	/* (S) 'Lua', 'C', 'main', 'tail' */
  const char *source;	/* (S) */
  size_t srclen;	/* (S) */
  int currentline;	/* (l) */
  int linedefined;	/* (S) */
  int lastlinedefined;	/* (S) */
  unsigned char nups;	/* (u) number of upvalues */
  unsigned char nparams;/* (u) number of parameters */
  char isvararg;        /* (u) */
  char istailcall;	/* (t) */
  unsigned short ftransfer;   /* (r) index of first value transferred */
  unsigned short ntransfer;   /* (r) number of transferred values */
  char short_src[LUA_IDSIZE]; /* (S) */
  /* private part */
  struct CallInfo *i_ci;  /* active function */
};

/* }====================================================================== */


/******************************************************************************
* Copyright (C) 1994-2022 Lua.org, PUC-Rio.
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************/


#endif

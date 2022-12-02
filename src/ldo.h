/*
** $Id: ldo.h $
** Stack and Call structure of Lua
** Lua的堆栈和调用结构
** See Copyright Notice in lua.h
*/

#ifndef ldo_h
#define ldo_h


#include "lobject.h"
#include "lstate.h"
#include "lzio.h"


/*
** Macro to check stack size and grow stack if needed.  Parameters
** 'pre'/'pos' allow the macro to preserve a pointer into the
** stack across reallocations, doing the work only when needed.
** It also allows the running of one GC step when the stack is
** reallocated.
** 'condmovestack' is used in heavy tests to force a stack reallocation
** at every check.
** 宏检查堆栈大小，并根据需要增长堆栈。参数'pre'/'pos'允许宏在重新分配时保留堆栈中的指针，
** 仅在需要时执行此操作。它还允许在重新分配堆栈时运行一个GC步骤在繁重的测试中使用'condmovestack'
** 来强制每次检查时重新分配堆栈。
*/
#define luaD_checkstackaux(L,n,pre,pos)  \
	if (l_unlikely(L->stack_last - L->top <= (n))) \
	  { pre; luaD_growstack(L, n, 1); pos; } \
        else { condmovestack(L,pre,pos); }

/* 
   In general, 'pre'/'pos' are empty (nothing to save) 
   通常，'pre'/'pos'为空（无需保存）
*/
#define luaD_checkstack(L,n)	luaD_checkstackaux(L,n,(void)0,(void)0)



#define savestack(L,p)		((char *)(p) - (char *)L->stack)
#define restorestack(L,n)	((StkId)((char *)L->stack + (n)))


/* 
   macro to check stack size, preserving 'p' 
   宏检查堆栈大小，保留'p'
*/
#define checkstackGCp(L,n,p)  \
  luaD_checkstackaux(L, n, \
    ptrdiff_t t__ = savestack(L, p);  /* save 'p' 保存'p' */ \
    luaC_checkGC(L),  /* stack grow uses memory 堆栈增长使用内存 */ \
    p = restorestack(L, t__))  /* 'pos' part: restore 'p' 。'pos'部分：还原'p' */


/* 
   macro to check stack size and GC 
   宏以检查堆栈大小和GC
*/
#define checkstackGC(L,fsize)  \
	luaD_checkstackaux(L, (fsize), luaC_checkGC(L), (void)0)


/* 
   type of protected functions, to be ran by 'runprotected' 
   要由 'runprotected' 运行的受保护函数的类型
*/
typedef void (*Pfunc) (lua_State *L, void *ud);

LUAI_FUNC void luaD_seterrorobj (lua_State *L, int errcode, StkId oldtop);
LUAI_FUNC int luaD_protectedparser (lua_State *L, ZIO *z, const char *name,
                                                  const char *mode);
LUAI_FUNC void luaD_hook (lua_State *L, int event, int line,
                                        int fTransfer, int nTransfer);
LUAI_FUNC void luaD_hookcall (lua_State *L, CallInfo *ci);
LUAI_FUNC int luaD_pretailcall (lua_State *L, CallInfo *ci, StkId func,                                                    int narg1, int delta);
LUAI_FUNC CallInfo *luaD_precall (lua_State *L, StkId func, int nResults);
LUAI_FUNC void luaD_call (lua_State *L, StkId func, int nResults);
LUAI_FUNC void luaD_callnoyield (lua_State *L, StkId func, int nResults);
LUAI_FUNC StkId luaD_tryfuncTM (lua_State *L, StkId func);
LUAI_FUNC int luaD_closeprotected (lua_State *L, ptrdiff_t level, int status);
LUAI_FUNC int luaD_pcall (lua_State *L, Pfunc func, void *u,
                                        ptrdiff_t oldtop, ptrdiff_t ef);
LUAI_FUNC void luaD_poscall (lua_State *L, CallInfo *ci, int nres);
LUAI_FUNC int luaD_reallocstack (lua_State *L, int newsize, int raiseerror);
LUAI_FUNC int luaD_growstack (lua_State *L, int n, int raiseerror);
LUAI_FUNC void luaD_shrinkstack (lua_State *L);
LUAI_FUNC void luaD_inctop (lua_State *L);

LUAI_FUNC l_noret luaD_throw (lua_State *L, int errcode);
LUAI_FUNC int luaD_rawrunprotected (lua_State *L, Pfunc f, void *ud);

#endif


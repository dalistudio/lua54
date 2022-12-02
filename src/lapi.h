/*
** $Id: lapi.h $
** Auxiliary functions from Lua API
** Lua API的辅助函数
** See Copyright Notice in lua.h
*/

#ifndef lapi_h
#define lapi_h


#include "llimits.h"
#include "lstate.h"


/* 
    Increments 'L->top', checking for stack overflows 
	递增 'L->top' ，检查堆栈溢出
*/
#define api_incr_top(L)   {L->top++; api_check(L, L->top <= L->ci->top, \
				"stack overflow");}


/*
** If a call returns too many multiple returns, the callee may not have
** stack space to accommodate all results. In this case, this macro
** increases its stack space ('L->ci->top').
** 如果调用返回太多的多次返回，则被调用方可能没有堆栈空间来容纳所有结果。在这种情况下，该宏会增加其堆栈空间('L->ci->top')
*/
#define adjustresults(L,nres) \
    { if ((nres) <= LUA_MULTRET && L->ci->top < L->top) L->ci->top = L->top; }


/* 
    Ensure the stack has at least 'n' elements 
    确保堆栈至少有 ‘n’ 个元素
*/
#define api_checknelems(L,n)	api_check(L, (n) < (L->top - L->ci->func), \
				  "not enough elements in the stack")


/*
** To reduce the overhead of returning from C functions, the presence of
** to-be-closed variables in these functions is coded in the CallInfo's
** field 'nresults', in a way that functions with no to-be-closed variables
** with zero, one, or "all" wanted results have no overhead. Functions
** with other number of wanted results, as well as functions with
** variables to be closed, have an extra check.
** 为了减少从C函数返回的开销，在CallInfo的字段 ‘nresults’ 中对这些函数中存在的待闭变量进行编码，
** 从而使没有待闭变量的函数没有开销，其结果为零、一或“所有”。具有其他数量的所需结果的函数，以及具有
** 要关闭的变量的函数，都需要额外检查。
*/

#define hastocloseCfunc(n)	((n) < LUA_MULTRET)

/* Map [-1, inf) (range of 'nresults') into (-inf, -2] */
#define codeNresults(n)		(-(n) - 3)
#define decodeNresults(n)	(-(n) - 3)

#endif

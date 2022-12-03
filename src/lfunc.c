/*
** $Id: lfunc.c $
** Auxiliary functions to manipulate prototypes and closures
** 操作原型和闭包的辅助函数
** See Copyright Notice in lua.h
*/

#define lfunc_c
#define LUA_CORE

#include "lprefix.h"


#include <stddef.h>

#include "lua.h"

#include "ldebug.h"
#include "ldo.h"
#include "lfunc.h"
#include "lgc.h"
#include "lmem.h"
#include "lobject.h"
#include "lstate.h"


// 新建C的闭包函数
CClosure *luaF_newCclosure (lua_State *L, int nupvals) {
  GCObject *o = luaC_newobj(L, LUA_VCCL, sizeCclosure(nupvals));
  CClosure *c = gco2ccl(o);
  c->nupvalues = cast_byte(nupvals);
  return c;
}

// 新建Lua的闭包函数
LClosure *luaF_newLclosure (lua_State *L, int nupvals) {
  GCObject *o = luaC_newobj(L, LUA_VLCL, sizeLclosure(nupvals));
  LClosure *c = gco2lcl(o);
  c->p = NULL;
  c->nupvalues = cast_byte(nupvals);
  while (nupvals--) c->upvals[nupvals] = NULL;
  return c;
}


/*
** fill a closure with new closed upvalues
** 用新的闭合上值填充闭包
*/
void luaF_initupvals (lua_State *L, LClosure *cl) {
  int i;
  for (i = 0; i < cl->nupvalues; i++) {
    GCObject *o = luaC_newobj(L, LUA_VUPVAL, sizeof(UpVal));
    UpVal *uv = gco2upv(o);
    uv->v = &uv->u.value;  /* make it closed 使其关闭 */
    setnilvalue(uv->v);
    cl->upvals[i] = uv;
    luaC_objbarrier(L, cl, uv);
  }
}


/*
** Create a new upvalue at the given level, and link it to the list of
** open upvalues of 'L' after entry 'prev'.
** 在给定级别创建一个新的上值，并将其链接到条目'prev'之后的'L' 的打开上值列表
**/
static UpVal *newupval (lua_State *L, int tbc, StkId level, UpVal **prev) {
  GCObject *o = luaC_newobj(L, LUA_VUPVAL, sizeof(UpVal));
  UpVal *uv = gco2upv(o);
  UpVal *next = *prev;
  uv->v = s2v(level);  /* current value lives in the stack 当前值存在于堆栈中 */
  uv->tbc = tbc;
  uv->u.open.next = next;  /* link it to list of open upvalues 将其链接到上值列表 */
  uv->u.open.previous = prev;
  if (next)
    next->u.open.previous = &uv->u.open.next;
  *prev = uv;
  if (!isintwups(L)) {  /* thread not in list of threads with upvalues? 线程不在具有上值的线程列表中？*/
    L->twups = G(L)->twups;  /* link it to the list 将其链接到列表 */
    G(L)->twups = L;
  }
  return uv;
}


/*
** Find and reuse, or create if it does not exist, an upvalue
** at the given level.
** 查找并重用，如果不存在，则在给定级别创建一个上值
*/
UpVal *luaF_findupval (lua_State *L, StkId level) {
  UpVal **pp = &L->openupval;
  UpVal *p;
  lua_assert(isintwups(L) || L->openupval == NULL);
  while ((p = *pp) != NULL && uplevel(p) >= level) {  /* search for it */
    lua_assert(!isdead(G(L), p));
    if (uplevel(p) == level)  /* corresponding upvalue? 对应的上值*/
      return p;  /* return it 返回它 */
    pp = &p->u.open.next;
  }
  /* not found: create a new upvalue after 'pp' 找不到：在'pp'之后创建新的上值 */
  return newupval(L, 0, level, pp);
}


/*
** Call closing method for object 'obj' with error message 'err'. The
** boolean 'yy' controls whether the call is yieldable.
** (This function assumes EXTRA_STACK.)
** 调用对象'obj'的关闭方法，错误消息为'err'。布尔值'yy'控制调用是否可接受。（此函数假设EXTRA_STACK）
*/
static void callclosemethod (lua_State *L, TValue *obj, TValue *err, int yy) {
  StkId top = L->top;
  const TValue *tm = luaT_gettmbyobj(L, obj, TM_CLOSE);
  setobj2s(L, top, tm);  /* will call metamethod... 将调用元方法 */
  setobj2s(L, top + 1, obj);  /* with 'self' as the 1st argument。'self'作为第一个参数 */
  setobj2s(L, top + 2, err);  /* and error msg. as 2nd argument。和错误消息。作为第二个参数 */
  L->top = top + 3;  /* add function and arguments 添加函数和参数 */
  if (yy)
    luaD_call(L, top, 0);
  else
    luaD_callnoyield(L, top, 0);
}


/*
** Check whether object at given level has a close metamethod and raise
** an error if not.
** 检查给定级别的对象是否具有闭合元方法，如果没有，则引发错误。
*/
static void checkclosemth (lua_State *L, StkId level) {
  const TValue *tm = luaT_gettmbyobj(L, s2v(level), TM_CLOSE);
  if (ttisnil(tm)) {  /* no metamethod? 没有元方法 */
    int idx = cast_int(level - L->ci->func);  /* variable index 变量索引 */
    const char *vname = luaG_findlocal(L, L->ci, idx, NULL);
    if (vname == NULL) vname = "?";
    luaG_runerror(L, "variable '%s' got a non-closable value", vname);
  }
}


/*
** Prepare and call a closing method.
** If status is CLOSEKTOP, the call to the closing method will be pushed
** at the top of the stack. Otherwise, values can be pushed right after
** the 'level' of the upvalue being closed, as everything after that
** won't be used again.
** 准备并调用结束方法。如果状态为CLOSEKTOP，将在堆栈顶部推送对关闭方法的调用。否则，
** 可以在关闭上值的'level' 之后立即推送值，因为之后的所有内容都将不再使用。
*/
static void prepcallclosemth (lua_State *L, StkId level, int status, int yy) {
  TValue *uv = s2v(level);  /* value being closed 正在关闭的值 */
  TValue *errobj;
  if (status == CLOSEKTOP)
    errobj = &G(L)->nilvalue;  /* error object is nil 错误对象为空 */
  else {  /* 'luaD_seterrorobj' will set top to level + 2 */
    errobj = s2v(level + 1);  /* error object goes after 'uv' 错误对象位于'uv'之后 */
    luaD_seterrorobj(L, status, level + 1);  /* set error object 设置错误对象 */
  }
  callclosemethod(L, uv, errobj, yy);
}


/*
** Maximum value for deltas in 'tbclist', dependent on the type
** of delta. (This macro assumes that an 'L' is in scope where it
** is used.)
** 'tbclist'中详情的最大值，取决于详情的类型。（此宏假定'L'在使用它的范围内）
*/
#define MAXDELTA  \
	((256ul << ((sizeof(L->stack->tbclist.delta) - 1) * 8)) - 1)


/*
** Insert a variable in the list of to-be-closed variables.
** 在要关闭的变量列表中插入一个变量
*/
void luaF_newtbcupval (lua_State *L, StkId level) {
  lua_assert(level > L->tbclist);
  if (l_isfalse(s2v(level)))
    return;  /* false doesn't need to be closed 不需要关闭 */
  checkclosemth(L, level);  /* value must have a close method 值必须具有close方法 */
  while (cast_uint(level - L->tbclist) > MAXDELTA) {
    L->tbclist += MAXDELTA;  /* create a dummy node at maximum delta 以最大增量创建虚拟节点 */
    L->tbclist->tbclist.delta = 0;
  }
  level->tbclist.delta = cast(unsigned short, level - L->tbclist);
  L->tbclist = level;
}


void luaF_unlinkupval (UpVal *uv) {
  lua_assert(upisopen(uv));
  *uv->u.open.previous = uv->u.open.next;
  if (uv->u.open.next)
    uv->u.open.next->u.open.previous = uv->u.open.previous;
}


/*
** Close all upvalues up to the given stack level.
** 关闭给定堆栈级别的所有上值
*/
void luaF_closeupval (lua_State *L, StkId level) {
  UpVal *uv;
  StkId upl;  /* stack index pointed by 'uv' 堆栈索引由'uv'指向 */
  while ((uv = L->openupval) != NULL && (upl = uplevel(uv)) >= level) {
    TValue *slot = &uv->u.value;  /* new position for value 值的新指针 */
    lua_assert(uplevel(uv) < L->top);
    luaF_unlinkupval(uv);  /* remove upvalue from 'openupval' list 从'openupval'列表中删除上值 */
    setobj(L, slot, uv->v);  /* move value to upvalue slot 将值移动到上值槽中 */
    uv->v = slot;  /* now current value lives here 现在，当前的价值就在这里 */
    if (!iswhite(uv)) {  /* neither white nor dead? 即不是白色也不是死的？ */
      nw2black(uv);  /* closed upvalues cannot be gray 闭合值不能为灰色 */
      luaC_barrier(L, uv, slot);
    }
  }
}


/*
** Remove firt element from the tbclist plus its dummy nodes.
** 从 tbclist 及其虚拟基点中删除第一个元素
*/
static void poptbclist (lua_State *L) {
  StkId tbc = L->tbclist;
  lua_assert(tbc->tbclist.delta > 0);  /* first element cannot be dummy 第一个元素不能为空 */
  tbc -= tbc->tbclist.delta;
  while (tbc > L->stack && tbc->tbclist.delta == 0)
    tbc -= MAXDELTA;  /* remove dummy nodes 删除虚拟节点 */
  L->tbclist = tbc;
}


/*
** Close all upvalues and to-be-closed variables up to the given stack
** level.
** 关闭给定堆栈级别的所有上值和要关闭的变量
*/
void luaF_close (lua_State *L, StkId level, int status, int yy) {
  ptrdiff_t levelrel = savestack(L, level);
  luaF_closeupval(L, level);  /* first, close the upvalues 首先，关闭上值 */
  while (L->tbclist >= level) {  /* traverse tbc's down to that level */
    StkId tbc = L->tbclist;  /* get variable index 获取变量索引 */
    poptbclist(L);  /* remove it from list 将其从列表中删除 */
    prepcallclosemth(L, tbc, status, yy);  /* close variable 闭合变量 */
    level = restorestack(L, levelrel);
  }
}

// 新建原型
Proto *luaF_newproto (lua_State *L) {
  GCObject *o = luaC_newobj(L, LUA_VPROTO, sizeof(Proto));
  Proto *f = gco2p(o);
  f->k = NULL;
  f->sizek = 0;
  f->p = NULL;
  f->sizep = 0;
  f->code = NULL; // 代码
  f->sizecode = 0; // 代码大小
  f->lineinfo = NULL; // 行号表
  f->sizelineinfo = 0; // 行号大小
  f->abslineinfo = NULL; // 
  f->sizeabslineinfo = 0;
  f->upvalues = NULL;
  f->sizeupvalues = 0;
  f->numparams = 0;
  f->is_vararg = 0;
  f->maxstacksize = 0;
  f->locvars = NULL;
  f->sizelocvars = 0;
  f->linedefined = 0;
  f->lastlinedefined = 0;
  f->source = NULL;
  return f;
}

// 释放原型
void luaF_freeproto (lua_State *L, Proto *f) {
  luaM_freearray(L, f->code, f->sizecode);
  luaM_freearray(L, f->p, f->sizep);
  luaM_freearray(L, f->k, f->sizek);
  luaM_freearray(L, f->lineinfo, f->sizelineinfo);
  luaM_freearray(L, f->abslineinfo, f->sizeabslineinfo);
  luaM_freearray(L, f->locvars, f->sizelocvars);
  luaM_freearray(L, f->upvalues, f->sizeupvalues);
  luaM_free(L, f);
}


/*
** Look for n-th local variable at line 'line' in function 'func'.
** Returns NULL if not found.
** 在函数'func'的第'line'查找第n个局部变量。如果找不到，则返回NULL。
*/
const char *luaF_getlocalname (const Proto *f, int local_number, int pc) {
  int i;
  for (i = 0; i<f->sizelocvars && f->locvars[i].startpc <= pc; i++) {
    if (pc < f->locvars[i].endpc) {  /* is variable active? 变量是否处于活动状态？*/
      local_number--;
      if (local_number == 0)
        return getstr(f->locvars[i].varname);
    }
  }
  return NULL;  /* not found 没找到 */
}


/*
** $Id: lcorolib.c $
** Coroutine Library
** 协程库
** See Copyright Notice in lua.h
*/

#define lcorolib_c
#define LUA_LIB

#include "../src/lprefix.h"


#include <stdlib.h>

#include "../src/lua.h"

#include "../src/lauxlib.h"
#include "../src/lualib.h"


static lua_State *getco (lua_State *L) {
  lua_State *co = lua_tothread(L, 1);
  luaL_argexpected(L, co, 1, "thread");
  return co;
}


/*
** Resumes a coroutine. Returns the number of results for non-error
** cases or -1 for errors.
** 继续合作。返回非错误案例的结果数，或返回错误的-1
*/
static int auxresume (lua_State *L, lua_State *co, int narg) {
  int status, nres;
  if (l_unlikely(!lua_checkstack(co, narg))) {
    lua_pushliteral(L, "too many arguments to resume");
    return -1;  /* error flag 错误标志 */
  }
  lua_xmove(L, co, narg);
  status = lua_resume(co, L, narg, &nres);
  if (l_likely(status == LUA_OK || status == LUA_YIELD)) {
    if (l_unlikely(!lua_checkstack(L, nres + 1))) {
      lua_pop(co, nres);  /* remove results anyway 删除结果 */
      lua_pushliteral(L, "too many results to resume");
      return -1;  /* error flag 错误标志 */
    }
    lua_xmove(co, L, nres);  /* move yielded values 移动让出值 */
    return nres;
  }
  else {
    lua_xmove(co, L, 1);  /* move error message 移动错误消息 */
    return -1;  /* error flag 错误标志 */
  }
}


static int luaB_coresume (lua_State *L) {
  lua_State *co = getco(L);
  int r;
  r = auxresume(L, co, lua_gettop(L) - 1);
  if (l_unlikely(r < 0)) {
    lua_pushboolean(L, 0);
    lua_insert(L, -2);
    return 2;  /* return false + error message 返回false + 错误消息 */
  }
  else {
    lua_pushboolean(L, 1);
    lua_insert(L, -(r + 1));
    return r + 1;  /* return true + 'resume' returns 返回true + 'resume' 返回 */
  }
}


static int luaB_auxwrap (lua_State *L) {
  lua_State *co = lua_tothread(L, lua_upvalueindex(1));
  int r = auxresume(L, co, lua_gettop(L));
  if (l_unlikely(r < 0)) {  /* error? 错误？ */
    int stat = lua_status(co);
    if (stat != LUA_OK && stat != LUA_YIELD) {  /* error in the coroutine? 协程中的错误？ */
      stat = lua_resetthread(co);  /* close its tbc variables 关闭其 tbc 变量 */
      lua_assert(stat != LUA_OK);
      lua_xmove(co, L, 1);  /* move error message to the caller 将错误消息移动到调用 */
    }
    if (stat != LUA_ERRMEM &&  /* not a memory error and ... 不是内存错误，并且... */
        lua_type(L, -1) == LUA_TSTRING) {  /* ... error object is a string? 错误对象是字符串？*/
      luaL_where(L, 1);  /* add extra info, if available 添加额外信息（如果可用）*/
      lua_insert(L, -2);
      lua_concat(L, 2);
    }
    return lua_error(L);  /* propagate error 传播错误 */
  }
  return r;
}


static int luaB_cocreate (lua_State *L) {
  lua_State *NL;
  luaL_checktype(L, 1, LUA_TFUNCTION);
  NL = lua_newthread(L);
  lua_pushvalue(L, 1);  /* move function to top 将函数移动到顶部 */
  lua_xmove(L, NL, 1);  /* move function from L to NL 将函数从L移动到NL */
  return 1;
}


static int luaB_cowrap (lua_State *L) {
  luaB_cocreate(L);
  lua_pushcclosure(L, luaB_auxwrap, 1);
  return 1;
}


static int luaB_yield (lua_State *L) {
  return lua_yield(L, lua_gettop(L));
}


#define COS_RUN		0
#define COS_DEAD	1
#define COS_YIELD	2
#define COS_NORM	3


static const char *const statname[] =
  {"running", "dead", "suspended", "normal"};


static int auxstatus (lua_State *L, lua_State *co) {
  if (L == co) return COS_RUN;
  else {
    switch (lua_status(co)) {
      case LUA_YIELD:
        return COS_YIELD;
      case LUA_OK: {
        lua_Debug ar;
        if (lua_getstack(co, 0, &ar))  /* does it have frames? 它有框架吗？*/
          return COS_NORM;  /* it is running 它正在运行 */
        else if (lua_gettop(co) == 0)
            return COS_DEAD;
        else
          return COS_YIELD;  /* initial state 初始化状态 */
      }
      default:  /* some error occurred 发生了一些错误 */
        return COS_DEAD;
    }
  }
}


static int luaB_costatus (lua_State *L) {
  lua_State *co = getco(L);
  lua_pushstring(L, statname[auxstatus(L, co)]);
  return 1;
}


static int luaB_yieldable (lua_State *L) {
  lua_State *co = lua_isnone(L, 1) ? L : getco(L);
  lua_pushboolean(L, lua_isyieldable(co));
  return 1;
}


static int luaB_corunning (lua_State *L) {
  int ismain = lua_pushthread(L);
  lua_pushboolean(L, ismain);
  return 2;
}


static int luaB_close (lua_State *L) {
  lua_State *co = getco(L);
  int status = auxstatus(L, co);
  switch (status) {
    case COS_DEAD: case COS_YIELD: {
      status = lua_resetthread(co);
      if (status == LUA_OK) {
        lua_pushboolean(L, 1);
        return 1;
      }
      else {
        lua_pushboolean(L, 0);
        lua_xmove(co, L, 1);  /* move error message 移除错误消息 */
        return 2;
      }
    }
    default:  /* normal or running coroutine 正常或运行的协程 */
      return luaL_error(L, "cannot close a %s coroutine", statname[status]);
  }
}


static const luaL_Reg co_funcs[] = {
  {"create", luaB_cocreate}, // 创建
  {"resume", luaB_coresume}, // 开始或恢复
  {"running", luaB_corunning}, // 运行
  {"status", luaB_costatus}, // 状态
  {"wrap", luaB_cowrap}, // 创建
  {"yield", luaB_yield}, // 让出
  {"isyieldable", luaB_yieldable}, // 是否让出
  {"close", luaB_close}, // 关闭
  {NULL, NULL}
};



LUAMOD_API int luaopen_coroutine (lua_State *L) {
  luaL_newlib(L, co_funcs);
  return 1;
}


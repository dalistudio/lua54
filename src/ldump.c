/*
** $Id: ldump.c $
** save precompiled Lua chunks
** 保存预编译的Lua块
** See Copyright Notice in lua.h
*/

#define ldump_c
#define LUA_CORE

#include "lprefix.h"


#include <stddef.h>

#include "lua.h"

#include "lobject.h"
#include "lstate.h"
#include "lundump.h"


typedef struct {
  lua_State *L; // 状态机
  lua_Writer writer; // 写入者
  void *data; // 数据
  int strip; // 删除调试信息
  int status; // 状态
} DumpState;


/*
** All high-level dumps go through dumpVector; you can change it to
** change the endianness of the result
** 所有高级转储都经过dumpVector；您可以更改它以更改结果的结尾。
*/
#define dumpVector(D,v,n)	dumpBlock(D,v,(n)*sizeof((v)[0]))

#define dumpLiteral(D, s)	dumpBlock(D,s,sizeof(s) - sizeof(char))

// 转储块
static void dumpBlock (DumpState *D, const void *b, size_t size) {
  if (D->status == 0 && size > 0) {
    lua_unlock(D->L); // 解锁
    D->status = (*D->writer)(D->L, b, size, D->data);
    lua_lock(D->L); // 上锁
  }
}


#define dumpVar(D,x)		dumpVector(D,&x,1)

// 转储字节
static void dumpByte (DumpState *D, int y) {
  lu_byte x = (lu_byte)y;
  dumpVar(D, x);
}


/* dumpInt Buff Size */
#define DIBS    ((sizeof(size_t) * 8 / 7) + 1)

// 转储大小
static void dumpSize (DumpState *D, size_t x) {
  lu_byte buff[DIBS];
  int n = 0;
  do {
    buff[DIBS - (++n)] = x & 0x7f;  /* fill buffer in reverse order 按相反顺序填充缓冲区 */
    x >>= 7;
  } while (x != 0);
  buff[DIBS - 1] |= 0x80;  /* mark last byte 标记最后一个字节 */
  dumpVector(D, buff + DIBS - n, n);
}

// 转储整数
static void dumpInt (DumpState *D, int x) {
  dumpSize(D, x);
}

// 转储数值
static void dumpNumber (DumpState *D, lua_Number x) {
  dumpVar(D, x);
}

// 转储整数
static void dumpInteger (DumpState *D, lua_Integer x) {
  dumpVar(D, x);
}

// 转储字符串
static void dumpString (DumpState *D, const TString *s) {
  if (s == NULL)
    dumpSize(D, 0);
  else {
    size_t size = tsslen(s);
    const char *str = getstr(s);
    dumpSize(D, size + 1);
    dumpVector(D, str, size);
  }
}

// 转储代码
static void dumpCode (DumpState *D, const Proto *f) {
  dumpInt(D, f->sizecode);
  dumpVector(D, f->code, f->sizecode);
}


static void dumpFunction(DumpState *D, const Proto *f, TString *psource);

// 转储常量
static void dumpConstants (DumpState *D, const Proto *f) {
  int i;
  int n = f->sizek;
  dumpInt(D, n);
  for (i = 0; i < n; i++) {
    const TValue *o = &f->k[i];
    int tt = ttypetag(o);
    dumpByte(D, tt);
    switch (tt) {
      case LUA_VNUMFLT:
        dumpNumber(D, fltvalue(o));
        break;
      case LUA_VNUMINT:
        dumpInteger(D, ivalue(o));
        break;
      case LUA_VSHRSTR:
      case LUA_VLNGSTR:
        dumpString(D, tsvalue(o));
        break;
      default:
        lua_assert(tt == LUA_VNIL || tt == LUA_VFALSE || tt == LUA_VTRUE);
    }
  }
}

// 转储原型
static void dumpProtos (DumpState *D, const Proto *f) {
  int i;
  int n = f->sizep;
  dumpInt(D, n);
  for (i = 0; i < n; i++)
    dumpFunction(D, f->p[i], f->source);
}

// 转储上值
static void dumpUpvalues (DumpState *D, const Proto *f) {
  int i, n = f->sizeupvalues;
  dumpInt(D, n);
  for (i = 0; i < n; i++) {
    dumpByte(D, f->upvalues[i].instack);
    dumpByte(D, f->upvalues[i].idx);
    dumpByte(D, f->upvalues[i].kind);
  }
}

// 转储调试
static void dumpDebug (DumpState *D, const Proto *f) {
  int i, n;
  n = (D->strip) ? 0 : f->sizelineinfo;
  dumpInt(D, n);
  dumpVector(D, f->lineinfo, n);
  n = (D->strip) ? 0 : f->sizeabslineinfo;
  dumpInt(D, n);
  for (i = 0; i < n; i++) {
    dumpInt(D, f->abslineinfo[i].pc);
    dumpInt(D, f->abslineinfo[i].line);
  }
  n = (D->strip) ? 0 : f->sizelocvars;
  dumpInt(D, n);
  for (i = 0; i < n; i++) {
    dumpString(D, f->locvars[i].varname);
    dumpInt(D, f->locvars[i].startpc);
    dumpInt(D, f->locvars[i].endpc);
  }
  n = (D->strip) ? 0 : f->sizeupvalues;
  dumpInt(D, n);
  for (i = 0; i < n; i++)
    dumpString(D, f->upvalues[i].name);
}

// 转储函数
static void dumpFunction (DumpState *D, const Proto *f, TString *psource) {
  if (D->strip || f->source == psource)
    dumpString(D, NULL);  /* no debug info or same source as its parent 没有调试信息或与其父级相同的源 */
  else
    dumpString(D, f->source); // 源文件名
  dumpInt(D, f->linedefined); // 开始行号
  dumpInt(D, f->lastlinedefined); // 最后行号
  dumpByte(D, f->numparams); // 固定参数个数
  dumpByte(D, f->is_vararg); // 是否是 vararg 函数
  dumpByte(D, f->maxstacksize); // 寄存器数量
  dumpCode(D, f); // 指令表
  dumpConstants(D, f); // 常量表
  dumpUpvalues(D, f); // 上值表
  dumpProtos(D, f); // 子函数原型
  dumpDebug(D, f); // 调试
}

// 转储文件头
static void dumpHeader (DumpState *D) {
  dumpLiteral(D, LUA_SIGNATURE); // 魔术字
  dumpByte(D, LUAC_VERSION); // 版本
  dumpByte(D, LUAC_FORMAT); // 格式
  dumpLiteral(D, LUAC_DATA); // LUA诞生年份1993
  dumpByte(D, sizeof(Instruction)); // 指令集
  dumpByte(D, sizeof(lua_Integer)); // 整数
  dumpByte(D, sizeof(lua_Number)); // 数值（浮点）
  dumpInteger(D, LUAC_INT); // 验证整数
  dumpNumber(D, LUAC_NUM); // 验证浮点
}


/*
** dump Lua function as precompiled chunk
** 将 Lua函数转储为预编译块
*/
int luaU_dump(lua_State *L, const Proto *f, lua_Writer w, void *data,
              int strip) {
  DumpState D;
  D.L = L;
  D.writer = w;
  D.data = data;
  D.strip = strip;
  D.status = 0;
  dumpHeader(&D);
  dumpByte(&D, f->sizeupvalues);
  dumpFunction(&D, f, NULL);
  return D.status;
}


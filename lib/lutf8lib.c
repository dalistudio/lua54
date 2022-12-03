/*
** $Id: lutf8lib.c $
** Standard library for UTF-8 manipulation
** UTF8操作的标准库
** See Copyright Notice in lua.h
*/

#define lutf8lib_c
#define LUA_LIB

#include "../src/lprefix.h"


#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "../src/lua.h"

#include "../src/lauxlib.h"
#include "../src/lualib.h"


#define MAXUNICODE	0x10FFFFu

#define MAXUTF		0x7FFFFFFFu

/*
** Integer type for decoded UTF-8 values; MAXUTF needs 31 bits.
** 解码UTF8值的整数类型；MAXUTF需要31位
*/
#if (UINT_MAX >> 30) >= 1
typedef unsigned int utfint;
#else
typedef unsigned long utfint;
#endif


#define iscont(p)	((*(p) & 0xC0) == 0x80)


/* from strlib 来自字符串库 */
/* 
   translate a relative string position: negative means back from end 
   平移相对字符串位置：负值标识从未尾返回
*/
static lua_Integer u_posrelat (lua_Integer pos, size_t len) {
  if (pos >= 0) return pos;
  else if (0u - (size_t)pos > len) return 0;
  else return (lua_Integer)len + pos + 1;
}


/*
** Decode one UTF-8 sequence, returning NULL if byte sequence is
** invalid.  The array 'limits' stores the minimum value for each
** sequence length, to check for overlong representations. Its first
** entry forces an error for non-ascii bytes with no continuation
** bytes (count == 0).
** 解码一个UTF8序列，如果字节序列无效，则返回NULL。数组'limits'存储每个序列长度的最小值，以检查是否存在超长表示。
** 它的第一个条目强制非ASCII字节出现错误，没有连续字节（count == 0）
*/
static const char *utf8_decode (const char *s, utfint *val, int strict) {
  static const utfint limits[] =
        {~(utfint)0, 0x80, 0x800, 0x10000u, 0x200000u, 0x4000000u};
  unsigned int c = (unsigned char)s[0];
  utfint res = 0;  /* final result 最终结果 */
  if (c < 0x80)  /* ascii? 是否未ASCII? */
    res = c;
  else {
    int count = 0;  /* to count number of continuation bytes 连续字节数 */
    for (; c & 0x40; c <<= 1) {  /* while it needs continuation bytes... 而它需要继续字节 */
      unsigned int cc = (unsigned char)s[++count];  /* read next byte 读取下一字节 */
      if ((cc & 0xC0) != 0x80)  /* not a continuation byte? 不是连续字节？*/
        return NULL;  /* invalid byte sequence 无效字节序列 */
      res = (res << 6) | (cc & 0x3F);  /* add lower 6 bits from cont. byte 从连续字节添加较低的6位 */
    }
    res |= ((utfint)(c & 0x7F) << (count * 5));  /* add first byte 添加第一个字节 */
    if (count > 5 || res > MAXUTF || res < limits[count])
      return NULL;  /* invalid byte sequence 无效字节序列 */
    s += count;  /* skip continuation bytes read 跳过读取的连续字节  */
  }
  if (strict) {
    /* check for invalid code points; too large or surrogates 检查无效代码点；太大或代用品 */
    if (res > MAXUNICODE || (0xD800u <= res && res <= 0xDFFFu))
      return NULL;
  }
  if (val) *val = res;
  return s + 1;  /* +1 to include first byte。+1包含第一个字节 */
}


/*
** utf8len(s [, i [, j [, lax]]]) --> number of characters that
** start in the range [i,j], or nil + current position if 's' is not
** well formed in that interval
*/
static int utflen (lua_State *L) {
  lua_Integer n = 0;  /* counter for the number of characters 字符数计算器 */
  size_t len;  /* string length in bytes 字符串长度（字节） */
  const char *s = luaL_checklstring(L, 1, &len);
  lua_Integer posi = u_posrelat(luaL_optinteger(L, 2, 1), len);
  lua_Integer posj = u_posrelat(luaL_optinteger(L, 3, -1), len);
  int lax = lua_toboolean(L, 4);
  luaL_argcheck(L, 1 <= posi && --posi <= (lua_Integer)len, 2,
                   "initial position out of bounds");
  luaL_argcheck(L, --posj < (lua_Integer)len, 3,
                   "final position out of bounds");
  while (posi <= posj) {
    const char *s1 = utf8_decode(s + posi, NULL, !lax);
    if (s1 == NULL) {  /* conversion error? 转换错误? */
      luaL_pushfail(L);  /* return fail ... 返回失败 */
      lua_pushinteger(L, posi + 1);  /* ... and current position 和当前位置 */
      return 2;
    }
    posi = s1 - s;
    n++;
  }
  lua_pushinteger(L, n);
  return 1;
}


/*
** codepoint(s, [i, [j [, lax]]]) -> returns codepoints for all
** characters that start in the range [i,j]
*/
static int codepoint (lua_State *L) {
  size_t len;
  const char *s = luaL_checklstring(L, 1, &len);
  lua_Integer posi = u_posrelat(luaL_optinteger(L, 2, 1), len);
  lua_Integer pose = u_posrelat(luaL_optinteger(L, 3, posi), len);
  int lax = lua_toboolean(L, 4);
  int n;
  const char *se;
  luaL_argcheck(L, posi >= 1, 2, "out of bounds");
  luaL_argcheck(L, pose <= (lua_Integer)len, 3, "out of bounds");
  if (posi > pose) return 0;  /* empty interval; return no values 空区间；不返回值*/
  if (pose - posi >= INT_MAX)  /* (lua_Integer -> int) overflow? */
    return luaL_error(L, "string slice too long");
  n = (int)(pose -  posi) + 1;  /* upper bound for number of returns 返回次数上限 */
  luaL_checkstack(L, n, "string slice too long");
  n = 0;  /* count the number of returns 计算退货数量 */
  se = s + pose;  /* string end 字符串尾 */
  for (s += posi - 1; s < se;) {
    utfint code;
    s = utf8_decode(s, &code, !lax);
    if (s == NULL)
      return luaL_error(L, "invalid UTF-8 code");
    lua_pushinteger(L, code);
    n++;
  }
  return n;
}


static void pushutfchar (lua_State *L, int arg) {
  lua_Unsigned code = (lua_Unsigned)luaL_checkinteger(L, arg);
  luaL_argcheck(L, code <= MAXUTF, arg, "value out of range");
  lua_pushfstring(L, "%U", (long)code);
}


/*
** utfchar(n1, n2, ...)  -> char(n1)..char(n2)...
*/
static int utfchar (lua_State *L) {
  int n = lua_gettop(L);  /* number of arguments 参数的数量 */
  if (n == 1)  /* optimize common case of single char 优化单个字符的常见情况 */
    pushutfchar(L, 1);
  else {
    int i;
    luaL_Buffer b;
    luaL_buffinit(L, &b);
    for (i = 1; i <= n; i++) {
      pushutfchar(L, i);
      luaL_addvalue(&b);
    }
    luaL_pushresult(&b);
  }
  return 1;
}


/*
** offset(s, n, [i])  -> index where n-th character counting from
**   position 'i' starts; 0 means character at 'i'.
*/
static int byteoffset (lua_State *L) {
  size_t len;
  const char *s = luaL_checklstring(L, 1, &len);
  lua_Integer n  = luaL_checkinteger(L, 2);
  lua_Integer posi = (n >= 0) ? 1 : len + 1;
  posi = u_posrelat(luaL_optinteger(L, 3, posi), len);
  luaL_argcheck(L, 1 <= posi && --posi <= (lua_Integer)len, 3,
                   "position out of bounds");
  if (n == 0) {
    /* find beginning of current byte sequence 查找当前字节序列的开头 */
    while (posi > 0 && iscont(s + posi)) posi--;
  }
  else {
    if (iscont(s + posi))
      return luaL_error(L, "initial position is a continuation byte");
    if (n < 0) {
       while (n < 0 && posi > 0) {  /* move back 向后移动 */
         do {  /* find beginning of previous character 查找上一个字符的开头 */
           posi--;
         } while (posi > 0 && iscont(s + posi));
         n++;
       }
     }
     else {
       n--;  /* do not move for 1st character 不移动第一个字符 */
       while (n > 0 && posi < (lua_Integer)len) {
         do {  /* find beginning of next character 查找下一个字符的开头 */
           posi++;
         } while (iscont(s + posi));  /* (cannot pass final '\0') */
         n--;
       }
     }
  }
  if (n == 0)  /* did it find given character? 它找到给定的字符了吗？ */
    lua_pushinteger(L, posi + 1);
  else  /* no such character 没有这样的字符 */
    luaL_pushfail(L);
  return 1;
}


static int iter_aux (lua_State *L, int strict) {
  size_t len;
  const char *s = luaL_checklstring(L, 1, &len);
  lua_Unsigned n = (lua_Unsigned)lua_tointeger(L, 2);
  if (n < len) {
    while (iscont(s + n)) n++;  /* skip continuation bytes 跳过连续字节 */
  }
  if (n >= len)  /* (also handles original 'n' being negative) */
    return 0;  /* no more codepoints 不再有代码点 */
  else {
    utfint code;
    const char *next = utf8_decode(s + n, &code, strict);
    if (next == NULL)
      return luaL_error(L, "invalid UTF-8 code");
    lua_pushinteger(L, n + 1);
    lua_pushinteger(L, code);
    return 2;
  }
}


static int iter_auxstrict (lua_State *L) {
  return iter_aux(L, 1);
}

static int iter_auxlax (lua_State *L) {
  return iter_aux(L, 0);
}


static int iter_codes (lua_State *L) {
  int lax = lua_toboolean(L, 2);
  luaL_checkstring(L, 1);
  lua_pushcfunction(L, lax ? iter_auxlax : iter_auxstrict);
  lua_pushvalue(L, 1);
  lua_pushinteger(L, 0);
  return 3;
}


/* pattern to match a single UTF-8 character 模式以匹配单个UTF8字符 */
#define UTF8PATT	"[\0-\x7F\xC2-\xFD][\x80-\xBF]*"


static const luaL_Reg funcs[] = {
  {"offset", byteoffset}, // 字节偏移
  {"codepoint", codepoint}, // 代码点
  {"char", utfchar}, // UTF字节
  {"len", utflen}, // UTF长度
  {"codes", iter_codes}, // 迭代器
  /* placeholders 占位符 */
  {"charpattern", NULL}, // 字符模式
  {NULL, NULL}
};


LUAMOD_API int luaopen_utf8 (lua_State *L) {
  luaL_newlib(L, funcs);
  lua_pushlstring(L, UTF8PATT, sizeof(UTF8PATT)/sizeof(char) - 1);
  lua_setfield(L, -2, "charpattern");
  return 1;
}


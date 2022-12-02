/*
** $Id: lstring.c $
** String table (keeps all strings handled by Lua)
** 字符串表（保留Lua处理的所有字符串）
** See Copyright Notice in lua.h
*/

#define lstring_c
#define LUA_CORE

#include "lprefix.h"


#include <string.h>

#include "lua.h"

#include "ldebug.h"
#include "ldo.h"
#include "lmem.h"
#include "lobject.h"
#include "lstate.h"
#include "lstring.h"


/*
** Maximum size for string table. 字符串表的最大大小
*/
#define MAXSTRTB	cast_int(luaM_limitN(MAX_INT, TString*))


/*
** equality for long strings 长字符串相等
*/
int luaS_eqlngstr (TString *a, TString *b) {
  size_t len = a->u.lnglen;
  lua_assert(a->tt == LUA_VLNGSTR && b->tt == LUA_VLNGSTR);
  return (a == b) ||  /* same instance or... 同一实例或 */
    ((len == b->u.lnglen) &&  /* equal length and ...  等长 */
     (memcmp(getstr(a), getstr(b), len) == 0));  /* equal contents 相等的内容 */
}


unsigned int luaS_hash (const char *str, size_t l, unsigned int seed) {
  unsigned int h = seed ^ cast_uint(l);
  for (; l > 0; l--)
    h ^= ((h<<5) + (h>>2) + cast_byte(str[l - 1]));
  return h;
}


unsigned int luaS_hashlongstr (TString *ts) {
  lua_assert(ts->tt == LUA_VLNGSTR);
  if (ts->extra == 0) {  /* no hash? 不是哈希？ */
    size_t len = ts->u.lnglen;
    ts->hash = luaS_hash(getstr(ts), len, ts->hash);
    ts->extra = 1;  /* now it has its hash 现在它有了哈希 */
  }
  return ts->hash;
}


static void tablerehash (TString **vect, int osize, int nsize) {
  int i;
  for (i = osize; i < nsize; i++)  /* clear new elements 清除新元素 */
    vect[i] = NULL;
  for (i = 0; i < osize; i++) {  /* rehash old part of the array 重新刷新旧部分数组 */
    TString *p = vect[i];
    vect[i] = NULL;
    while (p) {  /* for each string in the list 对于列表中的每个字符串 */
      TString *hnext = p->u.hnext;  /* save next 保存下一个 */
      unsigned int h = lmod(p->hash, nsize);  /* new position 新指针 */
      p->u.hnext = vect[h];  /* chain it into array 将其链接到数组中 */
      vect[h] = p;
      p = hnext;
    }
  }
}


/*
** Resize the string table. If allocation fails, keep the current size.
** (This can degrade performance, but any non-zero size should work
** correctly.)
** 调整字符串表的大小。如果分配失败，请保持当前大小。（这会降低性能，但任何非零大小都应该正常工作。）
*/
void luaS_resize (lua_State *L, int nsize) {
  stringtable *tb = &G(L)->strt;
  int osize = tb->size;
  TString **newvect;
  if (nsize < osize)  /* shrinking table? 缩小表？*/
    tablerehash(tb->hash, osize, nsize);  /* depopulate shrinking part 减少收缩部分 */
  newvect = luaM_reallocvector(L, tb->hash, osize, nsize, TString*);
  if (l_unlikely(newvect == NULL)) {  /* reallocation failed? 重新分配失败？ */
    if (nsize < osize)  /* was it shrinking table? 是在缩小表吗？ */
      tablerehash(tb->hash, nsize, osize);  /* restore to original size 恢复到原始大小 */
    /* leave table as it was 让表保持原样 */
  }
  else {  /* allocation succeeded 分配成功 */
    tb->hash = newvect;
    tb->size = nsize;
    if (nsize > osize)
      tablerehash(newvect, osize, nsize);  /* rehash for new size 重新调整新尺寸 */
  }
}


/*
** Clear API string cache. (Entries cannot be empty, so fill them with
** a non-collectable string.)
** 清除API字符串缓冲。（条目不能为空，因此请使用不可收集的字符串填充它们）
*/
void luaS_clearcache (global_State *g) {
  int i, j;
  for (i = 0; i < STRCACHE_N; i++)
    for (j = 0; j < STRCACHE_M; j++) {
      if (iswhite(g->strcache[i][j]))  /* will entry be collected? 进入时是否收集？ */
        g->strcache[i][j] = g->memerrmsg;  /* replace it with something fixed 用固定的东西替换它 */
    }
}


/*
** Initialize the string table and the string cache
** 初始化字符串表和字符串缓冲
*/
void luaS_init (lua_State *L) {
  global_State *g = G(L);
  int i, j;
  stringtable *tb = &G(L)->strt;
  tb->hash = luaM_newvector(L, MINSTRTABSIZE, TString*);
  tablerehash(tb->hash, 0, MINSTRTABSIZE);  /* clear array 清除数组 */
  tb->size = MINSTRTABSIZE;
  /* pre-create memory-error message 预创建内存错误消息 */
  g->memerrmsg = luaS_newliteral(L, MEMERRMSG);
  luaC_fix(L, obj2gco(g->memerrmsg));  /* it should never be collected 它永远不应该被收集 */
  for (i = 0; i < STRCACHE_N; i++)  /* fill cache with valid strings 用有效字符串填充缓存 */
    for (j = 0; j < STRCACHE_M; j++)
      g->strcache[i][j] = g->memerrmsg;
}



/*
** creates a new string object 创建新的字符串对象
*/
static TString *createstrobj (lua_State *L, size_t l, int tag, unsigned int h) {
  TString *ts;
  GCObject *o;
  size_t totalsize;  /* total size of TString object 对象的总大小 */
  totalsize = sizelstring(l);
  o = luaC_newobj(L, tag, totalsize);
  ts = gco2ts(o);
  ts->hash = h;
  ts->extra = 0;
  getstr(ts)[l] = '\0';  /* ending 0 */
  return ts;
}


TString *luaS_createlngstrobj (lua_State *L, size_t l) {
  TString *ts = createstrobj(L, l, LUA_VLNGSTR, G(L)->seed);
  ts->u.lnglen = l;
  return ts;
}


void luaS_remove (lua_State *L, TString *ts) {
  stringtable *tb = &G(L)->strt;
  TString **p = &tb->hash[lmod(ts->hash, tb->size)];
  while (*p != ts)  /* find previous element 查找上一个元素 */
    p = &(*p)->u.hnext;
  *p = (*p)->u.hnext;  /* remove element from its list 从其列表中删除元素 */
  tb->nuse--;
}


static void growstrtab (lua_State *L, stringtable *tb) {
  if (l_unlikely(tb->nuse == MAX_INT)) {  /* too many strings? 字符串太多？ */
    luaC_fullgc(L, 1);  /* try to free some... 尝试释放一些 */
    if (tb->nuse == MAX_INT)  /* still too many? 还是太多链？ */
      luaM_error(L);  /* cannot even create a message... 设置无法创建消息 */
  }
  if (tb->size <= MAXSTRTB / 2)  /* can grow string table? 可以增长字符串表吗？ */
    luaS_resize(L, tb->size * 2);
}


/*
** Checks whether short string exists and reuses it or creates a new one.
** 检查短字符串是否存在并重用它或创建新字符串。
*/
static TString *internshrstr (lua_State *L, const char *str, size_t l) {
  TString *ts;
  global_State *g = G(L);
  stringtable *tb = &g->strt;
  unsigned int h = luaS_hash(str, l, g->seed);
  TString **list = &tb->hash[lmod(h, tb->size)];
  lua_assert(str != NULL);  /* otherwise 'memcmp'/'memcpy' are undefined 否则未定义 'memcmp'/'memcpy' */
  for (ts = *list; ts != NULL; ts = ts->u.hnext) {
    if (l == ts->shrlen && (memcmp(str, getstr(ts), l * sizeof(char)) == 0)) {
      /* found! 找到 */
      if (isdead(g, ts))  /* dead (but not collected yet)? 死亡（但尚未收集）？*/
        changewhite(ts);  /* resurrect it 复活它 */
      return ts;
    }
  }
  /* else must create a new string 否则必须创建新字符串 */
  if (tb->nuse >= tb->size) {  /* need to grow string table? 需要郑家字符串表吗？ */
    growstrtab(L, tb);
    list = &tb->hash[lmod(h, tb->size)];  /* rehash with new size 用新尺寸重新冲洗 */
  }
  ts = createstrobj(L, l, LUA_VSHRSTR, h);
  memcpy(getstr(ts), str, l * sizeof(char));
  ts->shrlen = cast_byte(l);
  ts->u.hnext = *list;
  *list = ts;
  tb->nuse++;
  return ts;
}


/*
** new string (with explicit length)
** 新字符串（具有显式长度）
*/
TString *luaS_newlstr (lua_State *L, const char *str, size_t l) {
  if (l <= LUAI_MAXSHORTLEN)  /* short string? 短字符串？*/
    return internshrstr(L, str, l);
  else {
    TString *ts;
    if (l_unlikely(l >= (MAX_SIZE - sizeof(TString))/sizeof(char)))
      luaM_toobig(L);
    ts = luaS_createlngstrobj(L, l);
    memcpy(getstr(ts), str, l * sizeof(char));
    return ts;
  }
}


/*
** Create or reuse a zero-terminated string, first checking in the
** cache (using the string address as a key). The cache can contain
** only zero-terminated strings, so it is safe to use 'strcmp' to
** check hits.
** 创建或重用以零结尾的字符串。首先检查缓存（使用字符串地址作为关键字）。
** 缓存只能包含以零结尾的字符串，因此使用'strcmp'检查命中的安全的。
*/
TString *luaS_new (lua_State *L, const char *str) {
  unsigned int i = point2uint(str) % STRCACHE_N;  /* hash 哈希 */
  int j;
  TString **p = G(L)->strcache[i];
  for (j = 0; j < STRCACHE_M; j++) {
    if (strcmp(str, getstr(p[j])) == 0)  /* hit? 命中？ */
      return p[j];  /* that is it */
  }
  /* normal route 正常路线 */
  for (j = STRCACHE_M - 1; j > 0; j--)
    p[j] = p[j - 1];  /* move out last element 移除最后一个元素 */
  /* new element is first in the list 新元素是列表中的第一个 */
  p[0] = luaS_newlstr(L, str, strlen(str));
  return p[0];
}


Udata *luaS_newudata (lua_State *L, size_t s, int nuvalue) {
  Udata *u;
  int i;
  GCObject *o;
  if (l_unlikely(s > MAX_SIZE - udatamemoffset(nuvalue)))
    luaM_toobig(L);
  o = luaC_newobj(L, LUA_VUSERDATA, sizeudata(nuvalue, s));
  u = gco2u(o);
  u->len = s;
  u->nuvalue = nuvalue;
  u->metatable = NULL;
  for (i = 0; i < nuvalue; i++)
    setnilvalue(&u->uv[i].uv);
  return u;
}


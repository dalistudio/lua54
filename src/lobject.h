/*
** $Id: lobject.h $
** Type definitions for Lua objects
** Lua对象的类型定义
** See Copyright Notice in lua.h
*/


#ifndef lobject_h
#define lobject_h


#include <stdarg.h>


#include "llimits.h"
#include "lua.h"


/*
** Extra types for collectable non-values
** 可收集non-values的额外类型
*/
#define LUA_TUPVAL	LUA_NUMTYPES  /* upvalues 上值 */
#define LUA_TPROTO	(LUA_NUMTYPES+1)  /* function prototypes 函数原型 */
#define LUA_TDEADKEY	(LUA_NUMTYPES+2)  /* removed keys in tables 删除链表中的键 */



/*
** number of all possible types (including LUA_TNONE but excluding DEADKEY)
** 所有可能类型的数量(包括 LUA_TNONE ，但不包括 DEADKEY)
*/
#define LUA_TOTALTYPES		(LUA_TPROTO + 2)


/*
** tags for Tagged Values have the following use of bits:
** 标记值的标记使用以下位：
** bits 0-3: actual tag (a LUA_T* constant) 实际标签
** bits 4-5: variant bits 可变比特位
** bit 6: whether value is collectable 值是否可回收
*/

/* 
   add variant bits to a type 
   添加变量比特位到类型
*/
#define makevariant(t,v)	((t) | ((v) << 4))



/*
** Union of all Lua values
*/
typedef union Value {
  struct GCObject *gc;    /* collectable objects  收集对象*/
  void *p;         /* light userdata 轻量级用户数据 */
  lua_CFunction f; /* light C functions 轻量级C函数 */
  lua_Integer i;   /* integer numbers 整数数值 */
  lua_Number n;    /* float numbers 浮点数值 */
} Value;


/*
** Tagged Values. This is the basic representation of values in Lua:
** an actual value plus a tag with its type.
** 标记值。这是Lua中值的基本表示：实际值加上带有其类型的标记。
*/

#define TValuefields	Value value_; lu_byte tt_

typedef struct TValue {
  TValuefields;
} TValue;


#define val_(o)		((o)->value_)
#define valraw(o)	(val_(o))


/* 
   raw type tag of a TValue 
   TValue的原始类型标签
*/
#define rawtt(o)	((o)->tt_)

/* 
   tag with no variants (bits 0-3) 
   无变量的标签(bits 0-3)
*/
#define novariant(t)	((t) & 0x0F)

/* 
   type tag of a TValue (bits 0-3 for tags + variant bits 4-5) 
   TValue的类型标签（0-3位是标签+4-5位是变量）
*/
#define withvariant(t)	((t) & 0x3F)
#define ttypetag(o)	withvariant(rawtt(o))

/* 
   type of a TValue 
   TValue的类型
*/
#define ttype(o)	(novariant(rawtt(o)))


/* 
   Macros to test type 
   测试类型的宏
*/
#define checktag(o,t)		(rawtt(o) == (t))
#define checktype(o,t)		(ttype(o) == (t))


/* 
   Macros for internal tests 
   内部测试宏
*/

/* 
   collectable object has the same tag as the original value 
   可收集对象具有与原始值相同的标签
*/
#define righttt(obj)		(ttypetag(obj) == gcvalue(obj)->tt)

/*
** Any value being manipulated by the program either is non
** collectable, or the collectable object has the right tag
** and it is not dead. The option 'L == NULL' allows other
** macros using this one to be used where L is not available.
*/
#define checkliveness(L,obj) \
	((void)L, lua_longassert(!iscollectable(obj) || \
		(righttt(obj) && (L == NULL || !isdead(G(L),gcvalue(obj))))))


/* 
   Macros to set values 
   设置值的宏
*/

/* 
   set a value's tag 
   设置值的标签
*/
#define settt_(o,t)	((o)->tt_=(t))


/* 
   main macro to copy values (from 'obj2' to 'obj1') 
   用于复制值的主宏(从 'obj2' 到 'obj1')
*/
#define setobj(L,obj1,obj2) \
	{ TValue *io1=(obj1); const TValue *io2=(obj2); \
          io1->value_ = io2->value_; settt_(io1, io2->tt_); \
	  checkliveness(L,io1); lua_assert(!isnonstrictnil(io1)); }

/*
** Different types of assignments, according to source and destination.
** (They are mostly equal now, but may be different in the future.)
** 根据来源和目的地，不同类型的任务。
**（它们现在基本上是平等的，但未来可能会有所不同。）
*/

/* from stack to stack 从堆栈到堆栈 */
#define setobjs2s(L,o1,o2)	setobj(L,s2v(o1),s2v(o2))
/* to stack (not from same stack) 到堆栈（不来自同一个堆栈）*/
#define setobj2s(L,o1,o2)	setobj(L,s2v(o1),o2)
/* from table to same table 从表到同一个表 */
#define setobjt2t	setobj
/* to new object 到新对象 */
#define setobj2n	setobj
/* to table 到表 */
#define setobj2t	setobj


/*
** Entries in a Lua stack. Field 'tbclist' forms a list of all
** to-be-closed variables active in this stack. Dummy entries are
** used when the distance between two tbc variables does not fit
** in an unsigned short. They are represented by delta==0, and
** their real delta is always the maximum value that fits in
** that field.
*/
typedef union StackValue {
  TValue val;
  struct {
    TValuefields;
    unsigned short delta;
  } tbclist;
} StackValue;


/* index to stack elements 堆栈元素索引 */
typedef StackValue *StkId;

/* convert a 'StackValue' to a 'TValue' 将'StackValue'转换为'TValue' */
#define s2v(o)	(&(o)->val)



/*
** {==================================================================
** Nil 零值
** ===================================================================
*/

/* Standard nil 标准的零值 */
#define LUA_VNIL	makevariant(LUA_TNIL, 0)

/* Empty slot (which might be different from a slot containing nil) 空的槽（可能与包括空值的槽不同） */
#define LUA_VEMPTY	makevariant(LUA_TNIL, 1)

/* Value returned for a key not found in a table (absent key) 表中找不到键的返回值（缺少键）*/
#define LUA_VABSTKEY	makevariant(LUA_TNIL, 2)


/* macro to test for (any kind of) nil 要测试（任何类型）空的宏 */
#define ttisnil(v)		checktype((v), LUA_TNIL)


/* macro to test for a standard nil 测试标准空的宏*/
#define ttisstrictnil(o)	checktag((o), LUA_VNIL)


#define setnilvalue(obj) settt_(obj, LUA_VNIL)


#define isabstkey(v)		checktag((v), LUA_VABSTKEY)


/*
** macro to detect non-standard nils (used only in assertions)
** 用于检测非标准空的宏（仅在断言中使用）
*/
#define isnonstrictnil(v)	(ttisnil(v) && !ttisstrictnil(v))


/*
** By default, entries with any kind of nil are considered empty.
** (In any definition, values associated with absent keys must also
** be accepted as empty.)
** 默认情况下，具有任何类型的空的条目都视为空。
**（在任何定义中，与缺少键关联的值页必须被接受为空）
*/
#define isempty(v)		ttisnil(v)


/* 
   macro defining a value corresponding to an absent key 
   定义与缺少的键对应的值的宏
*/
#define ABSTKEYCONSTANT		{NULL}, LUA_VABSTKEY


/* mark an entry as empty 将条目标记为空 */
#define setempty(v)		settt_(v, LUA_VEMPTY)



/* }================================================================== */


/*
** {==================================================================
** Booleans 布尔值
** ===================================================================
*/


#define LUA_VFALSE	makevariant(LUA_TBOOLEAN, 0)
#define LUA_VTRUE	makevariant(LUA_TBOOLEAN, 1)

#define ttisboolean(o)		checktype((o), LUA_TBOOLEAN)
#define ttisfalse(o)		checktag((o), LUA_VFALSE)
#define ttistrue(o)		checktag((o), LUA_VTRUE)


#define l_isfalse(o)	(ttisfalse(o) || ttisnil(o))


#define setbfvalue(obj)		settt_(obj, LUA_VFALSE)
#define setbtvalue(obj)		settt_(obj, LUA_VTRUE)

/* }================================================================== */


/*
** {==================================================================
** Threads 线程
** ===================================================================
*/

#define LUA_VTHREAD		makevariant(LUA_TTHREAD, 0)

#define ttisthread(o)		checktag((o), ctb(LUA_VTHREAD))

#define thvalue(o)	check_exp(ttisthread(o), gco2th(val_(o).gc))

#define setthvalue(L,obj,x) \
  { TValue *io = (obj); lua_State *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_VTHREAD)); \
    checkliveness(L,io); }

#define setthvalue2s(L,o,t)	setthvalue(L,s2v(o),t)

/* }================================================================== */


/*
** {==================================================================
** Collectable Objects 可收集对象
** ===================================================================
*/

/*
** Common Header for all collectable objects (in macro form, to be
** included in other objects)
** 所有可收集对象的公共头（以宏形式，包括在其他对象中）
*/
#define CommonHeader	struct GCObject *next; lu_byte tt; lu_byte marked


/* Common type for all collectable objects 所有可收集对象的通用类型 */
typedef struct GCObject {
  CommonHeader;
} GCObject;


/* Bit mark for collectable types可收集类型的位标记*/
#define BIT_ISCOLLECTABLE	(1 << 6)

#define iscollectable(o)	(rawtt(o) & BIT_ISCOLLECTABLE)

/* mark a tag as collectable 将标记标签位可收藏 */
#define ctb(t)			((t) | BIT_ISCOLLECTABLE)

#define gcvalue(o)	check_exp(iscollectable(o), val_(o).gc)

#define gcvalueraw(v)	((v).gc)

#define setgcovalue(L,obj,x) \
  { TValue *io = (obj); GCObject *i_g=(x); \
    val_(io).gc = i_g; settt_(io, ctb(i_g->tt)); }

/* }================================================================== */


/*
** {==================================================================
** Numbers 数值
** ===================================================================
*/

/* Variant tags for numbers 数值的变量标签 */
#define LUA_VNUMINT	makevariant(LUA_TNUMBER, 0)  /* integer numbers 整数数值 */
#define LUA_VNUMFLT	makevariant(LUA_TNUMBER, 1)  /* float numbers 浮点数值 */

#define ttisnumber(o)		checktype((o), LUA_TNUMBER)
#define ttisfloat(o)		checktag((o), LUA_VNUMFLT)
#define ttisinteger(o)		checktag((o), LUA_VNUMINT)

#define nvalue(o)	check_exp(ttisnumber(o), \
	(ttisinteger(o) ? cast_num(ivalue(o)) : fltvalue(o)))
#define fltvalue(o)	check_exp(ttisfloat(o), val_(o).n)
#define ivalue(o)	check_exp(ttisinteger(o), val_(o).i)

#define fltvalueraw(v)	((v).n)
#define ivalueraw(v)	((v).i)

#define setfltvalue(obj,x) \
  { TValue *io=(obj); val_(io).n=(x); settt_(io, LUA_VNUMFLT); }

#define chgfltvalue(obj,x) \
  { TValue *io=(obj); lua_assert(ttisfloat(io)); val_(io).n=(x); }

#define setivalue(obj,x) \
  { TValue *io=(obj); val_(io).i=(x); settt_(io, LUA_VNUMINT); }

#define chgivalue(obj,x) \
  { TValue *io=(obj); lua_assert(ttisinteger(io)); val_(io).i=(x); }

/* }================================================================== */


/*
** {==================================================================
** Strings 字符串
** ===================================================================
*/

/* Variant tags for strings 字符串的变量标签 */
#define LUA_VSHRSTR	makevariant(LUA_TSTRING, 0)  /* short strings 短字符串 */
#define LUA_VLNGSTR	makevariant(LUA_TSTRING, 1)  /* long strings 长字符串 */

#define ttisstring(o)		checktype((o), LUA_TSTRING)
#define ttisshrstring(o)	checktag((o), ctb(LUA_VSHRSTR))
#define ttislngstring(o)	checktag((o), ctb(LUA_VLNGSTR))

#define tsvalueraw(v)	(gco2ts((v).gc))

#define tsvalue(o)	check_exp(ttisstring(o), gco2ts(val_(o).gc))

#define setsvalue(L,obj,x) \
  { TValue *io = (obj); TString *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(x_->tt)); \
    checkliveness(L,io); }

/* set a string to the stack 将字符串设置到堆栈 */
#define setsvalue2s(L,o,s)	setsvalue(L,s2v(o),s)

/* set a string to a new object 将字符串设置为新对象 */
#define setsvalue2n	setsvalue


/*
** Header for a string value.
** 字符串值的头
*/
typedef struct TString {
  CommonHeader;
  lu_byte extra;  /* reserved words for short strings; "has hash" for longs 短字符串的保留字 */
  lu_byte shrlen;  /* length for short strings 短字符串的长度 */
  unsigned int hash;
  union {
    size_t lnglen;  /* length for long strings 长字符串的长度 */
    struct TString *hnext;  /* linked list for hash table 哈希表的链表列表 */
  } u;
  char contents[1];
} TString;



/*
** Get the actual string (array of bytes) from a 'TString'.
** 从'TString'中获取实际字符串（字节数组）
*/
#define getstr(ts)  ((ts)->contents)


/* 
   get the actual string (array of bytes) from a Lua value 
   从Lua值中获取实际字符串（字节数组）
*/
#define svalue(o)       getstr(tsvalue(o))

/* get string length from 'TString *s' 从'TString *s'获取字符串长度 */
#define tsslen(s)	((s)->tt == LUA_VSHRSTR ? (s)->shrlen : (s)->u.lnglen)

/* get string length from 'TValue *o' 从'TValue *o'获取字符串长度 */
#define vslen(o)	tsslen(tsvalue(o))

/* }================================================================== */


/*
** {==================================================================
** Userdata 用户数据
** ===================================================================
*/


/*
** Light userdata should be a variant of userdata, but for compatibility
** reasons they are also different types.
** 轻量级用户数据应该是用户数据的变体，但出于兼容性原因，它们也是不同的类型。
*/
#define LUA_VLIGHTUSERDATA	makevariant(LUA_TLIGHTUSERDATA, 0)

#define LUA_VUSERDATA		makevariant(LUA_TUSERDATA, 0)

#define ttislightuserdata(o)	checktag((o), LUA_VLIGHTUSERDATA)
#define ttisfulluserdata(o)	checktag((o), ctb(LUA_VUSERDATA))

#define pvalue(o)	check_exp(ttislightuserdata(o), val_(o).p)
#define uvalue(o)	check_exp(ttisfulluserdata(o), gco2u(val_(o).gc))

#define pvalueraw(v)	((v).p)

#define setpvalue(obj,x) \
  { TValue *io=(obj); val_(io).p=(x); settt_(io, LUA_VLIGHTUSERDATA); }

#define setuvalue(L,obj,x) \
  { TValue *io = (obj); Udata *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_VUSERDATA)); \
    checkliveness(L,io); }


/* 
   Ensures that addresses after this type are always fully aligned. 
   确保此类型之后的地址始终完全对齐。
*/
typedef union UValue {
  TValue uv;
  LUAI_MAXALIGN;  /* ensures maximum alignment for udata bytes 确保udata字节的最大对齐*/
} UValue;


/*
** Header for userdata with user values;
** memory area follows the end of this structure.
** 带有用户值的用户数据头；存储区域跟随该结构的末尾。
*/
typedef struct Udata {
  CommonHeader;
  unsigned short nuvalue;  /* number of user values 用户值的数量 */
  size_t len;  /* number of bytes 字节数 */
  struct Table *metatable;
  GCObject *gclist;
  UValue uv[1];  /* user values 用户值 */
} Udata;


/*
** Header for userdata with no user values. These userdata do not need
** to be gray during GC, and therefore do not need a 'gclist' field.
** To simplify, the code always use 'Udata' for both kinds of userdata,
** making sure it never accesses 'gclist' on userdata with no user values.
** This structure here is used only to compute the correct size for
** this representation. (The 'bindata' field in its end ensures correct
** alignment for binary data following this header.)
*/
typedef struct Udata0 {
  CommonHeader;
  unsigned short nuvalue;  /* number of user values 用户值的数量 */
  size_t len;  /* number of bytes 字节数 */
  struct Table *metatable;
  union {LUAI_MAXALIGN;} bindata;
} Udata0;


/* 
   compute the offset of the memory area of a userdata 
   计算用户数据的内存区域的偏移量
*/
#define udatamemoffset(nuv) \
	((nuv) == 0 ? offsetof(Udata0, bindata)  \
                    : offsetof(Udata, uv) + (sizeof(UValue) * (nuv)))

/* 
   get the address of the memory block inside 'Udata' 
   获取'Udata'中内存块的地址
*/
#define getudatamem(u)	(cast_charp(u) + udatamemoffset((u)->nuvalue))

/* compute the size of a userdata 计算用户数据的大小 */
#define sizeudata(nuv,nb)	(udatamemoffset(nuv) + (nb))

/* }================================================================== */


/*
** {==================================================================
** Prototypes 原型
** ===================================================================
*/

#define LUA_VPROTO	makevariant(LUA_TPROTO, 0)


/*
** Description of an upvalue for function prototypes
** 函数原型的上值描述
*/
typedef struct Upvaldesc {
  TString *name;  /* upvalue name (for debug information) 上值名称（仅用于调试信息）*/
  lu_byte instack;  /* whether it is in stack (register) 是否在堆栈中（寄存器）*/
  lu_byte idx;  /* index of upvalue (in stack or in outer function's list) 上值索引（堆栈中或外部函数列表中）*/
  lu_byte kind;  /* kind of corresponding variable */
} Upvaldesc;


/*
** Description of a local variable for function prototypes
** (used for debug information)
** 函数原型的局部变量描述（用于调试信息）
*/
typedef struct LocVar {
  TString *varname;
  int startpc;  /* first point where variable is active 变量处于活动状态的第一个点 */
  int endpc;    /* first point where variable is dead 变量无效的第一个点 */
} LocVar;


/*
** Associates the absolute line source for a given instruction ('pc').
** The array 'lineinfo' gives, for each instruction, the difference in
** lines from the previous instruction. When that difference does not
** fit into a byte, Lua saves the absolute line for that instruction.
** (Lua also saves the absolute line periodically, to speed up the
** computation of a line number: we can use binary search in the
** absolute-line array, but we must traverse the 'lineinfo' array
** linearly to compute a line.)
*/
typedef struct AbsLineInfo {
  int pc;
  int line;
} AbsLineInfo;

/*
** Function Prototypes
** 函数原型
*/
typedef struct Proto {
  CommonHeader;
  lu_byte numparams;  /* number of fixed (named) parameters 固定（命名）参数的数量 */
  lu_byte is_vararg;
  lu_byte maxstacksize;  /* number of registers needed by this function 此函数所需的寄存器数量 */
  int sizeupvalues;  /* size of 'upvalues' 上值的大小 */
  int sizek;  /* size of 'k' */
  int sizecode;
  int sizelineinfo;
  int sizep;  /* size of 'p' */
  int sizelocvars;
  int sizeabslineinfo;  /* size of 'abslineinfo' */
  int linedefined;  /* debug information 调试信息 */
  int lastlinedefined;  /* debug information 调试信息 */
  TValue *k;  /* constants used by the function 函数使用的常量 */
  Instruction *code;  /* opcodes 字节码 */
  struct Proto **p;  /* functions defined inside the function 函数内部定义的函数 */
  Upvaldesc *upvalues;  /* upvalue information 上值信息 */
  ls_byte *lineinfo;  /* information about source lines (debug information) 关于源代码行的信息（调试信息）*/
  AbsLineInfo *abslineinfo;  /* idem 同上 */
  LocVar *locvars;  /* information about local variables (debug information) 有关本地变量的信息（调试信息）*/
  TString  *source;  /* used for debug information 用于调试信息 */
  GCObject *gclist;
} Proto;

/* }================================================================== */


/*
** {==================================================================
** Functions 函数
** ===================================================================
*/

#define LUA_VUPVAL	makevariant(LUA_TUPVAL, 0)


/* Variant tags for functions 函数的变量标签 */
#define LUA_VLCL	makevariant(LUA_TFUNCTION, 0)  /* Lua closure 闭包 */
#define LUA_VLCF	makevariant(LUA_TFUNCTION, 1)  /* light C function 轻量级C函数 */
#define LUA_VCCL	makevariant(LUA_TFUNCTION, 2)  /* C closure 闭包 */

#define ttisfunction(o)		checktype(o, LUA_TFUNCTION)
#define ttisLclosure(o)		checktag((o), ctb(LUA_VLCL))
#define ttislcf(o)		checktag((o), LUA_VLCF)
#define ttisCclosure(o)		checktag((o), ctb(LUA_VCCL))
#define ttisclosure(o)         (ttisLclosure(o) || ttisCclosure(o))


#define isLfunction(o)	ttisLclosure(o)

#define clvalue(o)	check_exp(ttisclosure(o), gco2cl(val_(o).gc))
#define clLvalue(o)	check_exp(ttisLclosure(o), gco2lcl(val_(o).gc))
#define fvalue(o)	check_exp(ttislcf(o), val_(o).f)
#define clCvalue(o)	check_exp(ttisCclosure(o), gco2ccl(val_(o).gc))

#define fvalueraw(v)	((v).f)

#define setclLvalue(L,obj,x) \
  { TValue *io = (obj); LClosure *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_VLCL)); \
    checkliveness(L,io); }

#define setclLvalue2s(L,o,cl)	setclLvalue(L,s2v(o),cl)

#define setfvalue(obj,x) \
  { TValue *io=(obj); val_(io).f=(x); settt_(io, LUA_VLCF); }

#define setclCvalue(L,obj,x) \
  { TValue *io = (obj); CClosure *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_VCCL)); \
    checkliveness(L,io); }


/*
** Upvalues for Lua closures
** Lua闭包的上值
*/
typedef struct UpVal {
  CommonHeader;
  lu_byte tbc;  /* true if it represents a to-be-closed variable 如果它表示要关闭的变量，则为true */
  TValue *v;  /* points to stack or to its own value 指向堆栈或其自身的值 */
  union {
    struct {  /* (when open) 打开时 */
      struct UpVal *next;  /* linked list 链接列表 */
      struct UpVal **previous;
    } open;
    TValue value;  /* the value (when closed) 值（关闭时）*/
  } u;
} UpVal;



#define ClosureHeader \
	CommonHeader; lu_byte nupvalues; GCObject *gclist

typedef struct CClosure {
  ClosureHeader;
  lua_CFunction f;
  TValue upvalue[1];  /* list of upvalues 上值列表 */
} CClosure;


typedef struct LClosure {
  ClosureHeader;
  struct Proto *p;
  UpVal *upvals[1];  /* list of upvalues 上值列表 */
} LClosure;


typedef union Closure {
  CClosure c;
  LClosure l;
} Closure;


#define getproto(o)	(clLvalue(o)->p)

/* }================================================================== */


/*
** {==================================================================
** Tables 表
** ===================================================================
*/

#define LUA_VTABLE	makevariant(LUA_TTABLE, 0)

#define ttistable(o)		checktag((o), ctb(LUA_VTABLE))

#define hvalue(o)	check_exp(ttistable(o), gco2t(val_(o).gc))

#define sethvalue(L,obj,x) \
  { TValue *io = (obj); Table *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_VTABLE)); \
    checkliveness(L,io); }

#define sethvalue2s(L,o,h)	sethvalue(L,s2v(o),h)


/*
** Nodes for Hash tables: A pack of two TValue's (key-value pairs)
** plus a 'next' field to link colliding entries. The distribution
** of the key's fields ('key_tt' and 'key_val') not forming a proper
** 'TValue' allows for a smaller size for 'Node' both in 4-byte
** and 8-byte alignments.
*/
typedef union Node {
  struct NodeKey {
    TValuefields;  /* fields for value 值的字段 */
    lu_byte key_tt;  /* key type 键类型 */
    int next;  /* for chaining 用于链接 */
    Value key_val;  /* key value 键类型 */
  } u;
  TValue i_val;  /* direct access to node's value as a proper 'TValue' 直接访问节点值作为适当的'TValue' */
} Node;


/* copy a value into a key 将值复制到键中 */
#define setnodekey(L,node,obj) \
	{ Node *n_=(node); const TValue *io_=(obj); \
	  n_->u.key_val = io_->value_; n_->u.key_tt = io_->tt_; \
	  checkliveness(L,io_); }


/* copy a value from a key 从键复制值 */
#define getnodekey(L,obj,node) \
	{ TValue *io_=(obj); const Node *n_=(node); \
	  io_->value_ = n_->u.key_val; io_->tt_ = n_->u.key_tt; \
	  checkliveness(L,io_); }


/*
** About 'alimit': if 'isrealasize(t)' is true, then 'alimit' is the
** real size of 'array'. Otherwise, the real size of 'array' is the
** smallest power of two not smaller than 'alimit' (or zero iff 'alimit'
** is zero); 'alimit' is then used as a hint for #t.
*/

#define BITRAS		(1 << 7)
#define isrealasize(t)		(!((t)->flags & BITRAS))
#define setrealasize(t)		((t)->flags &= cast_byte(~BITRAS))
#define setnorealasize(t)	((t)->flags |= BITRAS)


typedef struct Table {
  CommonHeader;
  lu_byte flags;  /* 1<<p means tagmethod(p) is not present */
  lu_byte lsizenode;  /* log2 of size of 'node' array */
  unsigned int alimit;  /* "limit" of 'array' array */
  TValue *array;  /* array part */
  Node *node;
  Node *lastfree;  /* any free position is before this position */
  struct Table *metatable;
  GCObject *gclist;
} Table;


/*
** Macros to manipulate keys inserted in nodes
** 用于操作插入节点中的关键点的宏
*/
#define keytt(node)		((node)->u.key_tt)
#define keyval(node)		((node)->u.key_val)

#define keyisnil(node)		(keytt(node) == LUA_TNIL)
#define keyisinteger(node)	(keytt(node) == LUA_VNUMINT)
#define keyival(node)		(keyval(node).i)
#define keyisshrstr(node)	(keytt(node) == ctb(LUA_VSHRSTR))
#define keystrval(node)		(gco2ts(keyval(node).gc))

#define setnilkey(node)		(keytt(node) = LUA_TNIL)

#define keyiscollectable(n)	(keytt(n) & BIT_ISCOLLECTABLE)

#define gckey(n)	(keyval(n).gc)
#define gckeyN(n)	(keyiscollectable(n) ? gckey(n) : NULL)


/*
** Dead keys in tables have the tag DEADKEY but keep their original
** gcvalue. This distinguishes them from regular keys but allows them to
** be found when searched in a special way. ('next' needs that to find
** keys removed from a table during a traversal.)
*/
#define setdeadkey(node)	(keytt(node) = LUA_TDEADKEY)
#define keyisdead(node)		(keytt(node) == LUA_TDEADKEY)

/* }================================================================== */



/*
** 'module' operation for hashing (size is always a power of 2)
** 哈希的'module'操作（大小时钟是2的幂）
*/
#define lmod(s,size) \
	(check_exp((size&(size-1))==0, (cast_int((s) & ((size)-1)))))


#define twoto(x)	(1<<(x))
#define sizenode(t)	(twoto((t)->lsizenode))


/* 
   size of buffer for 'luaO_utf8esc' function 
   'luaO_utf8esc'函数的缓冲区大小
*/
#define UTF8BUFFSZ	8

LUAI_FUNC int luaO_utf8esc (char *buff, unsigned long x);
LUAI_FUNC int luaO_ceillog2 (unsigned int x);
LUAI_FUNC int luaO_rawarith (lua_State *L, int op, const TValue *p1,
                             const TValue *p2, TValue *res);
LUAI_FUNC void luaO_arith (lua_State *L, int op, const TValue *p1,
                           const TValue *p2, StkId res);
LUAI_FUNC size_t luaO_str2num (const char *s, TValue *o);
LUAI_FUNC int luaO_hexavalue (int c);
LUAI_FUNC void luaO_tostring (lua_State *L, TValue *obj);
LUAI_FUNC const char *luaO_pushvfstring (lua_State *L, const char *fmt,
                                                       va_list argp);
LUAI_FUNC const char *luaO_pushfstring (lua_State *L, const char *fmt, ...);
LUAI_FUNC void luaO_chunkid (char *out, const char *source, size_t srclen);


#endif


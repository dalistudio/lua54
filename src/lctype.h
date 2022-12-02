/*
** $Id: lctype.h $
** 'ctype' functions for Lua
** Lua 'ctype' 函数
** See Copyright Notice in lua.h
*/

#ifndef lctype_h
#define lctype_h

#include "lua.h"


/*
** WARNING: the functions defined here do not necessarily correspond
** to the similar functions in the standard C ctype.h. They are
** optimized for the specific needs of Lua.
** 警告：此处定义的函数不一定对应于标准 C ctype.h中的类似函数。他们针对 Lua的特定需求进行了优化。
*/

#if !defined(LUA_USE_CTYPE)

#if 'A' == 65 && '0' == 48
/* ASCII case: can use its own tables; faster and fixed 可以使用自己的表格；更快且固定 */
#define LUA_USE_CTYPE	0
#else
/* must use standard C ctype */
#define LUA_USE_CTYPE	1
#endif

#endif


#if !LUA_USE_CTYPE	/* { */

#include <limits.h>

#include "llimits.h"


#define ALPHABIT	0
#define DIGITBIT	1
#define PRINTBIT	2
#define SPACEBIT	3
#define XDIGITBIT	4


#define MASK(B)		(1 << (B))


/*
** add 1 to char to allow index -1 (EOZ)
** 将 1 添加到字符以允许索引 -1
*/
#define testprop(c,p)	(luai_ctype_[(c)+1] & (p))

/*
** 'lalpha' (Lua alphabetic) and 'lalnum' (Lua alphanumeric) both include '_'
** 'lalpha'（Lua字母） 和 'lalnum'（Lua字母数字） 都包含'_'
*/
#define lislalpha(c)	testprop(c, MASK(ALPHABIT))
#define lislalnum(c)	testprop(c, (MASK(ALPHABIT) | MASK(DIGITBIT)))
#define lisdigit(c)	testprop(c, MASK(DIGITBIT))
#define lisspace(c)	testprop(c, MASK(SPACEBIT))
#define lisprint(c)	testprop(c, MASK(PRINTBIT))
#define lisxdigit(c)	testprop(c, MASK(XDIGITBIT))


/*
** In ASCII, this 'ltolower' is correct for alphabetic characters and
** for '.'. That is enough for Lua needs. ('check_exp' ensures that
** the character either is an upper-case letter or is unchanged by
** the transformation, which holds for lower-case letters and '.'.)
** 在 ASCII中，此'ltolower'对于字母字符和'.'是正确的。这足以满足Lua的需求。
** （'check_exp'确保字符要么是大写字母，要么通过转换保持不变，这适用于小写字母和'.'）
*/
#define ltolower(c)  \
  check_exp(('A' <= (c) && (c) <= 'Z') || (c) == ((c) | ('A' ^ 'a')),  \
            (c) | ('A' ^ 'a'))


/* one entry for each character and for -1 (EOZ) 每个字符和-1(EOZ) 对应一个条目 */
LUAI_DDEC(const lu_byte luai_ctype_[UCHAR_MAX + 2];)


#else			/* }{ */

/*
** use standard C ctypes
*/

#include <ctype.h>


#define lislalpha(c)	(isalpha(c) || (c) == '_')
#define lislalnum(c)	(isalnum(c) || (c) == '_')
#define lisdigit(c)	(isdigit(c))
#define lisspace(c)	(isspace(c))
#define lisprint(c)	(isprint(c))
#define lisxdigit(c)	(isxdigit(c))

#define ltolower(c)	(tolower(c))

#endif			/* } */

#endif


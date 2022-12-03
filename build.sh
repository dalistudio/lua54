#!/bin/bash
gcc -O2 linit.c src/lapi.c src/lctype.c src/lfunc.c src/ltable.c src/lundump.c src/ldump.c src/lgc.c src/lmem.c src/lparser.c src/ldebug.c src/lstate.c src/ltm.c src/lvm.c src/lcode.c src/ldo.c src/lobject.c src/lstring.c src/lzio.c src/llex.c src/lopcodes.c src/lauxlib.c src/loadlib.c lib/lbaselib.c lib/lstrlib.c lib/ltablib.c lib/lmathlib.c bin/lua.c -lm -ldl -DLUA_USE_LINUX -o lua

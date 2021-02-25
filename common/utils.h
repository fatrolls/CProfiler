
#ifndef UTILS_H
#define UTILS_H

#include "assert.h"
#define ASSERT assert

#ifndef BOOL
#define BOOL int
#define TRUE 1
#define FALSE 0
#endif

#include "stdio.h"
#include "stdlib.h"
#include "memory.h"

#define u32 unsigned int
#define s32 int
#define u16 unsigned short
#ifdef WIN32
#define s64 signed __int64
#endif

#endif
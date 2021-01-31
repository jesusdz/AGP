#pragma once

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdarg.h>

typedef char           i8;
typedef short          i16;
typedef int            i32;
typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;
typedef float          f32;
typedef double         f64;

struct String
{
    char* str;
    u32   length;
};

String readTextFile(const char *filename);

void freeString(String str);

void outputDebugString(const char* str);

#define ILOG(...)             \
{                                     \
    char logBuffer[1024] = {};        \
    sprintf(logBuffer, __VA_ARGS__);  \
    outputDebugString(logBuffer);     \
}

#define ELOG(...) ILOG(__VA_ARGS__)

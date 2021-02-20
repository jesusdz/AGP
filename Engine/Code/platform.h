#pragma once

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

typedef char                   i8;
typedef short                  i16;
typedef int                    i32;
typedef long long int          i64;
typedef unsigned char          u8;
typedef unsigned short         u16;
typedef unsigned int           u32;
typedef unsigned long long int u64;
typedef float                  f32;
typedef double                 f64;

struct String
{
    char* str;
    u32   length;
};

String ReadTextFile(const char *filename);

void FreeString(String str);

void LogString(const char* str);

#define ILOG(...)             \
{                                     \
char logBuffer[1024] = {};        \
sprintf(logBuffer, __VA_ARGS__);  \
LogString(logBuffer);     \
}

#define ELOG(...) ILOG(__VA_ARGS__)

#define ARRAY_COUNT(array) (sizeof(array)/sizeof(array[0]))

//
// platform.h : This file contains basic platform types and tools. Also, it exposes
// the necessary functions for the Engine to communicate with the Platform layer.
//

#pragma once

#define _CRT_SECURE_NO_WARNINGS
#include <assert.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

#pragma warning(disable : 4267) // conversion from X to Y, possible loss of data

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

struct Arena
{
    u32 size;
    u32 head;
    u8* data;
};

enum MouseButton
{
    LEFT,
    RIGHT,
    MOUSE_BUTTON_COUNT
};

enum Key
{
    K_SPACE,
    K_0, K_1, K_2, K_3, K_4, K_5, K_6, K_7, K_8, K_9,
    K_A, K_B, K_C, K_D, K_E, K_F, K_G, K_H, K_I, K_J, K_K, K_L, K_M,
    K_N, K_O, K_P, K_Q, K_R, K_S, K_T, K_U, K_V, K_W, K_X, K_Y, K_Z,
    K_ENTER, K_ESCAPE,
    KEY_COUNT
};

enum ButtonState
{
    BUTTON_IDLE,
    BUTTON_PRESS,
    BUTTON_PRESSED,
    BUTTON_RELEASE
};

struct Input
{
    glm::vec2   mousePos;
    glm::vec2   mouseDelta;
    ButtonState mouseButtons[MOUSE_BUTTON_COUNT];
    ButtonState keys[KEY_COUNT];
};

struct String
{
    const char* str;
    u32         len;
};

bool   SameString(const char* a, const char *b);
bool   SameString(String a, String b);
String CString(const char *cstr);
String MakeString(Arena& arena, const char *cstr);
#define InternString(arena, str) MakeString(arena, str)
String FormatString(Arena& arena, const char* format, ...);

String MakePath(Arena& arena, String dir, String filename);
String GetDirectoryPart(String path);

/**
 * Reads a whole file and returns a string with its contents. The returned string
 * is temporary and should be copied if it needs to persist for several frames.
 */
String ReadTextFile(const char *filepath);

/**
 * It retrieves a timestamp indicating the last time the file was modified.
 * Can be useful in order to check for file modifications to implement hot reloads.
 */
u64 GetFileLastWriteTimestamp(const char *filepath);

/**
 * It logs a string to whichever outputs are configured in the platform layer.
 * By default, the string is printed in the output console of VisualStudio.
 */
void LogString(const char* str);

/**
 * It logs a formatted string to whichever outputs are configured in the platform layer.
 * By default, the string is printed in the output console of VisualStudio.
 */
void LogFormattedString(const char* format, ...);

#define ILOG(...) LogFormattedString(__VA_ARGS__)
#define ELOG(...) ILOG(__VA_ARGS__)

#define ARRAY_COUNT(array) (sizeof(array)/sizeof(array[0]))

#define ASSERT(condition, message) assert((condition) && message)
#define INVALID_CODE_PATH(message) ASSERT(false, message)

#define KB(count) (1024*(count))
#define MB(count) (1024*KB(count))
#define GB(count) (1024*MB(count))

#define PI  3.14159265359f
#define TAU 6.28318530718f

#define MAKE_DWORD(high, low) (((high&0xffff)<<16) | ((low)&0xffff))
#define LOW_WORD(word)        ((word>>0 )&0xffff)
#define HIGH_WORD(word)       ((word>>16)&0xffff)

void MemCopy(void* dst, const void* src, u32 byteCount);

Arena CreateArena(u32 sizeInBytes);
void  DestroyArena(Arena& arena);
void  ResetArena(Arena& arena);

void* PushSize(Arena& arena, u32 byteCount);
void* PushData(Arena& arena, const void* data, u32 byteCount);
u8*   PushChar(Arena& arena, u8 c);

#define PUSH_ARRAY(arena, type, count) (type*)PushSize(arena, sizeof(type) * count)

Arena& GetGlobalScratchArena();

struct ScratchArena : public Arena
{
    u32 prevHead;

    ScratchArena()
    {
        Arena& globalScratchArena = GetGlobalScratchArena();
        prevHead = globalScratchArena.head;
        size = MB(1);
        head = 0;
        data = globalScratchArena.data + prevHead;
        PushSize(globalScratchArena, size);
    }

    ~ScratchArena()
    {
        Arena& globalScratchArena = GetGlobalScratchArena();
        globalScratchArena.head = prevHead;
    }
};


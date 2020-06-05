
struct Vec2
{
    union {
        struct { float x; float y; };
        struct { float u; float v; };
        struct { float r; float g; };
        float data[2] = {};
    };
};

struct Vec3
{
    union {
        struct { float x; float y; float z; };
        struct { float u; float v; float s; };
        struct { float r; float g; float b; };
        float data[3] = {};
    };
};

struct Vec4
{
    union {
        struct { float x; float y; float z; float w; };
        struct { float u; float v; float s; float t; };
        struct { float r; float g; float b; float a; };
        float data[4] = {};
    };
};

inline Vec2 V2(float a, float b) {
    Vec2 res = {a, b};
    return res;
}

inline Vec3 V3(float a, float b, float c) {
    Vec3 res = {a, b, c};
    return res;
}

inline Vec3 V3(const Vec2 &v, float c) {
    Vec3 res = {v.x, v.y, c};
    return res;
}

inline Vec4 V4(float a, float b, float c, float d) {
    Vec4 res = {a, b, c, d};
    return res;
}

inline Vec4 V4(const Vec3 &v, float d) {
    Vec4 res = {v.x, v.y, v.z, d};
    return res;
}

inline Vec4 V4(const Vec2 &v1, const Vec2 &v2) {
    Vec4 res = {v1.x, v1.y, v2.x, v2.y};
    return res;
}

inline Vec2 operator+(const Vec2 &a, const Vec2 &b) {
    Vec2 res = {a.x + b.x, a.y + b.y};
    return res;
}

inline Vec3 operator+(const Vec3 &a, const Vec3 &b) {
    Vec3 res = {a.x + b.x, a.y + b.y, a.z + b.z};
    return res;
}

inline Vec4 operator+(const Vec4 &a, const Vec4 &b) {
    Vec4 res = {a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
    return res;
}

inline Vec2 operator-(const Vec2 &a, const Vec2 &b) {
    Vec2 res = {a.x - b.x, a.y - b.y};
    return res;
}

inline Vec3 operator-(const Vec3 &a, const Vec3 &b) {
    Vec3 res = {a.x - b.x, a.y - b.y, a.z - b.z};
    return res;
}

inline Vec4 operator-(const Vec4 &a, const Vec4 &b) {
    Vec4 res = {a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
    return res;
}

struct GLDebugVertex33
{
    Vec3 a1;
    Vec3 a2;
};

struct GLDebugVertex32
{
    Vec3 a1;
    Vec2 a2;
};

struct GLDebugLine
{
    GLDebugVertex33 v1; // pos and color
    GLDebugVertex33 v2; // pos and color
};

struct GLDebugQuad
{
    GLDebugVertex32 vertices[6]; // pos and texcoord
};

#define GL_DEBUG_CONTEXT_MAX_LINES 9999
#define GL_DEBUG_CONTEXT_MAX_QUADS 1024

struct GLDebugContext
{
    Vec3 col;
    GLDebugLine lines[GL_DEBUG_CONTEXT_MAX_LINES];
    GLDebugQuad quads[GL_DEBUG_CONTEXT_MAX_QUADS];
    unsigned int numLines = 0;
    unsigned int numQuads = 0;

    unsigned int viewportWidth;
    unsigned int viewportHeight;
    Vec2 textCursorPos;
    Vec2 textAdvance;
    Vec2 glyphSize;
    float textCursorBaseX = 10.0;
};

void GLDebugInit(GLDebugContext *ctx, unsigned char *pixels, int width, int height);
void GLDebugClear(GLDebugContext *ctx);
void GLDebugSetColor(GLDebugContext *ctx, const Vec3 &color);
void GLDebugSetTextCursorPos(GLDebugContext *ctx, const Vec2 &cursorPos);
void GLDebugPushLine(GLDebugContext *ctx, const Vec3 &point1, const Vec3 &point2);
void GLDebugPushQuad(GLDebugContext *ctx, const Vec2 &topLeftCorner, const Vec2 &size, const Vec2 &topLeftTexCoord, const Vec2 &sizeTexCoord);
void GLDebugPushString(GLDebugContext *ctx, const char *text);
void GLDebugCleanup(GLDebugContext *ctx);

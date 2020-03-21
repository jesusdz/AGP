
struct GLDebugVec3
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

struct GLDebugRGB
{
    float r = 1.0f;
    float g = 1.0f;
    float b = 1.0f;
};

struct GLDebugVertex
{
    GLDebugVec3 pos;
    GLDebugRGB col;
};

struct GLDebugLine
{
    GLDebugVertex v1;
    GLDebugVertex v2;
};

#define GL_DEBUG_CONTEXT_MAX_LINES 9999

struct GLDebugContext
{
    GLDebugRGB col;
    GLDebugLine lines[GL_DEBUG_CONTEXT_MAX_LINES];
    unsigned int numLines = 0;
};

void GLDebugClear(GLDebugContext *ctx);
void GLDebugSetColor(GLDebugContext *ctx, float r, float g, float b);
void GLDebugAddLine(GLDebugContext *ctx, float x1, float y1, float z1, float x2, float y2, float z2);

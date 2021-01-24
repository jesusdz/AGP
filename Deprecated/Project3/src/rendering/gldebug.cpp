#include "gldebug.h"
#include <QtGlobal>
#include <stdio.h>

#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC
#include "../util/stb_truetype.h"
//#include "stb_rect_pack.h" // -- optional, but you really want it

static stbtt_pack_context pack_context;
static stbtt_packedchar chardata_for_range[256];
static int font_image_width = 64;
static int font_image_height = 64;

void GLDebugInit(GLDebugContext *ctx, unsigned char *pixels, int width, int height)
{
    FILE *file = fopen("res/fonts/dos.ttf", "rb");
    Q_ASSERT(file != nullptr);

    font_image_width = width;
    font_image_height = height;

    unsigned int buffer_size = 1024*1024;
    unsigned char *font_data = (unsigned char*) malloc(buffer_size);
    fread(font_data, 1, buffer_size, file);

//    stbtt_InitFont(&font, font_data, stbtt_GetFontOffsetForIndex(font_data,0));
//    float scale = stbtt_ScaleForPixelHeight(&font, 16);
//    int ascent, descent, linegap;
//    stbtt_GetFontVMetrics(&font, &ascent,&descent,&linegap);
//    int yadvance = *ascent - *descent + *linegap;

    int stride = width * sizeof(unsigned char);
    int padding = 1; // maybe 0 if we don't use bilinear filtering
    if (stbtt_PackBegin(&pack_context, pixels, font_image_width, font_image_height, stride, padding, nullptr))
    {
        stbtt_PackSetOversampling(&pack_context, 1, 1); // default values are 1 1
        int font_index = 0;
        float font_size = STBTT_POINT_SIZE(16);
        int first_unicode_char = 0;
        int char_count = 256;
        stbtt_PackFontRange(&pack_context, font_data, font_index, font_size, first_unicode_char, char_count, chardata_for_range);
    }
    else
    {
        // failure
        int foo = 0;
    }

    free(font_data);
    fclose(file);
}

void GLDebugCleanup(GLDebugContext *ctx)
{
    stbtt_PackEnd(&pack_context);
}

void GLDebugClear(GLDebugContext *ctx)
{
    ctx->col = V3(1.0, 1.0, 1.0);
    ctx->numLines = 0;
    ctx->numQuads = 0;

    ctx->textCursorPos = V2(10.0, ctx->viewportHeight - 20.0);
    ctx->textAdvance = V2(10.0, 20.0);
    ctx->glyphSize = V2(8.0, 16.0);
    ctx->textCursorBaseX = 10.0;
}

void GLDebugSetColor(GLDebugContext *ctx, const Vec3 &color)
{
    ctx->col = color;
}

void GLDebugSetTextCursorPos(GLDebugContext *ctx, const Vec2 &cursorPos)
{
    ctx->textCursorPos = cursorPos;
}

void GLDebugPushLine(GLDebugContext *ctx, const Vec3 &a, const Vec3 &b)
{
    Q_ASSERT(ctx->numLines < GL_DEBUG_CONTEXT_MAX_LINES);
    GLDebugVertex33 vertex1 = { a, ctx->col };
    GLDebugVertex33 vertex2 = { b, ctx->col };
    GLDebugLine line = {vertex1, vertex2};
    ctx->lines[ctx->numLines] = line;
    ctx->numLines++;
}

void GLDebugPushQuad(GLDebugContext *ctx,
                     const Vec2 &c1, const Vec2 &c2,   // top-left - bottom-right
                     const Vec2 &uv1, const Vec2 &uv2) // top-left - bottom-right
{
    Q_ASSERT(ctx->numQuads < GL_DEBUG_CONTEXT_MAX_QUADS);
    GLDebugQuad &quad = ctx->quads[ctx->numQuads];
    quad.vertices[0].a1 = V3(c1.x, c1.y, 0.0f);
    quad.vertices[1].a1 = V3(c1.x, c2.y, 0.0f);
    quad.vertices[2].a1 = V3(c2.x, c2.y, 0.0f);
    quad.vertices[3].a1 = V3(c1.x, c1.y, 0.0f);
    quad.vertices[4].a1 = V3(c2.x, c2.y, 0.0f);
    quad.vertices[5].a1 = V3(c2.x, c1.y, 0.0f);
    quad.vertices[0].a2 = V2(uv1.u, uv1.v);
    quad.vertices[1].a2 = V2(uv1.u, uv2.v);
    quad.vertices[2].a2 = V2(uv2.u, uv2.v);
    quad.vertices[3].a2 = V2(uv1.u, uv1.v);
    quad.vertices[4].a2 = V2(uv2.u, uv2.v);
    quad.vertices[5].a2 = V2(uv2.u, uv1.v);
    ctx->numQuads++;
}

void GLDebugPushString(GLDebugContext *ctx, const char *text)
{
    const char *src = text;

    int align_to_integer = 1;
    stbtt_aligned_quad quad;
    float xpos = ctx->textCursorPos.x;
    float ypos = ctx->textCursorPos.y;
    Vec2 topLeft;
    Vec2 botRight;
    Vec2 topLeftTexCoord;
    Vec2 botRightTexCoord;

    while (*src)
    {
        int char_index = (int)*src;
        stbtt_GetPackedQuad(chardata_for_range, font_image_width, font_image_height, char_index, &xpos, &ypos, &quad, align_to_integer);
        topLeft = V2(quad.x0, -quad.y0 + 2*ypos);
        botRight = V2(quad.x1, -quad.y1 + 2*ypos);
        topLeftTexCoord = V2(quad.s0, quad.t0);
        botRightTexCoord = V2(quad.s1, quad.t1);
        GLDebugPushQuad(ctx, topLeft, botRight, topLeftTexCoord, botRightTexCoord);
        src++;
    }
    ctx->textCursorPos.x  = ctx->textCursorBaseX;
    ctx->textCursorPos.y -= ctx->textAdvance.y;
}

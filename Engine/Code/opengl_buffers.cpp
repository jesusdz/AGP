
static GLenum GLenumFromBufferType[] = {
    GL_UNIFORM_BUFFER,
    GL_ARRAY_BUFFER,
    GL_ELEMENT_ARRAY_BUFFER
};
CASSERT(ARRAY_COUNT(GLenumFromBufferType) == BufferType_Count, "");

static GLenum GLenumFromBufferUsage[] = {
    GL_STATIC_DRAW,
    GL_STREAM_DRAW
};
CASSERT(ARRAY_COUNT(GLenumFromBufferUsage) == BufferUsage_Count, "");

static GLenum GLenumFromAccess[] = {
    GL_READ_ONLY,
    GL_WRITE_ONLY,
    GL_READ_WRITE
};
CASSERT(ARRAY_COUNT(GLenumFromAccess) == Access_Count, "");


static Buffer OpenGL_CreateBuffer(Device& device, u32 size, BufferType type, BufferUsage usage)
{
    Buffer buffer = {};
    buffer.size = size;
    buffer.type = type;

    const GLenum typeEnum = GLenumFromBufferType[type];
    const GLenum usageEnum = GLenumFromBufferUsage[usage];
    glGenBuffers(1, &buffer.handle);
    glBindBuffer(typeEnum, buffer.handle);
    glBufferData(typeEnum, buffer.size, NULL, usageEnum);
    glBindBuffer(typeEnum, 0);

    return buffer;
}

static void OpenGL_MapBuffer(Buffer& buffer, Access access)
{
    const GLenum typeEnum = GLenumFromBufferType[buffer.type];
    const GLenum accessEnum = GLenumFromAccess[access];
    glBindBuffer(typeEnum, buffer.handle);
    buffer.data = (u8*)glMapBuffer(typeEnum, accessEnum);
}

static void OpenGL_UnmapBuffer(Buffer& buffer)
{
    const GLenum typeEnum = GLenumFromBufferType[buffer.type];
    glBindBuffer(typeEnum, buffer.handle);
    glUnmapBuffer(typeEnum);
}

static void OpenGL_BindBuffer(const Buffer& buffer)
{
    if (buffer.handle)
    {
        const GLenum typeEnum = GLenumFromBufferType[buffer.type];
        glBindBuffer(typeEnum, buffer.handle);
    }
}


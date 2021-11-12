
#if USE_GFX_API_OPENGL
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
#endif


static bool IsPowerOf2(u32 value)
{
    return value && !(value & (value - 1));
}

static u32 Align(u32 value, u32 alignment)
{
    return (value + alignment - 1) & ~(alignment - 1);
}

Buffer CreateBufferRaw(u32 size, BufferType type, BufferUsage usage)
{
    Buffer buffer = {};

#if USE_GFX_API_OPENGL
    buffer.size = size;
    buffer.type = type;

    const GLenum typeEnum = GLenumFromBufferType[type];
    const GLenum usageEnum = GLenumFromBufferUsage[usage];
    glGenBuffers(1, &buffer.handle);
    glBindBuffer(typeEnum, buffer.handle);
    glBufferData(typeEnum, buffer.size, NULL, usageEnum);
    glBindBuffer(typeEnum, 0);
#endif

    return buffer;
}

#define CreateConstantBufferRaw(size)      CreateBufferRaw(size, BufferType_Uniforms, BufferUsage_StreamDraw)
#define CreateStaticVertexBufferRaw(size)  CreateBufferRaw(size, BufferType_Vertices, BufferUsage_StaticDraw)
#define CreateStaticIndexBufferRaw(size)   CreateBufferRaw(size, BufferType_Indices, BufferUsage_StaticDraw)
#define CreateDynamicVertexBufferRaw(size) CreateBufferRaw(size, BufferType_Vertices, BufferUsage_StreamDraw)

u32 CreateConstantBuffer(Device& device, u32 size)
{
    ASSERT(device.vertexBufferCount < ARRAY_COUNT(device.vertexBuffers), "Max number of vertex buffers reached");
    Buffer buffer = CreateBufferRaw(size, BufferType_Uniforms, BufferUsage_StreamDraw);
    device.vertexBuffers[device.vertexBufferCount++] = buffer;
    return device.vertexBufferCount - 1;
}

u32 CreateStaticVertexBuffer(Device& device, u32 size)
{
    ASSERT(device.vertexBufferCount < ARRAY_COUNT(device.vertexBuffers), "Max number of vertex buffers reached");
    Buffer buffer = CreateBufferRaw(size, BufferType_Vertices, BufferUsage_StaticDraw);
    device.vertexBuffers[device.vertexBufferCount++] = buffer;
    return device.vertexBufferCount - 1;
}

u32 CreateDynamicVertexBuffer(Device& device, u32 size)
{
    ASSERT(device.vertexBufferCount < ARRAY_COUNT(device.vertexBuffers), "Max number of vertex buffers reached");
    Buffer buffer = CreateBufferRaw(size, BufferType_Vertices, BufferUsage_StreamDraw);
    device.vertexBuffers[device.vertexBufferCount++] = buffer;
    return device.vertexBufferCount - 1;
}

u32 CreateStaticIndexBuffer(Device& device, u32 size)
{
    ASSERT(device.indexBufferCount < ARRAY_COUNT(device.indexBuffers), "Max number of index buffers reached");
    Buffer buffer = CreateBufferRaw(size, BufferType_Indices, BufferUsage_StaticDraw);
    device.indexBuffers[device.indexBufferCount++] = buffer;
    return device.indexBufferCount - 1;
}

void BindBuffer(const Buffer& buffer)
{
#if USE_GFX_API_OPENGL
    if (buffer.handle)
    {
        const GLenum typeEnum = GLenumFromBufferType[buffer.type];
        glBindBuffer(typeEnum, buffer.handle);
    }
#endif
}

void MapBuffer(Buffer& buffer, Access access)
{
#if USE_GFX_API_OPENGL
    const GLenum typeEnum = GLenumFromBufferType[buffer.type];
    const GLenum accessEnum = GLenumFromAccess[access];
    glBindBuffer(typeEnum, buffer.handle);
    buffer.data = (u8*)glMapBuffer(typeEnum, accessEnum);
    buffer.head = 0;
#endif
}

void UnmapBuffer(Buffer& buffer)
{
#if USE_GFX_API_OPENGL
    const GLenum typeEnum = GLenumFromBufferType[buffer.type];
    glBindBuffer(typeEnum, buffer.handle);
    glUnmapBuffer(typeEnum);
#endif
    buffer.data = 0;
    buffer.head = 0;
}

void AlignHead(Buffer& buffer, u32 alignment)
{
    ASSERT(IsPowerOf2(alignment), "The alignment must be a power of 2");
    buffer.head = Align(buffer.head, alignment);
}

void PushAlignedData(Buffer& buffer, const void* data, u32 size, u32 alignment)
{
#if USE_GFX_API_OPENGL
    ASSERT(buffer.data != NULL, "The buffer must be mapped first");
    AlignHead(buffer, alignment);
    ASSERT(buffer.head + size <= buffer.size, "Trying to push data out of bounds");
    MemCopy((u8*)buffer.data + buffer.head, data, size);
    buffer.head += size;
#endif
}

#define BufferPushData(buffer, data, size) PushAlignedData(buffer, data, size, 1)
#define BufferPushUInt(buffer, value) { u32 v = value; PushAlignedData(buffer, &v, sizeof(v), 4); }
#define BufferPushVec3(buffer, value) PushAlignedData(buffer, value_ptr(value), sizeof(value), sizeof(vec4))
#define BufferPushVec4(buffer, value) PushAlignedData(buffer, value_ptr(value), sizeof(value), sizeof(vec4))
#define BufferPushMat3(buffer, value) PushAlignedData(buffer, value_ptr(value), sizeof(value), sizeof(vec4))
#define BufferPushMat4(buffer, value) PushAlignedData(buffer, value_ptr(value), sizeof(value), sizeof(vec4))


// Sequence of constant buffers

Buffer& GetCurrentConstantBuffer( Device& device )
{
    ASSERT( device.currentConstantBufferIdx <= device.constantBufferCount, "Current buffer out of bounds" );
    while ( device.currentConstantBufferIdx >= device.constantBufferCount ) {
        device.constantBuffers[device.constantBufferCount++] = CreateConstantBufferRaw(device.uniformBufferMaxSize);
    }
    return device.constantBuffers[ device.currentConstantBufferIdx ];
}

void BeginConstantBufferRecording( Device& device )
{
    device.currentConstantBufferIdx = 0;
    Buffer& buffer = GetCurrentConstantBuffer(device);
    MapBuffer( buffer, Access_Write );
}

Buffer& GetMappedConstantBufferForRange( Device& device, u32 sizeInBytes )
{
    Buffer& buffer = GetCurrentConstantBuffer(device);

    AlignHead(buffer, device.uniformBufferAlignment);

    if ( buffer.head + sizeInBytes <= buffer.size )
    {
        return buffer;
    }
    else
    {
        UnmapBuffer(buffer);
        ASSERT( device.currentConstantBufferIdx < device.constantBufferCount, "Constant buffer memory is full" );
        device.currentConstantBufferIdx++;
        Buffer& nextBuffer = GetCurrentConstantBuffer(device);
        MapBuffer( nextBuffer, Access_Write );
        return nextBuffer;
    }
}

void EndConstantBufferRecording( Device& device )
{
    Buffer& buffer = device.constantBuffers[device.currentConstantBufferIdx];
    UnmapBuffer(buffer);
}


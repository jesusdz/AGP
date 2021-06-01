
static bool IsPowerOf2(u32 value)
{
    return value && !(value & (value - 1));
}

static u32 Align(u32 value, u32 alignment)
{
    return (value + alignment - 1) & ~(alignment - 1);
}

Buffer CreateBufferRaw(u32 size, GLenum type, GLenum usage)
{
    Buffer buffer = {};
    buffer.size = size;
    buffer.type = type;

    glGenBuffers(1, &buffer.handle);
    glBindBuffer(type, buffer.handle);
    glBufferData(type, buffer.size, NULL, usage);
    glBindBuffer(type, 0);

    return buffer;
}

#define CreateConstantBufferRaw(size)      CreateBufferRaw(size, GL_UNIFORM_BUFFER,       GL_STREAM_DRAW)
#define CreateStaticVertexBufferRaw(size)  CreateBufferRaw(size, GL_ARRAY_BUFFER,         GL_STATIC_DRAW)
#define CreateStaticIndexBufferRaw(size)   CreateBufferRaw(size, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW)
#define CreateDynamicVertexBufferRaw(size) CreateBufferRaw(size, GL_ARRAY_BUFFER,         GL_STREAM_DRAW)

u32 CreateConstantBuffer(Device& device, u32 size)
{
    ASSERT(device.vertexBufferCount < ARRAY_COUNT(device.vertexBuffers), "Max number of vertex buffers reached");
    Buffer buffer = CreateBufferRaw(size, GL_UNIFORM_BUFFER, GL_STREAM_DRAW);
    device.vertexBuffers[device.vertexBufferCount++] = buffer;
    return device.vertexBufferCount - 1;
}

u32 CreateStaticVertexBuffer(Device& device, u32 size)
{
    ASSERT(device.vertexBufferCount < ARRAY_COUNT(device.vertexBuffers), "Max number of vertex buffers reached");
    Buffer buffer = CreateBufferRaw(size, GL_ARRAY_BUFFER, GL_STATIC_DRAW);
    device.vertexBuffers[device.vertexBufferCount++] = buffer;
    return device.vertexBufferCount - 1;
}

u32 CreateDynamicVertexBuffer(Device& device, u32 size)
{
    ASSERT(device.vertexBufferCount < ARRAY_COUNT(device.vertexBuffers), "Max number of vertex buffers reached");
    Buffer buffer = CreateBufferRaw(size, GL_ARRAY_BUFFER, GL_STREAM_DRAW);
    device.vertexBuffers[device.vertexBufferCount++] = buffer;
    return device.vertexBufferCount - 1;
}

u32 CreateStaticIndexBuffer(Device& device, u32 size)
{
    ASSERT(device.indexBufferCount < ARRAY_COUNT(device.indexBuffers), "Max number of index buffers reached");
    Buffer buffer = CreateBufferRaw(size, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW);
    device.indexBuffers[device.indexBufferCount++] = buffer;
    return device.indexBufferCount - 1;
}

void BindBuffer(const Buffer& buffer)
{
    if (buffer.handle)
        glBindBuffer(buffer.type, buffer.handle);
}

void MapBuffer(Buffer& buffer, GLenum access)
{
    glBindBuffer(buffer.type, buffer.handle);
    buffer.data = (u8*)glMapBuffer(buffer.type, access);
    buffer.head = 0;
}

void UnmapBuffer(Buffer& buffer)
{
    glBindBuffer(buffer.type, buffer.handle);
    glUnmapBuffer(buffer.type);
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
    ASSERT(buffer.data != NULL, "The buffer must be mapped first");
    AlignHead(buffer, alignment);
    ASSERT(buffer.head + size <= buffer.size, "Trying to push data out of bounds");
    MemCopy((u8*)buffer.data + buffer.head, data, size);
    buffer.head += size;
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
    MapBuffer( buffer, GL_WRITE_ONLY );
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
        MapBuffer( nextBuffer, GL_WRITE_ONLY );
        return nextBuffer;
    }
}

void EndConstantBufferRecording( Device& device )
{
    Buffer& buffer = device.constantBuffers[device.currentConstantBufferIdx];
    UnmapBuffer(buffer);
}


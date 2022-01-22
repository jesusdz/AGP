
static bool IsPowerOf2(u32 value)
{
    return value && !(value & (value - 1));
}

static u32 Align(u32 value, u32 alignment)
{
    return (value + alignment - 1) & ~(alignment - 1);
}

static Buffer CreateBufferRaw(Device& device, u32 size, BufferType type, BufferUsage usage)
{
    Buffer buffer = {};

#if USE_GFX_API_OPENGL
    buffer = OpenGL_CreateBuffer(device, size, type, usage);
#elif USE_GFX_API_METAL
    buffer = Metal_CreateBuffer(device, size, type, usage);
#endif

    return buffer;
}

#define CreateConstantBufferRaw(size)      CreateBufferRaw(device, size, BufferType_Uniforms, BufferUsage_StreamDraw)
#define CreateStaticVertexBufferRaw(size)  CreateBufferRaw(device, size, BufferType_Vertices, BufferUsage_StaticDraw)
#define CreateStaticIndexBufferRaw(size)   CreateBufferRaw(device, size, BufferType_Indices, BufferUsage_StaticDraw)
#define CreateDynamicVertexBufferRaw(size) CreateBufferRaw(device, size, BufferType_Vertices, BufferUsage_StreamDraw)

u32 CreateConstantBuffer(Device& device, u32 size)
{
    ASSERT(device.vertexBufferCount < ARRAY_COUNT(device.vertexBuffers), "Max number of vertex buffers reached");
    Buffer buffer = CreateBufferRaw(device, size, BufferType_Uniforms, BufferUsage_StreamDraw);
    device.vertexBuffers[device.vertexBufferCount++] = buffer;
    return device.vertexBufferCount - 1;
}

u32 CreateStaticVertexBuffer(Device& device, u32 size)
{
    ASSERT(device.vertexBufferCount < ARRAY_COUNT(device.vertexBuffers), "Max number of vertex buffers reached");
    Buffer buffer = CreateBufferRaw(device, size, BufferType_Vertices, BufferUsage_StaticDraw);
    device.vertexBuffers[device.vertexBufferCount++] = buffer;
    return device.vertexBufferCount - 1;
}

u32 CreateDynamicVertexBuffer(Device& device, u32 size)
{
    ASSERT(device.vertexBufferCount < ARRAY_COUNT(device.vertexBuffers), "Max number of vertex buffers reached");
    Buffer buffer = CreateBufferRaw(device, size, BufferType_Vertices, BufferUsage_StreamDraw);
    device.vertexBuffers[device.vertexBufferCount++] = buffer;
    return device.vertexBufferCount - 1;
}

u32 CreateStaticIndexBuffer(Device& device, u32 size)
{
    ASSERT(device.indexBufferCount < ARRAY_COUNT(device.indexBuffers), "Max number of index buffers reached");
    Buffer buffer = CreateBufferRaw(device, size, BufferType_Indices, BufferUsage_StaticDraw);
    device.indexBuffers[device.indexBufferCount++] = buffer;
    return device.indexBufferCount - 1;
}

void BindBuffer(const Buffer& buffer)
{
#if USE_GFX_API_OPENGL
    OpenGL_BindBuffer(buffer);
#elif USE_GFX_API_METAL
    Metal_BindBuffer(buffer);
#endif
}

void MapBuffer(Buffer& buffer, Access access)
{
    ASSERT(!buffer.data, "The buffer is already mapped");
#if USE_GFX_API_OPENGL
    OpenGL_MapBuffer(buffer, access);
#elif USE_GFX_API_METAL
    Metal_MapBuffer(buffer, access);
#endif
    buffer.head = 0;
}

void UnmapBuffer(Buffer& buffer)
{
    ASSERT(buffer.data, "The buffer is not mapped");
#if USE_GFX_API_OPENGL
    OpenGL_UnmapBuffer(buffer);
#elif USE_GFX_API_METAL
    Metal_UnmapBuffer(buffer);
#endif
    buffer.data = 0;
    buffer.head = 0;
}

bool IsBufferMapped(const Buffer& buffer)
{
    const bool isMapped = (buffer.data != NULL);
    return isMapped;
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
        if (IsBufferMapped(buffer)) UnmapBuffer(buffer);
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


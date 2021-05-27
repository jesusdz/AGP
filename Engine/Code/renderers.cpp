
// SORTING ALGORITHMS

u32 Partition(u64* begin, u64* end)
{
    u64 pivot = *end;
    u32 endIndex = end - begin; // pivot index

    u32 pivotIndex = 0;
    for (u32 i = 0; i < endIndex; ++i)
    {
        if (*(begin + i) < pivot)
        {
            u64 tmp = *(begin + pivotIndex);
            *(begin + pivotIndex) = *(begin + i);
            *(begin + i) = tmp;
            pivotIndex++;
        }
    }

    u64 tmp = *(begin + pivotIndex);
    *(begin + pivotIndex) = *(begin + endIndex);
    *(begin + endIndex) = tmp;
    return pivotIndex;
}

void QSort(u64* begin, u64* end)
{
    if (begin < end)
    {
        u32 pi = Partition(begin, end);
        QSort(begin, begin + pi - 1);
        QSort(begin + pi + 1, end);
    }
}



// FORWARD RENDERER

void ForwardShading_Init(Device& device, ForwardRenderData& forwardRenderData)
{
    forwardRenderData.programIdx = LoadProgram(device, CString("shaders.glsl"), CString("FORWARD_RENDER"));
    Program& forwardRenderProgram = device.programs[forwardRenderData.programIdx];
    forwardRenderData.uniLoc_Albedo = glGetUniformLocation(forwardRenderProgram.handle, "uAlbedo");
    forwardRenderData.localParamsBlockSize = KB(1); // TODO: Get the size from the shader?
    forwardRenderData.instancingBufferIdx = CreateDynamicVertexBuffer(device, MB(1));
}

void ForwardShading_Update(Device& device, const Scene& scene, const Embedded& embedded, ForwardRenderData& forwardRenderData)
{
    Program& program = device.programs[forwardRenderData.programIdx];

    forwardRenderData.renderPrimitiveCount = 0;

#if defined(USE_INSTANCING)
    Buffer& instancingBuffer = device.vertexBuffers[forwardRenderData.instancingBufferIdx];
    MapBuffer(instancingBuffer, GL_WRITE_ONLY);

    static u64* renderPrimitivesToSort = new u64[MAX_RENDER_PRIMITIVES]; // TODO: this is a mem leak, put this in another place
    u32 renderPrimitivesToSortCount = 0;

    for (u32 entityIdx = 0; entityIdx < scene.entityCount; ++entityIdx)
    {
        const Entity& entity = scene.entities[entityIdx];
        const u32 meshIdx = HIGH_WORD(entity.meshSubmeshIdx);
        const u32 submeshIdx = LOW_WORD(entity.meshSubmeshIdx);

        switch (entity.type)
        {
            case EntityType_Mesh:
                {
                    u64 rp = ((u64)meshIdx << 48) | ((u64)submeshIdx << 32) | (entityIdx);
                    renderPrimitivesToSort[renderPrimitivesToSortCount++] = rp;
                }
                break;

            case EntityType_Model:
                {
                    Mesh& mesh = device.meshes[meshIdx];

                    for (u32 submeshIdx = 0; submeshIdx < mesh.submeshes.size(); ++submeshIdx)
                    {
                        u64 rp = ((u64)meshIdx << 48) | ((u64)submeshIdx << 32) | (entityIdx);
                        renderPrimitivesToSort[renderPrimitivesToSortCount++] = rp;
                    }
                }
                break;
        }
    }

    QSort((u64*)renderPrimitivesToSort, (u64*)renderPrimitivesToSort + renderPrimitivesToSortCount - 1);

    u16 prevMeshIdx = 0xffff;
    u16 prevSubmeshIdx = 0xffff;

    for (u32 primIdx = 0; primIdx < renderPrimitivesToSortCount; ++primIdx)
    {
        u64 rp = renderPrimitivesToSort[primIdx];
        u32 meshIdx    = (rp >> 48) & 0xffff;
        u32 submeshIdx = (rp >> 32) & 0xffff;
        u32 entityIdx  = (rp >>  0) & 0xffffffff;

        const Entity& entity = scene.entities[entityIdx];

        if (meshIdx != prevMeshIdx || submeshIdx != prevSubmeshIdx)
        {
            Mesh& mesh = device.meshes[meshIdx];

            RenderPrimitive renderPrimitive = {};
            renderPrimitive.vaoHandle = FindVAO(device, meshIdx, submeshIdx, program);

            if (submeshIdx < mesh.materialIndices.size())
            {
                u32 submeshMaterialIdx = mesh.materialIndices[submeshIdx];
                Material& submeshMaterial = device.materials[submeshMaterialIdx];
                renderPrimitive.albedoTextureHandle = device.textures[submeshMaterial.albedoTextureIdx].handle;
            }
            else
            {
                Material& defaultMaterial = device.materials[embedded.defaultMaterialIdx];
                renderPrimitive.albedoTextureHandle = device.textures[defaultMaterial.albedoTextureIdx].handle;
            }

            Submesh& submesh = mesh.submeshes[submeshIdx];
            renderPrimitive.indexCount = submesh.indexCount;
            renderPrimitive.indexOffset = submesh.indexOffset;

            renderPrimitive.instanceCount = 0;
            renderPrimitive.instancingOffset = instancingBuffer.head;

            forwardRenderData.renderPrimitives[forwardRenderData.renderPrimitiveCount++] = renderPrimitive;

            prevMeshIdx = meshIdx;
            prevSubmeshIdx = submeshIdx;
        }

        RenderPrimitive& renderPrimitive = forwardRenderData.renderPrimitives[forwardRenderData.renderPrimitiveCount - 1];

        const mat4&   world  = entity.worldMatrix;
        const mat4    worldViewProjection = scene.mainCamera.viewProjectionMatrix * world;
        BufferPushMat4(instancingBuffer, world);
        BufferPushMat4(instancingBuffer, worldViewProjection);
        renderPrimitive.instanceCount++;
    }

    UnmapBuffer(instancingBuffer);

#else

    for (u32 entityIdx = 0; entityIdx < scene.entityCount; ++entityIdx)
    {
        const Entity& entity = scene.entities[entityIdx];
        const mat4&   world  = entity.worldMatrix;
        const mat4    worldViewProjection = scene.mainCamera.viewProjectionMatrix * world;
        const u32     meshIdx = HIGH_WORD(entity.meshSubmeshIdx);
        const u32     submeshIdx = LOW_WORD(entity.meshSubmeshIdx);

        RenderPrimitive renderPrimitive = {};
        renderPrimitive.entityIdx = entityIdx;

        Buffer& constantBuffer = GetMappedConstantBufferForRange( device, forwardRenderData.localParamsBlockSize );
        renderPrimitive.localParamsBufferIdx = device.currentConstantBufferIdx;
        renderPrimitive.localParamsOffset = constantBuffer.head;
        BufferPushMat4(constantBuffer, world);
        BufferPushMat4(constantBuffer, worldViewProjection);
        renderPrimitive.localParamsSize = constantBuffer.head - renderPrimitive.localParamsOffset;

        switch (entity.type)
        {
            case EntityType_Mesh:
                {
                    renderPrimitive.vaoHandle = FindVAO(device, meshIdx, submeshIdx, program);

                    Material& defaultMaterial = device.materials[embedded.defaultMaterialIdx];
                    renderPrimitive.albedoTextureHandle = device.textures[defaultMaterial.albedoTextureIdx].handle;

                    Mesh& mesh = device.meshes[meshIdx];
                    Submesh& submesh = mesh.submeshes[submeshIdx];
                    renderPrimitive.indexCount = submesh.indexCount;
                    renderPrimitive.indexOffset = submesh.indexOffset;

                    ASSERT(forwardRenderData.renderPrimitiveCount < ARRAY_COUNT(forwardRenderData.renderPrimitives), "Max number of render primitives reached");
                    forwardRenderData.renderPrimitives[forwardRenderData.renderPrimitiveCount++] = renderPrimitive;
                }
                break;

            case EntityType_Model:
                {
                    Mesh& mesh = device.meshes[meshIdx];

                    for (u32 submeshIdx = 0; submeshIdx < mesh.submeshes.size(); ++submeshIdx)
                    {
                        renderPrimitive.vaoHandle = FindVAO(device, meshIdx, submeshIdx, program);

                        u32 submeshMaterialIdx = mesh.materialIndices[submeshIdx];
                        Material& submeshMaterial = device.materials[submeshMaterialIdx];
                        renderPrimitive.albedoTextureHandle = device.textures[submeshMaterial.albedoTextureIdx].handle;

                        Submesh& submesh = mesh.submeshes[submeshIdx];
                        renderPrimitive.indexCount = submesh.indexCount;
                        renderPrimitive.indexOffset = submesh.indexOffset;

                        ASSERT(forwardRenderData.renderPrimitiveCount < ARRAY_COUNT(forwardRenderData.renderPrimitives), "Max number of render primitives reached");
                        forwardRenderData.renderPrimitives[forwardRenderData.renderPrimitiveCount++] = renderPrimitive;
                    }
                }
                break;
        }
    }
#endif
}

void ForwardShading_Render(Device& device, const Embedded& embedded, const ForwardRenderData& forwardRender, const BufferRange& globalParamsRange)
{
    // TODO: Create PSO objects
    if (g_CullFace)
    {
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    }
    else
    {
        glDisable(GL_CULL_FACE);
    }

    const Program& program = device.programs[forwardRender.programIdx];
    glUseProgram(program.handle);

    if (device.glVersion < MAKE_GLVERSION(4, 2))
    {
        // TODO: Investigate if this only needs to be done once when loading the shader
        const GLuint globalParamsIdx = glGetUniformBlockIndex(program.handle, "GlobalParams");
        const GLuint localParamsIdx = glGetUniformBlockIndex(program.handle, "LocalParams");
        glUniformBlockBinding(program.handle, globalParamsIdx, BINDING(0));
#if !defined(USE_INSTANCING)
        glUniformBlockBinding(program.handle, localParamsIdx, BINDING(1));
#endif
    }

    // Bind GlobalParams uniform block
    glBindBufferRange(GL_UNIFORM_BUFFER, BINDING(0), device.constantBuffers[globalParamsRange.bufferIdx].handle, globalParamsRange.offset, globalParamsRange.size);

#if defined(USE_INSTANCING)
    Buffer& instancingBuffer = device.vertexBuffers[forwardRender.instancingBufferIdx];
    BindBuffer(instancingBuffer);
#endif

    // Render code
    for (u32 i = 0; i < forwardRender.renderPrimitiveCount; ++i)
    {
        const RenderPrimitive& renderPrimitive = forwardRender.renderPrimitives[i];

        // Bind texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, renderPrimitive.albedoTextureHandle);
        glUniform1i(forwardRender.uniLoc_Albedo, 0);

        // Bind geometry
        glBindVertexArray(renderPrimitive.vaoHandle);

#if defined(USE_INSTANCING)
        // Bind instancing buffer
        const u32 VertexStream_FirstInstancingStream = 6;
        const GLsizei stride = sizeof(mat4) * 2;
        u64 offset = renderPrimitive.instancingOffset;
        for (u32 location = VertexStream_FirstInstancingStream; location < 14; ++location)
        {
            glVertexAttribPointer(location, 4, GL_FLOAT, GL_FALSE, stride, (void*)(u64)offset);
            glVertexAttribDivisor(location, 1);
            glEnableVertexAttribArray(location);
            offset += sizeof(vec4);
        }

        // Draw
        glDrawElementsInstanced(GL_TRIANGLES, renderPrimitive.indexCount, GL_UNSIGNED_INT, (void*)(u64)renderPrimitive.indexOffset, renderPrimitive.instanceCount);
#else
        // Bind LocalParams uniform block
        GLuint bufferHandle = device.constantBuffers[renderPrimitive.localParamsBufferIdx].handle;
        glBindBufferRange(GL_UNIFORM_BUFFER, BINDING(1), bufferHandle, renderPrimitive.localParamsOffset, renderPrimitive.localParamsSize);

        // Draw
        glDrawElements(GL_TRIANGLES, renderPrimitive.indexCount, GL_UNSIGNED_INT, (void*)(u64)renderPrimitive.indexOffset);
#endif
    }
}





// DEFERRED RENDERER

void DeferredShading_Init(Device& device, DeferredRenderData& renderPathData)
{
    renderPathData.gbufferProgramIdx = LoadProgram(device, CString("shaders.glsl"), CString("GBUFFER"));
    Program& gbufferProgram = device.programs[renderPathData.gbufferProgramIdx];
    renderPathData.uniLoc_Albedo = glGetUniformLocation(gbufferProgram.handle, "uAlbedo");
    renderPathData.localParamsBlockSize = KB(1); // TODO: Get the size from the shader?
    renderPathData.instancingBuffer = CreateDynamicVertexBufferRaw(MB(1));

    renderPathData.shadingProgramIdx = LoadProgram(device, CString("shaders.glsl"), CString("DEFERRED_SHADING"));
    Program& shadingProgram = device.programs[renderPathData.shadingProgramIdx];
}

void DeferredShading_Update(Device& device, const Scene& scene, const Embedded& embedded, DeferredRenderData& renderPathData)
{
    Program& program = device.programs[renderPathData.gbufferProgramIdx];

    renderPathData.renderPrimitiveCount = 0;

#if defined(USE_INSTANCING)
    MapBuffer(renderPathData.instancingBuffer, GL_WRITE_ONLY);

    static u64* renderPrimitivesToSort = new u64[MAX_RENDER_PRIMITIVES]; // TODO: this is a mem leak, put this in another place
    u32 renderPrimitivesToSortCount = 0;

    for (u32 entityIdx = 0; entityIdx < scene.entityCount; ++entityIdx)
    {
        const Entity& entity = scene.entities[entityIdx];
        const u32 meshIdx = HIGH_WORD(entity.meshSubmeshIdx);
        const u32 submeshIdx = LOW_WORD(entity.meshSubmeshIdx);

        switch (entity.type)
        {
            case EntityType_Mesh:
                {
                    u64 rp = ((u64)meshIdx << 48) | ((u64)submeshIdx << 32) | (entityIdx);
                    renderPrimitivesToSort[renderPrimitivesToSortCount++] = rp;
                }
                break;

            case EntityType_Model:
                {
                    Mesh& mesh = device.meshes[meshIdx];

                    for (u32 submeshIdx = 0; submeshIdx < mesh.submeshes.size(); ++submeshIdx)
                    {
                        u64 rp = ((u64)meshIdx << 48) | ((u64)submeshIdx << 32) | (entityIdx);
                        renderPrimitivesToSort[renderPrimitivesToSortCount++] = rp;
                    }
                }
                break;
        }
    }

    QSort((u64*)renderPrimitivesToSort, (u64*)renderPrimitivesToSort + renderPrimitivesToSortCount - 1);

    u16 prevMeshIdx = 0xffff;
    u16 prevSubmeshIdx = 0xffff;

    for (u32 primIdx = 0; primIdx < renderPrimitivesToSortCount; ++primIdx)
    {
        u64 rp = renderPrimitivesToSort[primIdx];
        u32 meshIdx    = (rp >> 48) & 0xffff;
        u32 submeshIdx = (rp >> 32) & 0xffff;
        u32 entityIdx  = (rp >>  0) & 0xffffffff;

        const Entity& entity = scene.entities[entityIdx];

        if (meshIdx != prevMeshIdx || submeshIdx != prevSubmeshIdx)
        {
            Mesh& mesh = device.meshes[meshIdx];

            RenderPrimitive renderPrimitive = {};
            renderPrimitive.vaoHandle = FindVAO(device, meshIdx, submeshIdx, program);

            if (submeshIdx < mesh.materialIndices.size())
            {
                u32 submeshMaterialIdx = mesh.materialIndices[submeshIdx];
                Material& submeshMaterial = device.materials[submeshMaterialIdx];
                renderPrimitive.albedoTextureHandle = device.textures[submeshMaterial.albedoTextureIdx].handle;
            }
            else
            {
                Material& defaultMaterial = device.materials[embedded.defaultMaterialIdx];
                renderPrimitive.albedoTextureHandle = device.textures[defaultMaterial.albedoTextureIdx].handle;
            }

            Submesh& submesh = mesh.submeshes[submeshIdx];
            renderPrimitive.indexCount = submesh.indexCount;
            renderPrimitive.indexOffset = submesh.indexOffset;

            renderPrimitive.instanceCount = 0;
            renderPrimitive.instancingOffset = renderPathData.instancingBuffer.head;

            renderPathData.renderPrimitives[renderPathData.renderPrimitiveCount++] = renderPrimitive;

            prevMeshIdx = meshIdx;
            prevSubmeshIdx = submeshIdx;
        }

        RenderPrimitive& renderPrimitive = renderPathData.renderPrimitives[renderPathData.renderPrimitiveCount - 1];

        const mat4&   world  = entity.worldMatrix;
        const mat4    worldViewProjection = scene.mainCamera.viewProjectionMatrix * world;
        BufferPushMat4(renderPathData.instancingBuffer, world);
        BufferPushMat4(renderPathData.instancingBuffer, worldViewProjection);
        renderPrimitive.instanceCount++;
    }

    UnmapBuffer(renderPathData.instancingBuffer);

#else

    for (u32 entityIdx = 0; entityIdx < scene.entityCount; ++entityIdx)
    {
        const Entity& entity = scene.entities[entityIdx];
        const mat4&   world  = entity.worldMatrix;
        const mat4    worldViewProjection = scene.mainCamera.viewProjectionMatrix * world;
        const u32     meshIdx = HIGH_WORD(entity.meshSubmeshIdx);
        const u32     submeshIdx = LOW_WORD(entity.meshSubmeshIdx);

        RenderPrimitive renderPrimitive = {};
        renderPrimitive.entityIdx = entityIdx;

        Buffer& constantBuffer = GetMappedConstantBufferForRange( device, renderPathData.localParamsBlockSize );
        renderPrimitive.localParamsBufferIdx = device.currentConstantBufferIdx;
        renderPrimitive.localParamsOffset = constantBuffer.head;
        BufferPushMat4(constantBuffer, world);
        BufferPushMat4(constantBuffer, worldViewProjection);
        renderPrimitive.localParamsSize = constantBuffer.head - renderPrimitive.localParamsOffset;

        switch (entity.type)
        {
            case EntityType_Mesh:
                {
                    renderPrimitive.vaoHandle = FindVAO(device, meshIdx, submeshIdx, program);

                    Material& defaultMaterial = device.materials[embedded.defaultMaterialIdx];
                    renderPrimitive.albedoTextureHandle = device.textures[defaultMaterial.albedoTextureIdx].handle;

                    Mesh& mesh = device.meshes[meshIdx];
                    Submesh& submesh = mesh.submeshes[submeshIdx];
                    renderPrimitive.indexCount = submesh.indexCount;
                    renderPrimitive.indexOffset = submesh.indexOffset;

                    ASSERT(renderPathData.renderPrimitiveCount < ARRAY_COUNT(renderPathData.renderPrimitives), "Max number of render primitives reached");
                    renderPathData.renderPrimitives[renderPathData.renderPrimitiveCount++] = renderPrimitive;
                }
                break;

            case EntityType_Model:
                {
                    Mesh& mesh = device.meshes[meshIdx];

                    for (u32 submeshIdx = 0; submeshIdx < mesh.submeshes.size(); ++submeshIdx)
                    {
                        renderPrimitive.vaoHandle = FindVAO(device, meshIdx, submeshIdx, program);

                        u32 submeshMaterialIdx = mesh.materialIndices[submeshIdx];
                        Material& submeshMaterial = device.materials[submeshMaterialIdx];
                        renderPrimitive.albedoTextureHandle = device.textures[submeshMaterial.albedoTextureIdx].handle;

                        Submesh& submesh = mesh.submeshes[submeshIdx];
                        renderPrimitive.indexCount = submesh.indexCount;
                        renderPrimitive.indexOffset = submesh.indexOffset;

                        ASSERT(renderPathData.renderPrimitiveCount < ARRAY_COUNT(renderPathData.renderPrimitives), "Max number of render primitives reached");
                        renderPathData.renderPrimitives[renderPathData.renderPrimitiveCount++] = renderPrimitive;
                    }
                }
                break;
        }
    }
#endif
}

void DeferredShading_RenderOpaques(Device& device, const Embedded& embedded, const DeferredRenderData& renderPathData, const BufferRange& globalParamsRange)
{
    const Program& program = device.programs[renderPathData.gbufferProgramIdx];
    glUseProgram(program.handle);

    if (device.glVersion < MAKE_GLVERSION(4, 2))
    {
        // TODO: Investigate if this only needs to be done once when loading the shader
        const GLuint globalParamsIdx = glGetUniformBlockIndex(program.handle, "GlobalParams");
        const GLuint localParamsIdx = glGetUniformBlockIndex(program.handle, "LocalParams");
        glUniformBlockBinding(program.handle, globalParamsIdx, BINDING(0));
#if !defined(USE_INSTANCING)
        glUniformBlockBinding(program.handle, localParamsIdx, BINDING(1));
#endif
    }

    // Bind GlobalParams uniform block
    glBindBufferRange(GL_UNIFORM_BUFFER, BINDING(0), device.constantBuffers[globalParamsRange.bufferIdx].handle, globalParamsRange.offset, globalParamsRange.size);

#if defined(USE_INSTANCING)
    BindBuffer(renderPathData.instancingBuffer);
#endif

    // Render code
    for (u32 i = 0; i < renderPathData.renderPrimitiveCount; ++i)
    {
        const RenderPrimitive& renderPrimitive = renderPathData.renderPrimitives[i];

        // Bind texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, renderPrimitive.albedoTextureHandle);
        glUniform1i(renderPathData.uniLoc_Albedo, 0);

        // Bind geometry
        glBindVertexArray(renderPrimitive.vaoHandle);

#if defined(USE_INSTANCING)
        // Bind instancing buffer
        const u32 VertexStream_FirstInstancingStream = 6;
        const GLsizei stride = sizeof(mat4) * 2;
        u64 offset = renderPrimitive.instancingOffset;
        for (u32 location = VertexStream_FirstInstancingStream; location < 14; ++location)
        {
            glVertexAttribPointer(location, 4, GL_FLOAT, GL_FALSE, stride, (void*)(u64)offset);
            glVertexAttribDivisor(location, 1);
            glEnableVertexAttribArray(location);
            offset += sizeof(vec4);
        }

        // Draw
        glDrawElementsInstanced(GL_TRIANGLES, renderPrimitive.indexCount, GL_UNSIGNED_INT, (void*)(u64)renderPrimitive.indexOffset, renderPrimitive.instanceCount);
#else
        // Bind LocalParams uniform block
        GLuint bufferHandle = device.constantBuffers[renderPrimitive.localParamsBufferIdx].handle;
        glBindBufferRange(GL_UNIFORM_BUFFER, BINDING(1), bufferHandle, renderPrimitive.localParamsOffset, renderPrimitive.localParamsSize);

        // Draw
        glDrawElements(GL_TRIANGLES, renderPrimitive.indexCount, GL_UNSIGNED_INT, (void*)(u64)renderPrimitive.indexOffset);
#endif
    }
}

void DeferredShading_RenderLights(Device& device, const Embedded& embedded, const DeferredRenderData& renderPathData, const BufferRange& globalParamsRange)
{

}


/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Rendering resource manager.
 */

#include "engine/asset_manager.h"

#include "gpu/gpu_manager.h"

#include "render/render_manager.h"
#include "render/utility.h"
#include "render/vertex.h"

/** Manager of global renderer resources */
EngineGlobal<RenderManager> g_renderManager;

/** Create rendering resources. */
void RenderManager::init() {
    /* Create the simple vertex format. */
    VertexBufferLayoutArray buffers(1);
    buffers[0].stride = sizeof(SimpleVertex);
    VertexAttributeArray attributes(3);
    attributes[0].semantic = VertexAttribute::kPositionSemantic;
    attributes[0].index = 0;
    attributes[0].type = VertexAttribute::kFloatType;
    attributes[0].count = 3;
    attributes[0].buffer = 0;
    attributes[0].offset = offsetof(SimpleVertex, x);
    attributes[1].semantic = VertexAttribute::kNormalSemantic;
    attributes[1].index = 0;
    attributes[1].type = VertexAttribute::kFloatType;
    attributes[1].count = 3;
    attributes[1].buffer = 0;
    attributes[1].offset = offsetof(SimpleVertex, nx);
    attributes[2].semantic = VertexAttribute::kTexcoordSemantic;
    attributes[2].index = 0;
    attributes[2].type = VertexAttribute::kFloatType;
    attributes[2].count = 2;
    attributes[2].buffer = 0;
    attributes[2].offset = offsetof(SimpleVertex, u);
    m_simpleVertexFormat = g_gpuManager->createVertexFormat(buffers, attributes);

    /* Create the utility geometry. */
    RenderUtil::makeQuad(m_quadVertexData);
    RenderUtil::makeSphere(24, 24, m_sphereVertexData, m_sphereIndexData);
    RenderUtil::makeCone(20, m_coneVertexData, m_coneIndexData);

    /* Load the deferred light material. */
    ShaderPtr shader = g_assetManager->load<Shader>("engine/shaders/internal/deferred_light");
    m_deferredLightMaterial = new Material(shader);
}

/**
 * Ensure render targets are allocated and sufficiently sized.
 *
 * This function is called at the beginning of scene rendering to ensure that
 * all render targets required by the render path are allocated and of
 * sufficient size.
 *
 * @param path          Rendering path being used.
 * @param size          Required dimensions (in pixels).
 */
void RenderManager::allocRenderTargets(RenderPath path, glm::ivec2 size) {
    RenderTargets &rt = m_renderTargets;

    /* Allocate main offscreen rendering textures. */
    if (rt.screenBufferSize.x < size.x || rt.screenBufferSize.y < size.y) {
        rt.colourBuffer = nullptr;
        rt.depthBuffer = nullptr;

        /* Calculate new size. We dynamically resize the render targets to be
         * the maximum that they need to be to satisfy all render targets so
         * we don't constantly have to reallocate them. */
        rt.screenBufferSize.x = std::max(rt.screenBufferSize.x, size.x);
        rt.screenBufferSize.y = std::max(rt.screenBufferSize.y, size.y);
        logDebug(
            "Resizing screen buffers to %dx%d (for %dx%d)",
            rt.screenBufferSize.x, rt.screenBufferSize.y, size.x, size.y);

        /* Allocate the buffers. */
        GPUTexture2DDesc desc;
        desc.width = rt.screenBufferSize.x;
        desc.height = rt.screenBufferSize.y;
        desc.mips = 1;
        desc.flags = GPUTexture::kRenderTarget;
        desc.format = PixelFormat::kR8G8B8A8;
        rt.colourBuffer = g_gpuManager->createTexture(desc);
        desc.format = PixelFormat::kDepth24Stencil8;
        rt.depthBuffer = g_gpuManager->createTexture(desc);
    }

    /* Re-allocate G-Buffer textures if necessary. */
    if (path == RenderPath::kDeferred && (rt.deferredBufferSize.x < size.x || rt.deferredBufferSize.y < size.y)) {
        rt.deferredBufferA = nullptr;
        rt.deferredBufferB = nullptr;
        rt.deferredBufferC = nullptr;
        rt.deferredBufferD = nullptr;

        /* Calculate new size. We dynamically resize the render targets to be
         * the maximum that they need to be to satisfy all render targets so
         * we don't constantly have to reallocate them. */
        rt.deferredBufferSize.x = std::max(rt.deferredBufferSize.x, size.x);
        rt.deferredBufferSize.y = std::max(rt.deferredBufferSize.y, size.y);
        logDebug(
            "Resizing deferred buffers to %dx%d (for %dx%d)",
            rt.deferredBufferSize.x, rt.deferredBufferSize.y, size.x, size.y);

        /*
         * Allocate the buffers. The buffer layout is as follows:
         *
         *     | Format      | R          | G          | B          | A
         *  ---|-------------|------------|------------|------------|------------
         *   A | R10G10B10A2 | Normal.x   | Normal.y   | Normal.z   | -
         *  ---|-------------|------------|------------|------------|------------
         *   B | R8G8B8A8    | Diffuse.r  | Diffuse.g  | Diffuse.b  | -
         *  ---|-------------|------------|------------|------------|------------
         *   C | R8G8B8A8    | Specular.r | Specular.g | Specular.b | 1/Shininess
         *  ---|-------------|------------|------------|------------|------------
         *   D | D24S8       | Depth      | -          | -          | -
         *
         * These are all unsigned normalized textures, therefore the normals are
         * scaled to fit into the [0, 1] range, and the shininess is stored as
         * its reciprocal. Position is reconstructed from the depth buffer.
         */
        GPUTexture2DDesc desc;
        desc.width = rt.deferredBufferSize.x;
        desc.height = rt.deferredBufferSize.y;
        desc.mips = 1;
        desc.flags = GPUTexture::kRenderTarget;
        desc.format = PixelFormat::kR10G10B10A2;
        rt.deferredBufferA = g_gpuManager->createTexture(desc);
        desc.format = PixelFormat::kR8G8B8A8;
        rt.deferredBufferB = g_gpuManager->createTexture(desc);
        rt.deferredBufferC = g_gpuManager->createTexture(desc);
        desc.format = PixelFormat::kDepth24Stencil8;
        rt.deferredBufferD = g_gpuManager->createTexture(desc);

        // FIXME: separate bindTexture/Sampler, and do this only once.
        GPUSamplerStateDesc samplerDesc;
        samplerDesc.filterMode = SamplerFilterMode::kNearest;
        samplerDesc.maxAnisotropy = 1;
        samplerDesc.addressU = samplerDesc.addressV = samplerDesc.addressW = SamplerAddressMode::kClamp;
        GPUSamplerStatePtr sampler = g_gpuManager->createSamplerState(samplerDesc);

        /* Update deferred buffer texture bindings. */
        g_gpuManager->bindTexture(TextureSlots::kDeferredBufferA, rt.deferredBufferA, sampler);
        g_gpuManager->bindTexture(TextureSlots::kDeferredBufferB, rt.deferredBufferB, sampler);
        g_gpuManager->bindTexture(TextureSlots::kDeferredBufferC, rt.deferredBufferC, sampler);
        g_gpuManager->bindTexture(TextureSlots::kDeferredBufferD, rt.deferredBufferD, sampler);
    }
}

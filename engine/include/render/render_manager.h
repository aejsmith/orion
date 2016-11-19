/*
 * Copyright (C) 2015-2016 Alex Smith
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * @file
 * @brief               Rendering resource manager.
 */

#pragma once

#include "core/hash_table.h"

#include "gpu/render_pass.h"
#include "gpu/resource.h"
#include "gpu/state.h"
#include "gpu/texture.h"

#include "render/defs.h"
#include "render/geometry.h"
#include "render/render_thread.h"

#include "shader/material.h"

class RenderThread;

/** Manages global resources used throughout the renderer. */
class RenderManager : Noncopyable {
public:
    /** Structure containing primary render targets. */
    struct RenderTargets {
        /** Off-screen rendering textures. */
        GPUTexturePtr colourBuffer;         /**< Colour buffer. */
        GPUTexturePtr depthBuffer;          /**< Depth buffer. */
        glm::ivec2 screenBufferSize;        /**< Current size of screen buffers. */

        /** Deferred G-Buffer textures. */
        GPUTexturePtr deferredBufferA;      /**< Normals/shininess. */
        GPUTexturePtr deferredBufferB;      /**< Diffuse colour. */
        GPUTexturePtr deferredBufferC;      /**< Specular colour. */
        GPUTexturePtr deferredBufferD;      /**< Copy of depth buffer. */
        glm::ivec2 deferredBufferSize;      /**< Current size of G-Buffer. */

        RenderTargets() :
            screenBufferSize(0, 0),
            deferredBufferSize(0, 0)
        {}
    };

    /** Structure containing global rendering resources. */
    struct Resources {
        /** Vertex data layout for SimpleVertex. */
        GPUVertexDataLayoutPtr simpleVertexDataLayout;

        /** Standard resource set layouts. */
        GPUResourceSetLayoutPtr entityResourceSetLayout;
        GPUResourceSetLayoutPtr viewResourceSetLayout;
        GPUResourceSetLayoutPtr lightResourceSetLayout;
        GPUResourceSetLayoutPtr postEffectResourceSetLayout;

        /** Standard render passes. */
        GPURenderPassPtr sceneShadowMapPass;
        GPURenderPassPtr sceneGBufferPass;
        GPURenderPassPtr sceneLightPass;
        GPURenderPassPtr sceneForwardPass;
        GPURenderPassPtr sceneForwardClearPass;
        GPURenderPassPtr postEffectBlitPass;

        /** Basic geometry. */
        GPUVertexDataPtr quadVertexData;
        GPUVertexDataPtr sphereVertexData;
        GPUIndexDataPtr sphereIndexData;
        GPUVertexDataPtr coneVertexData;
        GPUIndexDataPtr coneIndexData;

        /** Deferred light material. */
        MaterialPtr deferredLightMaterial;

        /** @param geometry     Where to store quad geometry. */
        void quadGeometry(Geometry &geometry) const {
            geometry.vertices = this->quadVertexData;
            geometry.indices = nullptr;
            geometry.primitiveType = PrimitiveType::kTriangleList;
        }

        /** @param geometry     Where to store sphere geometry. */
        void sphereGeometry(Geometry &geometry) const {
            geometry.vertices = this->sphereVertexData;
            geometry.indices = this->sphereIndexData;
            geometry.primitiveType = PrimitiveType::kTriangleList;
        }

        /** @param geometry     Where to store cone geometry. */
        void coneGeometry(Geometry &geometry) const {
            geometry.vertices = this->coneVertexData;
            geometry.indices = this->coneIndexData;
            geometry.primitiveType = PrimitiveType::kTriangleList;
        }
    };

    RenderManager();

    void init();

    /** @return             Rendering thread. */
    RenderThread &renderThread() { return m_renderThread; }

    /**
     * Render target management.
     */

    void allocRenderTargets(RenderPath path, glm::ivec2 size);

    /** Get the currently allocated primary render targets.
     * @return              Reference to the render targets structure. */
    const RenderTargets &renderTargets() const { return m_renderTargets; }

    GPUTexture *allocTempRenderTarget(const GPUTextureDesc &desc);

    /**
     * Rendering resources.
     */

    /** @return             Global rendering resources. */
    const Resources &resources() const { return m_resources; }

    /**
     * Rendering parameters.
     */

    /** @return             Current shadow map resolution. */
    uint16_t shadowMapResolution() const { return m_shadowMapResolution; }
private:
    /** Structure containing a temporary render target. */
    struct TempRenderTarget {
        GPUTexturePtr texture;              /**< Texture. */
        bool allocated;                     /**< Whether the texture is in use. */
    };

    RenderThread m_renderThread;            /**< Rendering thread. */

    /** Primary render targets. */
    RenderTargets m_renderTargets;

    /** Pool of temporary render target textures. */
    MultiHashMap<GPUTextureDesc, TempRenderTarget> m_tempRenderTargets;

    /** Rendering resources. */
    Resources m_resources;

    /** Rendering parameters. */
    uint16_t m_shadowMapResolution;         /**< Shadow map resolution. */
};

extern RenderManager *g_renderManager;

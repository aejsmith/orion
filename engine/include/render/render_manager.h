/*
 * Copyright (C) 2015 Alex Smith
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

#include "gpu/texture.h"
#include "gpu/vertex_format.h"

#include "render/defs.h"
#include "render/geometry.h"

#include "shader/material.h"

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
    public:
        RenderTargets() :
            screenBufferSize(0, 0),
            deferredBufferSize(0, 0)
        {}
    };
public:
    RenderManager();

    void init();

    /**
     * Render target management.
     */

    void allocRenderTargets(RenderPath path, glm::ivec2 size);

    /** Get the currently allocated primary render targets.
     * @return              Reference to the render targets structure. */
    const RenderTargets &renderTargets() const { return m_renderTargets; }

    GPUTexture *allocTempRenderTarget(const GPUTextureDesc &desc);

    /**
     * Other rendering resources.
     */

    /** @return             Vertex format for SimpleVertex. */
    GPUVertexFormat *simpleVertexFormat() const { return m_simpleVertexFormat; }

    /** @param geometry     Where to store quad geometry. */
    void quadGeometry(Geometry &geometry) const {
        geometry.vertices = m_quadVertexData;
        geometry.indices = nullptr;
        geometry.primitiveType = PrimitiveType::kTriangleList;
    }

    /** @param geometry     Where to store sphere geometry. */
    void sphereGeometry(Geometry &geometry) const {
        geometry.vertices = m_sphereVertexData;
        geometry.indices = m_sphereIndexData;
        geometry.primitiveType = PrimitiveType::kTriangleList;
    }

    /** @param geometry     Where to store cone geometry. */
    void coneGeometry(Geometry &geometry) const {
        geometry.vertices = m_coneVertexData;
        geometry.indices = m_coneIndexData;
        geometry.primitiveType = PrimitiveType::kTriangleList;
    }

    /** @return             Deferred light material. */
    Material *deferredLightMaterial() const { return m_deferredLightMaterial; }

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
private:
    /** Primary render targets. */
    RenderTargets m_renderTargets;

    /** Pool of temporary render target textures. */
    MultiHashMap<GPUTextureDesc, TempRenderTarget> m_tempRenderTargets;

    /** Vertex format for SimpleVertex. */
    GPUVertexFormatPtr m_simpleVertexFormat;

    /** Utility geometry. */
    GPUVertexDataPtr m_quadVertexData;      /**< Vertex data for a quad. */
    GPUVertexDataPtr m_sphereVertexData;    /**< Vertex data for a sphere. */
    GPUIndexDataPtr m_sphereIndexData;      /**< Index data for a sphere. */
    GPUVertexDataPtr m_coneVertexData;      /**< Vertex data for a cone. */
    GPUIndexDataPtr m_coneIndexData;        /**< Index data for a cone. */

    /** Special materials. */
    MaterialPtr m_deferredLightMaterial;    /**< Deferred light material. */

    /** Rendering parameters. */
    uint16_t m_shadowMapResolution;         /**< Shadow map resolution. */
};

extern EngineGlobal<RenderManager> g_renderManager;

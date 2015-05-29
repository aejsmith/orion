/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Rendering resource manager.
 */

#pragma once

#include "engine/material.h"

#include "gpu/texture.h"
#include "gpu/vertex_format.h"

#include "render/scene_renderer.h"

/** Manages global resources used throughout the renderer. */
class RenderManager : Noncopyable {
public:
    /** Structure containing render targets. */
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
    void init();

    /**
     * Render target management.
     */

    void allocRenderTargets(RenderPath path, glm::ivec2 size);

    /** Get the currently allocated render targets.
     * @return              Reference to the render targets structure. */
    const RenderTargets &renderTargets() const { return m_renderTargets; }

    /**
     * Other rendering resources.
     */

    /** @return             Vertex format for SimpleVertex. */
    GPUVertexFormat *simpleVertexFormat() const { return m_simpleVertexFormat; }

    /** @return             Quad vertex/index data. */
    void quadGeometry(GPUVertexData *&vertices, GPUIndexData *&indices) const {
        vertices = m_quadVertexData;
        indices = nullptr;
    }

    /** @return             Sphere vertex/index data. */
    void sphereGeometry(GPUVertexData *&vertices, GPUIndexData *&indices) const {
        vertices = m_sphereVertexData;
        indices = m_sphereIndexData;
    }

    /** @return             Cone vertex/index data. */
    void coneGeometry(GPUVertexData *&vertices, GPUIndexData *&indices) const {
        vertices = m_coneVertexData;
        indices = m_coneIndexData;
    }

    /** @return             Deferred light material. */
    Material *deferredLightMaterial() const { return m_deferredLightMaterial; }
private:
    /** Render targets. */
    RenderTargets m_renderTargets;

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
};

extern EngineGlobal<RenderManager> g_renderManager;

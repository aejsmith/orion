/*
 * Copyright (C) 2015-2017 Alex Smith
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
 * @brief               Global rendering resources.
 */

#pragma once

#include "core/hash_table.h"

#include "engine/global_resource.h"

#include "gpu/resource.h"
#include "gpu/state.h"
#include "gpu/texture.h"

#include "render_core/geometry.h"
#include "render_core/material.h"

/** Manages global resources used throughout the renderer. */
class RenderResources : Noncopyable {
public:
    RenderResources();
    ~RenderResources();

    /** @return             Vertex data layout for SimpleVertex. */
    GPUVertexDataLayout *simpleVertexDataLayout() const { return m_simpleVertexDataLayout; }

    /** @return             Entity resource set layout. */
    GPUResourceSetLayout *entityResourceSetLayout() const { return m_entityResourceSetLayout; }
    /** @return             View resource set layout. */
    GPUResourceSetLayout *viewResourceSetLayout() const { return m_viewResourceSetLayout; }
    /** @return             Light resource set layout. */
    GPUResourceSetLayout *lightResourceSetLayout() const { return m_lightResourceSetLayout; }
    /** @return             Post effect resource set layout. */
    GPUResourceSetLayout *postEffectResourceSetLayout() const { return m_postEffectResourceSetLayout; }

    /**
     * Get geometry for a quad.
     *
     * Returns geometry for a quad, centered at the origin and extending from
     * -1 to +1 in the X and Y directions. The vertex data has positions,
     * normals and a single set of texture coordinates.
     *
     * @return              Quad geometry.
     */
    Geometry quadGeometry() const {
        Geometry geometry;
        geometry.vertices = m_quadVertexData;
        geometry.indices = nullptr;
        geometry.primitiveType = PrimitiveType::kTriangleList;
        return geometry;
    }

    /**
     * Get geometry for a sphere.
     *
     * Returns geometry for a sphere centered at the origin with a radius of 1.
     * The vertex data has positions, normals and a single set of texture
     * coordinates.
     *
     * @return              Sphere geometry.
     */
    Geometry sphereGeometry() const {
        Geometry geometry;
        geometry.vertices = m_sphereVertexData;
        geometry.indices = m_sphereIndexData;
        geometry.primitiveType = PrimitiveType::kTriangleList;
        return geometry;
    }

    /**
     * Get geometry for a cone.
     *
     * Returns geometry for a cone with the point on the origin, pointing
     * forward (down the negative Z axis), with a base radius of 1 and a height
     * of 1. The vertex data has positions only.
     *
     * @param baseVertices  Number of vertices around the base.
     * @param vertices      Where to return created vertex data object.
     * @param indices       Where to return created index data object.
     * 
     * @return              Cone geometry.
     */
    Geometry coneGeometry() const {
        Geometry geometry;
        geometry.vertices = m_coneVertexData;
        geometry.indices = m_coneIndexData;
        geometry.primitiveType = PrimitiveType::kTriangleList;
        return geometry;
    }
private:
    /** Vertex data layout for SimpleVertex. */
    GPUVertexDataLayoutPtr m_simpleVertexDataLayout;

    /** Standard resource set layouts. */
    GPUResourceSetLayoutPtr m_entityResourceSetLayout;
    GPUResourceSetLayoutPtr m_viewResourceSetLayout;
    GPUResourceSetLayoutPtr m_lightResourceSetLayout;
    GPUResourceSetLayoutPtr m_postEffectResourceSetLayout;

    /** Basic geometry. */
    GPUVertexDataPtr m_quadVertexData;
    GPUVertexDataPtr m_sphereVertexData;
    GPUIndexDataPtr m_sphereIndexData;
    GPUVertexDataPtr m_coneVertexData;
    GPUIndexDataPtr m_coneIndexData;
};

extern GlobalResource<RenderResources> g_renderResources;

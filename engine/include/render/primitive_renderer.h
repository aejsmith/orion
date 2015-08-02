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
 * @brief               Simple primitive renderer.
 */

#pragma once

#include "core/hash_table.h"

#include "gpu/vertex_data.h"

#include "render/draw_list.h"
#include "render/vertex.h"

class Material;
class SceneView;

/**
 * API for simple primitive rendering.
 *
 * This class provides an API for simple primitive rendering, similar to OpenGL
 * immediate mode. It is given a series of primitives which will be internally
 * queued up into buffers and then rendered when requested.
 *
 * Shaders which are used with this must provide a basic pass, this will be used
 * to render (no lighting support). Programs used by the pass will not have
 * entity or light uniforms available - all vertices used should be transformed.
 * View uniforms can optionally be made available by passing a SceneView to
 * draw().
 */
class PrimitiveRenderer {
public:
    PrimitiveRenderer();
    ~PrimitiveRenderer();

    void begin(PrimitiveType type, Material *material);

    /** Add a vertex to the current batch.
     * @param args          Arguments to pass to SimpleVertex constructor. */
    template <typename... Args>
    void addVertex(Args &&... args) {
        SimpleVertex vertex(std::forward<Args>(args)...);
        doAddVertex(vertex);
    }

    void draw(SceneView *view);
private:
    /** Key for a batch. */
    struct BatchKey {
        PrimitiveType type;             /**< Primitive type. */
        Material *material;             /**< Material to render with. */
    public:
        /** Compare this batch with another. */
        bool operator ==(const BatchKey &other) const {
            return type == other.type && material == other.material;
        }

        /** Get a hash from a primitive batch. */
        friend size_t hashValue(const BatchKey &batch) {
            size_t hash = hashValue(batch.type);
            hash = hashCombine(hash, batch.material);
            return hash;
        }
    };

    /** Data for a batch. */
    struct BatchData {
        /** Vector of vertices used during construction. */
        std::vector<SimpleVertex> vertices;

        /** Vertex data generated upon completion. */
        GPUVertexDataPtr gpu;
    };
private:
    void doAddVertex(const SimpleVertex &vertex);
private:
    /** Map of batches added, keyed by material/type. */
    HashMap<BatchKey, BatchData> m_batches;

    BatchData *m_currentBatch;          /**< Current batch that vertices should be added to. */
    DrawList m_drawList;                /**< Generated draw list. */
};

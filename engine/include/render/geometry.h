/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Geometry class.
 */

#pragma once

#include "gpu/index_data.h"
#include "gpu/vertex_data.h"

/**
 * A set of geometry (vertex/index data and primitive type).
 *
 * This structure bundles together vertex data, optional index data and a
 * primitive type for rendering. It holds only raw pointers to the vertex/index
 * data objects, as it is expected that a reference is held elsewhere.
 */
struct Geometry {
    GPUVertexData *vertices;            /**< Vertex data. */
    GPUIndexData *indices;              /**< Index data. */
    PrimitiveType primitiveType;        /**< Primitive type to render. */
};

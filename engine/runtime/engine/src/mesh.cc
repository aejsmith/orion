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
 * @brief               Mesh asset class.
 *
 * TODO:
 *  - Add a method to automatically calculate sub-mesh bounding boxes for when
 *    the vertex or index data is changed.
 *  - Bounding box changes need to propagate to MeshRenderer/SceneEntity.
 */

#include "engine/mesh.h"

#include "render_core/utility.h"

/** Set the indices of the sub-mesh.
 * @param indices       Index data to set. */
void SubMesh::setIndices(GPUIndexDataPtr indices) {
    m_indices = std::move(indices);
}

/**
 * Set the indices of the sub-mesh.
 *
 * Given a vector of indices, will create a GPU buffer containing those indices
 * and an index data referring to it, and set that as the SubMesh's indices.
 *
 * @param indices       Array of indices.
 */
void SubMesh::setIndices(const std::vector<uint16_t> &indices) {
    auto indexDataDesc = GPUIndexDataDesc().
        setBuffer (RenderUtil::buildGPUBuffer(GPUBuffer::kIndexBuffer, indices)).
        setType   (GPUIndexData::kUnsignedShortType).
        setCount  (indices.size());
    setIndices(g_gpuManager->createIndexData(std::move(indexDataDesc)));
}

/**
 * Set the indices of the sub-mesh.
 *
 * Given a vector of indices, will create a GPU buffer containing those indices
 * and an index data referring to it, and set that as the SubMesh's indices.
 *
 * @param indices       Array of indices.
 */
void SubMesh::setIndices(const std::vector<uint32_t> &indices) {
    auto indexDataDesc = GPUIndexDataDesc().
        setBuffer (RenderUtil::buildGPUBuffer(GPUBuffer::kIndexBuffer, indices)).
        setType   (GPUIndexData::kUnsignedIntType).
        setCount  (indices.size());
    setIndices(g_gpuManager->createIndexData(std::move(indexDataDesc)));
}

/**
 * Create a mesh.
 *
 * Creates a new mesh. The mesh is initially empty. Users should set the
 * vertices pointer to point to the mesh's vertex data, add one or more
 * materials, and finally add one or more submeshes.
 */
Mesh::Mesh() :
    m_numVertices (0)
{}

/** Destroy the mesh and all submeshes. */
Mesh::~Mesh() {
    for (SubMesh *subMesh : m_children)
        delete subMesh;
}

/** Look up a material index from a name.
 * @param name          Name of the material.
 * @param index         Where to store index.
 * @return              Whether the material name is known. */
bool Mesh::material(const std::string &name, size_t &index) const {
    auto ret = m_materials.find(name);
    if (ret != m_materials.end()) {
        index = ret->second;
        return true;
    } else {
        return false;
    }
}

/**
 * Add a submesh.
 *
 * Allocates a new submesh with the next available index. The indices pointers
 * in the submesh is initially null, which means that the submesh will
 * effectively render all of the mesh's vertices. The indices pointer can be set
 * to specify the vertex indices.
 *
 * @return              Pointer to created submesh.
 */
SubMesh &Mesh::addSubMesh() {
    SubMesh *subMesh = new SubMesh(*this);
    m_children.push_back(subMesh);
    return *subMesh;
}

/**
 * Add a material slot to the mesh.
 *
 * Adds a material slot to the mesh. Material slots in meshes are given a name,
 * which allows materials to be set by name on the mesh renderer. The name maps
 * to an index, which can be set in a SubMesh to refer to the material slot.
 *
 * @param name          Name for the material slot.
 *
 * @return              Index that the name maps to. If the name already exists,
 *                      the existing index will be returned.
 */
size_t Mesh::addMaterial(const std::string &name) {
    auto ret = m_materials.insert(std::make_pair(name, m_materials.size()));
    return ret.first->second;
}

/**
 * Get vertex data for the mesh.
 *
 * Gets the current vertex data object for the mesh. If any operations have been
 * performed that invalidate the vertex data since the last call to this, the
 * vertex data object will be recreated.
 *
 * @return              Vertex data for the mesh.
 */
GPUVertexData *Mesh::vertices() {
    check(m_numVertices > 0);

    if (!m_vertices) {
        GPUVertexDataLayoutPtr layout = g_gpuManager->getVertexDataLayout(m_layoutDesc);

        auto vertexDataDesc = GPUVertexDataDesc().
            setCount  (m_numVertices).
            setLayout (layout);
        vertexDataDesc.buffers = m_buffers;

        m_vertices = g_gpuManager->createVertexData(std::move(vertexDataDesc));
    }

    return m_vertices;
}

/**
 * Set vertex data for the mesh.
 *
 * Replaces all of the mesh's current vertex data and vertex layout with the
 * given data.
 *
 * @param data          New data for the mesh.
 */
void Mesh::setVertices(GPUVertexDataPtr data) {
    check(data);

    m_vertices = data;

    /* Update our cached information from the new object in case we need to
     * recreate it. */
    m_numVertices = m_vertices->count();
    m_layoutDesc  = m_vertices->layout()->desc();
    m_buffers     = m_vertices->buffers();
}

/**
 * Set the number of vertices in the mesh.
 *
 * Sets the total number of vertices in the mesh. At the moment, this destroys
 * all current data, which will need to be re-uploaded. This also invalidates
 * the current vertex data object, which will be recreated on the next call to
 * vertices().
 *
 * @todo                Allow retaining current data, by copying the existing
 *                      data to the newly-sized buffers.
 *
 * @param count         New vertex count.
 */
void Mesh::setNumVertices(size_t count) {
    m_numVertices = count;

    /* Invalidate data. */
    m_vertices = nullptr;

    /* Recreate buffers. */
    m_buffers.clear();
    m_buffers.resize(m_layoutDesc.bindings.size());
    for (size_t i = 0; i < m_layoutDesc.bindings.size(); i++) {
        auto desc = GPUBufferDesc().
            setType  (GPUBuffer::kVertexBuffer).
            setUsage (GPUBuffer::kStaticUsage).
            setSize  (m_layoutDesc.bindings[i].stride * m_numVertices);
        m_buffers[i] = g_gpuManager->createBuffer(desc);
    }
}

/** Check if the mesh has an attribute.
 * @param semantic      Attribute semantic.
 * @param index         Attribute index. */
bool Mesh::hasAttribute(VertexAttribute::Semantic semantic,
                        unsigned index) const
{
    for (const VertexAttribute &attribute : m_layoutDesc.attributes) {
        if (attribute.semantic == semantic && attribute.index == index)
            return true;
    }

    return false;
}

/**
 * Add an attribute to the mesh.
 *
 * Adds a new attribute to the mesh. This is done by adding a whole new buffer
 * to the mesh dedicated to the new attribute (i.e. it will not be interleaved
 * with any existing data). The attribute will initially have invalid data, it
 * must be set with setAttribute().
 *
 * This invalidates the current vertex data object, which will be recreated on
 * the next call to vertices().
 *
 * @param semantic      Attribute semantic.
 * @param index         Attribute index.
 * @param type          Attribute base data type.
 * @param normalised    Whether fixed-point values should be normalised when
 *                      accessed.
 * @param components    Number of vector components.
 */
void Mesh::addAttribute(VertexAttribute::Semantic semantic,
                        unsigned index,
                        VertexAttribute::Type type,
                        bool normalised,
                        size_t components)
{
    checkMsg(!hasAttribute(semantic, index),
             "addAttribute() on already existing attribute");

    /* Add to an entirely new binding. */
    size_t bindingIndex = m_layoutDesc.bindings.size();
    m_layoutDesc.bindings.emplace_back();
    VertexBinding &binding = m_layoutDesc.bindings.back();
    binding.stride         = VertexAttribute::size(type, components);

    m_layoutDesc.attributes.emplace_back();
    VertexAttribute &attribute = m_layoutDesc.attributes.back();
    attribute.semantic         = semantic;
    attribute.index            = index;
    attribute.type             = type;
    attribute.normalised       = normalised;
    attribute.components       = components;
    attribute.binding          = bindingIndex;
    attribute.offset           = 0;

    /* Create a buffer for it. */
    auto desc = GPUBufferDesc().
        setType  (GPUBuffer::kVertexBuffer).
        setUsage (GPUBuffer::kStaticUsage).
        setSize  (binding.stride * m_numVertices);
    m_buffers.emplace_back(g_gpuManager->createBuffer(desc));
    check(m_buffers.size() == m_layoutDesc.bindings.size());
}

/**
 * Add an attribute to the mesh with data.
 *
 * Equivalent to addAttribute() followed by setAttribute() on the same
 * attribute. See those functions for more details.
 *
 * @param semantic      Attribute semantic.
 * @param index         Attribute index.
 * @param type          Attribute base data type.
 * @param normalised    Whether fixed-point values should be normalised when
 *                      accessed.
 * @param components    Number of vector components.
 * @param data          Array of new vertex data.
 * @param stride        Stride between elements in data.
 */
void Mesh::addAttribute(VertexAttribute::Semantic semantic,
                        unsigned index,
                        VertexAttribute::Type type,
                        bool normalised,
                        size_t components,
                        const void *data,
                        size_t stride)
{
    addAttribute(semantic, index, type, normalised, components);
    setAttribute(semantic, index, type, components, data, stride);
}

/**
 * Update the data for an attribute from an array.
 *
 * Given an array of data, updates the GPU-side data for the specified attribute
 * from that array. The specified attribute must be present, the specified type
 * must match that of the attribute, and the size of the array must be at least
 * (stride * numVertices()).
 *
 * @param semantic      Attribute semantic.
 * @param index         Attribute index.
 * @param type          Attribute base data type.
 * @param components    Number of vector components.
 * @param data          Array of new vertex data.
 * @param stride        Stride between elements in data.
 */
void Mesh::setAttribute(VertexAttribute::Semantic semantic,
                        unsigned index,
                        VertexAttribute::Type type,
                        size_t components,
                        const void *data,
                        size_t stride)
{
    check(m_numVertices > 0);

    auto &attribute =
        [this, semantic, index] () -> const VertexAttribute & {
            for (const VertexAttribute &attribute : m_layoutDesc.attributes) {
                if (attribute.semantic == semantic && attribute.index == index)
                    return attribute;
            }

            unreachableMsg("setAttribute() on invalid attribute");
        } ();

    checkMsg(type == attribute.type && components == attribute.components,
             "setAttribute() with incorrect data type");

    GPUBuffer *buffer = m_buffers[attribute.binding];

    const size_t attribSize    = attribute.size();
    const size_t bindingStride = m_layoutDesc.bindings[attribute.binding].stride;

    if (stride == attribSize && attribSize == bindingStride) {
        /* Fast path where we can just upload directly into the buffer. */
        buffer->write(attribute.offset,
                      attribSize * m_numVertices,
                      data);
    } else if (attribSize == bindingStride) {
        const size_t dataSize = attribSize * m_numVertices;

        /* Extract from the supplied buffer to a staging buffer and upload from
         * that. */
        std::unique_ptr<uint8_t []> dest(new uint8_t[dataSize]);

        const uint8_t *source = reinterpret_cast<const uint8_t *>(data);

        size_t sourceOffset   = 0;
        size_t destOffset     = 0;

        for (size_t i = 0; i < m_numVertices; i++) {
            memcpy(&dest[destOffset], &source[sourceOffset], attribSize);

            sourceOffset += stride;
            destOffset   += bindingStride;
        }

        buffer->write(attribute.offset,
                      attribSize * m_numVertices,
                      dest.get());
    } else {
        /* Would need to read back the buffer and only update this attribute,
         * preserving others. */
        fatal("TODO: Partial buffer update");
    }
}

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
 * @brief               Wavefront mesh loader.
 *
 * TODO:
 *  - This is hideously slow. readLine() reads individual characters at a time,
 *    and there's a metric fuckton of vector reallocations going on.
 */

#include "core/hash_table.h"
#include "core/string.h"

#include "gpu/gpu_manager.h"

#include "mesh_loader.h"

/** Wavefront .obj mesh loader. */
class OBJLoader : public MeshLoader {
public:
    CLASS();

    OBJLoader();

    /** @return             File extension which this loader handles. */
    const char *extension() const override { return "obj"; }

    AssetPtr load() override;

private:
    /** Indexes into the vertex element arrays for a single vertex. */
    struct VertexKey {
        uint16_t position;
        uint16_t texcoord;
        uint16_t normal;
    public:
        /** Compare this key with another. */
        bool operator ==(const VertexKey &other) const {
            return position == other.position && texcoord == other.texcoord && normal == other.normal;
        }

        /** Get the hash for a vertex key. */
        friend size_t hashValue(const VertexKey &value) {
            size_t hash = hashValue(value.position);
            hash = hashCombine(hash, value.texcoord);
            hash = hashCombine(hash, value.normal);
            return hash;
        }
    };

private:
    template <typename VectorType>
    bool addVertexElement(const std::vector<std::string> &tokens, std::vector<VectorType> &array);

    bool addFace(const std::vector<std::string> &tokens);

private:
    /** Parser state. */
    size_t m_currentLine;               /**< Current line of the file (for error messages). */
    std::string m_currentMaterial;      /**< Current material name. */
    SubMeshDesc *m_currentSubMesh;      /**< Current submesh. */

    /** Vertex elements. */
    std::vector<glm::vec3> m_positions; /**< Positions ("v" declarations). */
    std::vector<glm::vec2> m_texcoords; /**< UVs ("vt" declarations). */
    std::vector<glm::vec3> m_normals;   /**< Normals ("vn" declarations). */

    /** Map from VertexKey to a buffer index. */
    HashMap<VertexKey, size_t> m_vertexMap;
};

#include "obj_loader.obj.cc"

/** Initialize the OBJ loader. */
OBJLoader::OBJLoader() :
    m_currentLine     (0),
    m_currentMaterial ("default"),
    m_currentSubMesh  (nullptr)
{}

/** Load an OBJ file.
 * @return              Pointer to loaded asset, null on failure. */
AssetPtr OBJLoader::load() {
    /* Add attributes. FIXME: We can have models without some of these. */
    addAttribute(VertexAttribute::kPositionSemantic, 0);
    addAttribute(VertexAttribute::kNormalSemantic, 0);
    addAttribute(VertexAttribute::kTexcoordSemantic, 0);

    /* Parse the file content. */
    std::string line;
    while (m_data->readLine(line)) {
        m_currentLine++;

        std::vector<std::string> tokens;
        String::tokenize(line, tokens, " \r");
        if (!tokens.size())
            continue;

        if (tokens[0] == "v") {
            if (!addVertexElement(tokens, m_positions))
                return nullptr;
        } else if (tokens[0] == "vt") {
            if (!addVertexElement(tokens, m_texcoords))
                return nullptr;
        } else if (tokens[0] == "vn") {
            if (!addVertexElement(tokens, m_normals))
                return nullptr;
        } else if (tokens[0] == "f") {
            if (!addFace(tokens))
                return nullptr;
        } else if (tokens[0] == "usemtl") {
            if (tokens.size() != 2) {
                logError("%s: %u: Expected single material name", m_path, m_currentLine);
                return nullptr;
            }

            if (tokens[1] != m_currentMaterial) {
                /* Begin a new submesh. */
                m_currentMaterial = tokens[1];
                m_currentSubMesh = nullptr;
            }
        } else if (tokens[0] == "g") {
            if (tokens.size() != 2) {
                /* Note multiple group names can be specified to give shared
                 * elements between groups but we  don't support this for now. */
                logError("%s: %u: Expected single group name", m_path, m_currentLine);
                return nullptr;
            }

            /* Begin a new submesh. TODO: Should we bother trying to handle
             * duplicate group names and bundling them together? Probably not
             * worth the effort. */
            m_currentSubMesh = nullptr;
        } else {
            /* Ignore unknown lines. Most of them are irrelevant to us. */
        }
    }

    return createMesh();
}

/** Handle a vertex element declaration.
 * @param tokens        Tokens from the current line.
 * @param array         Array to add to.
 * @return              Whether the declaration was valid. */
template <typename VectorType>
bool OBJLoader::addVertexElement(const std::vector<std::string> &tokens,
                                 std::vector<VectorType> &array)
{
    VectorType value;

    if (tokens.size() < value.length() + 1) {
        logError("%s: %u: Expected %d values", m_path, m_currentLine, value.length());
        return false;
    }

    for (size_t i = 0; i < value.length(); i++) {
        const char *str = tokens[i + 1].c_str(), *end;
        value[i] = strtof(str, const_cast<char **>(&end));
        if (end != str + tokens[i + 1].length()) {
            logError("%s: %u: Expected float value", m_path, m_currentLine);
            return false;
        }
    }

    array.push_back(value);
    return true;
}

/** Handle a face declaration.
 * @param tokens        Tokens from the current line.
 * @return              Whether the declaration was valid. */
bool OBJLoader::addFace(const std::vector<std::string> &tokens) {
    /* If we don't have a current submesh, we must begin a new one. */
    if (!m_currentSubMesh) {
        m_currentSubMesh = &addSubMesh();
        m_currentSubMesh->material = m_currentMaterial;
    }

    size_t numVertices = tokens.size() - 1;

    if (numVertices != 3 && numVertices != 4) {
        logError("%s: %u: Expected 3 or 4 vertices", m_path, m_currentLine);
        return false;
    }

    /* Each face gives 3 or 4 vertices as a set of indices into the sets of
     * vertex elements that have been declared. */
    std::vector<uint16_t> indices(numVertices);
    for (size_t i = 0; i < numVertices; i++) {
        std::vector<std::string> subTokens;
        String::tokenize(tokens[i + 1], subTokens, "/", -1, false);
        if (subTokens.size() != 3) {
            logError("%s: %u: Expected v/vt/vn", m_path, m_currentLine);
            return false;
        }

        /* Get the vertex elements referred to by this vertex. */
        VertexKey key;
        for (size_t j = 0; j < 3; j++) {
            const char *str = subTokens[j].c_str(), *end;
            uint16_t value = strtoul(str, const_cast<char **>(&end), 10);
            if (end != str + subTokens[j].length()) {
                logError("%s: %u: Expected integer value", m_path, m_currentLine);
                return false;
            }

            /* Indices are 1 based. */
            value -= 1;

            switch (j) {
                case 0:
                    if (value > m_positions.size()) {
                        logError("%s: %u: Invalid position index %u", m_path, m_currentLine, value);
                        return false;
                    }

                    key.position = value;
                    break;
                case 1:
                    if (value > m_texcoords.size()) {
                        logError("%s: %u: Invalid texture coordinate index %u", m_path, m_currentLine, value);
                        return false;
                    }

                    key.texcoord = value;
                    break;
                case 2:
                    if (value > m_normals.size()) {
                        logError("%s: %u: Invalid normal index %u", m_path, m_currentLine, value);
                        return false;
                    }

                    key.normal = value;
                    break;
            }
        }

        /* Add the vertex. */
        auto ret = m_vertexMap.emplace(key, 0);
        if (ret.second) {
            /* We succeeded in adding a new element, this means this is a new
             * vertex. Add one. */
            Vertex &vertex  = addVertex(ret.first->second);
            vertex.position = m_positions[key.position];
            vertex.normal   = m_normals[key.normal];
            vertex.texcoord = m_texcoords[key.texcoord];
        }

        indices[i] = ret.first->second;
    }

    /* Add the indices. If there's 4 it's a quad so add it as 2 triangles. */
    m_currentSubMesh->indices.push_back(indices[0]);
    m_currentSubMesh->indices.push_back(indices[1]);
    m_currentSubMesh->indices.push_back(indices[2]);
    if (numVertices == 4) {
        m_currentSubMesh->indices.push_back(indices[2]);
        m_currentSubMesh->indices.push_back(indices[3]);
        m_currentSubMesh->indices.push_back(indices[0]);
    }

    return true;
}

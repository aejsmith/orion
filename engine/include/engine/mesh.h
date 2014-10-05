/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Mesh asset class.
 */

#pragma once

#include "engine/asset.h"
#include "engine/material.h"

#include "gpu/index_data.h"
#include "gpu/vertex_data.h"

class Mesh;

/** Sub component of a Mesh. */
class SubMesh {
public:
	/** @return		Parent mesh. */
	Mesh *parent() const { return m_parent; }
public:
	GPUVertexDataPtr vertices;		/**< Local vertex data, overrides parent's vertex data. */
	GPUIndexDataPtr indices;		/**< Indices into vertex data. */
	size_t material;			/**< Material index in parent mesh. */
private:
	explicit SubMesh(Mesh *parent) : material(0), m_parent(parent) {}
	~SubMesh() {}
private:
	Mesh *m_parent;				/**< Parent mesh. */

	friend class Mesh;
};

/**
 * Mesh asset.
 *
 * This class stores a 3D mesh for rendering. A Mesh is comprised of one or
 * more SubMeshes. This allows different materials to be used on different
 * parts of a mesh.
 */
class Mesh : public Asset {
public:
	/** Type of the material map. */
	typedef std::map<std::string, size_t> MaterialMap;
public:
	Mesh();
	~Mesh();

	/** @return		Number of submeshes. */
	size_t numSubMeshes() const { return m_children.size(); }
	/** @return		Number of materials. */
	size_t numMaterials() const { return m_materials.size(); }

	/** Get a child at the specified index.
	 * @param index		Index of child. Not bounds checked.
	 * @return		Child at the specified index. */
	SubMesh *subMesh(size_t index) { return m_children[index]; }
	const SubMesh *subMesh(size_t index) const { return m_children[index]; }

	/** @return		Map of material names to indices. */
	const MaterialMap &materials() const { return m_materials; }

	bool material(const std::string &name, size_t &index) const;

	SubMesh *addSubMesh();
	size_t addMaterial(const std::string &name);
public:
	GPUVertexDataPtr sharedVertices;	/**< Vertex data shared by all submeshes. */
private:
	std::vector<SubMesh *> m_children;	/**< Child submeshes. */

	/**
	 * Map of material names.
	 *
	 * We store an array of known materials with a name, to allow materials
	 * to be set on a mesh renderer by name. SubMeshes specify a material
	 * index, which references and a table of the materials to use in the
	 * mesh renderer.
	 */
	MaterialMap m_materials;
};

/** Type of a mesh pointer. */
typedef TypedAssetPtr<Mesh> MeshPtr;

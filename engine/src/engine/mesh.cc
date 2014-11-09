/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Mesh asset class.
 */

#include "engine/mesh.h"

/**
 * Create a mesh.
 *
 * Creates a new mesh. The mesh is initially empty. Users should set the
 * vertices pointer to point to the mesh's vertex data (if submeshes share
 * vertex data), add one or more materials, and finally add one or more
 * submeshes.
 */
Mesh::Mesh() {}

/** Destroy the mesh and all submeshes. */
Mesh::~Mesh() {
	for(SubMesh *subMesh : m_children)
		delete subMesh;
}

/** Look up a material index from a name.
 * @param name		Name of the material.
 * @param index		Where to store index.
 * @return		Whether the material name is known. */
bool Mesh::material(const std::string &name, size_t &index) const {
	auto ret = m_materials.find(name);
	if(ret != m_materials.end()) {
		index = ret->second;
		return true;
	} else {
		return false;
	}
}

/**
 * Add a submesh.
 *
 * Allocates a new submesh with the next available index. The vertices and
 * indices pointers in the submesh are initially null, which means that the
 * submesh will effectively render all of the mesh's shared vertices. If the
 * mesh has no shared vertex data, vertex data must be specified in the submesh.
 * The indices pointer can be set to specify the vertex indices.
 *
 * @return		Pointer to created submesh.
 */
SubMesh *Mesh::addSubMesh() {
	SubMesh *subMesh = new SubMesh(this);
	m_children.push_back(subMesh);
	return subMesh;
}

/**
 * Add a material slot to the mesh.
 *
 * Adds a material slot to the mesh. Material slots in meshes are given a name,
 * which allows materials to be set by name on the mesh renderer. The name maps
 * to an index, which can be set in a SubMesh to refer to the material slot.
 *
 * @param name		Name for the material slot.
 *
 * @return		Index that the name maps to. If the name already exists,
 *			the existing index will be returned.
 */
size_t Mesh::addMaterial(const std::string &name) {
	auto ret = m_materials.insert(std::make_pair(name, m_materials.size()));
	return ret.first->second;
}
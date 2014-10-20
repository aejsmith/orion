/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Mesh renderer component.
 */

#include "gpu/gpu.h"

#include "render/scene_entity.h"

#include "world/mesh_renderer.h"

/** Scene entity for rendering a SubMesh. */
class SubMeshSceneEntity : public SceneEntity {
public:
	/** Initialize the scene entity.
	 * @param mesh		Submesh to render.
	 * @param parent	Parent mesh renderer. */
	SubMeshSceneEntity(SubMesh *subMesh, MeshRendererComponent *parent) :
		m_subMesh(subMesh),
		m_parent(parent)
	{}

	Material *material() const override;
	void draw() const override;
private:
	SubMesh *m_subMesh;		 /**< Submesh to render. */
	MeshRendererComponent *m_parent; /**< Parent mesh renderer. */
};

/** Get the material for the entity.
 * @return		Material for the entity. */
Material *SubMeshSceneEntity::material() const {
	return m_parent->m_materials[m_subMesh->material].get();
}

/** Draw the entity. */
void SubMeshSceneEntity::draw() const {
	GPUVertexData *vertices = (m_subMesh->vertices)
		? m_subMesh->vertices
		: m_subMesh->parent()->sharedVertices;

	g_gpu->draw(PrimitiveType::kTriangleList, vertices, m_subMesh->indices);
}

/** Initialize the mesh renderer.
 * @param entity	Entity the component belongs to.
 * @param mesh		Mesh to render. */
MeshRendererComponent::MeshRendererComponent(Entity *entity, Mesh *mesh) :
	RendererComponent(entity),
	m_mesh(mesh),
	m_materials(mesh->numMaterials())
{}

/** Get the material with the specified name.
 * @param name		Name of the material to get.
 * @return		Pointer to material set. */
Material *MeshRendererComponent::material(const std::string &name) const {
	size_t index = 0;
	bool ret = m_mesh->material(name, index);
	checkMsg(ret, "Material slot '%s' not found", name.c_str());

	return m_materials[index];
}

/** Get the material with the specified index.
 * @param index		Index of the material to get.
 * @return		Pointer to material set. */
Material *MeshRendererComponent::material(size_t index) const {
	check(index < m_materials.size());
	return m_materials[index];
}

/**
 * Set the material to use for part of this mesh.
 *
 * A mesh has one or more material slots defined which its submeshes refer to
 * to get the material they will be rendered with. This function sets the
 * material in the specified slot so that all submeshes using that slot will
 * take on that material.
 *
 * @param name		Name of the material to set.
 * @param material	Material to use.
 */
void MeshRendererComponent::setMaterial(const std::string &name, Material *material) {
	size_t index = 0;
	bool ret = m_mesh->material(name, index);
	checkMsg(ret, "Material slot '%s' not found", name.c_str());

	m_materials[index] = material;
}

/** Set the material to use for part of this mesh.
 * @param index		Index of the material to set.
 * @param material	Material to use. */
void MeshRendererComponent::setMaterial(size_t index, Material *material) {
	check(index < m_materials.size());
	m_materials[index] = material;
}

/** Create scene entities.
 * @param entities	List to populate. */
void MeshRendererComponent::createSceneEntities(SceneEntityList &entities) {
	for(size_t i = 0; i < m_mesh->numSubMeshes(); i++) {
		SubMeshSceneEntity *entity = new SubMeshSceneEntity(m_mesh->subMesh(i), this);
		entities.push_back(entity);
	}
}

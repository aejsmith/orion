/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Test game.
 */

#include "engine/asset_manager.h"
#include "engine/engine.h"
#include "engine/game.h"
#include "engine/material.h"
#include "engine/mesh.h"
#include "engine/texture.h"

#include "gpu/gpu.h"

#include "render/scene_entity.h"
#include "render/scene_view.h"

#include "world/behaviour.h"
#include "world/camera_component.h"
#include "world/entity.h"
#include "world/light_component.h"
#include "world/renderer_component.h"
#include "world/world.h"

/**
 * Rendering test code.
 */

/** Component which renders a mesh. */
class MeshRendererComponent : public RendererComponent {
public:
	MeshRendererComponent(Entity *entity, MeshPtr mesh);

	/** @return		Mesh that this component renders. */
	MeshPtr mesh() const { return m_mesh; }

	MaterialPtr material(const std::string &name) const;
	void setMaterial(const std::string &name, MaterialPtr material);
protected:
	virtual void createSceneEntities(SceneEntityList &entities) override;
private:
	MeshPtr m_mesh;			/**< Mesh to render. */

	/** Array of materials. */
	std::vector<MaterialPtr> m_materials;

	friend class SubMeshSceneEntity;
};

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
	const VertexDataPtr &vertices = (m_subMesh->vertices)
		? m_subMesh->vertices
		: m_subMesh->parent()->sharedVertices;

	g_gpu->draw(PrimitiveType::kTriangleList, vertices, m_subMesh->indices);
}

/** Initialize the mesh renderer.
 * @param entity	Entity the component belongs to.
 * @param mesh		Mesh to render. */
MeshRendererComponent::MeshRendererComponent(Entity *entity, MeshPtr mesh) :
	RendererComponent(entity),
	m_mesh(mesh),
	m_materials(mesh->numMaterials())
{}

/** Get the material with the specified name.
 * @param name		Name of the material to get.
 * @return		Pointer to material set. */
MaterialPtr MeshRendererComponent::material(const std::string &name) const {
	size_t index = 0;
	bool ret = m_mesh->material(name, index);
	orionCheck(ret, "Material slot '%s' not found", name.c_str());

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
void MeshRendererComponent::setMaterial(const std::string &name, MaterialPtr material) {
	size_t index = 0;
	bool ret = m_mesh->material(name, index);
	orionCheck(ret, "Material slot '%s' not found", name.c_str());

	m_materials[index] = material;
}

/** Create scene entities.
 * @param entities	List to populate. */
void MeshRendererComponent::createSceneEntities(SceneEntityList &entities) {
	// FIXME: This is a bit shit, should destroy them on deactivate and recreate each time
	// also need to handle changes to mesh material/submesh count etc
	for(size_t i = 0; i < m_mesh->numSubMeshes(); i++) {
		SubMeshSceneEntity *entity = new SubMeshSceneEntity(m_mesh->subMesh(i), this);
		entities.push_back(entity);
	}
}

/**
 * Game code.
 */

class CustomBehaviour : public BehaviourComponent {
public:
	CustomBehaviour(Entity *entity) : BehaviourComponent(entity) {}

	void activated() { orionLog(LogLevel::kDebug, "Entity was activated"); }
	void deactivated() { orionLog(LogLevel::kDebug, "Entity was deactivated"); }

	void tick(float dt) {
		entity()->rotate(dt * 90.0f, glm::vec3(0.0, 1.0, 0.0));
	}
};

/** Game class. */
class TestGame : public Game {
public:
	TestGame();
private:
	//Entity *makeCube(Entity *parent, const std::string &name);
	//Entity *makePlane(Entity *parent, const std::string &name);
private:
	World *m_world;			/**< Game world. */

	/** Rendering resources. */
	MaterialPtr m_cubeMaterial;
	MeshPtr m_cubeMesh;
	//VertexFormatPtr m_vertexFormat;
};

#if 0
Entity *TestGame::makeCube(Entity *parent, const std::string &name) {
	/* Indices into the below arrays for each face. */
	static size_t cubeIndices[] = {
		/* Front face. */
		0, 1, 2, 2, 3, 0,
		/* Back face. */
		5, 4, 7, 7, 6, 5,
		/* Left face. */
		4, 0, 3, 3, 7, 4,
		/* Right face. */
		1, 5, 6, 6, 2, 1,
		/* Top face. */
		3, 2, 6, 6, 7, 3,
		/* Bottom face. */
		4, 5, 1, 1, 0, 4,
	};

	/* Vertices of a cube. */
	static glm::vec3 cubeVertices[] = {
		glm::vec3(-0.5f, -0.5f, 0.5f),
		glm::vec3(0.5f, -0.5f, 0.5f),
		glm::vec3(0.5f, 0.5f, 0.5f),
		glm::vec3(-0.5f, 0.5f, 0.5f),
		glm::vec3(-0.5f, -0.5f, -0.5f),
		glm::vec3(0.5f, -0.5f, -0.5f),
		glm::vec3(0.5f, 0.5f, -0.5f),
		glm::vec3(-0.5f, 0.5f, -0.5f),
	};

	/* Normals for each face. */
	static glm::vec3 cubeNormals[] = {
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 0.0f, -1.0f),
		glm::vec3(-1.0f, 0.0f, 0.0f),
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(0.0f, -1.0f, 0.0f),
	};

	/* Texture coordinates for each face. */
	static glm::vec2 cubeTexcoords[] = {
		glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 0.0f),
		glm::vec2(1.0f, 1.0f), glm::vec2(1.0f, 1.0f),
		glm::vec2(0.0f, 1.0f), glm::vec2(0.0f, 0.0f),
	};

	GPUBufferPtr buffer = g_gpu->createBuffer(
		GPUBuffer::kVertexBuffer,
		GPUBuffer::kStaticDrawUsage,
		util::arraySize(cubeIndices) * sizeof(Vertex));

	{
		GPUBufferMapper<Vertex> data(buffer, GPUBuffer::kMapInvalidate, GPUBuffer::kWriteAccess);

		for(size_t i = 0; i < util::arraySize(cubeIndices); i++) {
			new(&data[i]) Vertex(
				cubeVertices[cubeIndices[i]],
				cubeNormals[i / 6],
				cubeTexcoords[i % 6]);
		}
	}

	VertexDataPtr vertices = g_gpu->createVertexData(util::arraySize(cubeIndices));
	vertices->setFormat(m_vertexFormat);
	vertices->setBuffer(0, buffer);
	vertices->finalize();

	Entity *entity = parent->createChild(name);
	StaticMeshRendererComponent *renderer = entity->createComponent<StaticMeshRendererComponent>(m_material, vertices, nullptr);
	renderer->setActive(true);

	return entity;
}

Entity *TestGame::makePlane(Entity *parent, const std::string &name) {
	/* Vertices of the plane. */
	static glm::vec3 planeVertices[] = {
		glm::vec3(-1.0f, -1.0f, 0.0f),
		glm::vec3(1.0f, -1.0f, 0.0f),
		glm::vec3(1.0f, 1.0f, 0.0f),
		glm::vec3(-1.0f, 1.0f, 0.0f),
	};

	/* We only have a single normal. */
	static glm::vec3 planeNormal(0.0f, 0.0f, 1.0f);

	/* Plane colour. */
	static glm::vec2 planeTexcoords[] = {
		glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 0.0f),
		glm::vec2(1.0f, 1.0f), glm::vec2(0.0f, 1.0f),
	};

	GPUBufferPtr buffer = g_gpu->createBuffer(
		GPUBuffer::kVertexBuffer,
		GPUBuffer::kStaticDrawUsage,
		6 * sizeof(Vertex));

	{
		GPUBufferMapper<Vertex> data(buffer, GPUBuffer::kMapInvalidate, GPUBuffer::kWriteAccess);

		new(&data[0]) Vertex(planeVertices[0], planeNormal, planeTexcoords[0]);
		new(&data[1]) Vertex(planeVertices[1], planeNormal, planeTexcoords[1]);
		new(&data[2]) Vertex(planeVertices[2], planeNormal, planeTexcoords[2]);
		new(&data[3]) Vertex(planeVertices[2], planeNormal, planeTexcoords[2]);
		new(&data[4]) Vertex(planeVertices[3], planeNormal, planeTexcoords[3]);
		new(&data[5]) Vertex(planeVertices[0], planeNormal, planeTexcoords[0]);
	}

	VertexDataPtr vertices = g_gpu->createVertexData(6);
	vertices->setFormat(m_vertexFormat);
	vertices->setBuffer(0, buffer);
	vertices->finalize();

	Entity *entity = parent->createChild(name);
	StaticMeshRendererComponent *renderer = entity->createComponent<StaticMeshRendererComponent>(m_material, vertices, nullptr);
	renderer->setActive(true);

	return entity;
}
#endif

/** Initialize the game world. */
TestGame::TestGame() {
	m_cubeMaterial = g_assetManager->load<Material>("game/materials/companion_cube");
	m_cubeMesh = g_assetManager->load<Mesh>("game/models/companion_cube");

#if 0
	m_vertexFormat = g_gpu->createVertexFormat();
	m_vertexFormat->addBuffer(0, sizeof(Vertex));
	m_vertexFormat->addAttribute(
		VertexAttribute::kPositionSemantic, 0,
		VertexAttribute::kFloatType, 3, 0, offsetof(Vertex, x));
	m_vertexFormat->addAttribute(
		VertexAttribute::kNormalSemantic, 0,
		VertexAttribute::kFloatType, 3, 0, offsetof(Vertex, nx));
	m_vertexFormat->addAttribute(
		VertexAttribute::kTexCoordSemantic, 0,
		VertexAttribute::kFloatType, 2, 0, offsetof(Vertex, u));
	m_vertexFormat->finalize();
#endif

	m_world = g_engine->createWorld();

	AmbientLightComponent *ambientLight = m_world->root()->createComponent<AmbientLightComponent>();
	ambientLight->setIntensity(0.1f);
	ambientLight->setActive(true);

	//Entity *floor = makePlane(m_world->root(), "floor");
	//floor->rotate(-90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
	//floor->setScale(glm::vec3(3.0f, 7.5f, 1.0f));
	//floor->setActive(true);

	//Entity *cube = makeCube(m_world->root(), "cube");
	Entity *cube = m_world->createEntity("cube");
	cube->setPosition(glm::vec3(0.0f, 0.5f, -4.0f));
	cube->setScale(glm::vec3(0.2f, 0.2f, 0.2f));
	cube->rotate(45.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	cube->setActive(true);
	MeshRendererComponent *renderer = cube->createComponent<MeshRendererComponent>(m_cubeMesh);
	renderer->setMaterial("Material.004", m_cubeMaterial);
	renderer->setActive(true);
	CustomBehaviour *behaviour = cube->createComponent<CustomBehaviour>();
	behaviour->setActive(true);

	Entity *camEntity = m_world->createEntity("camera");
	camEntity->setPosition(glm::vec3(0.0f, 1.5f, 0.0f));
	camEntity->setActive(true);
	CameraComponent *camera = camEntity->createComponent<CameraComponent>();
	camera->perspective(90.0f, 0.1f, 1000.0f);
	camera->setActive(true);

	Entity *lightEntity = m_world->createEntity("light");
	lightEntity->setPosition(glm::vec3(0.0f, 2.0f, -2.0f));
	lightEntity->setActive(true);
	PointLightComponent *pointLight = lightEntity->createComponent<PointLightComponent>();
	pointLight->setActive(true);
}

/**
 * Game code interface.
 */

/** Get the engine configuration.
 * @param config	Engine configuration to fill in. */
void game::engineConfiguration(EngineConfiguration &config) {
	config.title = "Cubes";
	config.graphicsAPI = EngineConfiguration::kGLGraphicsAPI;
	config.displayWidth = 1440;
	config.displayHeight = 900;
	config.displayFullscreen = false;
	config.displayVsync = false;
}

/** Create the Game instance.
 * @return		Created Game instance. */
Game *game::createGame() {
	return new TestGame;
}

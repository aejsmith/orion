/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Test game.
 */

#include "engine/asset_manager.h"
#include "engine/engine.h"
#include "engine/game.h"
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

struct Vertex {
	float x, y, z, _pad1;
	float nx, ny, nz, _pad2;
	float u, v, _pad3, _pad4;
public:
	Vertex(const glm::vec3 &pos, const glm::vec3 &normal, const glm::vec2 &texcoord) :
		x(pos.x), y(pos.y), z(pos.z),
		nx(normal.x), ny(normal.y), nz(normal.z),
		u(texcoord.x), v(texcoord.y)
	{}

	Vertex() {}
};

class StaticMeshSceneEntity : public SceneEntity {
public:
	StaticMeshSceneEntity(VertexDataPtr vertices, IndexDataPtr indices) :
		m_vertices(vertices),
		m_indices(indices)
	{}

	void render() {
		g_gpu->draw(PrimitiveType::kTriangleList, m_vertices, m_indices);
	}
private:
	VertexDataPtr m_vertices;
	IndexDataPtr m_indices;
};

class StaticMeshRendererComponent : public RendererComponent {
public:
	StaticMeshRendererComponent(Entity *entity, VertexDataPtr vertices, IndexDataPtr indices) :
		RendererComponent(entity)
	{
		m_sceneEntity = new StaticMeshSceneEntity(vertices, indices);
	}

	virtual void createSceneEntities(SceneEntityList &entities) override {
		entities.push_back(m_sceneEntity);
	}
private:
	StaticMeshSceneEntity *m_sceneEntity;
};

class CustomBehaviour : public BehaviourComponent {
public:
	CustomBehaviour(Entity *entity) : BehaviourComponent(entity) {}

	void activated() { orionLog(LogLevel::kDebug, "Entity was activated"); }
	void deactivated() { orionLog(LogLevel::kDebug, "Entity was deactivated"); }

	void tick(float dt) {
		entity()->rotate(dt * 90.0f, glm::vec3(0.0, 1.0, 0.0));
	}
};

/**
 * Game code.
 */

/** Game class. */
class TestGame : public Game {
public:
	TestGame();
private:
	Entity *makeCube(Entity *parent, const std::string &name);
	Entity *makePlane(Entity *parent, const std::string &name);
private:
	World *m_world;			/**< Game world. */

	/** Rendering resources. */
	Texture2DPtr m_texture;
	VertexFormatPtr m_vertexFormat;
};

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
	StaticMeshRendererComponent *renderer = entity->createComponent<StaticMeshRendererComponent>(vertices, nullptr);
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
	StaticMeshRendererComponent *renderer = entity->createComponent<StaticMeshRendererComponent>(vertices, nullptr);
	renderer->setActive(true);

	return entity;
}

/** Initialize the game world. */
TestGame::TestGame() {
	m_texture = g_assetManager->load<Texture2D>("game/textures/test");
	orionLog(LogLevel::kDebug, "Got asset %p", m_texture.get());
	g_gpu->bindTexture(0, m_texture->gpu());

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

	m_world = g_engine->createWorld();

	AmbientLightComponent *ambientLight = m_world->root()->createComponent<AmbientLightComponent>();
	ambientLight->setIntensity(0.1f);
	ambientLight->setActive(true);

	Entity *floor = makePlane(m_world->root(), "floor");
	floor->rotate(-90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
	floor->setScale(glm::vec3(3.0f, 7.5f, 1.0f));
	floor->setActive(true);

	Entity *cube = makeCube(m_world->root(), "cube");
	cube->setPosition(glm::vec3(0.0f, 0.5f, -4.0f));
	cube->rotate(45.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	cube->setActive(true);
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
	config.title = "Orion";
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
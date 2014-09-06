/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Main entry point of the engine.
 */

#include "asset/asset_loader.h"
#include "asset/asset_manager.h"
#include "asset/texture.h"

#include "core/engine.h"

#include "gpu/gpu.h"

#include "lib/path.h"
#include "lib/utility.h"

#include "render/scene_entity.h"
#include "render/scene_view.h"

#include "world/behaviour.h"
#include "world/camera_component.h"
#include "world/entity.h"
#include "world/light_component.h"
#include "world/renderer_component.h"
#include "world/world.h"

/**
 * Asset test code.
 */

class TestAsset : public Asset {
public:
	TestAsset() {}
};

class TestAssetLoader : public AssetLoader {
public:
	TestAssetLoader() : AssetLoader("tga") {}

	/** Load the asset.
	 * @param stream	Stream containing asset data.
	 * @param attributes	Attributes specified in metadata.
	 * @param path		Path to asset (supplied so that useful error
	 *			messages can be logged).
	 * @return		Pointer to loaded asset, null on failure. */
	Asset *load(DataStream *stream, rapidjson::Value &attributes, const char *path) override {
		orion_log(LogLevel::kDebug, "TestAssetLoader::load('%s')", path);
		orion_log(LogLevel::kDebug, "meow = '%s'", attributes["meow"].GetString());
		return new TestAsset;
	}
};

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
		g_engine->gpu()->draw(PrimitiveType::kTriangleList, m_vertices, m_indices);
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
		m_scene_entity = new StaticMeshSceneEntity(vertices, indices);
	}

	virtual void create_scene_entities(SceneEntityList &entities) {
		entities.push_back(m_scene_entity);
	}
private:
	StaticMeshSceneEntity *m_scene_entity;
};

class CustomBehaviour : public BehaviourComponent {
public:
	CustomBehaviour(Entity *entity) : BehaviourComponent(entity) {}

	void activated() { orion_log(LogLevel::kDebug, "Entity was activated"); }
	void deactivated() { orion_log(LogLevel::kDebug, "Entity was deactivated"); }

	void tick(float dt) {
		entity()->rotate(dt * 90.0f, glm::vec3(0.0, 1.0, 0.0));
	}
};

static VertexFormatPtr test_vertex_format;

static Entity *make_cube(Entity *parent, const std::string &name) {
	/* Indices into the below arrays for each face. */
	static size_t cube_indices[] = {
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
	static glm::vec3 cube_vertices[] = {
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
	static glm::vec3 cube_normals[] = {
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 0.0f, -1.0f),
		glm::vec3(-1.0f, 0.0f, 0.0f),
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(0.0f, -1.0f, 0.0f),
	};

	/* Texture coordinates for each face. */
	static glm::vec2 cube_texcoords[] = {
		glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 0.0f),
		glm::vec2(1.0f, 1.0f), glm::vec2(1.0f, 1.0f),
		glm::vec2(0.0f, 1.0f), glm::vec2(0.0f, 0.0f),
	};

	GPUBufferPtr buffer = g_engine->gpu()->create_buffer(
		GPUBuffer::kVertexBuffer,
		GPUBuffer::kStaticDrawUsage,
		util::array_size(cube_indices) * sizeof(Vertex));

	{
		GPUBufferMapper<Vertex> data(buffer, GPUBuffer::kMapInvalidate, GPUBuffer::kWriteAccess);

		for(size_t i = 0; i < util::array_size(cube_indices); i++) {
			new(&data[i]) Vertex(
				cube_vertices[cube_indices[i]],
				cube_normals[i / 6],
				cube_texcoords[i % 6]);
		}
	}

	VertexDataPtr vertices = g_engine->gpu()->create_vertex_data(util::array_size(cube_indices));
	vertices->set_format(test_vertex_format);
	vertices->set_buffer(0, buffer);
	vertices->finalize();

	Entity *entity = parent->create_child(name);
	StaticMeshRendererComponent *renderer = entity->create_component<StaticMeshRendererComponent>(vertices, nullptr);
	renderer->set_active(true);

	return entity;
}

static Entity *make_plane(Entity *parent, const std::string &name) {
	/* Vertices of the plane. */
	static glm::vec3 plane_vertices[] = {
		glm::vec3(-1.0f, -1.0f, 0.0f),
		glm::vec3(1.0f, -1.0f, 0.0f),
		glm::vec3(1.0f, 1.0f, 0.0f),
		glm::vec3(-1.0f, 1.0f, 0.0f),
	};

	/* We only have a single normal. */
	static glm::vec3 plane_normal(0.0f, 0.0f, 1.0f);

	/* Plane colour. */
	static glm::vec2 plane_texcoords[] = {
		glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 0.0f),
		glm::vec2(1.0f, 1.0f), glm::vec2(0.0f, 1.0f),
	};

	GPUBufferPtr buffer = g_engine->gpu()->create_buffer(
		GPUBuffer::kVertexBuffer,
		GPUBuffer::kStaticDrawUsage,
		6 * sizeof(Vertex));

	{
		GPUBufferMapper<Vertex> data(buffer, GPUBuffer::kMapInvalidate, GPUBuffer::kWriteAccess);

		new(&data[0]) Vertex(plane_vertices[0], plane_normal, plane_texcoords[0]);
		new(&data[1]) Vertex(plane_vertices[1], plane_normal, plane_texcoords[1]);
		new(&data[2]) Vertex(plane_vertices[2], plane_normal, plane_texcoords[2]);
		new(&data[3]) Vertex(plane_vertices[2], plane_normal, plane_texcoords[2]);
		new(&data[4]) Vertex(plane_vertices[3], plane_normal, plane_texcoords[3]);
		new(&data[5]) Vertex(plane_vertices[0], plane_normal, plane_texcoords[0]);
	}

	VertexDataPtr vertices = g_engine->gpu()->create_vertex_data(6);
	vertices->set_format(test_vertex_format);
	vertices->set_buffer(0, buffer);
	vertices->finalize();

	Entity *entity = parent->create_child(name);
	StaticMeshRendererComponent *renderer = entity->create_component<StaticMeshRendererComponent>(vertices, nullptr);
	renderer->set_active(true);

	return entity;
}

/** Main function of the engine.
 * @param argc		Argument count.
 * @param argv		Argument array. */
int main(int argc, char **argv) {
	EngineConfiguration config;
	config.title = "Orion";
	config.graphics_api = EngineConfiguration::kGLGraphicsAPI;
	config.display_width = 1440;
	config.display_height = 900;
	config.display_fullscreen = false;
	config.display_vsync = false;

	Engine engine(config);

	g_engine->assets()->register_loader(new TestAssetLoader);

	TypedAssetPtr<TestAsset> asset = g_engine->assets()->load<TestAsset>("game/textures/test");
	orion_log(LogLevel::kDebug, "Got asset %p", asset.get());

	test_vertex_format = g_engine->gpu()->create_vertex_format();
	test_vertex_format->add_buffer(0, sizeof(Vertex));
	test_vertex_format->add_attribute(
		VertexAttribute::kPositionSemantic, 0,
		VertexAttribute::kFloatType, 3, 0, offsetof(Vertex, x));
	test_vertex_format->add_attribute(
		VertexAttribute::kNormalSemantic, 0,
		VertexAttribute::kFloatType, 3, 0, offsetof(Vertex, nx));
	test_vertex_format->add_attribute(
		VertexAttribute::kTexCoordSemantic, 0,
		VertexAttribute::kFloatType, 2, 0, offsetof(Vertex, u));
	test_vertex_format->finalize();

	Texture2D *texture = new Texture2D(1024, 1024);
	{
		std::unique_ptr<uint32_t []> buf(new uint32_t[1024 * 1024]);
		for(size_t i = 0; i < 1024; i++) {
			for(size_t j = 0; j < 1024; j++) {
				if(i >= 256 && i < 768 && j >= 256 && j < 768) {
					buf[(i * 1024) + j] = 0xffff0000;
				} else {
					buf[(i * 1024) + j] = 0xffffffff;
				}
			}
		}
		texture->update(buf.get());
	}
	g_engine->gpu()->bind_texture(0, texture->gpu());

	World *world = engine.create_world();

	AmbientLightComponent *ambient_light = world->root()->create_component<AmbientLightComponent>();
	ambient_light->set_intensity(0.1f);
	ambient_light->set_active(true);

	Entity *floor = make_plane(world->root(), "floor");
	floor->rotate(-90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
	floor->set_scale(glm::vec3(3.0f, 7.5f, 1.0f));
	floor->set_active(true);

	Entity *cube = make_cube(world->root(), "cube");
	cube->set_position(glm::vec3(0.0f, 0.5f, -4.0f));
	cube->rotate(45.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	cube->set_active(true);
	CustomBehaviour *behaviour = cube->create_component<CustomBehaviour>();
	behaviour->set_active(true);

	Entity *cam_entity = world->create_entity("camera");
	cam_entity->set_position(glm::vec3(0.0f, 1.5f, 0.0f));
	cam_entity->set_active(true);
	CameraComponent *camera = cam_entity->create_component<CameraComponent>();
	camera->perspective(90.0f, 0.1f, 1000.0f);
	camera->set_active(true);

	Entity *light_entity = world->create_entity("light");
	light_entity->set_position(glm::vec3(0.0f, 2.0f, -2.0f));
	light_entity->set_active(true);
	PointLightComponent *point_light = light_entity->create_component<PointLightComponent>();
	point_light->set_active(true);

	engine.run();

	/* FIXME: This is somewhat a hack for now. We have a problem with
	 * destruction of global objects, in particular GPU resource pointers.
	 * These are effectively destroyed when m_gpu is deleted above, but
	 * their destructors will be called after this, leading to crashes. */
	_Exit(0);
}

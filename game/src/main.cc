/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Main entry point of the engine.
 */

#include "core/engine.h"

#include "gpu/gpu.h"

#include "world/behaviour.h"
#include "world/entity.h"
#include "world/world.h"

struct Vertex {
	float x, y, z, _pad1;
	float nx, ny, nz, _pad2;
	float r, g, b, a;
public:
	Vertex(const glm::vec3 &pos, const glm::vec3 &normal, const glm::vec4 &colour) :
		x(pos.x), y(pos.y), z(pos.z),
		nx(normal.x), ny(normal.y), nz(normal.z),
		r(colour.x), g(colour.y), b(colour.z), a(colour.w)
	{}

	Vertex() {}
};

struct CameraUniforms {
	float view[16];
	float projection[16];
	float view_projection[16];
};

class CustomBehaviour : public Behaviour {
public:
	CustomBehaviour(Entity *entity) : Behaviour(entity) {}

	void activated() { orion_log(LogLevel::kDebug, "Entity was activated"); }
	void deactivated() { orion_log(LogLevel::kDebug, "Entity was deactivated"); }
};

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

	World *world = new World;
	Entity *entity = new Entity("test", world->root());
	entity->set_position(glm::vec3(0.0, 0.0, -10.0));
	entity->set_active(true);
	Entity *child = new Entity("child", entity);
	child->set_position(glm::vec3(0.0, 2.0, 0.0));
	child->set_active(true);
	CustomBehaviour *behaviour = new CustomBehaviour(child);
	behaviour->set_active(true);

	GPUBufferPtr vertex_buffer = g_gpu->create_buffer(
		GPUBuffer::kVertexBuffer,
		GPUBuffer::kStaticDrawUsage,
		3 * sizeof(Vertex));

	{
		GPUBufferMapper<Vertex> data(vertex_buffer,
			GPUBuffer::kMapInvalidate,
			GPUBuffer::kWriteAccess);

		new(&data[0]) Vertex(
			glm::vec3(0.0, 1.0, 0.0),
			glm::vec3(0.0, 0.0, 1.0),
			glm::vec4(1.0, 0.0, 0.0, 1.0));
		new(&data[1]) Vertex(
			glm::vec3(-1.0, -1.0, 0.0),
			glm::vec3(0.0, 0.0, 1.0),
			glm::vec4(0.0, 1.0, 0.0, 1.0));
		new(&data[2]) Vertex(
			glm::vec3(1.0, -1.0, 0.0),
			glm::vec3(0.0, 0.0, 1.0),
			glm::vec4(0.0, 0.0, 1.0, 1.0));
	}

	VertexFormatPtr format = g_gpu->create_vertex_format();
	format->add_buffer(0, sizeof(Vertex));
	format->add_attribute(
		VertexAttribute::kPositionSemantic, 0,
		VertexAttribute::kFloatType, 3, 0, offsetof(Vertex, x));
	format->add_attribute(
		VertexAttribute::kNormalSemantic, 0,
		VertexAttribute::kFloatType, 3, 0, offsetof(Vertex, nx));
	format->add_attribute(
		VertexAttribute::kDiffuseSemantic, 0,
		VertexAttribute::kFloatType, 4, 0, offsetof(Vertex, r));
	format->finalize();

	VertexDataPtr vertices = g_gpu->create_vertex_data(3);
	vertices->set_format(format);
	vertices->set_buffer(0, vertex_buffer);
	vertices->finalize();

	GPUProgramPtr vertex_program = g_gpu->load_program(
		"engine/assets/shaders/test_vtx.glsl",
		GPUProgram::kVertexProgram);
	vertex_program->bind_uniforms("EntityUniforms", 0);
	vertex_program->bind_uniforms("CameraUniforms", 1);

	GPUProgramPtr frag_program = g_gpu->load_program(
		"engine/assets/shaders/test_frag.glsl",
		GPUProgram::kFragmentProgram);

	GPUPipelinePtr pipeline = g_gpu->create_pipeline();
	pipeline->set_program(GPUProgram::kVertexProgram, vertex_program);
	pipeline->set_program(GPUProgram::kFragmentProgram, frag_program);
	pipeline->finalize();

	glm::vec3 camera_position(0.0, 0.0, 0.0);
	glm::quat camera_orientation(1.0, 0.0, 0.0, 0.0);
	glm::mat4 view =
		glm::mat4_cast(glm::inverse(camera_orientation)) *
		glm::translate(glm::mat4(), -camera_position);

	float aspect = 1440.0f / 900.0f;
	float fovx = glm::radians(90.0f);
	float fovy = 2.0f * atanf(tanf(fovx * 0.5f) / aspect);
	glm::mat4 projection = glm::perspective(fovy, aspect, 0.1f, 1000.0f);

	GPUBufferPtr uniform_buffer = g_gpu->create_buffer(
		GPUBuffer::kUniformBuffer,
		GPUBuffer::kDynamicDrawUsage,
		sizeof(CameraUniforms));

	{
		GPUBufferMapper<CameraUniforms> uniforms(
			uniform_buffer,
			GPUBuffer::kMapInvalidate,
			GPUBuffer::kWriteAccess);

		glm::mat4 view_projection = projection * view;
		memcpy(&uniforms->view, glm::value_ptr(view), sizeof(uniforms->view));
		memcpy(&uniforms->projection, glm::value_ptr(projection), sizeof(uniforms->projection));
		memcpy(&uniforms->view_projection, glm::value_ptr(view_projection), sizeof(uniforms->view_projection));
	}

	while(true) {
		entity->rotate(0.02f, glm::vec3(0.0, 0.0, 1.0));

		g_gpu->clear(
			RenderBuffer::kColourBuffer | RenderBuffer::kDepthBuffer,
			glm::vec4(0.0, 0.0, 0.4, 1.0), 1.0, 0);

		g_gpu->bind_pipeline(pipeline);
		g_gpu->bind_uniform_buffer(0, child->uniforms());
		g_gpu->bind_uniform_buffer(1, uniform_buffer);
		g_gpu->draw(PrimitiveType::kTriangleList, vertices, nullptr);

		if(!engine.loop())
			break;
	}

	return 0;
}

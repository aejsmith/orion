/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Main entry point of the engine.
 */

#include "core/engine.h"

#include "gpu/gpu.h"

#include <glm/gtc/type_ptr.hpp>

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

struct ObjectParams {
	float mvp_matrix[16];
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

	GPUBufferPtr vertex_buffer = g_gpu->create_buffer(
		GPUBuffer::kVertexBuffer,
		GPUBuffer::kStaticDrawUsage,
		3 * sizeof(Vertex));

	{
		GPUBufferMapper<Vertex> data(
			vertex_buffer,
			GPUBuffer::kMapInvalidate,
			GPUBuffer::kWriteAccess);

		new(&data[0]) Vertex(
			glm::vec3(0.0, 1.0, -1.0),
			glm::vec3(0.0, 0.0, 1.0),
			glm::vec4(1.0, 0.0, 0.0, 1.0));
		new(&data[1]) Vertex(
			glm::vec3(-1.0, -1.0, -1.0),
			glm::vec3(0.0, 0.0, 1.0),
			glm::vec4(0.0, 1.0, 0.0, 1.0));
		new(&data[2]) Vertex(
			glm::vec3(1.0, -1.0, -1.0),
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
	vertex_program->bind_uniforms("ObjectParams", 0);

	GPUProgramPtr frag_program = g_gpu->load_program(
		"engine/assets/shaders/test_frag.glsl",
		GPUProgram::kFragmentProgram);

	GPUPipelinePtr pipeline = g_gpu->create_pipeline();
	pipeline->set_program(GPUProgram::kVertexProgram, vertex_program);
	pipeline->set_program(GPUProgram::kFragmentProgram, frag_program);
	pipeline->finalize();

	glm::mat4 mvp_matrix(
		1.000000238418579, 0, 0, 0,
		0, 1.600000262260437, 0, 0,
		0, 0, -1.000199913978577, -1,
		0, 0, 9.801979064941406, 10);

	GPUBufferPtr uniform_buffer = g_gpu->create_buffer(
		GPUBuffer::kUniformBuffer,
		GPUBuffer::kDynamicDrawUsage,
		sizeof(ObjectParams));

	while(true) {
		g_gpu->clear(
			RenderBuffer::kColourBuffer | RenderBuffer::kDepthBuffer,
			glm::vec4(0.0, 0.0, 0.4, 1.0), 1.0, 0);

		{
			GPUBufferMapper<ObjectParams> params(
				uniform_buffer,
				GPUBuffer::kMapInvalidate,
				GPUBuffer::kWriteAccess);

			memcpy(&params->mvp_matrix, glm::value_ptr(mvp_matrix), sizeof(params->mvp_matrix));
		}

		g_gpu->bind_pipeline(pipeline);
		g_gpu->bind_uniform_buffer(0, uniform_buffer);
		g_gpu->draw(PrimitiveType::kTriangleList, vertices, nullptr);

		if(!engine.loop())
			break;
	}

	return 0;
}

/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Main entry point of the engine.
 */

#include "core/engine.h"

#include "gpu/gpu.h"

struct Vertex {
	float x, y, z, _pad1;
	float nx, ny, nz, _pad2;
	float r, g, b, _pad3;
public:
	Vertex(const glm::vec3 &pos, const glm::vec3 &normal, const glm::vec3 &colour) :
		x(pos.x), y(pos.y), z(pos.z),
		nx(normal.x), ny(normal.y), nz(normal.z),
		r(colour.x), g(colour.y), b(colour.z)
	{}

	Vertex() {}
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

	std::vector<Vertex> data;
	data.push_back(Vertex(
		glm::vec3(0.0, 1.0, -1.0),
		glm::vec3(0.0, 0.0, 1.0),
		glm::vec3(1.0, 0.0, 0.0)));
	data.push_back(Vertex(
		glm::vec3(-1.0, -1.0, -1.0),
		glm::vec3(0.0, 0.0, 1.0),
		glm::vec3(0.0, 1.0, 0.0)));
	data.push_back(Vertex(
		glm::vec3(1.0, -1.0, -1.0),
		glm::vec3(0.0, 0.0, 1.0),
		glm::vec3(0.0, 0.0, 1.0)));

	GPUBufferPtr buffer = g_gpu->create_buffer(
		GPUBuffer::kVertexBuffer,
		GPUBuffer::kStaticDrawUsage,
		sizeof(Vertex) * data.size());
	buffer->write(&data[0], data.size() * sizeof(Vertex), 0);

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
		VertexAttribute::kFloatType, 3, 0, offsetof(Vertex, r));

	format->finalize();

	VertexDataPtr vertices = g_gpu->create_vertex_data(3);
	vertices->set_format(format);
	vertices->set_buffer(0, buffer);
	vertices->finalize();

	GPUProgramPtr vertex_program = g_gpu->load_program(
		"engine/assets/shaders/test_vtx.glsl",
		GPUProgram::kVertexProgram);
	GPUProgramPtr frag_program = g_gpu->load_program(
		"engine/assets/shaders/test_frag.glsl",
		GPUProgram::kFragmentProgram);

	GPUPipelinePtr pipeline = g_gpu->create_pipeline();
	pipeline->set_program(GPUProgram::kVertexProgram, vertex_program);
	pipeline->set_program(GPUProgram::kFragmentProgram, frag_program);
	pipeline->finalize();

	while(true) {
		g_gpu->clear(
			RenderBuffer::kColourBuffer | RenderBuffer::kDepthBuffer,
			glm::vec4(0.0, 0.0, 0.75, 1.0), 1.0, 0);

		g_gpu->bind_pipeline(pipeline);
		g_gpu->draw(PrimitiveType::kTriangleList, vertices, nullptr);

		if(!engine.loop())
			break;
	}

	return 0;
}

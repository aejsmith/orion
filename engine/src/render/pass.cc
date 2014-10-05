/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Shader pass class.
 *
 * @todo		Having the loading shaders array is a bit irritating.
 */

#include "core/filesystem.h"

#include "gpu/gpu.h"

#include "render/pass.h"
#include "render/scene_light.h"
#include "render/shader.h"
#include "render/uniform_buffer.h"

/** Initialize the pass.
 * @param parent	Parent shader.
 * @param type		Type of the pass. */
Pass::Pass(Shader *parent, Type type) :
	m_parent(parent),
	m_type(type)
{}

/** Destroy the pass. */
Pass::~Pass() {}

/**
 * Set pass draw state.
 *
 * Sets the draw state for this pass. Pass draw state is independent from the
 * material, therefore can be set once for all entities/materials being drawn
 * with this pass.
 */
void Pass::setDrawState() const {
	check(m_pipeline);
	g_gpu->bindPipeline(m_pipeline);
}

/** Add a uniform block declaration to a source string.
 * @param source	Source string to add to.
 * @param uniformStruct	Uniform structure to add. */
static void declareUniformBlock(std::string &source, const UniformStruct *uniformStruct) {
	source += util::format("layout(std140) uniform %s {\n", uniformStruct->name);

	for(const UniformStructMember &member : uniformStruct->members) {
		source += util::format("\t%s %s;\n",
			ShaderParameter::glslType(member.type),
			member.name);
	}

	if(uniformStruct->instanceName && strlen(uniformStruct->instanceName)) {
		source += util::format("} %s;\n\n", uniformStruct->instanceName);
	} else {
		source += "};\n\n";
	}
}

/** Insert standard declarations into shader source.
 * @param source	Source string to add to. */
static void addStandardDeclarations(std::string &source) {
	/* Insert declarations for standard uniform blocks into the source. */
	for(const UniformStruct *uniformStruct : UniformStruct::structList())
		declareUniformBlock(source, uniformStruct);

	/* Light type definitions. */
	source += util::format("const int kAmbientLight = %d;\n", SceneLight::kAmbientLight);
	source += util::format("const int kDirectionalLight = %d;\n", SceneLight::kDirectionalLight);
	source += util::format("const int kPointLight = %d;\n", SceneLight::kPointLight);
	source += util::format("const int kSpotLight = %d;\n\n", SceneLight::kSpotLight);
}

/** Add a GPU shader to the pass.
 * @param stage		Stage to add this shader to.
 * @param path		Filesystem path to shader source.
 * @return		Whether the stage was loaded successfully. */
bool Pass::loadStage(GPUShader::Type stage, const Path &path) {
	check(!m_loadingShaders[stage]);

	std::unique_ptr<File> file(g_filesystem->openFile(path));
	if(!file) {
		logError("Cannot find shader source file '%s'", path.c_str());
		return false;
	}

	std::unique_ptr<char []> buf(new char[file->size() + 1]);
	buf[file->size()] = 0;
	if(!file->read(buf.get(), file->size())) {
		logError("Failed to read shader source file '%s'", path.c_str());
		return false;
	}

	/* Add standard declarations to the shader. */
	std::string source;
	addStandardDeclarations(source);

	/* If there is a shader-specific uniform structure, add it. */
	if(m_parent->uniformStruct())
		declareUniformBlock(source, m_parent->uniformStruct());

	/* Add in the loaded program source. */
	source += buf.get();

	/* Compile the shader. */
	GPUShaderPtr shader = g_gpu->compileShader(stage, source);
	if(!shader)
		return false;

	/* Bind the uniform blocks. */
	GPUShader::ResourceList uniformBlocks;
	shader->queryUniformBlocks(uniformBlocks);
	for(const GPUShader::Resource &uniformBlock : uniformBlocks) {
		const UniformStruct *uniformStruct = (uniformBlock.name == "MaterialUniforms")
			? m_parent->uniformStruct()
			: UniformStruct::lookup(uniformBlock.name);
		if(!uniformStruct) {
			logError("Shader '%s' refers to unknown uniform block '%s'", path.c_str(), uniformBlock.name.c_str());
			return nullptr;
		}

		shader->bindUniformBlock(uniformBlock.index, uniformStruct->slot);
	}

	/* Bind texture samplers. */
	GPUShader::ResourceList samplers;
	shader->querySamplers(samplers);
	for(const GPUShader::Resource &sampler : samplers) {
		// TODO: global textures.
		const ShaderParameter *param = m_parent->lookupParameter(sampler.name);
		if(!param || param->type != ShaderParameter::kTextureType) {
			logError("Shader '%s' refers to unknown texture '%s'", path.c_str(), sampler.name.c_str());
			return nullptr;
		}

		shader->bindSampler(sampler.index, param->textureSlot);
	}

	m_loadingShaders[stage] = shader;
	return true;
}

/** Finalize the pass (called from Shader::addPass). */
void Pass::finalize() {
	m_pipeline = g_gpu->createPipeline(m_loadingShaders);
}

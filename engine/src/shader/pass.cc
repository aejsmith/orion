/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Shader pass class.
 *
 * @todo                Cache of loaded shaders, identify ones which are
 *                      identical and match them (e.g. ones which are the same
 *                      despite not being compiled with the same keywords.
 *                      Loading code would move from here to the shader cache.
 * @todo                Improved shader variation system (preprocessor defines).
 *                      Fixed sized array for light/shadow variations isn't
 *                      very nice. Want a system that allows shaders to define
 *                      what variations they actually support (e.g. Unity's
 *                      multi_compile pragma), and materials to be able to set
 *                      keywords. We would then compile for all possible
 *                      combinations, and then when getting a variation, form
 *                      a key by filtering selected keywords to ones supported
 *                      by the shader and look up based on that.
 * @todo                Don't need to compile shadow variation for ambient.
 */

#include "core/filesystem.h"
#include "core/string.h"

#include "gpu/gpu_manager.h"

#include "render/scene_light.h"

#include "shader/pass.h"
#include "shader/shader.h"
#include "shader/uniform_buffer.h"

/** Array of pass variation strings, indexed by pass type. */
static const char *passShaderVariations[Pass::kNumTypes] = {
    "BASIC_PASS",
    "FORWARD_PASS",
    "DEFERRED_PASS",
    "SHADOW_CASTER_PASS",
};

/** Array of light variation strings, indexed by light type. */
static const char *lightShaderVariations[SceneLight::kNumTypes] = {
    "AMBIENT_LIGHT",
    "DIRECTIONAL_LIGHT",
    "POINT_LIGHT",
    "SPOT_LIGHT",
};

/** Shadow variation string. */
static const char *const shadowVariation = "SHADOW";

/** Initialize the pass.
 * @param parent        Parent shader.
 * @param type          Type of the pass. */
Pass::Pass(Shader *parent, Type type) :
    m_parent(parent),
    m_type(type),
    m_variations((type == kForwardPass) ? SceneLight::kNumTypes * 2 : 1)
{}

/** Destroy the pass. */
Pass::~Pass() {}

/**
 * Set pass draw state.
 *
 * Sets the draw state for this pass. Pass draw state is independent from the
 * material, therefore can be set once for all entities/materials being drawn
 * with this pass.
 *
 * @param light         Light that the pass is being drawn with. This is used
 *                      to select which shader variations to use. It is ignored
 *                      for non-lit pass types.
 */
void Pass::setDrawState(SceneLight *light) const {
    /* Find the variation to use. */
    size_t index;
    if (m_type == kForwardPass) {
        index = light->type() * 2;
        if (light->castShadows())
            index++;
    } else {
        index = 0;
    }

    /* Bind the variation. */
    const Variation &variation = m_variations[index];
    check(variation.pipeline);
    g_gpuManager->bindPipeline(variation.pipeline);
}

/** Add a keyword definition.
 * @param source        Source string to add to.
 * @param keyword       Keyword to define. */
static void defineKeyword(std::string &source, const char *keyword) {
    source += String::format("#define %s 1\n", keyword);
}

/** Add a uniform block declaration to a source string.
 * @param source        Source string to add to.
 * @param uniformStruct Uniform structure to add. */
static void declareUniformBlock(std::string &source, const UniformStruct *uniformStruct) {
    source += String::format("layout(std140) uniform %s {\n", uniformStruct->name);

    for (const UniformStructMember &member : uniformStruct->members) {
        source += String::format("\t%s %s;\n",
            ShaderParameter::glslType(member.type),
            member.name);
    }

    if (uniformStruct->instanceName && strlen(uniformStruct->instanceName)) {
        source += String::format("} %s;\n\n", uniformStruct->instanceName);
    } else {
        source += "};\n\n";
    }
}

/** Load and pre-process a shader source file.
 * @param path          Path to file to open.
 * @param source        String to return shader source in.
 * @param depth         Recursion count.
 * @return              Whether the source was successfully loaded. */
static bool loadSource(const Path &path, std::string &source, unsigned depth = 0) {
    std::unique_ptr<File> file(g_filesystem->openFile(path));
    if (!file) {
        logError("Cannot find shader source file '%s'", path.c_str());
        return false;
    }

    std::string line;
    while (file->readLine(line)) {
        if (line.substr(0, 9) == "#include ") {
            if (depth == 16) {
                logError("Too many nested includes in '%s'", path.c_str());
                return false;
            }

            size_t pos = 9;

            while (isspace(line[pos]))
                pos++;
            if (line[pos++] != '"') {
                logError("Expected path string after #include in '%s'", path.c_str());
                return false;
            }

            size_t end = line.find_first_of('"', pos);

            /* Path is relative to the directory containing the current file. */
            Path includePath = path.directoryName() / line.substr(pos, end - pos);

            /* Load the source for this file. */
            if (!loadSource(includePath, source, depth + 1))
                return false;
        } else {
            source += line;
            source += "\n";
        }
    }

    return true;
}

/** Compile a single variation.
 * @param source        Source string to compile.
 * @param stage         Shader stage.
 * @param parent        Parent shader.
 * @param path          Path to source, used for error messages.
 * @return              Pointer to compiled shader, null on failure. */
static GPUShaderPtr compileVariation(const std::string &source, GPUShader::Type stage, Shader *parent, const Path &path) {
    /* Compile the shader. */
    GPUShaderPtr shader = g_gpuManager->compileShader(stage, source);
    if (!shader)
        return nullptr;

    /* Bind the uniform blocks. */
    GPUShader::ResourceList uniformBlocks;
    shader->queryUniformBlocks(uniformBlocks);
    for (const GPUShader::Resource &uniformBlock : uniformBlocks) {
        const UniformStruct *uniformStruct = (uniformBlock.name == "MaterialUniforms")
            ? parent->uniformStruct()
            : UniformStruct::lookup(uniformBlock.name);
        if (!uniformStruct) {
            logError("Shader '%s' refers to unknown uniform block '%s'", path.c_str(), uniformBlock.name.c_str());
            return nullptr;
        }

        shader->bindUniformBlock(uniformBlock.index, uniformStruct->slot);
    }

    /* Bind texture samplers. */
    GPUShader::ResourceList samplers;
    shader->querySamplers(samplers);
    for (const GPUShader::Resource &sampler : samplers) {
        // TODO: global textures.
        const ShaderParameter *param = parent->lookupParameter(sampler.name);
        if (!param || param->type != ShaderParameter::kTextureType) {
            logError("Shader '%s' refers to unknown texture '%s'", path.c_str(), sampler.name.c_str());
            return nullptr;
        }

        shader->bindSampler(sampler.index, param->textureSlot);
    }

    return shader;
}

/** Add a GPU shader to the pass.
 * @param stage         Stage to add this shader to.
 * @param path          Filesystem path to shader source.
 * @param keywords      Set of shader variation keywords.
 * @return              Whether the stage was loaded successfully. */
bool Pass::loadStage(GPUShader::Type stage, const Path &path, const KeywordSet &keywords) {
    std::unique_ptr<File> file(g_filesystem->openFile(path));
    if (!file) {
        logError("Cannot find shader source file '%s'", path.c_str());
        return false;
    }

    std::string source;

    /* Add pass type definition and user-specified keywords. */
    defineKeyword(source, passShaderVariations[m_type]);
    for (const std::string &keyword : keywords)
        defineKeyword(source, keyword.c_str());

    source += "\n";

    /* Insert declarations for standard uniform blocks into the source. */
    for (const UniformStruct *uniformStruct : UniformStruct::structList())
        declareUniformBlock(source, uniformStruct);

    /* If there is a shader-specific uniform structure, add it. */
    if (m_parent->uniformStruct())
        declareUniformBlock(source, m_parent->uniformStruct());

    /* Read in the main shader source. */
    if (!loadSource(path, source))
        return false;

    if (m_type == kForwardPass) {
        /* Build each of the light type variations. */
        for (unsigned i = 0; i < SceneLight::kNumTypes; i++) {
            for (unsigned j = 0; j < 2; j++) {
                /* Build a source string with this variation defined. */
                std::string variationSource;
                defineKeyword(variationSource, lightShaderVariations[i]);
                if (j)
                    defineKeyword(variationSource, shadowVariation);
                variationSource += source;

                m_variations[(i * 2) + j].shaders[stage] = compileVariation(
                    variationSource,
                    stage,
                    m_parent,
                    path);
                if (!m_variations[(i * 2) + j].shaders[stage])
                    return false;
            }
        }
    } else {
        /* Single variation. */
        m_variations[0].shaders[stage] = compileVariation(source, stage, m_parent, path);
        if (!m_variations[0].shaders[stage])
            return false;
    }

    return true;
}

/** Finalize the pass (called from Shader::addPass). */
void Pass::finalize() {
    for (size_t i = 0; i < m_variations.size(); i++)
        m_variations[i].pipeline = g_gpuManager->createPipeline(m_variations[i].shaders);
}

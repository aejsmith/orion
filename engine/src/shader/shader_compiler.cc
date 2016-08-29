/*
 * Copyright (C) 2016 Alex Smith
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * @file
 * @brief               Shader compiler.
 *
 * This code implements loading and compilation of GLSL shaders. On Vulkan, the
 * driver does not accept GLSL code directly, rather it takes SPIR-V bytecode.
 * Because of this, we must provide our own GLSL compiler. For this purpose we
 * use glslang, the Khronos reference compiler. Despite OpenGL requiring GLSL
 * to be passed directly to the driver, we also use glslang to compile to SPIR-V
 * for that as well. This means that we have a unified target to write our
 * shaders for. Our shaders are all written using Vulkan semantics, which we
 * compile to SPIR-V with glslang. For OpenGL, we use SPIRV-Cross to modify the
 * shader such that it is usable with OpenGL and convert it back to GLSL
 * targetting the right version, then pass this to the driver.
 *
 * Currently the whole process from the original GLSL source is done at runtime.
 * In future it is intended that a "compiled" game would include only the SPIR-V
 * binaries in its shader assets, and the shader compiler would not exist in
 * the game build (however SPIRV-Cross would).
 */

#include <glslang/Public/ShaderLang.h>

#include <SPIRV/GlslangToSpv.h>

#include <sstream>

#include "core/filesystem.h"
#include "core/path.h"
#include "core/string.h"

#include "gpu/vertex_data.h"

#include "shader/resource.h"
#include "shader/shader_compiler.h"

/** Target GLSL version. */
static const unsigned kTargetGLSLVersion = 450;

namespace glslang {
    extern const TBuiltInResource DefaultTBuiltInResource;
}

/** Generate enum definitions.
 * @tparam T            Type to generate definitions for.
 * @param source        Source string to modify. */
template <typename T>
static void generateEnumDefinitions(std::string &source) {
    const MetaType::EnumConstantArray &constants = MetaType::lookup<T>().enumConstants();
    for (const MetaType::EnumConstant &constant : constants) {
        source += String::format(
            "#define %s %llu\n",
            constant.first, constant.second);
    }

    source += "\n";
}

/** Generate a uniform block declaration.
 * @param source        Source string to modify.
 * @param uniforms      Uniform structure to generate. */
static void generateUniformBlock(std::string &source, const UniformStruct *uniforms) {
    source += String::format(
        "layout(std140, set = %u, binding = %u) uniform %s {\n",
        uniforms->set, ResourceSlots::kUniforms, uniforms->name);

    for (const UniformStructMember &member : uniforms->members()) {
        source += String::format("    %s %s;\n",
            ShaderParameter::glslType(member.type),
            member.name);
    }

    if (uniforms->instanceName && strlen(uniforms->instanceName)) {
        source += String::format("} %s;\n\n", uniforms->instanceName);
    } else {
        source += "};\n\n";
    }
}

/** Generate the source to pass to the compiler.
 * @param options       Shader compiler options.
 * @return              Generated source string. */
static std::string generateSource(const ShaderCompiler::Options &options) {
    /* Add a version string. */
    std::string source = String::format("#version %u\n\n", kTargetGLSLVersion);

    source += "#extension GL_GOOGLE_include_directive : enable\n\n";

    /* For vertex shaders, insert attribute semantic definitions. */
    if (options.stage == ShaderStage::kVertex) {
        source += String::format(
            "#define kPositionSemantic %u\n",
            VertexAttribute::glslIndex(VertexAttribute::kPositionSemantic, 0));
        source += String::format(
            "#define kNormalSemantic %u\n",
            VertexAttribute::glslIndex(VertexAttribute::kNormalSemantic, 0));
        source += String::format(
            "#define kTexcoordSemantic %u\n",
            VertexAttribute::glslIndex(VertexAttribute::kTexcoordSemantic, 0));
        source += String::format(
            "#define kDiffuseSemantic %u\n",
            VertexAttribute::glslIndex(VertexAttribute::kDiffuseSemantic, 0));
        source += String::format(
            "#define kSpecularSemantic %u\n\n",
            VertexAttribute::glslIndex(VertexAttribute::kSpecularSemantic, 0));
    }

    /* Add resource set/slot definitions. */
    generateEnumDefinitions<ResourceSets::Value>(source);
    generateEnumDefinitions<ResourceSlots::Value>(source);

    /* Define keywords. */
    for (const std::string &keyword : options.keywords)
        source += String::format("#define %s 1\n", keyword.c_str());
    if (!options.keywords.empty())
        source += "\n";

    /* Insert declarations for standard uniform blocks into the source. */
    for (const UniformStruct *uniformStruct : UniformStruct::structList())
        generateUniformBlock(source, uniformStruct);

    /* If there is a shader-specific uniform structure, add it. */
    if (options.uniforms)
        generateUniformBlock(source, options.uniforms);

    /* Define other parameters (e.g. textures). */
    for (const ShaderCompiler::ParameterDefinition &parameter : options.parameters) {
        source += String::format(
            "layout(set = %u, binding = %u) uniform %s %s;\n",
            ResourceSets::kMaterialResources, parameter.second.resourceSlot,
            parameter.second.glslType(), parameter.first.c_str());
    }
    if (!options.parameters.empty())
        source += "\n";

    /* Include the source file. This will be pulled in by the include logic. */
    source += String::format("#include \"%s\"\n", options.path.c_str());
    return source;
}

/** Filters on messages generated by glslang. */
static const char *kParseLogFilters[] = {
    "names containing consecutive underscores",
};

/** Display the messages generated by glslang.
 * @param path          Path to the shader.
 * @param log           Information log. */
static void logGLSLMessages(const Path &path, const char *log) {
    std::istringstream stream(log);
    std::string line;
    while (std::getline(stream, line)) {
        for (const char *filter : kParseLogFilters) {
            if (line.find(filter) != std::string::npos) {
                line.clear();
                break;
            }
        }

        if (line.empty())
            continue;

        const std::string errorPrefix = "ERROR: ";
        const std::string warningPrefix = "WARNING: ";

        if (!line.compare(0, errorPrefix.length(), errorPrefix)) {
            std::string message = line.substr(errorPrefix.length());
            logError("%s", message.c_str());
        } else if (!line.compare(0, warningPrefix.length(), warningPrefix)) {
            std::string message = line.substr(warningPrefix.length());
            logWarning("%s", message.c_str());
        }
    }
}

/** Display the messages generated by the SPIR-V generator.
 * @param path          Path to the shader.
 * @param logger        SPIR-V generator logger.
 * @return              Whether the SPIR-V was generated successfully. */
static bool logSPIRVMessages(const Path &path, const spv::SpvBuildLogger &logger) {
    std::istringstream stream(logger.getAllMessages());
    std::string line;
    while (std::getline(stream, line)) {
        if (line.empty())
            continue;

        const std::string warningPrefix = "warning: ";
        bool warning = !line.compare(0, warningPrefix.length(), warningPrefix);

        logWrite(
            (warning) ? LogLevel::kWarning : LogLevel::kError,
            "%s: SPIR-V %s",
            path.c_str(), line.c_str());

        if (!warning)
            return false;
    }

    return true;
}

/** Include handler for the GLSL preprocessor. */
class SourceIncluder : public glslang::TShader::Includer {
public:
    /** Built-in (top level) source file name. */
    static constexpr auto kBuiltInFileName = "<built-in>";

    /** Maximum include depth. */
    static const size_t kMaximumIncludeDepth = 16;

    /** Include result which stores its data in a std::string. */
    struct StringResult : IncludeResult {
        std::string data;

        StringResult(const std::string &inName, std::string &&inData) :
            IncludeResult(inName, nullptr, inData.length(), nullptr),
            data(std::move(inData))
        {
            /* Bleh. Our data has to be initialised after we call base ctor. */
            const_cast<const char *&>(this->file_data) = this->data.c_str();
        }
    };

    /** Include a source file.
     * @param path          Path to the source file to include.
     * @param type          Type of the inclusion.
     * @param from          Path of the including file.
     * @param depth         Inclusion stack depth.
     * @return              Inclusion result. */
    IncludeResult *include(const char *path, IncludeType type, const char *from, size_t depth) override {
        if (type != EIncludeRelative) {
            return makeErrorResult("unsupported include type");
        } else if (depth >= kMaximumIncludeDepth) {
            return makeErrorResult("exceeded maximum include depth");
        }

        Path directoryName;
        if (std::strcmp(from, kBuiltInFileName))
            directoryName = Path(from).directoryName();
        Path fileName = directoryName / path;

        std::unique_ptr<File> file(g_filesystem->openFile(fileName));
        if (!file)
            return makeErrorResult(String::format("failed to open '%s'", path));

        std::string data;
        data.resize(file->size());
        if (!file->read(&data[0], file->size()))
            return makeErrorResult(String::format("failed to read '%s'", path));

        return new StringResult(fileName.str(), std::move(data));
    }

    /** Release an include result.
     * @param result        Result to release. */
    void releaseInclude(IncludeResult *result) override {
        delete static_cast<StringResult *>(result);
    }
private:
    /** Make an error result.
     * @param message       Error message. */
    static IncludeResult *makeErrorResult(std::string &&message) {
        return new StringResult(std::string(""), std::move(message));
    }
};

/** Compile a GLSL shader to SPIR-V.
 * @param options       Shader compilation options.
 * @param spirv         Where to store generated SPIR-V bytecode.
 * @return              Whether the shader was successfully compiled. */
bool ShaderCompiler::compile(const ShaderCompiler::Options &options, std::vector<uint32_t> &spirv) {
    /* Generate the source string to pass to the compiler, containing built in
     * definitions. This #includes the real source file, so the logic for
     * loading that is in the includer. */
    std::string source = generateSource(options);

    // FIXME: This needs locking. glslang has global mutable state.
    glslang::InitializeProcess();
    auto guard = makeScopeGuard([] { glslang::FinalizeProcess(); });

    /* Convert stage to a glslang type. */
    EShLanguage glslangStage;
    switch (options.stage) {
        case ShaderStage::kVertex:
            glslangStage = EShLangVertex;
            break;
        case ShaderStage::kFragment:
            glslangStage = EShLangFragment;
            break;
        default:
            checkMsg(false, "Unhandled shader stage type");
            unreachable();
    }

    const EShMessages messages = static_cast<EShMessages>(EShMsgVulkanRules | EShMsgSpvRules);

    /* Parse the shader. */
    glslang::TShader shader(glslangStage);
    const char *sourceString = source.c_str();
    int sourceLength = source.length();
    const char *sourceName = SourceIncluder::kBuiltInFileName;
    shader.setStringsWithLengthsAndNames(&sourceString, &sourceLength, &sourceName, 1);
    SourceIncluder includer;
    bool parsed = shader.parse(
        &glslang::DefaultTBuiltInResource,
        kTargetGLSLVersion, ENoProfile, false, false,
        messages,
        includer);
    logGLSLMessages(options.path.c_str(), shader.getInfoLog());
    if (!parsed)
        return false;

    /* Link the shader. */
    glslang::TProgram program;
    program.addShader(&shader);
    bool linked = program.link(messages);
    logGLSLMessages(options.path.c_str(), program.getInfoLog());
    if (!linked)
        return false;

    /* Generate SPIR-V. */
    glslang::TIntermediate *intermediate = program.getIntermediate(glslangStage);
    check(intermediate);
    spv::SpvBuildLogger logger;
    glslang::GlslangToSpv(*intermediate, spirv, &logger);
    if (!logSPIRVMessages(options.path.c_str(), logger))
        return false;

    return true;
}

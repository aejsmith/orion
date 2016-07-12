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
    source += String::format("layout(std140) uniform %s {\n", uniforms->name);

    for (const UniformStructMember &member : uniforms->members) {
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

/** Load and pre-process a shader source file.
 * @param options       Shader compiler options.
 * @param path          Path to file to open.
 * @param source        Source string to modify.
 * @param depth         Recursion count.
 * @return              Whether the source was successfully loaded. */
static bool loadSource(
    const ShaderCompiler::Options &options,
    const Path &path,
    std::string &source,
    unsigned depth = 0)
{
    std::unique_ptr<File> file(g_filesystem->openFile(path));
    if (!file) {
        logError("Cannot find shader source file '%s'", path.c_str());
        return false;
    }

    std::string line;
    while (file->readLine(line)) {
        if (line.substr(0, 9) == "#include ") {
            if (depth == 16) {
                logError(
                    "Too many nested includes in '%s' (included from '%s')",
                    path.c_str(), options.path.c_str());
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
            if (!loadSource(options, includePath, source, depth + 1))
                return false;
        } else {
            source += line;
            source += "\n";
        }
    }

    return true;
}

/** Generate the source to pass to the compiler.
 * @param options       Shader compiler options.
 * @param source        Source string to modify.
 * @return              Whether the source was successfully loaded. */
static bool generateSource(const ShaderCompiler::Options &options, std::string &source) {
    /* Add a version string. */
    source += String::format("#version %u\n\n", kTargetGLSLVersion);

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

    return loadSource(options, options.path, source);
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
            logError("%s: %s", path.c_str(), message.c_str());
        } else if (!line.compare(0, warningPrefix.length(), warningPrefix)) {
            std::string message = line.substr(warningPrefix.length());
            logWarning("%s: %s", path.c_str(), message.c_str());
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

/** Compile a GLSL shader to SPIR-V.
 * @param options       Shader compilation options.
 * @param spirv         Where to store generated SPIR-V bytecode.
 * @return              Whether the shader was successfully compiled. */
bool ShaderCompiler::compile(const ShaderCompiler::Options &options, std::vector<uint32_t> &spirv) {
    /* Generate the source string. */
    std::string source;
    if (!generateSource(options, source))
        return false;

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
    }

    const EShMessages messages = static_cast<EShMessages>(EShMsgVulkanRules | EShMsgSpvRules);

    /* Parse the shader. */
    glslang::TShader shader(glslangStage);
    const char *sourceString = source.c_str();
    shader.setStrings(&sourceString, 1);
    bool parsed = shader.parse(&glslang::DefaultTBuiltInResource, kTargetGLSLVersion, false, messages);
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

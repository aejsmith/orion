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
 */

#pragma once

#include "shader/defs.h"
#include "shader/uniform_buffer.h"

namespace ShaderCompiler {
    /** Structure containing shader compilation options. */
    struct Options {
        Path path;                      /**< Path to shader to compile. */
        unsigned stage;                 /**< Stage that the shader is being compiled for. */
        ShaderKeywordSet keywords;      /**< Compilation keywords. */
        const UniformStruct *uniforms;  /**< Optional uniform structure to define in the file. */
    };

    extern bool compile(const Options &options, std::vector<uint32_t> &spirv);
}

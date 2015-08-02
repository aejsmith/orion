/*
 * Copyright (C) 2015 Alex Smith
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
 * @brief               Shader manager class.
 */

#pragma once

#include "core/hash_table.h"

#include "shader/shader.h"

/** Manages global shader parameters. */
class ShaderManager : Noncopyable {
public:
    ShaderManager();

    void setGlobalTexture(const std::string &name, unsigned slot);
    void setGlobalUniformBlock(const std::string &name, unsigned slot);

    bool lookupGlobalTexture(const std::string &name, unsigned &slot);
    bool lookupGlobalUniformBlock(const std::string &name, unsigned &slot);
private:
    /** Type of the global texture/uniform block maps. */
    typedef HashMap<std::string, unsigned> ResourceMap;
private:
    ResourceMap m_globalTextures;       /**< Globally set texture bindings. */
    ResourceMap m_globalUniformBlocks;  /**< Globally set uniform block bindings. */
};

extern EngineGlobal<ShaderManager> g_shaderManager;

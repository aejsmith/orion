/**
 * @file
 * @copyright           2015 Alex Smith
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

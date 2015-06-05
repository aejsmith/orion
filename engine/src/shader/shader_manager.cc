/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Shader manager.
 */

#include "shader/shader_manager.h"
#include "shader/slots.h"
#include "shader/uniform_buffer.h"

/** Global shader manager. */
EngineGlobal<ShaderManager> g_shaderManager;

/** Initialise the shader manager. */
ShaderManager::ShaderManager() {
    /* Bind built-in uniform block types. */
    for (UniformStruct *uniformStruct : UniformStruct::structList())
        setGlobalUniformBlock(uniformStruct->name, uniformStruct->slot);

    setGlobalUniformBlock("MaterialUniforms", UniformSlots::kMaterialUniforms);

    /* Bind built-in texture names. */
    setGlobalTexture("deferredBufferA", TextureSlots::kDeferredBufferA);
    setGlobalTexture("deferredBufferB", TextureSlots::kDeferredBufferB);
    setGlobalTexture("deferredBufferC", TextureSlots::kDeferredBufferC);
    setGlobalTexture("deferredBufferD", TextureSlots::kDeferredBufferD);
    setGlobalTexture("shadowMap", TextureSlots::kShadowMap);
}

/**
 * Set a global texture name.
 *
 * Sets a global texture name to be bound to the specified slot. Any shader
 * loaded that refers to the given name will have it bound to the specified
 * slot. This should be used only before any shaders are loaded - changes are
 * not propagated to already loaded shaders.
 *
 * @param name          Texture name.
 * @param slot          Slot to bind to.
 */
void ShaderManager::setGlobalTexture(const std::string &name, unsigned slot) {
    auto ret = m_globalTextures.insert(std::make_pair(name, slot));
    checkMsg(ret.second, "Setting duplicate global texture '%s'", name.c_str());
}

/**
 * Set a global uniform block.
 *
 * Sets a global uniform block name to be bound to the specified slot. Any
 * shader loaded that refers to the given name will have it bound to the
 * specified slot. This should be used only before any shaders are loaded -
 * changes are not propagated to already loaded shaders.
 *
 * @param name          Uniform block name.
 * @param slot          Slot to bind to.
 */
void ShaderManager::setGlobalUniformBlock(const std::string &name, unsigned slot) {
    auto ret = m_globalUniformBlocks.insert(std::make_pair(name, slot));
    checkMsg(ret.second, "Setting duplicate global uniform block '%s'", name.c_str());
}

/** Look up a global texture binding.
 * @param name          Name of texture to look up.
 * @param slot          Where to store slot name is bound to.
 * @return              Whether the name was found. */
bool ShaderManager::lookupGlobalTexture(const std::string &name, unsigned &slot) {
    auto ret = m_globalTextures.find(name);
    if (ret != m_globalTextures.end()) {
        slot = ret->second;
        return true;
    } else {
        return false;
    }
}

/** Look up a global uniform block binding.
 * @param name          Name of uniform block to look up.
 * @param slot          Where to store slot name is bound to.
 * @return              Whether the name was found. */
bool ShaderManager::lookupGlobalUniformBlock(const std::string &name, unsigned &slot) {
    auto ret = m_globalUniformBlocks.find(name);
    if (ret != m_globalUniformBlocks.end()) {
        slot = ret->second;
        return true;
    } else {
        return false;
    }
}

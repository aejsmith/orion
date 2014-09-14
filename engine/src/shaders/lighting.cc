/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Generic lighting shader.
 */

#include "render/shader.h"

/** Generic lighting shader. */
class LightingShader : public Shader {
public:
	UNIFORM_STRUCT_BEGIN(Uniforms)
		UNIFORM_STRUCT_MEMBER(glm::vec3, specular);
		UNIFORM_STRUCT_MEMBER(float, shininess);
	UNIFORM_STRUCT_END;
public:
	LightingShader() : Shader("lighting", Uniforms::kUniformStruct) {
		addTextureParameter("diffuse", TextureSlots::kDiffuseTexture);
		addExtraParameter("meow", ShaderParameter::kIntType);
	}
private:
	static LightingShader m_instance;
};

IMPLEMENT_UNIFORM_STRUCT(LightingShader::Uniforms);
LightingShader LightingShader::m_instance;

/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Scene light class.
 */

#pragma once

#include "render/uniform_buffer.h"

/** Per-light uniform buffer structure. */
UNIFORM_STRUCT_BEGIN(LightUniforms)
	UNIFORM_STRUCT_MEMBER(glm::vec3, position);
	UNIFORM_STRUCT_MEMBER(float, intensity);
	UNIFORM_STRUCT_MEMBER(glm::vec3, direction);
	UNIFORM_STRUCT_MEMBER(float, cosCutoff);
	UNIFORM_STRUCT_MEMBER(glm::vec3, colour);
	UNIFORM_STRUCT_MEMBER(float, range);
	UNIFORM_STRUCT_MEMBER(float, attenuationConstant);
	UNIFORM_STRUCT_MEMBER(float, attenuationLinear);
	UNIFORM_STRUCT_MEMBER(float, attenuationExp);
UNIFORM_STRUCT_END;

/** Renderer representation of a light source. */
class SceneLight {
public:
	/** Type of a light. */
	enum Type {
		kAmbientLight,		/**< Ambient light. */
		kDirectionalLight,	/**< Directional light. */
		kPointLight,		/**< Point light. */
		kSpotLight,		/**< Spot light. */
		kNumTypes,
	};
public:
	explicit SceneLight(Type type);
	~SceneLight();

	void setDirection(const glm::vec3 &direction);
	void setColour(const glm::vec3 &colour);
	void setIntensity(float intensity);
	void setCutoff(float cutoff);
	void setRange(float range);
	void setAttenuation(float constant, float linear, float exp);

	/** @return		Type of the light. */
	Type type() const { return m_type; }
	/** @return		Position of the light. */
	const glm::vec3 &position() const { return m_position; }
	/** @return		Direction of the light (directional/spot). */
	const glm::vec3 &direction() const { return m_direction; }
	/** @return		Colour that the light emits. */
	const glm::vec3 &colour() const { return m_colour; }
	/** @return		Diffuse intensity. */
	float intensity() const { return m_intensity; }
	/** @return		Angle of effect (spot). */
	float cutoff() const { return m_cutoff; }
	/** @return		Range of the light (point/spot). */
	float range() const { return m_range; }
	/** @return		Constant attenuation factor (point/spot). */
	float attenuationConstant() const { return m_attenuationConstant; }
	/** @return		Linear attenuation factor (point/spot). */
	float attenuationLinear() const { return m_attenuationLinear; }
	/** @return		Exponential attenuation factor (point/spot). */
	float attenuationExp() const { return m_attenuationExp; }

	/** @return		GPU buffer containing light uniforms. */
	GPUBuffer *uniforms() const { return m_uniforms.gpu(); }
private:
	void setPosition(const glm::vec3 &position);
private:
	Type m_type;			/**< Type of the light. */

	glm::vec3 m_position;		/**< Position of the light. */
	glm::vec3 m_direction;		/**< Direction of the light (directional/spot). */
	glm::vec3 m_colour;		/**< Colour that the light emits. */
	float m_intensity;		/**< Diffuse intensity. */
	float m_cutoff;			/**< Angle of effect (spot). */
	float m_range;			/**< Range of the light (point/spot). */
	float m_attenuationConstant;	/**< Constant attenuation factor (point/spot). */
	float m_attenuationLinear;	/**< Linear attenuation factor (point/spot). */
	float m_attenuationExp;		/**< Exponential attenuation factor (point/spot). */

	/** Uniform buffer containing lighting parameters. */
	UniformBuffer<LightUniforms> m_uniforms;

	friend class Scene;
};

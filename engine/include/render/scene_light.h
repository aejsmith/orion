/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Scene light class.
 */

#ifndef ORION_RENDER_SCENE_LIGHT_H
#define ORION_RENDER_SCENE_LIGHT_H

#include "gpu/uniform_buffer.h"

/** Per-light uniform buffer structure. */
struct LightUniforms {
	float position[3];
	int type;
	float direction[3];
	float intensity;
	float colour[3];
	float cos_cutoff;
	float range;
	float attenuation_constant;
	float attenuation_linear;
	float attenuation_exp;
};

/** Renderer representation of a light source. */
class SceneLight {
public:
	/** Type of a light. */
	enum Type {
		kAmbientLight,		/**< Ambient light. */
		kDirectionalLight,	/**< Directional light. */
		kPointLight,		/**< Point light. */
		kSpotLight,		/**< Spot light. */
	};
public:
	explicit SceneLight(Type type);
	~SceneLight();

	void set_direction(const glm::vec3 &direction);
	void set_colour(const glm::vec3 &colour);
	void set_intensity(float intensity);
	void set_cutoff(float cutoff);
	void set_range(float range);
	void set_attenuation(float constant, float linear, float exp);

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
	float attenuation_constant() const { return m_attenuation_constant; }
	/** @return		Linear attenuation factor (point/spot). */
	float attenuation_linear() const { return m_attenuation_linear; }
	/** @return		Exponential attenuation factor (point/spot). */
	float attenuation_exp() const { return m_attenuation_exp; }

	GPUBufferPtr uniforms();
private:
	void set_position(const glm::vec3 &position);
private:
	Type m_type;			/**< Type of the light. */

	glm::vec3 m_position;		/**< Position of the light. */
	glm::vec3 m_direction;		/**< Direction of the light (directional/spot). */
	glm::vec3 m_colour;		/**< Colour that the light emits. */
	float m_intensity;		/**< Diffuse intensity. */
	float m_cutoff;			/**< Angle of effect (spot). */
	float m_range;			/**< Range of the light (point/spot). */
	float m_attenuation_constant;	/**< Constant attenuation factor (point/spot). */
	float m_attenuation_linear;	/**< Linear attenuation factor (point/spot). */
	float m_attenuation_exp;	/**< Exponential attenuation factor (point/spot). */

	/** Uniform buffer containing lighting parameters. */
	DynamicUniformBuffer<LightUniforms> m_uniforms;

	friend class Scene;
};

#endif /* ORION_RENDER_SCENE_LIGHT_H */

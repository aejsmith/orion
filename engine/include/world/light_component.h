/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Light component classes.
 */

#ifndef ORION_WORLD_LIGHT_COMPONENT_H
#define ORION_WORLD_LIGHT_COMPONENT_H

#include "render/scene_light.h"

#include "world/component.h"

/**
 * Base light component class.
 *
 * This component implements a light source in the world. This class cannot be
 * created directly, you must create one of the specific light type classes.
 */
class LightComponent : public Component {
public:
	ORION_COMPONENT(Component::kLightType);
public:
	~LightComponent();

	void set_colour(const glm::vec3 &colour);
	void set_intensity(float intensity);

	/** @return		Colour that the light emits. */
	const glm::vec3 &colour() const { return m_scene_light.colour(); }
	/** @return		Diffuse intensity. */
	float intensity() const { return m_scene_light.intensity(); }
protected:
	LightComponent(Entity *entity, SceneLight::Type type);

	void transformed();
	void activated();
	void deactivated();

	/**
	 * Methods made public by derived classes that require them.
	 */

	void set_direction(const glm::vec3 &direction);
	void set_cutoff(float cutoff);
	void set_range(float range);
	void set_attenuation(float constant, float linear, float exp);

	glm::vec3 direction() const;

	/** @return		Angle of effect. */
	float cutoff() const { return m_scene_light.cutoff(); }
	/** @return		Range of the light. */
	float range() const { return m_scene_light.range(); }
	/** @return		Constant attenuation factor. */
	float attenuation_constant() const { return m_scene_light.attenuation_constant(); }
	/** @return		Linear attenuation factor. */
	float attenuation_linear() const { return m_scene_light.attenuation_linear(); }
	/** @return		Exponential attenuation factor. */
	float attenuation_exp() const { return m_scene_light.attenuation_exp(); }
protected:
	/** Scene light implementing this light. */
	SceneLight m_scene_light;
};

/**
 * Ambient light component.
 *
 * This component adds ambient lighting to the world. The ambient light is a
 * single colour value/intensity that is added on to the overall shading, to
 * simulate the effect of light scattered about the entire scene. The position
 * is ignored, the light affects the whole scene.
 */
class AmbientLightComponent : public LightComponent {
public:
	explicit AmbientLightComponent(Entity *entity);
};

/**
 * Directional light component.
 *
 * This component adds a directional light source to the world. A directional
 * light is one that affects the whole world equally from a certain direction,
 * with no distance cutoff. The position of the light is irrelevant.
 *
 * The light direction is stored using the entity orientation: the default
 * direction is (0, 0, -1), and the orientation is applied to that to give the
 * light direction. Because of this, the actual light direction in the world is
 * affected by the parent entity's rotation.
 */
class DirectionalLightComponent : public LightComponent {
public:
	explicit DirectionalLightComponent(Entity *entity);

	using LightComponent::set_direction;
	using LightComponent::direction;
};

/**
 * Point light component.
 *
 * This component adds a point light source to the world. A point light is a
 * light that radiates out from a point in the world. It has a limited range,
 * and attenuation across that range.
 */
class PointLightComponent : public LightComponent {
public:
	explicit PointLightComponent(Entity *entity);

	using LightComponent::set_range;
	using LightComponent::set_attenuation;
	using LightComponent::range;
	using LightComponent::attenuation_constant;
	using LightComponent::attenuation_linear;
	using LightComponent::attenuation_exp;
};

/**
 * Spot light component.
 *
 * This component adds a spot light source to the world. A spot light radiates
 * out in a cone in a certain direction from a point in the world. It has a
 * limited range, and attenuation across that range.
 *
 * See DirectionalLightComponent for details on how the light direction is
 * stored.
 */
class SpotLightComponent : public LightComponent {
public:
	explicit SpotLightComponent(Entity *entity);

	using LightComponent::set_direction;
	using LightComponent::set_cutoff;
	using LightComponent::set_range;
	using LightComponent::set_attenuation;
	using LightComponent::direction;
	using LightComponent::cutoff;
	using LightComponent::range;
	using LightComponent::attenuation_constant;
	using LightComponent::attenuation_linear;
	using LightComponent::attenuation_exp;
};

/** Set the colour of the light.
 * @param colour	New light colour. */
inline void LightComponent::set_colour(const glm::vec3 &colour) {
	m_scene_light.set_colour(colour);
}

/** Set the intensity of the light.
 * @param intensity	New light intensity. */
inline void LightComponent::set_intensity(float intensity) {
	m_scene_light.set_intensity(intensity);
}

/** Set the cutoff angle.
 * @param cutoff	New cutoff angle. Must be <= 45 degrees. */
inline void LightComponent::set_cutoff(float cutoff) {
	m_scene_light.set_cutoff(cutoff);
}

/** Set the range of the light.
 * @param range		Range of the light. */
inline void LightComponent::set_range(float range) {
	m_scene_light.set_range(range);
}

/** Set the attenuation factors.
 * @param constant	Constant attenuation factor.
 * @param linear	Linear attenuation factor.
 * @param exp		Exponentional attenuation factor. */
inline void LightComponent::set_attenuation(float constant, float linear, float exp) {
	m_scene_light.set_attenuation(constant, linear, exp);
}

#endif /* ORION_WORLD_LIGHT_COMPONENT_H */

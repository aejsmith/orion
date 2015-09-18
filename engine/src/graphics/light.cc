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
 * @brief               Light component classes.
 */

#include "engine/entity.h"
#include "engine/world.h"

#include "graphics/light.h"

#include "render/scene.h"
#include "render/scene_light.h"

/** Default direction of a light. */
static const glm::vec3 kDefaultDirection = glm::vec3(0.0f, 0.0f, -1.0f);

/** Initialize a light component.
 * @param entity        Entity that the component belongs to.
 * @param type          SceneLight type. */
Light::Light(Entity *entity, SceneLight::Type type) :
    Component(Component::kLightType, entity),
    m_sceneLight(type)
{
    /* Set default colour/intensity. */
    setColour(glm::vec3(1.0f, 1.0f, 1.0f));
    setIntensity(0.8f);

    /* Don't cast shadows by default. */
    setCastShadows(false);
}

/** Initialize an ambient light component.
 * @param entity        Entity that the component belongs to. */
AmbientLight::AmbientLight(Entity *entity) :
    Light(entity, SceneLight::kAmbientLight)
{}

/** Initialize a directional light component.
 * @param entity        Entity that the component belongs to. */
DirectionalLight::DirectionalLight(Entity *entity) :
    Light(entity, SceneLight::kDirectionalLight)
{}

/** Initialize a point light component.
 * @param entity        Entity that the component belongs to. */
PointLight::PointLight(Entity *entity) :
    Light(entity, SceneLight::kPointLight)
{
    /* Set default parameters. */
    setRange(100.0f);
    setAttenuation(1.0f, 0.045f, 0.0075f);
}

/** Initialize a spot light component.
 * @param entity        Entity that the component belongs to. */
SpotLight::SpotLight(Entity *entity) :
    Light(entity, SceneLight::kSpotLight)
{
    /* Set default parameters. */
    setRange(50.0f);
    setAttenuation(1.0f, 0.09f, 0.032f);
    setCutoff(20.0f);
}

/** Destroy the light. */
Light::~Light() {}

/**
 * Set the light direction.
 *
 * Sets the light direction. As the light direction is stored using the entity
 * orientation, this will change that. This does not set the absolute world
 * direction, it sets the direction relative to the parent entity.
 *
 * @param direction     New light direction.
 */
void Light::setDirection(const glm::vec3 &direction) {
    /* Set orientation to rotate the default direction to the given one. */
    glm::vec3 d = glm::normalize(direction);
    entity()->setOrientation(Math::quatRotateBetween(kDefaultDirection, d));
}

/** @return             Current light direction. */
glm::vec3 Light::direction() const {
    /* Here we return the direction relative to the parent. TODO: Absolute
     * direction function. */
    return orientation() * kDefaultDirection;
}

/** Called when the entity's transformation is changed.
 * @param changed       Flags indicating changes made. */
void Light::transformed(unsigned changed) {
    /* Update SceneLight's direction vector. Here we want to set the absolute
     * direction. */
    glm::vec3 direction = worldOrientation() * kDefaultDirection;
    m_sceneLight.setDirection(direction);

    /* Scene manager needs to know the light has moved. */
    if (activeInWorld())
        world()->scene()->transformLight(&m_sceneLight, worldPosition());
}

/** Called when the component becomes active in the world. */
void Light::activated() {
    world()->scene()->addLight(&m_sceneLight, worldPosition());
}

/** Called when the component becomes inactive in the world. */
void Light::deactivated() {
    world()->scene()->removeLight(&m_sceneLight);
}

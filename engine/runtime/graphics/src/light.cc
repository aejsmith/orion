/*
 * Copyright (C) 2015-2017 Alex Smith
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

#include "graphics/graphics_system.h"
#include "graphics/light.h"

#include "render/render_world.h"

/** Default direction of a light. */
static const glm::vec3 kDefaultDirection = glm::vec3(0.0f, 0.0f, -1.0f);

/** Initialize a light component.
 * @param type          Light type. */
Light::Light(RenderLight::Type type) :
    m_renderLight(type)
{
    /* Set default colour/intensity. */
    setColour(glm::vec3(1.0f, 1.0f, 1.0f));
    setIntensity(0.8f);

    /* Don't cast shadows by default. */
    setCastsShadows(false);
    setShadowBiasConstant(-0.001);
}

/** Initialize an ambient light component. */
AmbientLight::AmbientLight() :
    Light(RenderLight::kAmbientLight)
{}

/** Initialize a directional light component. */
DirectionalLight::DirectionalLight() :
    Light(RenderLight::kDirectionalLight)
{}

/** Initialize a point light component. */
PointLight::PointLight() :
    Light(RenderLight::kPointLight)
{
    /* Set default parameters. */
    setRange(100.0f);
    setAttenuation(glm::vec3(1.0f, 0.045f, 0.0075f));
}

/** Initialize a spot light component. */
SpotLight::SpotLight() :
    Light(RenderLight::kSpotLight)
{
    /* Set default parameters. */
    setRange(50.0f);
    setAttenuation(glm::vec3(1.0f, 0.09f, 0.032f));
    setCutoff(20.0f);
}

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
    // FIXME: Doesn't handle name changes on the entity properly.
    m_renderLight.name = entity()->path();

    /* Update the RenderLight. Here we want to set the absolute values. */
    glm::vec3 direction = worldOrientation() * kDefaultDirection;
    m_renderLight.setDirection(direction);
    m_renderLight.setPosition(worldPosition());
}

/** Called when the component becomes active in the world. */
void Light::activated() {
    auto &system = getSystem<GraphicsSystem>();
    m_renderLight.setWorld(&system.renderWorld());
}

/** Called when the component becomes inactive in the world. */
void Light::deactivated() {
    m_renderLight.setWorld(nullptr);
}

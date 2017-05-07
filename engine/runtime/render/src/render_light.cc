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
 * @brief               Renderer light class.
 *
 * TODO:
 *  - Use a frustum rather than a bounding box for spot light culling.
 */

#include "render/render_light.h"
#include "render/render_world.h"

#include "render_core/render_resources.h"

IMPLEMENT_UNIFORM_STRUCT(LightUniforms, "light", ResourceSets::kLightResources);

/**
 * Construct the light.
 *
 * Constructs the light. Note that most light parameters are not initialized,
 * these must be set manually.
 *
 * @param type          Type of the light.
 */
RenderLight::RenderLight(Type type) :
    m_world(nullptr),
    m_type(type),
    m_flags(0)
{
    m_resources = g_gpuManager->createResourceSet(g_renderResources->lightResourceSetLayout());
    m_resources->bindUniformBuffer(ResourceSlots::kUniforms, m_uniforms.gpu());
}

/** Destroy the light. */
RenderLight::~RenderLight() {
    setWorld(nullptr);
}

/** Set the world for the light.
 * @param world         New world (null to remove). */
void RenderLight::setWorld(RenderWorld *world) {
    if (m_world)
        m_world->removeLight(this);

    m_world = world;

    if (m_world)
        m_world->addLight(this);
}

/** Set the light position.
 * @param position      New light position. */
void RenderLight::setPosition(const glm::vec3 &position) {
    m_position = position;
    m_uniforms.write()->position = m_position;

    updateWorld();
    updateVolumeTransform();
    updateShadowViews();
}

/** Set the direction of the light.
 * @param direction     New light direction. */
void RenderLight::setDirection(const glm::vec3 &direction) {
    m_direction = glm::normalize(direction);
    m_uniforms.write()->direction = m_direction;

    updateVolumeTransform();
    updateShadowViews();
}

/** Set the colour of the light.
 * @param colour        New light colour. */
void RenderLight::setColour(const glm::vec3 &colour) {
    m_colour = colour;
    m_uniforms.write()->colour = m_colour;
}

/** Set the intensity of the light.
 * @param intensity     New light intensity. */
void RenderLight::setIntensity(float intensity) {
    m_intensity = intensity;
    m_uniforms.write()->intensity = m_intensity;
}

/** Set the cutoff angle (for spot lights).
 * @param cutoff        New cutoff angle. Must be <= 45 degrees. */
void RenderLight::setCutoff(float cutoff) {
    checkMsg(cutoff <= 45.0f, "Cutoff angle must be <= 45");

    m_cutoff = cutoff;

    /* Shaders use a precomputed cosine of cutoff angle to avoid having to
     * calculate it for every pixel. */
    m_uniforms.write()->cosCutoff = cosf(glm::radians(m_cutoff));

    updateVolumeTransform();
    updateShadowViews();
}

/** Set the range of the light (for point/spot lights).
 * @param range         Range of the light. */
void RenderLight::setRange(float range) {
    m_range = range;
    m_uniforms.write()->range = m_range;

    updateVolumeTransform();
    updateShadowViews();
}

/** Set the attenuation factors (for point/spot lights).
 * @param constant      Constant attenuation factor.
 * @param linear        Linear attenuation factor.
 * @param exp           Exponentional attenuation factor. */
void RenderLight::setAttenuation(float constant, float linear, float exp) {
    m_attenuationConstant = constant;
    m_attenuationLinear = linear;
    m_attenuationExp = exp;

    LightUniforms *uniforms = m_uniforms.write();
    uniforms->attenuationConstant = m_attenuationConstant;
    uniforms->attenuationLinear = m_attenuationLinear;
    uniforms->attenuationExp = m_attenuationExp;
}

/** Set the flags for the light.
 * @param flags         New flags. */
void RenderLight::setFlags(uint32_t flags) {
    const bool changedShadows = (m_flags & kCastsShadows) != (flags & kCastsShadows);

    m_flags = flags;

    if (changedShadows)
        updateShadowViews();
}

/** Set shadow bias parameters.
 * @param constant      Constant attenuation factor. */
void RenderLight::setShadowBias(float constant) {
    m_shadowBiasConstant = constant;

    LightUniforms *uniforms = m_uniforms.write();
    uniforms->shadowBiasConstant = m_shadowBiasConstant;
}

/** Flush pending updates and get resources.
 * @return              Resource set containing per-entity resources. */
GPUResourceSet *RenderLight::getResources() {
    m_uniforms.flush();
    return m_resources;
}

/** Get light volume geometry.
 * @return              Geometry for the light volume. */
Geometry RenderLight::volumeGeometry() const {
    switch (m_type) {
        case kPointLight:
            return g_renderResources->sphereGeometry();
            break;
        case kSpotLight:
            return g_renderResources->coneGeometry();
            break;
        default:
            return g_renderResources->quadGeometry();
            break;
    }
}

/** Determine if the light is visible to a view.
 * @param view          View to test against.
 * @return              Whether the light should be culled. */
bool RenderLight::cull(RenderView &view) const {
    /* Ignore lights which would have no effect. */
    if (!m_intensity || !glm::length(m_colour))
        return true;

    switch (m_type) {
        case kAmbientLight:
        case kDirectionalLight:
            return false;

        case kPointLight:
        {
            Sphere sphere(m_position, m_range);
            return !Math::intersect(view.frustum(), sphere);
        }

        case kSpotLight:
            return !Math::intersect(view.frustum(), m_boundingBox);

        default:
            return true;
    }
}

/** Update the light volume transformation. */
void RenderLight::updateVolumeTransform() {
    switch (m_type) {
        case kAmbientLight:
        case kDirectionalLight:
            /* Volume is a full-screen quad. The light volume shader does not
             * use the transformation here, make no change to it. */
            break;

        case kPointLight:
            /* Volume is a sphere. Geometry has radius of 1, we must scale this
             * to the light's range. */
            m_volumeTransform.set(
                m_position,
                glm::quat(),
                glm::vec3(m_range, m_range, m_range));
            m_uniforms.write()->volumeTransform = m_volumeTransform.matrix();
            break;

        case kSpotLight:
        {
            /* Volume is a cone. Geometry is pointing in the negative Z
             * direction, with a base radius of 1 and a height of 1. We must
             * scale the radius to the cutoff angle and the height to the
             * light's range. */
            float radius = m_range * tanf(glm::radians(m_cutoff));
            glm::vec3 scale(radius, radius, m_range);

            /* Rotate geometry to be centered on the direction. */
            glm::quat orientation = Math::quatRotateBetween(glm::vec3(0.0f, 0.0f, -1.0f), m_direction);

            m_volumeTransform.set(m_position, orientation, scale);
            m_uniforms.write()->volumeTransform = m_volumeTransform.matrix();

            /* Fit a bounding box around the light's cone. */
            BoundingBox base(
                glm::vec3(-1.0f, -1.0f, -1.0f),
                glm::vec3(1.0f, 1.0f, 0.0f));
            m_boundingBox = base.transform(m_volumeTransform.matrix());
            break;
        }

        default:
            break;
    }
}

/** Update the shadow views. */
void RenderLight::updateShadowViews() {
    if (!castsShadows())
        return;

    unsigned numViews = 0;

    switch (m_type) {
        case kSpotLight:
        {
            numViews = 1;

            /* View is centered on light pointing in its direction. */
            m_shadowViews[0].setTransform(m_position, Math::quatLookAt(m_direction));

            /* Projection is a perspective projection covering the light's range. */
            m_shadowViews[0].perspective(m_cutoff * 2, 0.1f, m_range);

            /* Shader shadow calculation for a spot light requires transformation
             * of the world space position of the pixel being lit into shadow
             * space, i.e. the view-projection transformation. We don't make the
             * shadow view uniforms available to shaders rendering with the
             * shadow map, so we need a copy of it in the light uniforms. We
             * apply a bias to this matrix to map the X and Y coordinates into
             * the [0, 1] range for shadow map sampling, as just the VP
             * transformation yields NDC coordinates, i.e. in the range [-1, 1].
             * Z is already in the [0, 1] range in NDC. */
            const glm::mat4 shadowBiasMatrix(
                0.5, 0.0, 0.0, 0.0,
                0.0, 0.5, 0.0, 0.0,
                0.0, 0.0, 1.0, 0.0,
                0.5, 0.5, 0.0, 1.0);
            m_uniforms.write()->shadowSpace
                = shadowBiasMatrix * m_shadowViews[0].projection() * m_shadowViews[0].view();
            break;
        }

        case kPointLight:
        {
            numViews = CubeFace::kNumFaces;

            const struct {
                glm::vec3 direction;
                glm::vec3 up;
            } cubeFaces[CubeFace::kNumFaces] = {
                { glm::vec3(1.0f, 0.0f, 0.0f),  glm::vec3(0.0f, -1.0f, 0.0f) },
                { glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f) },
                { glm::vec3(0.0f, 1.0f, 0.0f),  glm::vec3(0.0f, 0.0f, 1.0f)  },
                { glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f) },
                { glm::vec3(0.0f, 0.0f, 1.0f),  glm::vec3(0.0f, -1.0f, 0.0f) },
                { glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f) },
            };

            for (unsigned i = 0; i < numViews; i++) {
                /* View is centered on the light looking in the direction of the
                 * face being rendered. */
                m_shadowViews[i].setTransform(
                    m_position,
                    Math::quatLookAt(cubeFaces[i].direction, cubeFaces[i].up));

                /* Perspective projection covering the whole face, limited to
                 * the light's range. */
                m_shadowViews[i].perspective(90.0f, 0.1f, m_range);
            }

            /* Shadow calculation in shader requires the near plane value used
             * in our projection. */
            m_uniforms.write()->shadowZNear = 0.1f;
            break;
        }

        default:
            /* TODO. */
            break;
    }
}

/** Update the light in the world. */
void RenderLight::updateWorld() {
    if (m_world)
        m_world->updateLight(this);
}

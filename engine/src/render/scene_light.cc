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
 * @brief               Scene light class.
 *
 * TODO:
 *  - Use a frustum rather than a bounding box for spot light culling.
 */

#include "render/render_manager.h"
#include "render/scene_light.h"

IMPLEMENT_UNIFORM_STRUCT(LightUniforms, "light", UniformSlots::kLightUniforms);

/**
 * Construct the light.
 *
 * Constructs the light. Note that most light parameters are not initialized,
 * these must be set manually.
 *
 * @param type          Type of the light.
 */
SceneLight::SceneLight(Type type) :
    m_type(type)
{}

/** Destroy the light. */
SceneLight::~SceneLight() {}

/** Set the direction of the light.
 * @param direction     New light direction. */
void SceneLight::setDirection(const glm::vec3 &direction) {
    m_direction = glm::normalize(direction);
    m_uniforms->direction = m_direction;
    updateVolumeTransform();
    updateShadowViews();
}

/** Set the colour of the light.
 * @param colour        New light colour. */
void SceneLight::setColour(const glm::vec3 &colour) {
    m_colour = colour;
    m_uniforms->colour = m_colour;
}

/** Set the intensity of the light.
 * @param intensity     New light intensity. */
void SceneLight::setIntensity(float intensity) {
    m_intensity = intensity;
    m_uniforms->intensity = m_intensity;
}

/** Set the cutoff angle (for spot lights).
 * @param cutoff        New cutoff angle. Must be <= 45 degrees. */
void SceneLight::setCutoff(float cutoff) {
    checkMsg(cutoff <= 45.0f, "Cutoff angle must be <= 45");

    m_cutoff = cutoff;

    /* Shaders use a precomputed cosine of cutoff angle to avoid having to
     * calculate it for every pixel. */
    m_uniforms->cosCutoff = cosf(glm::radians(m_cutoff));

    updateVolumeTransform();
    updateShadowViews();
}

/** Set the range of the light (for point/spot lights).
 * @param range         Range of the light. */
void SceneLight::setRange(float range) {
    m_range = range;
    m_uniforms->range = m_range;

    updateVolumeTransform();
    updateShadowViews();
}

/** Set the attenuation factors (for point/spot lights).
 * @param constant      Constant attenuation factor.
 * @param linear        Linear attenuation factor.
 * @param exp           Exponentional attenuation factor. */
void SceneLight::setAttenuation(float constant, float linear, float exp) {
    m_attenuationConstant = constant;
    m_attenuationLinear = linear;
    m_attenuationExp = exp;

    m_uniforms->attenuationConstant = m_attenuationConstant;
    m_uniforms->attenuationLinear = m_attenuationLinear;
    m_uniforms->attenuationExp = m_attenuationExp;
}

/** Set whether the light casts shadows.
 * @param castShadows   Whether the light casts shadows. */
void SceneLight::setCastShadows(bool castShadows) {
    if (castShadows != m_castShadows) {
        m_castShadows = castShadows;
        updateShadowViews();
    }
}

/** Set the light position (private function called from Scene).
 * @param position      New light position. */
void SceneLight::setPosition(const glm::vec3 &position) {
    m_position = position;
    m_uniforms->position = m_position;

    updateVolumeTransform();
    updateShadowViews();
}

/** Get light volume geometry.
 * @param geometry      Geometry structure to fill in. */
void SceneLight::volumeGeometry(Geometry &geometry) const {
    switch (m_type) {
        case kPointLight:
            g_renderManager->sphereGeometry(geometry);
            break;
        case kSpotLight:
            g_renderManager->coneGeometry(geometry);
            break;
        default:
            g_renderManager->quadGeometry(geometry);
            break;
    }
}

/** Allocate a shadow map for this light.
 * @return              Pointer to allocated shadow map. */
GPUTexture *SceneLight::allocShadowMap() const {
    GPUTextureDesc desc;
    desc.width = desc.height = g_renderManager->shadowMapResolution();
    desc.mips = 1;
    desc.flags = GPUTexture::kRenderTarget;
    desc.format = PixelFormat::kDepth24Stencil8;

    switch (m_type) {
        case kSpotLight:
            desc.type = GPUTexture::kTexture2D;
            break;
        case kPointLight:
            desc.type = GPUTexture::kTextureCube;
            break;
        default:
            fatal("TODO");
    }

    return g_renderManager->allocTempRenderTarget(desc);
}

/** Determine if the light is visible to a view.
 * @param view          View to test against.
 * @return              Whether the light should be culled. */
bool SceneLight::cull(SceneView *view) const {
    switch (m_type) {
        case kAmbientLight:
        case kDirectionalLight:
            return false;

        case kPointLight:
        {
            Sphere sphere(m_position, m_range);
            return !Math::intersect(view->frustum(), sphere);
        }

        case kSpotLight:
            return !Math::intersect(view->frustum(), m_boundingBox);

        default:
            return true;
    }
}

/** Update the light volume transformation. */
void SceneLight::updateVolumeTransform() {
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
            m_uniforms->volumeTransform = m_volumeTransform.matrix();
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
            m_uniforms->volumeTransform = m_volumeTransform.matrix();

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
void SceneLight::updateShadowViews() {
    if (!m_castShadows)
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
             * apply a bias to this matrix to map coordinates into the [0, 1]
             * range for shadow map sampling, as just the VP transformation
             * yields NDC coordinates, i.e. in the range [-1, 1]. */
            static const glm::mat4 shadowBiasMatrix(
                0.5, 0.0, 0.0, 0.0,
                0.0, 0.5, 0.0, 0.0,
                0.0, 0.0, 0.5, 0.0,
                0.5, 0.5, 0.5, 1.0);
            m_uniforms->shadowSpace
                = shadowBiasMatrix * m_shadowViews[0].projection() * m_shadowViews[0].view();
            break;
        }

        case kPointLight:
        {
            numViews = CubeFace::kNumFaces;

            static const struct {
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
            m_uniforms->shadowZNear = 0.1f;
            break;
        }

        default:
            /* TODO. */
            break;
    }

    /* Viewport should cover the whole shadow map. */
    uint16_t shadowMapResolution = g_renderManager->shadowMapResolution();
    IntRect viewport(0, 0, shadowMapResolution, shadowMapResolution);
    for (unsigned i = 0; i < numViews; i++)
        m_shadowViews[i].setViewport(viewport);
}

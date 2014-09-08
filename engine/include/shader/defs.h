/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Shader system definitions.
 */

#pragma once

#include "core/core.h"

/**
 * Standard uniform buffer binding point indices.
 *
 * We define a set of uniform buffer binding point indices for standard things
 * like per-entity/light/view uniforms. These will be bound automatically by
 * the renderer. Shader-specific uniform buffers can be bound to indices in the
 * custom range, and should be bound by shader code.
 */
namespace ShaderUniforms {
	enum {
		/** Uniforms for the entity currently being rendered. */
		kEntityUniforms = 0,
		/** Uniforms for the light for the current pass. */
		kLightUniforms = 1,
		/** Uniforms for the view the scene is being rendered from. */
		kViewUniforms = 2,

		/** Custom (shader-specific) uniform block range. */
		kCustomUniformsStart = 8,
		kCustomUniformsEnd = 15,
	};
}

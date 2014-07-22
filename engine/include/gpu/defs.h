/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		GPU interface global definitions.
 */

#ifndef ORION_GPU_DEFS_H
#define ORION_GPU_DEFS_H

#include "core/defs.h"

/** Possible primitive types. */
enum class PrimitiveType {
	/** List of triangles, 3 vertices per triangle. */
	kTriangleList,
	/** Triangle strip, 3 vertices for the first triangle and 1 for every other. */
	kTriangleStrip,
	/** Triangle fan, 3 vertices for the first triangle and 1 for every other. */
	kTriangleFan,
	/** Point list, 1 vertex each. */
	kPointList,
};

/** Render buffer definitions. */
enum RenderBuffer {
	kColourBuffer = (1 << 0),	/**< Colour buffer. */
	kDepthBuffer = (1 << 1),	/**< Depth buffer. */
	kStencilBuffer = (1 << 2),	/**< Stencil buffer. */
};

/**
 * Colour blending functions.
 *
 * The blending function determines how a new colour ("source" colour) is
 * combined with the colour already in the framebuffer ("destination" colour).
 */
enum class BlendFunc {
	kDisabled,			/**< Disable blending. */
	kAdd,				/**< Add source and destination colour. */
	kSubtract,			/**< Subtract destination from source. */
	kReverseSubtract,		/**< Subtract source from destination. */
	kMin,				/**< Set each RGBA component to the minimum from the 2 colours. */
	kMax,				/**< Set each RGBA component to the maximum from the 2 colours. */
};

/**
 * Colour blending factors.
 *
 * The blending factors specify how to scale the source and desination colours
 * when blending is enabled.
 */
enum class BlendFactor {
	kZero,				/**< Multiply by 0. */
	kOne,				/**< Multiply by 1. */
	kSourceColour,			/**< Multiply by the source colour. */
	kDestColour,			/**< Multiply by the source colour. */
	kOneMinusSourceColour,		/**< Multiply by (1 - source colour). */
	kOneMinusDestColour,		/**< Multiply by (1 - dest colour). */
	kSourceAlpha,			/**< Multiply by the source alpha. */
	kDestAlpha,			/**< Multiply by the source alpha. */
	kOneMinusSourceAlpha,		/**< Multiply by (1 - source alpha). */
	kOneMinusDestAlpha,		/**< Multiply by (1 - dest alpha). */
};

/** Comparison function for depth/stencil tests. */
enum class ComparisonFunc {
	kAlways,			/**< Always passes (depth testing disabled). */
	kNever,				/**< Always fails. */
	kEqual,				/**< Pass if incoming == current. */
	kNotEqual,			/**< Pass if incoming != current. */
	kLess,				/**< Pass if incoming < current. */
	kLessOrEqual,			/**< Pass if incoming <= current. */
	kGreater,			/**< Pass if incoming > current. */
	kGreaterOrEqual,		/**< Pass if incoming >= current. */
};

/** Face culling mode. */
enum class CullMode {
	kDisabled,			/**< Disable face culling. */
	kBack,				/**< Cull back-facing polygons. */
	kFront,				/**< Cull front-facing polygons. */
};

#endif /* ORION_GPU_DEFS_H */

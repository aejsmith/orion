/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               GPU interface global definitions.
 */

#pragma once

#include "core/refcounted.h"

/**
 * Constants/limitations.
 */

/** Maximum number of colour render targets. */
static const size_t kMaxColourRenderTargets = 8;

/**
 * Miscellaneous definitions.
 */

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

/** Buffers to clear. */
namespace ClearBuffer {
    enum {
        /** Colour buffer. */
        kColourBuffer = (1 << 0),
        /** Depth buffer. */
        kDepthBuffer = (1 << 1),
        /** Stencil buffer. */
        kStencilBuffer = (1 << 2),
    };
};

/** Faces for a cubemap. */
namespace CubeFace {
    enum {
        kPositiveX,             /**< Positive X direction. */
        kNegativeX,             /**< Negative X direction. */
        kPositiveY,             /**< Positive Y direction. */
        kNegativeY,             /**< Negative Y direction. */
        kPositiveZ,             /**< Positive Z direction. */
        kNegativeZ,             /**< Negative Z direction. */
        kNumFaces,
    };
}

/** Shader stage definitions. */
namespace ShaderStage {
    enum {
        kVertex,                /**< Vertex shader. */
        kFragment,              /**< Fragment/pixel shader. */
        kNumStages,
    };
}

/**
 * Blend state definitions.
 */

/**
 * Colour blending functions.
 *
 * The blending function determines how a new colour ("source" colour) is
 * combined with the colour already in the framebuffer ("destination" colour).
 */
enum class BlendFunc {
    kAdd,                       /**< Add source and destination colour. */
    kSubtract,                  /**< Subtract destination from source. */
    kReverseSubtract,           /**< Subtract source from destination. */
    kMin,                       /**< Set each RGBA component to the minimum from the 2 colours. */
    kMax,                       /**< Set each RGBA component to the maximum from the 2 colours. */
};

/**
 * Colour blending factors.
 *
 * The blending factors specify how to scale the source and desination colours
 * when blending is enabled.
 */
enum class BlendFactor {
    kZero,                      /**< Multiply by 0. */
    kOne,                       /**< Multiply by 1. */
    kSourceColour,              /**< Multiply by the source colour. */
    kDestColour,                /**< Multiply by the source colour. */
    kOneMinusSourceColour,      /**< Multiply by (1 - source colour). */
    kOneMinusDestColour,        /**< Multiply by (1 - dest colour). */
    kSourceAlpha,               /**< Multiply by the source alpha. */
    kDestAlpha,                 /**< Multiply by the source alpha. */
    kOneMinusSourceAlpha,       /**< Multiply by (1 - source alpha). */
    kOneMinusDestAlpha,         /**< Multiply by (1 - dest alpha). */
};

/**
 * Depth/stencil state definitions.
 */

/** Comparison function for depth/stencil tests. */
enum class ComparisonFunc {
    kAlways,                    /**< Always passes (depth testing disabled). */
    kNever,                     /**< Always fails. */
    kEqual,                     /**< Pass if incoming == current. */
    kNotEqual,                  /**< Pass if incoming != current. */
    kLess,                      /**< Pass if incoming < current. */
    kLessOrEqual,               /**< Pass if incoming <= current. */
    kGreater,                   /**< Pass if incoming > current. */
    kGreaterOrEqual,            /**< Pass if incoming >= current. */
};

/**
 * Rasterizer state definitions.
 */

/** Face culling mode. */
enum class CullMode {
    kDisabled,                  /**< Disable face culling. */
    kBack,                      /**< Cull back-facing polygons. */
    kFront,                     /**< Cull front-facing polygons. */
};

/**
 * Sampler state definitions.
 */

/** Method for resolving texture coordinates outside the (0, 1) range. */
enum class SamplerAddressMode {
    kClamp,                     /**< Clamp to (0, 1). */
    kWrap,                      /**< Tile the texture, i.e. wrap coordinates. */
};

/** Texture filtering mode. */
enum class SamplerFilterMode {
    kNearest,                   /**< Use nearest point. */
    kBilinear,                  /**< Linear interpolation within mip, single mip level. */
    kTrilinear,                 /**< Linear interpolation within mip and between mip levels. */
    kAnisotropic,               /**< Anisotropic filtering. */
};

/**
 * Resource base class.
 */

/**
 * Base class for GPU resources.
 *
 * All GPU resources derive from this class. It includes reference counting
 * functionality so that resources will only be freed once they have no more
 * users. GPU resources are also not copyable.
 */
class GPUResource : public Refcounted, Noncopyable {
protected:
    GPUResource() {}
};

/** Type of a GPU resource pointer. */
template <typename T> using GPUResourcePtr = ReferencePtr<T>;

/**
 * Type of a global GPU resource pointer.
 *
 * Use this to store a global pointer to a GPU resource. It is implemented
 * using EngineGlobal and ensures that the pointer will be released before the
 * GPUManager is shut down.
 */
template <typename T>
using GlobalGPUResource = EngineGlobal<T, GPUResourcePtr<T>>;

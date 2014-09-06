/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Pixel format definitions.
 *
 * @todo		Supported pixel format information in GPUInterface.
 */

#ifndef ORION_LIB_PIXEL_FORMAT_H
#define ORION_LIB_PIXEL_FORMAT_H

#include "core/defs.h"

/** Pixel format definitions. */
struct PixelFormat {
	enum Impl {
		/**< Unknown format. */
		kUnknown,

		/** Colour formats. */
		kRGBA8,			/**< RGBA, unsigned normalized, 8 bits per component. */
		kRGB8,			/**< RGB, unsigned normalized, 8 bits per component. */
		kRG8,			/**< RG, unsigned normalized, 8 bits per component. */
		kR8,			/**< R, unsigned normalized, 8 bits per component. */
		kRGBA16,		/**< RGBA, unsigned normalized, 16 bits per component. */
		kRGB16,			/**< RGB, unsigned normalized, 16 bits per component. */
		kRG16,			/**< RG, unsigned normalized, 16 bits per component. */
		kR16,			/**< R, unsigned normalized, 16 bits per component. */
		kRGBA16Float,		/**< RGBA, float, 16 bits per component. */
		kRGB16Float,		/**< RGB, float, 16 bits per component. */
		kRG16Float,		/**< RG, float, 16 bits per component. */
		kR16Float,		/**< R, float, 16 bits per component. */
		kRGBA32Float,		/**< RGBA, float, 32 bits per component. */
		kRGB32Float,		/**< RGB, float, 32 bits per component. */
		kRG32Float,		/**< RG, float, 32 bits per component. */
		kR32Float,		/**< R, float, 32 bits per component. */
		kRGBA8Signed,		/**< RGBA, signed integral, 8 bits per component. */
		kRGB8Signed,		/**< RGB, signed integral, 8 bits per component. */
		kRG8Signed,		/**< RG, signed integral, 8 bits per component. */
		kR8Signed,		/**< R, signed integral, 8 bits per component. */
		kRGBA16Signed,		/**< RGBA, signed integral, 16 bits per component. */
		kRGB16Signed,		/**< RGB, signed integral, 16 bits per component. */
		kRG16Signed,		/**< RG, signed integral, 16 bits per component. */
		kR16Signed,		/**< R, signed integral, 16 bits per component. */
		kRGBA32Signed,		/**< RGBA, signed integral, 32 bits per component. */
		kRGB32Signed,		/**< RGB, signed integral, 32 bits per component. */
		kRG32Signed,		/**< RG, signed integral, 32 bits per component. */
		kR32Signed,		/**< R, signed integral, 32 bits per component. */
		kRGBA8Unsigned,		/**< RGBA, unsigned integral, 8 bits per component. */
		kRGB8Unsigned,		/**< RGB, unsigned integral, 8 bits per component. */
		kRG8Unsigned,		/**< RG, unsigned integral, 8 bits per component. */
		kR8Unsigned,		/**< R, unsigned integral, 8 bits per component. */
		kRGBA16Unsigned,	/**< RGBA, unsigned integral, 16 bits per component. */
		kRGB16Unsigned,		/**< RGB, unsigned integral, 16 bits per component. */
		kRG16Unsigned,		/**< RG, unsigned integral, 16 bits per component. */
		kR16Unsigned,		/**< R, unsigned integral, 16 bits per component. */
		kRGBA32Unsigned,	/**< RGBA, unsigned integral, 32 bits per component. */
		kRGB32Unsigned,		/**< RGB, unsigned integral, 32 bits per component. */
		kRG32Unsigned,		/**< RG, unsigned integral, 32 bits per component. */
		kR32Unsigned,		/**< R, unsigned integral, 32 bits per component. */

		/** Depth/stencil formats. */
		kDepth16,		/**< Depth, unsigned normalized, 16 bits. */
		kDepth24,		/**< Depth, unsigned normalized, 24 bits. */
		kDepth24Stencil8,	/**< Depth/stencil, unsigned normalized, 24 bits/8 bits. */

		/** Number of pixel formats. */
		kNumFormats,
	};

	PixelFormat() : m_value(kUnknown) {}
	PixelFormat(Impl value) : m_value(value) {}
	operator Impl() const { return m_value; }
private:
	Impl m_value;
};

#endif /* ORION_LIB_PIXEL_FORMAT_H */

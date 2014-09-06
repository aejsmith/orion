/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Texture asset class.
 */

#ifndef ORION_ASSET_TEXTURE_H
#define ORION_ASSET_TEXTURE_H

#include "asset/asset.h"

#include "gpu/texture.h"

/** Base texture asset class. */
class TextureBase : public Asset {
public:
	/** Destroy the texture. */
	~TextureBase() {}

	/** @return		GPU texture implementing this texture. */
	GPUTexturePtr gpu() const { return m_gpu; }

	/** @return		Pixel format for the texture. */
	PixelFormat format() const { return m_gpu->format(); }
	/** @return		Number of mip levels. */
	unsigned mips() const { return m_gpu->mips(); }
protected:
	/** Private constructor, does not actually create the texture. */
	TextureBase() {}
protected:
	GPUTexturePtr m_gpu;		/**< GPU texture pointer. */
};

/** Type of a base texture pointer. */
typedef TypedAssetPtr<TextureBase> TextureBasePtr;

/** Class implementing a 2D texture. */
class Texture2D : public TextureBase {
public:
	Texture2D(
		uint32_t width,
		uint32_t height,
		PixelFormat format = PixelFormat::kRGBA8,
		unsigned mips = 0,
		uint32_t flags = GPUTexture::kAutoMipmap);

	void update(const void *data, bool update_mipmap = true);
	void update(const Rect &area, const void *data, bool update_mipmap = true);
	void update(unsigned mip, const Rect &area, const void *data);

	/** @return		Width of the texture. */
	uint32_t width() const { return m_gpu->width(); }
	/** @return		Height of the texture. */
	uint32_t height() const { return m_gpu->height(); }
};

/** Type of a 2D texture pointer. */
typedef TypedAssetPtr<Texture2D> Texture2DPtr;

#endif /* ORION_ASSET_TEXTURE_H */

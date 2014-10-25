/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Texture asset class.
 */

#pragma once

#include "engine/asset.h"

#include "gpu/state.h"
#include "gpu/texture.h"

/** Base texture asset class. */
class TextureBase : public Asset {
public:
	~TextureBase() {}

	/** @return		Pixel format for the texture. */
	PixelFormat format() const { return m_gpu->format(); }
	/** @return		Number of mip levels. */
	unsigned mips() const { return m_gpu->mips(); }
	/** @return		Texture filtering mode. */
	SamplerFilterMode filterMode() const { return m_filterMode; }
	/** @return		Anisotropic filtering level. */
	unsigned anisotropy() const { return m_anisotropy; }
	/** @return		Addressing mode. */
	SamplerAddressMode addressMode() const { return m_addressMode; }

	void setFilterMode(SamplerFilterMode mode);
	void setAnisotropy(unsigned anisotropy);
	void setAddressMode(SamplerAddressMode mode);

	/** @return		GPU texture implementing this texture. */
	GPUTexture *gpu() const { return m_gpu; }
	/** @return		GPU sampler state for the texture. */
	GPUSamplerState *sampler() const { return m_sampler; }
protected:
	TextureBase();

	void updateSamplerState();
protected:
	GPUTexturePtr m_gpu;			/**< GPU texture pointer. */
	GPUSamplerStatePtr m_sampler;		/**< GPU sampler state. */

	/** Texture sampling parameters. */
	SamplerFilterMode m_filterMode;		/**< Filtering mode. */
	unsigned m_anisotropy;			/**< Anisotropic filtering level. */
	SamplerAddressMode m_addressMode;	/**< Addressing mode. */
};

/** Type of a base texture pointer. */
typedef TypedAssetPtr<TextureBase> TextureBasePtr;

/** Class implementing a 2D texture. */
class Texture2D : public TextureBase {
public:
	Texture2D(
		uint32_t width,
		uint32_t height,
		PixelFormat format = PixelFormat::kR8G8B8A8,
		unsigned mips = 0,
		uint32_t flags = GPUTexture::kAutoMipmap);

	void update(const void *data, bool updateMipmap = true);
	void update(const Rect &area, const void *data, bool updateMipmap = true);
	void update(unsigned mip, const Rect &area, const void *data);

	/** @return		Width of the texture. */
	uint32_t width() const { return m_gpu->width(); }
	/** @return		Height of the texture. */
	uint32_t height() const { return m_gpu->height(); }
};

/** Type of a 2D texture pointer. */
typedef TypedAssetPtr<Texture2D> Texture2DPtr;

/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		GL texture implementation.
 */

#pragma once

#include "gpu/texture.h"

/** OpenGL texture implementation. */
class GLTexture : public GPUTexture {
public:
	explicit GLTexture(const GPUTexture2DDesc &desc);
	explicit GLTexture(const GPUTexture2DArrayDesc &desc);
	explicit GLTexture(const GPUTextureCubeDesc &desc);
	explicit GLTexture(const GPUTexture3DDesc &desc);

	~GLTexture();

	void update(const IntRect &area, const void *data, unsigned mip, unsigned layer) override;
	void update(const IntBox &area, const void *data, unsigned mip) override;
	void generateMipmap() override;

	void bind(unsigned index);

	/** @return		GL texture ID. */
	GLuint texture() const { return m_texture; }
	/** @return		GL target. */
	GLenum glTarget() const { return m_glTarget; }
private:
	template <typename Desc> GLTexture(const Desc &desc, GLenum target);

	void bindForModification();
private:
	GLuint m_texture;		/**< GL texture handle. */
	GLenum m_glTarget;		/**< Target for the texture. */
};

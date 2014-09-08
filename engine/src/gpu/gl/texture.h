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
	template <typename Desc>
	GLTexture(const Desc &desc, GLenum target);

	explicit GLTexture(const GPUTexture2DDesc &desc);
	explicit GLTexture(const GPUTexture2DArrayDesc &desc);
	explicit GLTexture(const GPUTextureCubeDesc &desc);
	explicit GLTexture(const GPUTexture3DDesc &desc);

	~GLTexture();

	void update(const Rect &area, const void *data, unsigned mip, unsigned layer) override;
	void update(const Box &area, const void *data, unsigned mip) override;
	void generate_mipmap() override;

	void bind(unsigned index);
private:
	void bind_for_modification();
private:
	GLuint m_texture;		/**< GL texture handle. */
	GLenum m_gl_target;		/**< Target for the texture. */
};

/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               GL texture implementation.
 */

#pragma once

#include "gpu/texture.h"

/** OpenGL texture implementation. */
class GLTexture : public GPUTexture {
public:
    explicit GLTexture(const GPUTextureDesc &desc);

    ~GLTexture();

    void update(const IntRect &area, const void *data, unsigned mip, unsigned layer) override;
    void update(const IntBox &area, const void *data, unsigned mip) override;
    void generateMipmap() override;

    void bind(unsigned index);

    /** @return             GL texture ID. */
    GLuint texture() const { return m_texture; }
    /** @return             GL target. */
    GLenum glTarget() const { return m_glTarget; }
private:
    void bindForModification();
private:
    GLuint m_texture;               /**< GL texture handle. */
    GLenum m_glTarget;              /**< Target for the texture. */
};

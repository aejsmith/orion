/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               OpenGL buffer implementation.
 */

#pragma once

#include "gl.h"

/** OpenGL GPU buffer implementation. */
class GLBuffer : public GPUBuffer {
public:
    GLBuffer(Type type, Usage usage, size_t size);
    ~GLBuffer();

    void bind() const;
    void bindIndexed(unsigned index) const;

    /** @return             GL buffer ID. */
    GLuint buffer() const { return m_buffer; }
protected:
    void writeImpl(size_t offset, size_t size, const void *buf) override;
    void *mapImpl(size_t offset, size_t size, uint32_t flags, uint32_t access) override;
    void unmapImpl() override;
private:
    GLuint m_buffer;                /**< Buffer object ID. */
    GLenum m_glTarget;              /**< GL target. */
    GLenum m_glUsage;               /**< GL usage. */
};

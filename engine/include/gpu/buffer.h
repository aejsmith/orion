/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Hardware buffer classes.
 */

#ifndef ORION_GPU_BUFFER_H
#define ORION_GPU_BUFFER_H

#include "core/defs.h"

#include "lib/noncopyable.h"

#include <memory>

/**
 * Class for storing data on the GPU.
 *
 * This class encapsulates a buffer in GPU memory. There are multiple buffer
 * types, the type of the buffer must be declared at creation time. The
 * implementation of the class is API-specific, therefore instances must be
 * created with GPUInterface::create_buffer().
 */
class GPUBuffer : Noncopyable {
public:
	/** Enum of possible buffer types. */
	enum Type {
		kVertexBuffer,		/**< Vertex buffer. */
		kIndexBuffer,		/**< Index buffer. */
		kUniformBuffer,		/**< Uniform buffer. */
	};

	/** Enum describing intended buffer usage. */
	enum Usage {
		/** Stream: modified once and used at most a few times. */
		kStreamDrawUsage,	/**< Used for drawing operations. */
		kStreamReadUsage,	/**< Used to read from. */
		kStreamCopyUsage,	/**< Used for reading and drawing. */

		/** Static: modified once and used many times. */
		kStaticDrawUsage,	/**< Used for drawing operations. */
		kStaticReadUsage,	/**< Used to read from. */
		kStaticCopyUsage,	/**< Used for reading and drawing. */

		/** Dynamic: modified repeatedly and used many times. */
		kDynamicDrawUsage,	/**< Used for drawing operations. */
		kDynamicReadUsage,	/**< Used to read from. */
		kDynamicCopyUsage,	/**< Used for reading and drawing. */
	};
public:
	/** Destroy the buffer. */
	virtual ~GPUBuffer() {}

	/**
	 * Write data to the buffer.
	 *
	 * Replaces some or all of the current buffer content with new data.
	 * The area to write must lie within the bounds of the buffer, i.e.
	 * (offset + size) must be less than or equal to the buffer size.
	 *
	 * @param buf		Buffer containing data to write.
	 * @param size		Size of the data to write.
	 * @param offset	Offset to write at.
	 */
	virtual void write(const void *buf, size_t size, size_t offset) = 0;

	/** Get the type of the buffer.
	 * @return		Type of the buffer. */
	Type type() const { return m_type; }

	/** Get the usage hint for the buffer.
	 * @return		Buffer usage hint. */
	Usage usage() const { return m_usage; }

	/** Get the total buffer size.
	 * @return		Total buffer size. */
	size_t size() const { return m_size; }
protected:
	/** Construct the GPU buffer.
	 * @param type		Type of the buffer.
	 * @param usage		Usage hint.
	 * @param size		Buffer size. */
	GPUBuffer(Type type, Usage usage, size_t size) :
		m_type(type), m_usage(usage), m_size(size)
	{}
protected:
	Type m_type;			/**< Type of the buffer */
	Usage m_usage;			/**< Buffer usage hint. */
	size_t m_size;			/**< Buffer size. */
};

/** Type of a pointer to a GPU buffer. */
typedef std::shared_ptr<GPUBuffer> GPUBufferPtr;

#endif /* ORION_GPU_BUFFER_H */

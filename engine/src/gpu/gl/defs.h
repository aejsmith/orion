/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		OpenGL internal definitions and helper functions.
 */

#ifndef ORION_GPU_GL_DEFS_H
#define ORION_GPU_GL_DEFS_H

#include "gpu/gpu.h"

#include <GL/glew.h>

#include <SDL.h>

namespace gl {

/** Convert a vertex attribute type to a GL type.
 * @param type		Attribute type.
 * @param gl		GL type.
 * @return		Whether type is supported. */
static inline GLenum convert_attribute_type(VertexAttribute::Type type) {
	switch(type) {
	case VertexAttribute::kByteType:
		return GL_BYTE;
	case VertexAttribute::kUnsignedByteType:
		return GL_UNSIGNED_BYTE;
	case VertexAttribute::kShortType:
		return GL_SHORT;
	case VertexAttribute::kUnsignedShortType:
		return GL_UNSIGNED_SHORT;
	case VertexAttribute::kIntType:
		return GL_INT;
	case VertexAttribute::kUnsignedIntType:
		return GL_UNSIGNED_INT;
	case VertexAttribute::kFloatType:
		return GL_FLOAT;
	case VertexAttribute::kDoubleType:
		return GL_DOUBLE;
	default:
		return 0;
	}
}

/** Convert a buffer type to a GL buffer target.
 * @param type		Buffer type.
 * @return		OpenGL buffer target. */
static inline GLenum convert_buffer_type(GPUBuffer::Type type) {
	switch(type) {
	case GPUBuffer::kVertexBuffer:
		return GL_ARRAY_BUFFER;
	case GPUBuffer::kIndexBuffer:
		return GL_ELEMENT_ARRAY_BUFFER;
	case GPUBuffer::kUniformBuffer:
		return GL_UNIFORM_BUFFER;
	default:
		return 0;
	}
}

/** Convert a buffer usage hint to a GL usage hint.
 * @param usage		Usage hint.
 * @return		OpenGL usage hint value. */
static inline GLenum convert_buffer_usage(GPUBuffer::Usage usage) {
	switch(usage) {
	case GPUBuffer::kStreamDrawUsage:
		return GL_STREAM_DRAW;
	case GPUBuffer::kStreamReadUsage:
		return GL_STREAM_READ;
	case GPUBuffer::kStreamCopyUsage:
		return GL_STREAM_COPY;
	case GPUBuffer::kStaticDrawUsage:
		return GL_STATIC_DRAW;
	case GPUBuffer::kStaticReadUsage:
		return GL_STATIC_READ;
	case GPUBuffer::kStaticCopyUsage:
		return GL_STATIC_COPY;
	case GPUBuffer::kDynamicDrawUsage:
		return GL_DYNAMIC_DRAW;
	case GPUBuffer::kDynamicReadUsage:
		return GL_DYNAMIC_READ;
	case GPUBuffer::kDynamicCopyUsage:
		return GL_DYNAMIC_COPY;
	default:
		return 0;
	}
}

/** Convert an index data type to a GL data type.
 * @param type		Type to convert.
 * @return		GL data type. */
static inline GLenum convert_index_type(IndexData::Type type) {
	switch(type) {
	case IndexData::kUnsignedByteType:
		return GL_UNSIGNED_BYTE;
	case IndexData::kUnsignedShortType:
		return GL_UNSIGNED_SHORT;
	case IndexData::kUnsignedIntType:
		return GL_UNSIGNED_INT;
	default:
		return 0;
	}
}

/** Convert a primitive type to a GL primitive type.
 * @param type		Type to convert.
 * @return		GL primitive type. */
static inline GLenum convert_primitive_type(PrimitiveType type) {
	switch(type) {
	case PrimitiveType::kTriangleList:
		return GL_TRIANGLES;
	case PrimitiveType::kTriangleStrip:
		return GL_TRIANGLE_STRIP;
	case PrimitiveType::kTriangleFan:
		return GL_TRIANGLE_FAN;
	case PrimitiveType::kPointList:
		return GL_POINTS;
	default:
		return 0;
	}
}

/** Convert a program type to a GL shader type.
 * @param type		Type to convert.
 * @return		Converted GL type. */
static inline GLenum convert_program_type(GPUProgram::Type type) {
	switch(type) {
	case GPUProgram::kVertexProgram:
		return GL_VERTEX_SHADER;
	case GPUProgram::kFragmentProgram:
		return GL_FRAGMENT_SHADER;
	default:
		return 0;
	}
}

/** Convert a program type to a GL bitfield type.
 * @param type		Type to convert.
 * @return		Converted GL bitfield type. */
static inline GLbitfield convert_program_type_bitfield(GPUProgram::Type type) {
	switch(type) {
	case GPUProgram::kVertexProgram:
		return GL_VERTEX_SHADER_BIT;
	case GPUProgram::kFragmentProgram:
		return GL_FRAGMENT_SHADER_BIT;
	default:
		return 0;
	}
}

}

#endif /* ORION_GPU_GL_DEFS_H */

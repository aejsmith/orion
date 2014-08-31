/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		OpenGL GPU interface implementation.
 */

#ifndef ORION_GPU_GL_GL_H
#define ORION_GPU_GL_GL_H

#include "state.h"

#include "gpu/gpu.h"

#include <SDL.h>

#include <array>
#include <set>

/** Define to 1 to enable ARB_debug_output. */
#define ORION_GL_DEBUG			1

/** Define to 1 to enable ARB_debug_output notification messages (excessive). */
#define ORION_GL_DEBUG_NOTIFICATIONS	0

#include "state.h"

/** OpenGL feature information.
 * @todo		Add a bitmap of features that we often need to check
 *			for so we don't have to do a string comparison every
 *			time they are checked. */
struct GLFeatures {
	/** List of extensions. */
	std::set<std::string> extensions;

	/** Cached glGet* parameters. */
	GLint max_texture_units;	/**< GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS */
public:
	/** Check whether an extension is supported.
	 * @param extension	Extension to check for.
	 * @return		Whether the extension is supported. */
	bool operator [](const char *extension) const {
		return this->extensions.find(extension) != this->extensions.end();
	}
};

/** Structure mapping PixelFormat to GL types. */
struct GLPixelFormat {
	GLenum internal_format;		/**< Internal texture format. */
	GLenum format;			/**< Pixel data format. */
	GLenum type;			/**< Pixel data type. */
public:
	GLPixelFormat(GLenum i = GL_NONE, GLenum f = GL_NONE, GLenum t = GL_NONE) :
		internal_format(i),
		format(f),
		type(t)
	{}
};

/** OpenGL GPU interface implementation. */
class GLGPUInterface : public GPUInterface {
public:
	/** Type of the pixel format mapping array. */
	typedef std::array<GLPixelFormat, PixelFormat::kNumFormats> PixelFormatArray;
public:
	GLGPUInterface();
	~GLGPUInterface();

	void init(SDL_Window *window) override;

	/**
	 * GPU interface methods.
	 */

	GPUBufferPtr create_buffer(GPUBuffer::Type type, GPUBuffer::Usage usage, size_t size) override;
	VertexDataPtr create_vertex_data(size_t vertices) override;
	GPUPipelinePtr create_pipeline() override;
	GPUProgramPtr load_program(const char *path, GPUProgram::Type type) override;
	GPUTexturePtr create_texture(const GPUTexture2DDesc &desc) override;
	GPUTexturePtr create_texture(const GPUTexture2DArrayDesc &desc) override;
	GPUTexturePtr create_texture(const GPUTextureCubeDesc &desc) override;
	GPUTexturePtr create_texture(const GPUTexture3DDesc &desc) override;

	void bind_pipeline(const GPUPipelinePtr &pipeline) override;
	void bind_uniform_buffer(unsigned index, const GPUBufferPtr &buffer) override;
	void set_blend_mode(BlendFunc func, BlendFactor source_factor, BlendFactor dest_factor) override;
	void set_depth_mode(ComparisonFunc func, bool enable_write) override;

	void end_frame(bool vsync) override;

	void clear(unsigned buffers, const glm::vec4 &colour, float depth, uint32_t stencil) override;
	void draw(PrimitiveType type, const VertexDataPtr &vertices, const IndexDataPtr &indices) override;
public:
	/** GL feature information. */
	GLFeatures features;
	/** Mapping of engine pixel formats to GL types. */
	PixelFormatArray pixel_formats;
	/** Cached GL state. */
	GLState state;
	/** Default VAO when no object-specific VAO is in use. */
	GLuint default_vao;
private:
	void init_features();
	void init_pixel_formats();

	static GLEWAPIENTRY void debug_callback(
		GLenum source,
		GLenum type,
		GLuint id,
		GLenum severity,
		GLsizei length,
		const GLchar *message,
		const GLvoid *param);
private:
	SDL_GLContext m_sdl_context;	/**< SDL GL context. */
};

extern GLGPUInterface *g_opengl;

/**
 * Utility functions.
 */

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

/** Convert a blend function to a GL blend equation.
 * @param func		Function to convert.
 * @return		OpenGL blend equation. */
static inline GLenum convert_blend_func(BlendFunc func) {
	switch(func) {
	case BlendFunc::kAdd:
		return GL_FUNC_ADD;
	case BlendFunc::kSubtract:
		return GL_FUNC_SUBTRACT;
	case BlendFunc::kReverseSubtract:
		return GL_FUNC_REVERSE_SUBTRACT;
	case BlendFunc::kMin:
		return GL_MIN;
	case BlendFunc::kMax:
		return GL_MAX;
	default:
		return 0;
	}
}

/** Convert a blend factor to a GL blend factor.
 * @param func		Function to convert.
 * @return		OpenGL blend equation. */
static inline GLenum convert_blend_factor(BlendFactor factor) {
	switch(factor) {
	case BlendFactor::kZero:
		return GL_ZERO;
	case BlendFactor::kOne:
		return GL_ONE;
	case BlendFactor::kSourceColour:
		return GL_SRC_COLOR;
	case BlendFactor::kDestColour:
		return GL_DST_COLOR;
	case BlendFactor::kOneMinusSourceColour:
		return GL_ONE_MINUS_SRC_COLOR;
	case BlendFactor::kOneMinusDestColour:
		return GL_ONE_MINUS_DST_COLOR;
	case BlendFactor::kSourceAlpha:
		return GL_SRC_ALPHA;
	case BlendFactor::kDestAlpha:
		return GL_DST_ALPHA;
	case BlendFactor::kOneMinusSourceAlpha:
		return GL_ONE_MINUS_SRC_ALPHA;
	case BlendFactor::kOneMinusDestAlpha:
		return GL_ONE_MINUS_DST_ALPHA;
	default:
		return 0;
	}
}

/** Convert a buffer type to a GL buffer target.
 * @param type		Buffer type.
 * @return		GL buffer target. */
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

/** Convert a comparison function to a GL comparison function.
 * @param func		Function to convert.
 * @return		GL comparison function. */
static inline GLenum convert_comparison_func(ComparisonFunc func) {
	switch(func) {
	case ComparisonFunc::kAlways:
		return GL_ALWAYS;
	case ComparisonFunc::kNever:
		return GL_NEVER;
	case ComparisonFunc::kEqual:
		return GL_EQUAL;
	case ComparisonFunc::kNotEqual:
		return GL_NOTEQUAL;
	case ComparisonFunc::kLess:
		return GL_LESS;
	case ComparisonFunc::kLessOrEqual:
		return GL_LEQUAL;
	case ComparisonFunc::kGreater:
		return GL_GREATER;
	case ComparisonFunc::kGreaterOrEqual:
		return GL_GEQUAL;
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

#endif /* ORION_GPU_GL_GL_H */

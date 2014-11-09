/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Shader pass class.
 */

#pragma once

#include "core/path.h"

#include "gpu/pipeline.h"

#include <set>

class Shader;

/** Rendering pass. */
class Pass : Noncopyable {
public:
	/** Pass types. */
	enum Type {
		/**
		 * Always rendered, no lighting is applied. Also used for
		 * post-process and internal shaders. Every pass of this type
		 * will be executed in order once per entity.
		 */
		kBasicPass,

		/**
		 * Forward shading pass. Every pass of this type will be
		 * executed in order for each light affecting the entity.
		 */
		kForwardPass,

		/**
		 * Deferred base pass. Accumulates material properties into the
		 * G-Buffer. Only one pass of this type should be specified.
		 */
		kDeferredBasePass,

		/**
		 * Deferred output pass. Combines calculated lighting with
		 * textures. Only one pass of this type should be specified.
		 */
		kDeferredOutputPass,

		/** Number of Pass types. */
		kNumTypes,
	};

	/** Set of shader variation keywords. */
	typedef std::set<std::string> KeywordSet;
public:
	Pass(Shader *parent, Type type);
	~Pass();

	/** @return		Parent shader. */
	Shader *parent() const { return m_parent; }
	/** @return		Type of the pass. */
	Type type() const { return m_type; }

	bool loadStage(GPUShader::Type stage, const Path &path, const KeywordSet &keywords);

	void setDrawState() const;
private:
	void finalize();
private:
	Shader *m_parent;			/**< Parent shader. */
	Type m_type;				/**< Type of the pass. */
	GPUPipelinePtr m_pipeline;		/**< GPU pipeline. */

	/** Array of shaders, valid only during initialization. */
	GPUShaderArray m_loadingShaders;

	friend class Shader;
};
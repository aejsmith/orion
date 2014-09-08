/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Rendering pipeline object.
 */

#pragma once

#include "gpu/program.h"

#include <array>

/**
 * Rendering pipeline.
 *
 * This class groups together a set of GPU programs to use for each pipeline
 * stage. Before a pipeline object can be used for rendering it must be
 * finalized, after which it becomes immutable. A pipeline object has an API-
 * specific implementation, therefore instances must be created via
 * GPUInterface::createPipeline().
 *
 * @todo		Modern APIs (DX12, Metal, Mantle) have the concept of
 *			a pipeline state object that bundle up programs along
 *			with some bits of state like blend mode, output buffer
 *			format and vertex format, the goal being to avoid draw-
 *			time shader recompilation for different states. This
 *			should in future encapsulate this state as well. The
 *			main reason that I have not done this now is the need
 *			to keep vertex format, this would require us to have an
 *			individual pipeline for each shader/vertex format
 *			combination which is somewhat awkward to support.
 */
class GPUPipeline : public GPUResource {
public:
	void setProgram(GPUProgram::Type stage, const GPUProgramPtr &program);
	void finalize();

	/** Get the program for a stage.
	 * @param stage		Stage to get program for.
	 * @return		Program set for that stage. */
	GPUProgramPtr program(GPUProgram::Type stage) const { return m_programs[stage]; }
protected:
	/** Type of the program array. */
	typedef std::array<GPUProgramPtr, GPUProgram::kNumProgramTypes> ProgramArray;
protected:
	GPUPipeline();

	/** Called when the object is being finalized. */
	virtual void finalizeImpl() {}
protected:
	ProgramArray m_programs;	/**< Array of programs for each stage. */
	bool m_finalized;		/**< Whether the state has been finalized. */

	/* For the default implementation of createPipeline(). */
	friend class GPUInterface;
};

/** Type of a pipeline object pointer. */
typedef GPUResourcePtr<GPUPipeline> GPUPipelinePtr;

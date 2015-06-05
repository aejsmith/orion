/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               GPU program class.
 */

#pragma once

#include "gpu/defs.h"

#include <array>

/** GPU program class. */
class GPUProgram : public GPUResource {
public:
    /** Structure describing a resource. */
    struct Resource {
        std::string name;           /**< Name of the resource. */
        unsigned index;             /**< Index of the resource for use with bind functions. */
    };

    /** Type of a resource list, given as a pair of name and index.
     * @note                List rather than a vector to allow for sparse
     *                      indices. */
    typedef std::list<Resource> ResourceList;
public:
    /** @return             Stage that the program is for. */
    unsigned stage() const { return m_stage; }

    /** Query active uniform blocks in the program.
     * @param list          Resource list to fill in. */
    virtual void queryUniformBlocks(ResourceList &list) = 0;

    /** Query active texture samplers in the program.
     * @param list          Resource list to fill in. */
    virtual void querySamplers(ResourceList &list) = 0;

    /**
     * Bind a uniform block in the program.
     *
     * Specifies that the uniform block at the specified index (as returned
     * from queryUniformBlocks()) should refer to the uniform buffer which is
     * bound in the specified slot at the time of a draw call involving the
     * program.
     *
     * @param index         Index of uniform block.
     * @param slot          Uniform buffer slot.
     */
    virtual void bindUniformBlock(unsigned index, unsigned slot) = 0;

    /**
     * Bind a texture sampler in the program.
     *
     * Specifies that the texture sampler at the specified index (as returned
     * from querySamplers()) should refer to the texture which is bound in the
     * specified slot at the time of a draw call involving the program.
     *
     * @param index         Index of sampler.
     * @param slot          Texture slot.
     */
    virtual void bindSampler(unsigned index, unsigned slot) = 0;
protected:
    /** Initialize the program.
     * @param stage         Stage that the program is for. */
    explicit GPUProgram(unsigned stage) : m_stage(stage) {}
private:
    unsigned m_stage;                   /**< Type of the program. */
};

/** Type of a GPU program pointer. */
typedef GPUResourcePtr<GPUProgram> GPUProgramPtr;

/** Type of an array of GPU programs, indexed by stage. */
typedef std::array<GPUProgramPtr, ShaderStage::kNumStages> GPUProgramArray;

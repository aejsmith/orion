/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Draw list class.
 */

#pragma once

#include "render/pass.h"
#include "render/scene_entity.h"

#include <list>

/**
 * Draw call structure.
 *
 * This structure is an extension of DrawData which is stored in a DrawList. It
 * stores information for a single rendering pass for an entity.
 */
struct DrawCall : public DrawData {
    const Pass *pass;                   /**< Pass to draw with. */
    GPUBuffer *uniforms;                /**< Entity uniforms. */
public:
    DrawCall() {}
    DrawCall(const DrawData &source) : DrawData(source) {}
};

/** Class storing a list of draw calls. */
class DrawList {
public:
    void addDrawCall(const DrawData &source, const Pass *pass, GPUBuffer *uniforms);
    void addDrawCalls(const DrawData &source, Pass::Type passType, GPUBuffer *uniforms);

    void draw(SceneLight *light = nullptr) const;

    /** @return             Whether the draw list is empty. */
    bool empty() const { return m_drawCalls.empty(); }
private:
    /** List of draw calls. */
    std::list<DrawCall> m_drawCalls;
};

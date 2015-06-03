/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Draw list class.
 */

#pragma once

#include "render/geometry.h"
#include "render/scene_entity.h"

#include "shader/pass.h"

#include <list>

/**
 * Draw call structure.
 *
 * This structure is stored in a DrawList. It stores information for a single
 * rendering pass for an entity.
 */
struct DrawCall {
    Geometry geometry;                  /**< Geometry to draw. */
    Material *material;                 /**< Material to draw with. */
    GPUBuffer *uniforms;                /**< Entity uniforms. */
    const Pass *pass;                   /**< Pass to draw with. */
};

/** Class storing a list of draw calls. */
class DrawList {
public:
    void addDrawCall(const Geometry &geometry, Material *material, GPUBuffer *uniforms, const Pass *pass);

    void addDrawCalls(const Geometry &geometry, Material *material, GPUBuffer *uniforms, Pass::Type passType);
    void addDrawCalls(SceneEntity *entity, Pass::Type passType);

    void draw(SceneLight *light = nullptr) const;

    /** @return             Whether the draw list is empty. */
    bool empty() const { return m_drawCalls.empty(); }
private:
    /** List of draw calls. */
    std::list<DrawCall> m_drawCalls;
};

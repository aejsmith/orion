/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Draw list class.
 */

#include "engine/material.h"

#include "render/draw_list.h"

/** Add a draw call to the list.
 * @param geometry      Geometry to draw.
 * @param material      Material to draw with.
 * @param uniforms      Entity uniforms.
 * @param pass          Pass to draw with. */
void DrawList::addDrawCall(const Geometry &geometry, Material *material, GPUBuffer *uniforms, const Pass *pass) {
    m_drawCalls.emplace_back();

    DrawCall &drawCall = m_drawCalls.back();
    drawCall.geometry = geometry;
    drawCall.material = material;
    drawCall.uniforms = uniforms;
    drawCall.pass = pass;
}

/** Add draw calls for all passes to the list.
 * @param geometry      Geometry to draw.
 * @param material      Material to draw with.
 * @param uniforms      Entity uniforms.
 * @param passType      Pass type to use. */
void DrawList::addDrawCalls(const Geometry &geometry, Material *material, GPUBuffer *uniforms, Pass::Type passType) {
    Shader *shader = material->shader();
    for (size_t i = 0; i < shader->numPasses(passType); i++) {
        const Pass *pass = shader->pass(passType, i);
        addDrawCall(geometry, material, uniforms, pass);
    }
}

/** Add draw calls for all passes to the list.
 * @param entity        Entity to draw.
 * @param passType      Pass type to add. */
void DrawList::addDrawCalls(SceneEntity *entity, Pass::Type passType) {
    Geometry geometry;
    entity->geometry(geometry);

    addDrawCalls(geometry, entity->material(), entity->uniforms(), passType);
}

/** Perform all draw calls in the list.
 * @param light         Light being rendered with (can be null). */
void DrawList::draw(SceneLight *light) const {
    /* TODO: Track current pass/material/etc, minimize the state changes. */
    for (const DrawCall &drawCall : m_drawCalls) {
        drawCall.material->shader()->setDrawState(drawCall.material);
        drawCall.pass->setDrawState(light);

        /* Bind the entity uniforms. */
        if (drawCall.uniforms)
            g_gpuManager->bindUniformBuffer(UniformSlots::kEntityUniforms, drawCall.uniforms);

        g_gpuManager->draw(
            drawCall.geometry.primitiveType,
            drawCall.geometry.vertices,
            drawCall.geometry.indices);
    }
}

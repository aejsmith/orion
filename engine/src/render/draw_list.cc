/**
 * @file
 * @copyright           2014 Alex Smith
 * @brief               Draw list class.
 */

#include "engine/material.h"

#include "render/draw_list.h"

/** Add a draw call to the list.
 * @param source        Source draw data.
 * @param pass          Pass to add with.
 * @param uniforms      Entity uniforms for the draw call. */
void DrawList::addDrawCall(const DrawData &source, const Pass *pass, GPUBuffer *uniforms) {
    DrawCall drawCall(source);
    drawCall.pass = pass;
    drawCall.uniforms = uniforms;

    m_drawCalls.push_back(drawCall);
}

/** Add draw calls for all passes to the list.
 * @param source        Source draw data.
 * @param passType      Pass type to add.
 * @param uniforms      Entity uniforms for the draw call. */
void DrawList::addDrawCalls(const DrawData &source, Pass::Type passType, GPUBuffer *uniforms) {
    Shader *shader = source.material->shader();
    for (size_t i = 0; i < shader->numPasses(passType); i++) {
        const Pass *pass = shader->pass(passType, i);
        addDrawCall(source, pass, uniforms);
    }
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
            g_gpu->bindUniformBuffer(UniformSlots::kEntityUniforms, drawCall.uniforms);

        g_gpu->draw(drawCall.primitiveType, drawCall.vertices, drawCall.indices);
    }
}

/*
 * Copyright (C) 2015 Alex Smith
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * @file
 * @brief               Renderer base component.
 */

#include "engine/world.h"

#include "graphics/graphics_system.h"
#include "graphics/renderer.h"

#include "render/render_entity.h"
#include "render/render_world.h"

/** Initialise the component. */
Renderer::Renderer() :
    m_castsShadow(true)
{}

/** Destory the component. */
Renderer::~Renderer() {
    check(m_renderEntities.empty());
}

/** Set whether the rendered object casts a shadow.
 * @param castsShadow   Whether to cast a shadow. */
void Renderer::setCastsShadow(bool castsShadow) {
    if (castsShadow != m_castsShadow) {
        m_castsShadow = castsShadow;

        for (RenderEntity *renderEntity : m_renderEntities) {
            uint32_t flags = renderEntity->flags();

            if (castsShadow) {
                flags |= RenderEntity::kCastsShadow;
            } else {
                flags &= ~RenderEntity::kCastsShadow;
            }

            renderEntity->setFlags(flags);
        }
    }
}

/** Called when the entity's transformation is updated.
 * @param changed       Flags indicating changes made. */
void Renderer::transformed(unsigned changed) {
    /* Update all renderer entity transformations. */
    for (RenderEntity *renderEntity : m_renderEntities)
        renderEntity->setTransform(worldTransform());
}

/** Called when the component becomes active in the world. */
void Renderer::activated() {
    /* Renderer entities should not yet be created. Create them. */
    check(m_renderEntities.empty());
    createRenderEntities(m_renderEntities);
    check(!m_renderEntities.empty());

    auto &system = getSystem<GraphicsSystem>();

    /* Set properties and add them all to the renderer. */
    for (RenderEntity *renderEntity : m_renderEntities) {
        renderEntity->setTransform(worldTransform());
        renderEntity->setFlags((m_castsShadow) ? RenderEntity::kCastsShadow : 0);

        renderEntity->setWorld(&system.renderWorld());
    }
}

/** Called when the component becomes inactive in the world. */
void Renderer::deactivated() {
    while (!m_renderEntities.empty()) {
        RenderEntity *renderEntity = m_renderEntities.back();
        m_renderEntities.pop_back();

        delete renderEntity;
    }
}

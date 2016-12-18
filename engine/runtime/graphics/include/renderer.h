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

#pragma once

#include "engine/component.h"

#include <list>

class RenderEntity;

/**
 * Base class for a component which renders something.
 *
 * This class is the base class for components which render something in the
 * world. It implements the functionality to add RenderEntities to the renderer
 * and keeps them updated.
 */
class Renderer : public Component {
public:
    CLASS();

    VPROPERTY(bool, castsShadow);

    void setCastsShadow(bool castsShadow);

    /** @return             Whether the rendered object casts a shadow. */
    bool castsShadow() const { return m_castsShadow; }
protected:
    /** Type of a renderer entity list. */
    using RenderEntityList = std::list<RenderEntity *>;

    Renderer();
    ~Renderer();

    void transformed(unsigned changed) override;
    void activated() override;
    void deactivated() override;

    /**
     * Create renderer entities.
     *
     * This function is called each time the component is activated in the world
     * to create the RenderEntities which will be added to the renderer. The
     * entities' transformations will be set after this has been called. The
     * entities are all deleted upon deactivation of the component.
     *
     * @param entities      List to populate.
     */
    virtual void createRenderEntities(RenderEntityList &entities) = 0;
private:
    bool m_castsShadow;            /**< Whether the object casts a shadow. */

    /** List of renderer entities. */
    RenderEntityList m_renderEntities;
};

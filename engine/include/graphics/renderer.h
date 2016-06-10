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

class SceneEntity;

/**
 * Base class for a component which renders something.
 *
 * This class is the base class for components which render something in the
 * world. It implements the functionality to add SceneEntities to the renderer
 * and keeps them updated.
 */
class Renderer : public Component {
public:
    CLASS();

    VPROPERTY(bool, castShadow);

    void setCastShadow(bool castShadow);

    /** @return             Whether the rendered object casts a shadow. */
    bool castShadow() const { return m_castShadow; }
protected:
    /** Type of a scene entity list. */
    typedef std::list<SceneEntity *> SceneEntityList;

    explicit Renderer(Entity *entity);
    ~Renderer();

    void transformed(unsigned changed) override;
    void activated() override;
    void deactivated() override;

    /**
     * Create scene entities.
     *
     * This function is called each time the component is activated in the world
     * to create the SceneEntities which will be added to the renderer. The
     * entities' transformations will be set after this has been called. The
     * entities are all deleted upon deactivation of the component.
     *
     * @param entities      List to populate.
     */
    virtual void createSceneEntities(SceneEntityList &entities) = 0;
private:
    bool m_castShadow;             /**< Whether the object casts a shadow. */

    /** List of scene entities. */
    SceneEntityList m_sceneEntities;
};

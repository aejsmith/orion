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
 * @brief               Scene management.
 */

#pragma once

#include "core/core.h"

#include "render/scene_entity.h"

#include <functional>

class SceneLight;
class SceneView;
class World;

/**
 * Renderer's view of the world.
 *
 * The Scene class holds the renderer's view of a world. It only contains the
 * entities which are relevant to the renderer (renderable entities, lights,
 * etc.), and stores them in such a way to allow efficient rendering. The
 * renderer maintains separate views of entities from the world system, which
 * are updated as required by their world counterparts.
 */
class Scene {
public:
    explicit Scene(World *world);
    ~Scene();

    /** @return             World that the scene corresponds to. */
    World *world() const { return m_world; }

    void addEntity(SceneEntity *entity);
    void removeEntity(SceneEntity *entity);
    void queueEntityUpdate(SceneEntity *entity);

    void addLight(SceneLight *light, const glm::vec3 &position);
    void removeLight(SceneLight *light);
    void transformLight(SceneLight *light, const glm::vec3 &position);

    void visitVisibleEntities(SceneView *view, const std::function<void (SceneEntity *)> &func);
    void visitVisibleLights(SceneView *view, const std::function<void (SceneLight *)> &func);
private:
    World *m_world;                 /**< World that the scene corresponds to. */

    /** List of registered entities. */
    std::list<SceneEntity *> m_entities;
    /** List of registered lights. */
    std::list<SceneLight *> m_lights;
};

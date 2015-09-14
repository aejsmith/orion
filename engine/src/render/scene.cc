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

#include "render/scene.h"
#include "render/scene_light.h"

/** Initialize the scene. */
Scene::Scene(World *world) : m_world(world) {}

/** Destroy the scene. */
Scene::~Scene() {}

/** Add an entity to the scene.
 * @param entity        Entity to add. */
void Scene::addEntity(SceneEntity *entity) {
    entity->m_scene = this;
    m_entities.push_back(entity);
}

/** Remove an entity from the scene.
 * @param entity        Entity to remove. */
void Scene::removeEntity(SceneEntity *entity) {
    m_entities.remove(entity);
    entity->m_scene = nullptr;
}

/** Queue an update for an entity in the scene.
 * @param entity        Entity which requires an update. */
void Scene::queueEntityUpdate(SceneEntity *entity) {
    /* When we have proper scene management this will have to move stuff around
     * in the octree or whatever... */
    entity->m_updatePending = false;
}

/** Add an light to the scene.
 * @param light         Light to add.
 * @param transform     Transformation for the light. */
void Scene::addLight(SceneLight *light, const glm::vec3 &position) {
    light->setPosition(position);
    m_lights.push_back(light);
}

/** Remove a light from the scene.
 * @param light         Light to remove. */
void Scene::removeLight(SceneLight *light) {
    m_lights.remove(light);
}

/** Set the transformation of a light in the scene.
 * @param light         Light to transform.
 * @param position      New position. */
void Scene::transformLight(SceneLight *light, const glm::vec3 &position) {
    /* Same as above, proper management of lights. */
    light->setPosition(position);
}

/** Call a function on each entity visible from a view.
 * @param view          View into the scene.
 * @param func          Function to call on visible entities. */
void Scene::visitVisibleEntities(SceneView *view, const std::function<void (SceneEntity *)> &func) {
    for (SceneEntity *entity : m_entities) {
        if (Math::intersect(view->frustum(), entity->worldBoundingBox()))
            func(entity);
    }
}

/** Call a function on each light affecting a view.
 * @param view          View into the scene.
 * @param func          Function to call on visible lights. */
void Scene::visitVisibleLights(SceneView *view, const std::function<void (SceneLight *)> &func) {
    // TODO: Light culling. Directional/ambient lights always affect.
    for (SceneLight *light : m_lights) {
        /* Ignore lights that would have no contribution. */
        if (!light->intensity() || !glm::length(light->colour()))
            continue;

        func(light);
    }
}

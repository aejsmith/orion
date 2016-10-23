/*
 * Copyright (C) 2015-2016 Alex Smith
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
 * @brief               Test game.
 */

#include "cubes_game.h"
#include "player_controller.h"

#include "core/string.h"

#include "engine/asset_manager.h"
#include "engine/behaviour.h"
#include "engine/debug_manager.h"
#include "engine/engine.h"
#include "engine/entity.h"
#include "engine/texture.h"
#include "engine/world.h"

#include "gpu/gpu_manager.h"

#include "graphics/camera.h"
#include "graphics/light.h"
#include "graphics/mesh_renderer.h"
#include "graphics/skybox.h"

#include "input/input_handler.h"
#include "input/input_manager.h"

#include "physics/collision_shape.h"
#include "physics/rigid_body.h"

#include "render/effects/fxaa_effect.h"
#include "render/render_manager.h"
#include "render/utility.h"
#include "render/vertex.h"

/** Construct the game class. */
CubesGame::CubesGame() :
    m_numCubes(0),
    m_numLights(0)
{}

/** Get the engine configuration.
 * @param config        Engine configuration to fill in. */
void CubesGame::engineConfiguration(EngineConfiguration &config) {
    config.title = "Cubes";
    config.displayWidth = 1440;
    config.displayHeight = 900;
    config.displayFullscreen = false;
    config.displayVsync = false;
}

/** Initialize the game world. */
void CubesGame::init() {
    /* Load assets we need to create new cubes. */
    m_cubeMaterial = g_assetManager->load<Material>("game/materials/companion_cube");
    m_cubeMesh = g_assetManager->load<Mesh>("game/models/companion_cube");
    m_cubePhysicsMaterial = g_assetManager->load<PhysicsMaterial>("game/physics_materials/companion_cube");

    /* Load the world. */
    m_world = g_engine->loadWorld("game/worlds/main");

    // TODO: see below.
    std::function<void (Entity *)> getStats =
        [&] (Entity *entity) {
            if (entity->name.compare(0, 5, "cube_") == 0) {
                m_numCubes++;
            } else if (entity->name.compare(0, 6, "light_") == 0) {
                m_numLights++;
            }

            for (Entity *child : entity->children())
                getStats(child);
        };
    getStats(m_world->root());
}

/** Called at the start of the frame. */
void CubesGame::startFrame() {
    // TODO: This is best being handled generically, i.e. some renderer stats.
    g_debugManager->writeText(String::format("Cubes: %u\n", m_numCubes));
    g_debugManager->writeText(String::format("Lights: %u\n", m_numLights));
}

/** Spawn a cube in the world.
 * @param withLights    Whether to attach lights to the cube.
 * @return              Pointer to created cube entity. Entity is not initially
 *                      active. */
Entity *CubesGame::makeCube(bool withLights) {
    unsigned cubeNum = m_numCubes++;

    Entity *entity = m_world->createEntity(String::format("cube_%u", cubeNum));
    entity->setScale(glm::vec3(0.2f, 0.2f, 0.2f));

    MeshRenderer *renderer = entity->createComponent<MeshRenderer>();
    renderer->setMesh(m_cubeMesh);
    renderer->setMaterial("Material.004", m_cubeMaterial);
    renderer->setActive(true);

    BoxCollisionShape *collisionShape = entity->createComponent<BoxCollisionShape>();
    collisionShape->setHalfExtents(glm::vec3(2.9f, 2.9f, 2.9f));
    collisionShape->setActive(true);

    RigidBody *rigidBody = entity->createComponent<RigidBody>();
    rigidBody->setMaterial(m_cubePhysicsMaterial);
    rigidBody->setMass(10.0f);
    rigidBody->setActive(true);

    if (withLights) {
        const struct {
            glm::vec3 direction;
            glm::vec3 colour;
        } lights[4] = {
            { glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(1.0f, 0.0f, 0.0f) },
            { glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f) },
            { glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f) },
            { glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 1.0f) },
        };

        for (unsigned i = 0; i < 4; i++) {
            Entity *child = entity->createChild(String::format("light_%u", i));
            child->setActive(true);
            SpotLight *light = child->createComponent<SpotLight>();
            light->setDirection(lights[i].direction);
            light->setColour(lights[i].colour);
            light->setRange(200.0f);
            light->setAttenuation(glm::vec3(1.0f, 0.1f, 0.0f));
            light->setIntensity(1.5f);
            light->setCutoff(30.0f);
            light->setCastShadows(false);
            light->setActive(true);
        }

        m_numLights += 4;
    }

    return entity;
}

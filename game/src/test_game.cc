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
 * @brief               Test game.
 */

#include "player_controller.h"
#include "test_game.h"

#include "core/string.h"

#include "engine/asset_manager.h"
#include "engine/behaviour.h"
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

/** Create a 2D plane centered at the origin extending in the X/Y direction.
 * @param parent        Parent entity.
 * @param name          Name of the entity.
 * @param material      Material to use for the plane.
 * @param tiles         Texture tiling count. */
static inline Entity *createPlane(
    Entity *parent,
    const std::string &name,
    Material *material,
    float tiles)
{
    /* Vertices of the plane. */
    glm::vec3 vertices[] = {
        glm::vec3(-0.5f, -0.5f, 0.0f),
        glm::vec3(0.5f, -0.5f, 0.0f),
        glm::vec3(0.5f, 0.5f, 0.0f),
        glm::vec3(-0.5f, 0.5f, 0.0f),
    };

    /* We only have a single normal. */
    glm::vec3 normal(0.0f, 0.0f, 1.0f);

    /* Texture coordinates. */
    glm::vec2 texcoords[] = {
        glm::vec2(0.0f, 0.0f), glm::vec2(tiles, 0.0f),
        glm::vec2(tiles, tiles), glm::vec2(0.0f, tiles),
    };

    MeshPtr mesh(new Mesh());
    SubMesh *subMesh = mesh->addSubMesh();
    subMesh->material = mesh->addMaterial("default");
    subMesh->boundingBox.minimum = glm::vec3(-0.5f, -0.5f, 0.0f);
    subMesh->boundingBox.maximum = glm::vec3(0.5f, 0.5f, 0.0f);

    std::vector<SimpleVertex> data;
    data.emplace_back(vertices[0], normal, texcoords[0]);
    data.emplace_back(vertices[1], normal, texcoords[1]);
    data.emplace_back(vertices[2], normal, texcoords[2]);
    data.emplace_back(vertices[2], normal, texcoords[2]);
    data.emplace_back(vertices[3], normal, texcoords[3]);
    data.emplace_back(vertices[0], normal, texcoords[0]);

    GPUBufferArray buffers(1);
    buffers[0] = RenderUtil::buildGPUBuffer(GPUBuffer::kVertexBuffer, data);
    subMesh->vertices = g_gpuManager->createVertexData(
        data.size(),
        g_renderManager->simpleVertexFormat(),
        buffers);

    Entity *entity = parent->createChild(name);
    MeshRenderer *renderer = entity->createComponent<MeshRenderer>(mesh);
    renderer->setMaterial(subMesh->material, material);
    renderer->setCastShadow(false);
    renderer->setActive(true);

    return entity;
}

/** Initialize the game world. */
TestGame::TestGame() :
    m_numCubes(0)
{
    m_cubeMaterial = g_assetManager->load<Material>("game/materials/companion_cube");
    m_cubeMesh = g_assetManager->load<Mesh>("game/models/companion_cube");
    m_cubePhysicsMaterial = g_assetManager->load<PhysicsMaterial>("game/physics_materials/companion_cube");

    m_world = g_engine->createWorld();

    TextureCubePtr skyboxTexture = g_assetManager->load<TextureCube>("game/textures/skybox");
    Skybox *skybox = m_world->root()->createComponent<Skybox>(skyboxTexture);
    skybox->setActive(true);

    AmbientLight *ambientLight = m_world->root()->createComponent<AmbientLight>();
    ambientLight->setIntensity(0.05f);
    ambientLight->setActive(true);

    Entity *floor = createPlane(
        m_world->root(),
        "floor",
        g_assetManager->load<Material>("game/materials/floor"),
        16.0f);
    floor->rotate(-90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
    floor->setScale(glm::vec3(100.0f, 100.0f, 1.0f));
    floor->setActive(true);
    BoxCollisionShape *collisionShape = floor->createComponent<BoxCollisionShape>();
    collisionShape->setHalfExtents(glm::vec3(0.5f, 0.5f, 0.01f));
    collisionShape->setActive(true);
    RigidBody *rigidBody = floor->createComponent<RigidBody>();
    rigidBody->setMass(0.0f);
    rigidBody->setActive(true);

    Entity *cube = makeCube();
    cube->setPosition(glm::vec3(0.0f, 4.0f, -7.0f));
    cube->rotate(45.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    cube->rotate(20.0f, glm::vec3(0.0f, 0.0f, -1.0f));
    cube->setActive(true);

    cube = makeCube();
    cube->setPosition(glm::vec3(0.2f, 7.0f, -7.0f));
    cube->rotate(45.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    cube->rotate(20.0f, glm::vec3(0.0f, 0.0f, 1.0f));
    cube->setActive(true);

    Entity *playerEntity = m_world->createEntity("player");
    playerEntity->setPosition(glm::vec3(0.0f, 1.0f, 0.0f));
    playerEntity->setActive(true);
    Entity *camEntity = playerEntity->createChild("camera");
    camEntity->setPosition(glm::vec3(0.0f, 1.0f, 0.0f));
    camEntity->setActive(true);
    Camera *camera = camEntity->createComponent<Camera>();
    camera->perspective(90.0f, 0.25f, 100.0f);
    camera->setActive(true);
    PlayerController *controller = playerEntity->createComponent<PlayerController>(this, camera);
    controller->setActive(true);

    FXAAEffect *fxaaEffect = new FXAAEffect;
    camera->postEffectChain().addEffect(fxaaEffect);

    Entity *lightEntity = m_world->createEntity("light");
    lightEntity->setPosition(glm::vec3(2.0f, 3.0f, -7.0f));
    lightEntity->setActive(true);
    SpotLight *spotLight = lightEntity->createComponent<SpotLight>();
    spotLight->setDirection(glm::vec3(-0.8f, -1.0f, 0.0f));
    spotLight->setRange(20.0f);
    spotLight->setAttenuation(1.0f, 0.045f, 0.0075f);
    spotLight->setCutoff(45.0f);
    spotLight->setCastShadows(true);
    spotLight->setActive(true);

    lightEntity = m_world->createEntity("light2");
    lightEntity->setPosition(glm::vec3(-2.0f, 3.0f, -3.5f));
    lightEntity->setActive(true);
    PointLight *pointLight = lightEntity->createComponent<PointLight>();
    pointLight->setColour(glm::vec3(0.0f, 0.0f, 1.0f));
    pointLight->setIntensity(1.0f);
    pointLight->setRange(50.0f);
    pointLight->setAttenuation(1.0f, 0.09f, 0.032f);
    pointLight->setCastShadows(true);
    pointLight->setActive(true);

    lightEntity = m_world->createEntity("light3");
    lightEntity->setPosition(glm::vec3(2.0f, 3.0f, -3.5f));
    lightEntity->setActive(true);
    pointLight = lightEntity->createComponent<PointLight>();
    pointLight->setColour(glm::vec3(0.0f, 1.0f, 0.0f));
    pointLight->setIntensity(1.0f);
    pointLight->setRange(50.0f);
    pointLight->setAttenuation(1.0f, 0.09f, 0.032f);
    pointLight->setCastShadows(true);
    pointLight->setActive(true);

    lightEntity = m_world->createEntity("light4");
    lightEntity->setPosition(glm::vec3(0.0f, 3.0f, -9.0f));
    lightEntity->setActive(true);
    pointLight = lightEntity->createComponent<PointLight>();
    pointLight->setColour(glm::vec3(1.0f, 0.0f, 0.0f));
    pointLight->setIntensity(1.0f);
    pointLight->setRange(50.0f);
    pointLight->setAttenuation(1.0f, 0.09f, 0.032f);
    pointLight->setCastShadows(true);
    pointLight->setActive(true);
}

/** Spawn a cube in the world.
 * @param withLights    Whether to attach lights to the cube.
 * @return              Pointer to created cube entity. Entity is not initially
 *                      active. */
Entity *TestGame::makeCube(bool withLights) {
    unsigned cubeNum = m_numCubes++;

    Entity *entity = m_world->createEntity(String::format("cube_%u", cubeNum));
    entity->setScale(glm::vec3(0.2f, 0.2f, 0.2f));

    MeshRenderer *renderer = entity->createComponent<MeshRenderer>(m_cubeMesh);
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
            light->setAttenuation(1.0f, 0.1f, 0.0f);
            light->setIntensity(1.5f);
            light->setCutoff(30.0f);
            light->setCastShadows(false);
            light->setActive(true);
        }
    }

    return entity;
}

/**
 * Game code interface.
 */

/** Get the engine configuration.
 * @param config        Engine configuration to fill in. */
void game::engineConfiguration(EngineConfiguration &config) {
    config.title = "Cubes";
    config.graphicsAPI = EngineConfiguration::kGLGraphicsAPI;
    config.displayWidth = 1440;
    config.displayHeight = 900;
    config.displayFullscreen = false;
    config.displayVsync = false;
}

/** Create the Game instance.
 * @return              Created Game instance. */
Game *game::createGame() {
    return new TestGame;
}

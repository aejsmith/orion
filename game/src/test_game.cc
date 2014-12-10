/**
 * @file
 * @copyright           2014 Alex Smith
 * @brief               Test game.
 */

#include "engine/asset_manager.h"
#include "engine/engine.h"
#include "engine/game.h"
#include "engine/material.h"
#include "engine/mesh.h"
#include "engine/texture.h"

#include "gpu/gpu.h"

#include "render/render_manager.h"
#include "render/utility.h"
#include "render/vertex.h"

#include "world/behaviour.h"
#include "world/camera.h"
#include "world/entity.h"
#include "world/light.h"
#include "world/mesh_renderer.h"
#include "world/world.h"

/**
 * Game code.
 */

class Rotator : public BehaviourComponent {
public:
    Rotator(Entity *entity) : BehaviourComponent(entity) {}

    void tick(float dt) {
        entity()->rotate(dt * 90.0f, glm::vec3(0.0, 1.0, 0.0));
    }
};

/** Game class. */
class TestGame : public Game {
public:
    TestGame();
private:
    Entity *createPlane(Entity *parent, const std::string &name, Material *material, float tiles);
private:
    World *m_world;                 /**< Game world. */

    /** Rendering resources. */
    MaterialPtr m_cubeMaterial;
    MeshPtr m_cubeMesh;
    //Texture2DPtr m_mirrorTexture;
    //MaterialPtr m_mirrorMaterial;
};

/** Create a 2D plane centered at the origin extending in the X/Y direction.
 * @param parent        Parent entity.
 * @param name          Name of the entity.
 * @param material      Material to use for the plane.
 * @param tiles         Texture tiling count. */
Entity *TestGame::createPlane(Entity *parent, const std::string &name, Material *material, float tiles) {
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

    std::vector<SimpleVertex> data;
    data.emplace_back(vertices[0], normal, texcoords[0]);
    data.emplace_back(vertices[1], normal, texcoords[1]);
    data.emplace_back(vertices[2], normal, texcoords[2]);
    data.emplace_back(vertices[2], normal, texcoords[2]);
    data.emplace_back(vertices[3], normal, texcoords[3]);
    data.emplace_back(vertices[0], normal, texcoords[0]);

    GPUBufferArray buffers(1);
    buffers[0] = buildGPUBuffer(GPUBuffer::kVertexBuffer, data);
    subMesh->vertices = g_gpu->createVertexData(
        data.size(),
        g_renderManager->simpleVertexFormat(),
        buffers);

    Entity *entity = parent->createChild(name);
    MeshRenderer *renderer = entity->createComponent<MeshRenderer>(mesh);
    renderer->setMaterial(subMesh->material, material);
    renderer->setActive(true);

    return entity;
}

/** Initialize the game world. */
TestGame::TestGame() {
    m_cubeMaterial = g_assetManager->load<Material>("game/materials/companion_cube");
    m_cubeMesh = g_assetManager->load<Mesh>("game/models/companion_cube");

    m_world = g_engine->createWorld();

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

    Entity *cube = m_world->createEntity("cube");
    cube->setPosition(glm::vec3(0.0f, 1.0f, -6.0f));
    cube->setScale(glm::vec3(0.2f, 0.2f, 0.2f));
    cube->rotate(45.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    cube->setActive(true);
    MeshRenderer *renderer = cube->createComponent<MeshRenderer>(m_cubeMesh);
    renderer->setMaterial("Material.004", m_cubeMaterial);
    renderer->setActive(true);
    Rotator *rotator = cube->createComponent<Rotator>();
    rotator->setActive(true);

    Entity *camEntity = m_world->createEntity("camera");
    camEntity->setPosition(glm::vec3(0.0f, 2.0f, 0.0f));
    camEntity->setActive(true);
    Camera *camera = camEntity->createComponent<Camera>();
    camera->perspective(90.0f, 0.25f, 100.0f);
    camera->setActive(true);

    Entity *lightEntity = m_world->createEntity("light");
    lightEntity->setPosition(glm::vec3(0.0f, 2.0f, -2.0f));
    lightEntity->setActive(true);
    PointLight *pointLight = lightEntity->createComponent<PointLight>();
    pointLight->setActive(true);

    lightEntity = m_world->createEntity("light2");
    lightEntity->setPosition(glm::vec3(-2.0f, 3.0f, -3.5f));
    lightEntity->setActive(true);
    pointLight = lightEntity->createComponent<PointLight>();
    pointLight->setColour(glm::vec3(0.0f, 0.0f, 1.0f));
    pointLight->setIntensity(1.0f);
    pointLight->setRange(50.0f);
    pointLight->setAttenuation(1.0f, 0.09f, 0.032f);
    pointLight->setActive(true);

    lightEntity = m_world->createEntity("light3");
    lightEntity->setPosition(glm::vec3(2.0f, 3.0f, -3.5f));
    lightEntity->setActive(true);
    pointLight = lightEntity->createComponent<PointLight>();
    pointLight->setColour(glm::vec3(0.0f, 1.0f, 0.0f));
    pointLight->setIntensity(1.0f);
    pointLight->setRange(50.0f);
    pointLight->setAttenuation(1.0f, 0.09f, 0.032f);
    pointLight->setActive(true);

    lightEntity = m_world->createEntity("light4");
    lightEntity->setPosition(glm::vec3(0.0f, 3.0f, -8.0f));
    lightEntity->setActive(true);
    pointLight = lightEntity->createComponent<PointLight>();
    pointLight->setColour(glm::vec3(1.0f, 0.0f, 0.0f));
    pointLight->setIntensity(1.0f);
    pointLight->setRange(50.0f);
    pointLight->setAttenuation(1.0f, 0.09f, 0.032f);
    pointLight->setActive(true);

    //m_mirrorTexture = new Texture2D(256, 256, PixelFormat::kR8G8B8A8, 1, GPUTexture::kRenderTarget);

    //m_mirrorMaterial = new Material(g_assetManager->load<Shader>("engine/shaders/lit_specular"));
    //m_mirrorMaterial->setValue("shininess", 32.0f);
    //m_mirrorMaterial->setValue("specularColour", glm::vec3(0.5f, 0.5f, 0.5f));
    //m_mirrorMaterial->setValue("diffuseTexture", TextureBasePtr(m_mirrorTexture));

    //Entity *mirror = createPlane(m_world->root(), "mirror", m_mirrorMaterial, 1.0f);
    //mirror->setPosition(glm::vec3(0.0f, 2.5f, -10.0f));
    //mirror->setScale(glm::vec3(5.0f, 5.0f, 5.0f));
    //mirror->setActive(true);

    //camEntity = mirror->createChild("camera");
    //camEntity->rotate(180.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    //camEntity->setActive(true);
    //camera = camEntity->createComponent<Camera>();
    //camera->perspective(70.0f, 0.1f, 1000.0f);
    //camera->setRenderTarget(m_mirrorTexture->renderTexture());
    //camera->setActive(true);
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

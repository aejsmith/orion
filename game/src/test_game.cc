/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Test game.
 */

#include "player_controller.h"

#include "engine/asset_manager.h"
#include "engine/behaviour.h"
#include "engine/engine.h"
#include "engine/entity.h"
#include "engine/game.h"
#include "engine/mesh.h"
#include "engine/texture.h"
#include "engine/world.h"

#include "gpu/gpu_manager.h"

#include "graphics/camera.h"
#include "graphics/light.h"
#include "graphics/mesh_renderer.h"

#include "input/input_handler.h"
#include "input/input_manager.h"

#include "render/render_manager.h"
#include "render/utility.h"
#include "render/vertex.h"

#include "shader/material.h"

/**
 * Game code.
 */

class CubeBehaviour : public Behaviour, public InputHandler {
public:
    CubeBehaviour(Entity *entity) :
        Behaviour(entity),
        m_rotating(false)
    {}

    void activated() override {
        registerInputHandler();
    }

    void deactivated() override {
        unregisterInputHandler();
    }

    void tick(float dt) override {
        if (g_inputManager->getButtonState(InputCode::kUp)) {
            if (g_inputManager->getModifiers() & InputModifier::kShift) {
                entity()->translate(glm::vec3(0.0f, 0.0f, dt * -1.5f));
            } else {
                entity()->translate(glm::vec3(0.0f, dt * 1.5f, 0.0f));
            }
        }

        if (g_inputManager->getButtonState(InputCode::kDown)) {
            if (g_inputManager->getModifiers() & InputModifier::kShift) {
                entity()->translate(glm::vec3(0.0f, 0.0f, dt * 1.5f));
            } else {
                entity()->translate(glm::vec3(0.0f, dt * -1.5f, 0.0f));
            }
        }

        if (g_inputManager->getButtonState(InputCode::kLeft))
            entity()->translate(glm::vec3(dt * -1.5f, 0.0f, 0.0f));

        if (g_inputManager->getButtonState(InputCode::kRight))
            entity()->translate(glm::vec3(dt * 1.5f, 0.0f, 0.0f));

        if (m_rotating)
            entity()->rotate(dt * 90.0f, glm::vec3(0.0, 1.0, 0.0));
    }
protected:
    bool handleButtonDown(const ButtonEvent &event) override {
        switch (event.code) {
            case InputCode::kR:
                m_rotating = !m_rotating;
                return true;
            default:
                return false;
        }
    }
private:
    bool m_rotating;
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
    cube->setPosition(glm::vec3(0.0f, 0.57f, -7.0f));
    cube->setScale(glm::vec3(0.2f, 0.2f, 0.2f));
    cube->rotate(45.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    cube->setActive(true);
    MeshRenderer *renderer = cube->createComponent<MeshRenderer>(m_cubeMesh);
    renderer->setMaterial("Material.004", m_cubeMaterial);
    renderer->setActive(true);
    CubeBehaviour *behaviour = cube->createComponent<CubeBehaviour>();
    behaviour->setActive(true);

    Entity *playerEntity = m_world->createEntity("player");
    playerEntity->setPosition(glm::vec3(0.0f, 1.0f, 0.0f));
    playerEntity->setActive(true);
    Entity *camEntity = playerEntity->createChild("camera");
    camEntity->setPosition(glm::vec3(0.0f, 1.0f, 0.0f));
    camEntity->setActive(true);
    Camera *camera = camEntity->createComponent<Camera>();
    camera->perspective(90.0f, 0.25f, 100.0f);
    camera->setActive(true);
    PlayerController *controller = playerEntity->createComponent<PlayerController>(camera);
    controller->setActive(true);

    Entity *lightEntity = m_world->createEntity("light");
    lightEntity->setPosition(glm::vec3(2.0f, 3.0f, -7.0f));
    //lightEntity->setPosition(glm::vec3(0.0f, 3.0f, -9.5f));
    lightEntity->setActive(true);
    SpotLight *spotLight = lightEntity->createComponent<SpotLight>();
    spotLight->setDirection(glm::vec3(-0.8f, -1.0f, 0.0f));
    //spotLight->setDirection(glm::vec3(0.0f, -1.0f, 1.0f));
    spotLight->setRange(50.0f);
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

/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Player controller class.
 */

#include "player_controller.h"
#include "test_game.h"

#include "input/input_manager.h"

#include "physics/rigid_body.h"

/** Movement Velocity. */
static const float kMovementVelocity = 5.0f;

/** Cube firing rate (cubes per second). */
static const unsigned kCubeRate = 5;

/** Initial moving cube velocity. */
static const glm::vec3 kInitialCubeVelocity(0.0f, 0.0f, -15.0f);

/** Initialise the player controller.
 * @param entity        Entity that the controller is attached to.
 * @param game          Game class.
 * @param camera        Camera that the controller should move. */
PlayerController::PlayerController(Entity *entity, TestGame *game, Camera *camera) :
    Behaviour(entity),
    m_game(game),
    m_camera(camera),
    m_sinceLastCube(0.0f)
{}

/** Called when the controller is activated. */
void PlayerController::activated() {
    g_inputManager->setMouseCaptured(true);
    registerInputHandler();
}

/** Called when the controller is deactivated. */
void PlayerController::deactivated() {
    unregisterInputHandler();
    g_inputManager->setMouseCaptured(false);
}

/** Called every frame to update the controller.
 * @param dt            Time delta. */
void PlayerController::tick(float dt) {
    if (g_inputManager->getButtonState(InputCode::kW)) {
        entity()->translate(
            m_camera->entity()->worldOrientation() * glm::vec3(0.0f, 0.0f, dt * -kMovementVelocity));
    }

    if (g_inputManager->getButtonState(InputCode::kS)) {
        entity()->translate(
            m_camera->entity()->worldOrientation() * glm::vec3(0.0f, 0.0f, dt * kMovementVelocity));
    }

    if (g_inputManager->getButtonState(InputCode::kA)) {
        entity()->translate(
            m_camera->entity()->worldOrientation() * glm::vec3(dt * -kMovementVelocity, 0.0f, 0.0f));
    }

    if (g_inputManager->getButtonState(InputCode::kD)) {
        entity()->translate(
            m_camera->entity()->worldOrientation() * glm::vec3(dt * kMovementVelocity, 0.0f, 0.0f));
    }

    if (g_inputManager->getButtonState(InputCode::kLeftCtrl)) {
        entity()->translate(
            glm::vec3(0.0f, dt * -kMovementVelocity, 0.0f));
    }

    if (g_inputManager->getButtonState(InputCode::kSpace)) {
        entity()->translate(
            glm::vec3(0.0f, dt * kMovementVelocity, 0.0f));
    }

    if (g_inputManager->getButtonState(InputCode::kMouseRight)) {
        m_sinceLastCube += dt;

        if (m_sinceLastCube >= 1.0f / static_cast<float>(kCubeRate)) {
            m_sinceLastCube -= 1.0f / static_cast<float>(kCubeRate);
            fireCube();
        }
    }
}

/** Handle a button down event.
 * @param event         Input event details.
 * @return              Whether to continue processing the event. */
bool PlayerController::handleButtonDown(const ButtonEvent &event) {
    switch (event.code) {
        case InputCode::kMouseLeft:
            if (event.modifiers & InputModifier::kLeftAlt) {
                placeCube();
            } else {
                fireCube();
            }

            break;
        case InputCode::kMouseRight:
            m_sinceLastCube = 1.0f / static_cast<float>(kCubeRate);
            break;
        default:
            break;
    }

    return true;
}

/** Handle an axis movement event.
 * @param event         Input event details.
 * @return              Whether to continue processing the event. */
bool PlayerController::handleAxis(const AxisEvent &event) {
    switch (event.code) {
        case InputCode::kMouseX:
            entity()->rotate(-event.delta / 4, glm::vec3(0.0f, 1.0f, 0.0f));
            break;
        case InputCode::kMouseY:
            m_camera->entity()->rotate(-event.delta / 4, glm::vec3(1.0f, 0.0f, 0.0f));
            break;
        default:
            break;
    }

    return true;
}

/** Place a stationary cube in the world. */
void PlayerController::placeCube() {
    Entity *cube = m_game->makeCube();

    cube->setPosition(position() + (orientation() * glm::vec3(0.0f, 0.0f, -4.0f)));
    cube->setOrientation(orientation());

    cube->setActive(true);
}

/** Fire a cube. */
void PlayerController::fireCube() {
    Entity *cube = m_game->makeCube();

    glm::quat cubeOrientation = m_camera->entity()->worldOrientation();

    cube->setPosition(position() + (cubeOrientation * glm::vec3(0.0f, 0.0f, -4.0f)));
    cube->setOrientation(cubeOrientation);

    cube->setActive(true);

    RigidBody *rigidBody = cube->findComponent<RigidBody>();
    check(rigidBody);
    rigidBody->setVelocity(cubeOrientation * kInitialCubeVelocity);
}

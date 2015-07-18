/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Player controller class.
 */

#include "player_controller.h"

#include "input/input_manager.h"

/** Movement Velocity. */
static const float kMovementVelocity = 5.0f;

/** Initialise the player controller.
 * @param entity        Entity that the controller is attached to.
 * @param camera        Camera that the controller should move. */
PlayerController::PlayerController(Entity *entity, Camera *camera) :
    Behaviour(entity),
    m_camera(camera)
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
}

/** Handle a button down event.
 * @param event         Input event details.
 * @return              Whether to continue processing the event. */
bool PlayerController::handleButtonDown(const ButtonEvent &event) {
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

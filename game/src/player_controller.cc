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

/** Initialise the player controller. */
PlayerController::PlayerController() :
    m_game(static_cast<TestGame *>(g_engine->game())),
    m_direction(0.0f),
    m_firingCubes(false),
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
    entity()->translate(
        this->camera->entity()->worldOrientation() *
            (dt * kMovementVelocity * glm::vec3(m_direction.x, 0.0f, m_direction.z)));
    entity()->translate(
        dt * kMovementVelocity * glm::vec3(0.0f, m_direction.y, 0.0f));

    if (m_firingCubes) {
        m_sinceLastCube += dt;

        if (m_sinceLastCube >= 1.0f / static_cast<float>(kCubeRate)) {
            m_sinceLastCube -= 1.0f / static_cast<float>(kCubeRate);
            makeCube(g_inputManager->getModifiers());
        }
    }
}

/** Handle a button down event.
 * @param event         Input event details.
 * @return              Whether to continue processing the event. */
bool PlayerController::handleButtonDown(const ButtonEvent &event) {
    switch (event.code) {
        case InputCode::kW:
            m_direction.z -= 1.0f;
            break;
        case InputCode::kS:
            m_direction.z += 1.0f;
            break;
        case InputCode::kA:
            m_direction.x -= 1.0f;
            break;
        case InputCode::kD:
            m_direction.x += 1.0f;
            break;
        case InputCode::kLeftCtrl:
            m_direction.y -= 1.0f;
            break;
        case InputCode::kSpace:
            m_direction.y += 1.0f;
            break;
        case InputCode::kMouseLeft:
            makeCube(event.modifiers);
            break;
        case InputCode::kMouseRight:
            m_firingCubes = true;
            m_sinceLastCube = 1.0f / static_cast<float>(kCubeRate);
            break;
        default:
            break;
    }

    return true;
}

/** Handle a button up event.
 * @param event         Input event details.
 * @return              Whether to continue processing the event. */
bool PlayerController::handleButtonUp(const ButtonEvent &event) {
    switch (event.code) {
        case InputCode::kW:
            m_direction.z += 1.0f;
            break;
        case InputCode::kS:
            m_direction.z -= 1.0f;
            break;
        case InputCode::kA:
            m_direction.x += 1.0f;
            break;
        case InputCode::kD:
            m_direction.x -= 1.0f;
            break;
        case InputCode::kLeftCtrl:
            m_direction.y += 1.0f;
            break;
        case InputCode::kSpace:
            m_direction.y -= 1.0f;
            break;
        case InputCode::kMouseRight:
            m_firingCubes = false;
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
            this->camera->entity()->rotate(-event.delta / 4, glm::vec3(1.0f, 0.0f, 0.0f));
            break;
        default:
            break;
    }

    return true;
}

/** Make a cube in the world.
 * @param modifiers     Current modifier keys. */
void PlayerController::makeCube(uint32_t modifiers) {
    Entity *cube = m_game->makeCube(modifiers & InputModifier::kLeftShift);
    cube->setActive(true);

    if (modifiers & InputModifier::kLeftAlt) {
        cube->setPosition(position() + (orientation() * glm::vec3(0.0f, 0.0f, -4.0f)));
        cube->setOrientation(orientation());
    } else {
        glm::quat cubeOrientation = this->camera->entity()->worldOrientation();

        cube->setPosition(position() + (cubeOrientation * glm::vec3(0.0f, 0.0f, -4.0f)));
        cube->setOrientation(cubeOrientation);

        RigidBody *rigidBody = cube->findComponent<RigidBody>();
        check(rigidBody);
        rigidBody->setVelocity(cubeOrientation * kInitialCubeVelocity);
    }
}

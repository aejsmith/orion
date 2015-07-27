/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Player controller class.
 */

#pragma once

#include "engine/behaviour.h"

#include "graphics/camera.h"

#include "input/input_handler.h"

class TestGame;

/** Behaviour class which takes input and translates it to player movement. */
class PlayerController : public Behaviour, public InputHandler {
public:
    PlayerController(Entity *entity, TestGame *game, Camera *camera);

    void activated() override;
    void deactivated() override;
    void tick(float dt) override;
protected:
    bool handleButtonDown(const ButtonEvent &event) override;
    bool handleAxis(const AxisEvent &event) override;

    void placeCube();
    void fireCube();
private:
    TestGame *m_game;               /**< Game class. */
    Camera *m_camera;               /**< Camera that the component is controlling. */
    float m_sinceLastCube;          /**< Time since last cube. */
};

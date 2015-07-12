/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Player controller class.
 */

#pragma once

#include "engine/behaviour.h"

#include "graphics/camera.h"

#include "input/input_handler.h"

/** Behaviour class which takes input and translates it to player movement. */
class PlayerController : public BehaviourComponent, public InputHandler {
public:
    PlayerController(Entity *entity, Camera *camera);

    void activated() override;
    void deactivated() override;
    void tick(float dt) override;
protected:
    bool handleButtonDown(const ButtonEvent &event) override;
    bool handleAxis(const AxisEvent &event) override;
private:
    Camera *m_camera;               /**< Camera that the component is controlling. */
};

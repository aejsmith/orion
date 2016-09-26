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

#pragma once

#include "engine/behaviour.h"

#include "graphics/camera.h"

#include "input/input_handler.h"

class CubesGame;

/** Behaviour class which takes input and translates it to player movement. */
class PlayerController : public Behaviour, public InputHandler {
public:
    CLASS();

    PlayerController();

    /** Camera that the component is controlling. */
    PROPERTY() ObjectPtr<Camera> camera;

    void activated() override;
    void deactivated() override;
    void tick(float dt) override;
protected:
    bool handleButtonDown(const ButtonEvent &event) override;
    bool handleButtonUp(const ButtonEvent &event) override;
    bool handleAxis(const AxisEvent &event) override;

    void makeCube(uint32_t modifiers);
private:
    CubesGame *m_game;              /**< Game class. */
    glm::vec3 m_direction;          /**< Current movement direction. */
    bool m_firingCubes;             /**< Whether cubes are being fired. */
    float m_sinceLastCube;          /**< Time since last cube. */
};

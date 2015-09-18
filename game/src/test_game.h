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
 * @brief               Game main class.
 *
 * TODO:
 *  - Need some sort of level-wide systems for stuff that's created and
 *    destroyed within a level.
 */

#pragma once

#include "engine/game.h"
#include "engine/mesh.h"

#include "physics/physics_material.h"

#include "shader/material.h"

/** Game class. */
class TestGame : public Game {
public:
    TestGame();

    void startFrame() override;

    Entity *makeCube(bool withLights = false);
private:
    World *m_world;                 /**< Game world. */
    unsigned m_numCubes;            /**< Number of cubes in the world. */
    unsigned m_numLights;           /**< Number of lights in the world. */

    /** Cube resources. */
    MaterialPtr m_cubeMaterial;
    MeshPtr m_cubeMesh;
    PhysicsMaterialPtr m_cubePhysicsMaterial;
};

/**
 * @file
 * @copyright           2015 Alex Smith
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

    Entity *makeCube();
private:
    World *m_world;                 /**< Game world. */
    unsigned m_numCubes;            /**< Number of cubes spawned. */

    /** Cube resources. */
    MaterialPtr m_cubeMaterial;
    MeshPtr m_cubeMesh;
    PhysicsMaterialPtr m_cubePhysicsMaterial;
};

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
 * @brief               Physics world class.
 */

#pragma once

#include "core/core.h"

class btDiscreteDynamicsWorld;

/** Physics state for a world. */
class PhysicsWorld {
public:
    PhysicsWorld();
    ~PhysicsWorld();

    void tick(float dt);

    /** @return             Gravity vector. */
    const glm::vec3 &gravity() const { return m_gravity; }

    void setGravity(const glm::vec3 &gravity);
private:
    glm::vec3 m_gravity;            /**< Gravity vector. */

    /** Bullet world. */
    std::unique_ptr<btDiscreteDynamicsWorld> m_btWorld;

    friend class RigidBody;
};

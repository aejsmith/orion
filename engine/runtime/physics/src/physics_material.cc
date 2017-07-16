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
 * @brief               Physics material class.
 */

#include "physics_priv.h"

#include "physics/physics_material.h"

/**
 * Initialise the physics material to default values.
 *
 * Initialises the physics material to default values of 0.5 for the friction
 * coefficient and 0.6 for the restitution coefficient.
 */
PhysicsMaterial::PhysicsMaterial() :
    m_restitution (0.6f),
    m_friction    (0.5f)
{}

/** Set the restitution (bounciness) coefficient.
 * @param restitution   New restitution coefficient. */
void PhysicsMaterial::setRestitution(float restitution) {
    check(restitution >= 0.0f && restitution <= 1.0f);

    m_restitution = restitution;
}

/** Set the friction coefficient.
 * @param friction      New friction coefficient. */
void PhysicsMaterial::setFriction(float friction) {
    check(friction >= 0.0f);

    m_friction = friction;
}

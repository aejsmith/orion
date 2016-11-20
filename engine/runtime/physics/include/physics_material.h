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

#pragma once

#include "engine/asset.h"

/**
 * Physics material.
 *
 * A physics material is used by the physics simulation to define the physical
 * surface properties. To use a physics material, it should be attached to a
 * RigidBody.
 */
class PhysicsMaterial : public Asset {
public:
    CLASS();

    PhysicsMaterial();

    VPROPERTY(float, restitution);
    VPROPERTY(float, friction);

    /** @return             Restitution (bounciness) coefficient. */
    float restitution() const { return m_restitution; }
    /** @return             Friction coefficient. */
    float friction() const { return m_friction; }

    void setRestitution(float restitution);
    void setFriction(float friction);
protected:
    ~PhysicsMaterial() {}
private:
    float m_restitution;                /**< Restitution (bounciness) coefficient. */
    float m_friction;                   /**< Friction coefficient. */
};

/** Type of a physics material pointer. */
using PhysicsMaterialPtr = TypedAssetPtr<PhysicsMaterial>;

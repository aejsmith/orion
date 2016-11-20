/*
 * Copyright (C) 2015-2016 Alex Smith
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
 * @brief               Component class.
 */

#include "engine/component.h"
#include "engine/entity.h"
#include "engine/serialiser.h"

/** Construct the component. */
Component::Component() :
    m_entity(nullptr),
    m_active(false)
{}

/** Private destructor. To destroy a component use destroy(). */
Component::~Component() {}

/**
 * Destroy the component.
 *
 * Deactives the component and removes it from its parent. Once no more other
 * references remain to the component it will be deleted.
 */
void Component::destroy() {
    setActive(false);

    /* Remove from the parent. */
    m_entity->removeComponent(this);
}

/** Serialise the component.
 * @param serialiser    Serialiser to write to. */
void Component::serialise(Serialiser &serialiser) const {
    /* Serialise a reference to our entity (see deserialise()). */
    serialiser.write("entity", m_entity);

    /* Serialise properties. */
    Object::serialise(serialiser);
}

/** Deserialise the component.
 * @param serialiser    Serialiser to read from. */
void Component::deserialise(Serialiser &serialiser) {
    /* At this point we are not associated with our entity. Similarly to
     * Entity::deserialise(), the first thing we must do *before* we deserialise
     * any properties is to set up this association and ensure the entity is
     * instantiated. We are added to the entity's component list by
     * Entity::deserialise(), which ensures that order of components is
     * maintained. */
    serialiser.read("entity", m_entity);

    /* Deserialise properties. */
    Object::deserialise(serialiser);
}

/**
 * Set whether the component is active.
 *
 * Sets the component's active property. Note that a component is only really
 * active if the entity it is attached to is active in the world.
 *
 * @param active        Whether the component should be active.
 */
void Component::setActive(bool active) {
    bool wasActive = activeInWorld();

    m_active = active;
    if (m_active) {
        if (!wasActive && m_entity->activeInWorld())
            activated();
    } else {
        if (wasActive)
            deactivated();
    }
}

/**
 * Get whether the component is really active.
 *
 * A component is only active when its active property is set to true and the
 * entity it is attached to is active in the world. This is a convenience
 * function to check both of those.
 *
 * @return              Whether the component is really active.
 */
bool Component::activeInWorld() const {
    return (m_active && m_entity->activeInWorld());
}

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
 * @brief               Component class.
 */

#include "engine/component.h"
#include "engine/entity.h"

/** Construct the component.
 * @param type          Component type ID.
 * @param entity        Entity the component belongs to. */
Component::Component(Type type, Entity *entity) :
    m_type(type),
    m_entity(entity),
    m_active(false)
{}

/** Private destructor. To destroy a component use destroy(). */
Component::~Component() {}

/** Destroy the component. */
void Component::destroy() {
    setActive(false);

    /* Remove from the parent. */
    m_entity->removeComponent(this);

    delete this;
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

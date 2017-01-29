/*
 * Copyright (C) 2017 Alex Smith
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
 * @brief               Event listener/notifier classes.
 */

#pragma once

#include "core/core.h"

#include <list>
#include <type_traits>

template <typename ListenerType> class Notifier;

/**
 * Base class for an event-receiving class.
 *
 * This class doesn't define any event handling methods itself. Each event type
 * should derive from this and add its own virtual method(s) for the event.
 *
 * @tparam Derived      The derived type of the class.
 */
template <typename DerivedType>
class Listener {
public:
    Listener() :
        m_notifier(nullptr)
    {}

    ~Listener() {
        if (m_notifier)
            m_notifier->remove(static_cast<DerivedType *>(this));
    }
private:
    /** Notifier that the listener is registered with. */
    Notifier<DerivedType> *m_notifier;

    friend class Notifier<DerivedType>;
};

/**
 * Event notifier class.
 *
 * This is a class with which Listeners can be registered in the event source
 * so that they can receive events.
 *
 * Event handlers will be called on Listeners in the order in which they are
 * registered.
 */
template <typename ListenerType>
class Notifier {
public:
    static_assert(
        std::is_base_of<Listener<ListenerType>, ListenerType>::value,
        "ListenerType must be derived from Listener");

    ~Notifier() {
        /* Update all the listeners still attached so that they no longer refer
         * to this notifier. */
        for (ListenerType *listener : m_listeners)
            listener->m_notifier = nullptr;
    }

    /** Add a listener.
     * @param listener      Listener to add. Must not be attached to any other
     *                      notifier. */
    void add(ListenerType *listener) {
        assert(!listener->m_notifier);

        m_listeners.push_back(listener);
        listener->m_notifier = this;
    }

    /** Remove a listener.
     * @param listener      Listener to remove. */
    void remove(ListenerType *listener) {
        assert(listener->m_notifier == this);

        m_listeners.remove(listener);
        listener->m_notifier = nullptr;
    }

    /** Notify all listeners of an event.
     * @param function      Function to call on all listeners. */
    template <typename Func>
    void notify(Func func) {
        for (ListenerType *listener : m_listeners)
            func(listener);
    }
private:
    /** List of listeners. TODO: Intrusive list. */
    std::list<ListenerType *> m_listeners;
};

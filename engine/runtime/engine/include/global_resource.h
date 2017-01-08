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
 * @brief               Global resource wrapper.
 */

#pragma once

#include "core/core.h"

#include <new>

/** Base class for GlobalResource. */
class GlobalResourceBase {
protected:
    GlobalResourceBase() {}
    ~GlobalResourceBase() {}

    /** Destroy the resource. */
    virtual void destroy() = 0;

    void registerResource();

    static void destroyAll();

    friend class Engine;
};

/**
 * Class managing a global resource.
 *
 * This class can be used to hold a global resource, which will be created when
 * init() is first called, and destroyed in a well-defined order: all
 * GlobalResources will be destroyed in the reverse order from which they were
 * constructed.
 */
template <typename Resource>
class GlobalResource final : public GlobalResourceBase {
public:
    GlobalResource() :
        m_pointer(nullptr)
    {}

    ~GlobalResource() {}

    /**
     * Initialise the resource.
     *
     * Initialises the resource if it is not already initialised. Note that this
     * is not thread-safe.
     */
    template <typename... Args>
    void init(Args &&...args) {
        if (!m_pointer) {
            /* Set the pointer first, which allows the constructor to access
             * the pointer to itself (necessary in a few places, e.g. if a
             * global resource constructor calls some other code that needs to
             * access that resource. */
            m_pointer = reinterpret_cast<Resource *>(m_storage);

            /* Register in global list. Also do first, so any resources that may
             * be created by the constructor are destroyed before this one. */
            registerResource();

            /* Construct. */
            new(m_storage) Resource(std::forward<Args>(args)...);
        }
    }

    /** @return             Reference to resource. */
    Resource &operator *() const { return *m_pointer; }

    /** @return             Pointer to resource. */
    Resource *operator ->() const { return m_pointer; }
protected:
    /** Destroy the resource. */
    void destroy() override {
        assert(m_pointer);
        m_pointer->~Resource();
        m_pointer = nullptr;
    }
private:
    /** Statically-allocated storage space, used with placement new. */
    alignas(Resource) uint8_t m_storage[sizeof(Resource)];

    /**
     * Pointer to the resource.
     *
     * Only valid when the resource is initialised, null when not. Helps catch
     * accesses to uninitialised resources, as such references will get a null
     * pointer.
     */
    Resource *m_pointer;
};

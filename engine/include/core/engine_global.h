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
 * @brief               Engine global holder class.
 */

#pragma once

#include "core/defs.h"

#include <list>

/** Base for an engine global holder. */
class EngineGlobalBase : Noncopyable {
public:
    EngineGlobalBase() : m_initialized(false) {}
    ~EngineGlobalBase();

    static void destroyAll();
protected:
    void init();

    /** Destroy the object. */
    virtual void destroy() = 0;
private:
    bool m_initialized;             /**< Whether the object has been initialized. */

    static std::list<EngineGlobalBase *> m_globals;
};

/** Default pointer type for EngineGlobal. */
template <typename T>
class DefaultEngineGlobalPtr {
public:
    DefaultEngineGlobalPtr() : m_pointer(nullptr) {}
    ~DefaultEngineGlobalPtr() { reset(); }

    T *get() const { return m_pointer; }
    void reset() { delete m_pointer; m_pointer = nullptr; }
    T *operator =(T *pointer) { m_pointer = pointer; return pointer; }
private:
    T *m_pointer;
};

/**
 * Engine global object holder.
 *
 * This class holds an engine global object. Compared to regular global objects,
 * these must be explicitly initialized, and have a defined destruction order:
 * all EngineGlobals are destroyed in the order in which they are initialized.
 *
 * Usage is as follows:
 *
 *   EngineGlobal<FooManager> g_fooManager;
 *   ...
 *   g_fooManager() = new FooManager;
 *   g_fooManager->doSomething();
 *
 * Initialization is performed using operator (), which returns a reference to
 * a PointerType which can be set to the created object. This may seem odd, but
 * it allows the desired destruction order to be implemented: say the object's
 * constructor initializes some other globals. These should be destroyed before
 * the object itself, as they may depend on the object. If we simply provided
 * an assign method which was passed a pointer to the created object, that would
 * register itself after the object's constructor had been run, and therefore
 * would be destroyed before any globals created by the object's constructor.
 *
 * A global can only be initialized once: calling operator () more than once
 * results in an abort.
 *
 * @tparam T            Type of the object to store.
 * @tparam PointerType  Type of a pointer to the object. Must be a smart
 *                      pointer implementing a reset() method which will be
 *                      called to destroy the object. Default implementation
 *                      uses delete to destroy the object.
 */
template <typename T, typename PointerType = DefaultEngineGlobalPtr<T>>
class EngineGlobal : public EngineGlobalBase {
public:
    /** @return         Reference to global object. */
    FORCEINLINE T &operator *() const { return *m_pointer.get(); }
    /** @return         Pointer to global object. */
    FORCEINLINE T *operator ->() const { return m_pointer.get(); }
    /** @return         Whether the global is initialized. */
    FORCEINLINE operator bool() const { return m_pointer.get(); }
    /** @return         Pointer value. */
    FORCEINLINE operator const PointerType &() const { return m_pointer; }
    /** @return         Pointer value. */
    FORCEINLINE operator T *() const { return m_pointer.get(); }

    /**
     * Allow initialization of the global object.
     *
     * Allows initialization of the global object. Registers the global in the
     * list of objects to be deleted, then returns a reference to the object
     * pointer to allow it to be set. Will abort if the object is already
     * initialized.
     *
     * @return          Reference to object pointer.
     */
    PointerType &operator ()() {
        init();
        return m_pointer;
    }
protected:
    /** Destroy the object. */
    void destroy() override {
        m_pointer.reset();
    }
private:
    PointerType m_pointer;          /**< Pointer to the object. */
};

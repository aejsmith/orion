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
 * @brief               Reference counted object base class.
 */

#pragma once

#include "core/core.h"

#include <algorithm>
#include <type_traits>

/**
 * Base class providing reference counting functionality.
 *
 * This class provides reference counting functionality to derived classes. It
 * maintains a reference count which is modified using the retain() and
 * release() methods. When the reference count reaches 0, the released() method
 * is called, this method can be overridden for custom functionality. The
 * default implementation deletes the object.
 *
 * The retain and release methods are marked const to allow for const pointers
 * to a reference counted object.
 *
 * TODO:
 *  - Threading: does this need to be atomic?
 *  - Inline release() for non-debug builds (currently out of line due to
 *    assertion).
 */
class Refcounted {
public:
    Refcounted() : m_refcount(0) {}

    /** Increase the object's reference count.
     * @return              New value of the reference count. */
    int32_t retain() const { return ++m_refcount; }

    int32_t release() const;

    /** @return             Current reference count. */
    int32_t refcount() const { return m_refcount; }
protected:
    virtual ~Refcounted();
private:
    virtual void released();
private:
    mutable int32_t m_refcount;     /**< Object reference count. */
};

/**
 * Reference counting smart pointer.
 *
 * This class implements a smart pointer to a reference counted object which
 * provides retain() and release() methods (need not necessarily derive from
 * Refcounted).
 *
 * This class allows implicit conversions to and from pointers to the referenced
 * type. It is typically safe to take raw pointers to reference counted objects
 * as arguments as long as you expect that the caller should hold a reference.
 * Similarly, it should be safe to return raw pointers to objects as long as
 * you know a reference should be held to it somewhere, e.g. Material::shader()
 * returns a Shader *, because it itself holds a reference to the Shader, and
 * knows that something should have a reference to itself. If the caller intends
 * to store the returned pointer for long term usage, it should assign it to a
 * ReferencePtr.
 *
 * @tparam T            Type of the reference counted object that the pointer
 *                      should point to.
 */
template <typename T>
class ReferencePtr {
public:
    /** Type of the referenced object. */
    using ReferencedType = T;

    /** Create a null pointer. */
    ReferencePtr() : m_object(nullptr) {}

    /** Create a null pointer. */
    ReferencePtr(std::nullptr_t) : m_object(nullptr) {}

    /** Point to an object, increasing its reference count.
     * @param ptr           Object to reference. */
    template <typename U, typename = typename std::enable_if<std::is_convertible<U *, ReferencedType *>::value>::type>
    ReferencePtr(U *ptr) :
        m_object(ptr)
    {
        if (m_object)
            m_object->retain();
    }

    /** Copy another pointer, increasing the reference count.
     * @param other         Pointer to copy. */
    ReferencePtr(const ReferencePtr &other) :
        m_object(other.m_object)
    {
        if (m_object)
            m_object->retain();
    }

    /** Copy another pointer of compatible type, increasing the reference count.
     * @param other         Pointer to copy. */
    template <typename U, typename = typename std::enable_if<std::is_convertible<U *, ReferencedType *>::value>::type>
    ReferencePtr(const ReferencePtr<U> &other) :
        m_object(other.get())
    {
        if (m_object)
            m_object->retain();
    }

    /** Move another pointer to this one.
     * @param other         Pointer to move, becomes invalid. */
    ReferencePtr(ReferencePtr &&other) :
        m_object(other.m_object)
    {
        other.m_object = nullptr;
    }

    /** Destroy the pointer, releasing the object. */
    ~ReferencePtr() { reset(); }

    /** Assign from a basic pointer, increasing the reference count.
     * @param ptr           Pointer to assign. */
    ReferencePtr &operator =(ReferencedType *ptr) {
        reset(ptr);
        return *this;
    }

    /** Copy another pointer, increasing the reference count.
     * @param other         Pointer to copy. */
    ReferencePtr &operator =(const ReferencePtr &other) {
        reset(other.m_object);
        return *this;
    }

    /** Copy another pointer of compatible type, increasing the reference count.
     * @param other         Pointer to copy. */
    template <typename U>
    typename std::enable_if<std::is_convertible<U *, ReferencedType *>::value, ReferencePtr &>::type
    operator =(const ReferencePtr<U> &other) {
        reset(other.get());
        return *this;
    }

    /** Move another pointer to this one.
     * @param other         Pointer to move, becomes invalid. */
    ReferencePtr &operator =(ReferencePtr &&other) {
        if (m_object)
            m_object->release();

        m_object = other.m_object;
        other.m_object = nullptr;
        return *this;
    }

    /** @return             Whether the pointer is valid. */
    explicit operator bool() const { return m_object != nullptr; }

    /** @return             Raw pointer value. */
    operator ReferencedType *() const { return m_object; }

    /** @return             Reference to object. */
    ReferencedType &operator *() const { return *m_object; }

    /** @return             Pointer to object. */
    ReferencedType *operator ->() const { return m_object; }

    /** @return             Value of the pointer. */
    ReferencedType *get() const { return m_object; }

    /** Release the current pointer and replace it.
     * @param ptr           New pointer, defaults to null. */
    void reset(ReferencedType *ptr = nullptr) {
        if (ptr)
            ptr->retain();

        std::swap(m_object, ptr);

        if (ptr)
            ptr->release();
    }

    /** Swap the pointer with another pointer.
     * @param other         Pointer to swap with. */
    void swap(ReferencePtr &other) {
        std::swap(m_object, other.m_object);
    }

    /** Static cast for ReferencePtr.
     * @tparam T            Type to cast to.
     * @param ptr           Pointer to cast.
     * @return              Casted pointer. */
    template <typename U>
    ReferencePtr<U> staticCast() const {
        return ReferencePtr<U>(static_cast<U *>(m_object));
    }

    /** Dynamic cast for ReferencePtr.
     * @tparam T            Type to cast to.
     * @param ptr           Pointer to cast.
     * @return              Casted pointer. */
    template <typename U>
    ReferencePtr<U> dynamicCast() const {
        return ReferencePtr<U>(dynamic_cast<U *>(m_object));
    }
private:
    ReferencedType *m_object;       /**< Referenced object. */
};

/** Compare ReferencePtrs for equality. */
template <typename T, typename U>
inline bool operator ==(const ReferencePtr<T> &a, const ReferencePtr<U> &b) {
    return a.get() == b.get();
}

/** Compare ReferencePtrs for inequality. */
template <typename T, typename U>
inline bool operator !=(const ReferencePtr<T> &a, const ReferencePtr<U> &b) {
    return !(a == b);
}

/** Check order of ReferencePtrs. */
template <typename T, typename U>
inline bool operator <(const ReferencePtr<T> &a, const ReferencePtr<U> &b) {
    return a.get() < b.get();
}

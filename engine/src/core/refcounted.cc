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

#include "core/refcounted.h"

/**
 * Destroy a reference counted object.
 *
 * Destroys the reference counted object. The reference count must currently be
 * 0, this is checked.
 */
Refcounted::~Refcounted() {
    check(m_refcount == 0);
}

/**
 * Decrease the object reference count.
 *
 * Decreases the object's reference count. If the reference count reaches 0,
 * the released() method will be called. The reference count must not currently
 * be 0.
 *
 * @return              New value of the reference count.
 */
int32_t Refcounted::release() const {
    check(m_refcount > 0);

    int32_t ret = --m_refcount;
    if (ret == 0)
        const_cast<Refcounted *>(this)->released();

    return ret;
}

/** Called when the object is released. */
void Refcounted::released() {
    delete this;
}

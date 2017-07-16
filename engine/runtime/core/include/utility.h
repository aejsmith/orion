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
 * @brief               Utility functions/definitions.
 */

#pragma once

#include "core/defs.h"

/** Get the size of an array.
 * @param array         Array to get size of. */
template <typename T, size_t N>
constexpr size_t arraySize(T (&array)[N]) {
    return N;
}

/** Class to call a function when it is destroyed.
 * @see                 makeScopeGuard(). */
template <typename Function>
class ScopeGuard {
public:
    ScopeGuard(Function f) :
        m_function (std::move(f)),
        m_active   (true)
    {}

    ~ScopeGuard() {
        if (m_active)
            m_function();
    }

    ScopeGuard(ScopeGuard &&other) :
        m_function (std::move(other.m_function)),
        m_active   (other.m_active)
    {
        other.cancel();
    }

    /** Cancel the guard (don't call the function). */
    void cancel() {
        m_active = false;
    }
private:
    Function m_function;
    bool m_active;

    ScopeGuard() = delete;
    ScopeGuard(const ScopeGuard &) = delete;
    ScopeGuard &operator =(const ScopeGuard &) = delete;
};

/**
 * Helper to call a function at the end of a scope.
 *
 * The returned object will call the specified function when it is destroyed,
 * unless cancelled by calling cancel().
 *
 * Usage: auto guard = makeScopeGuard([] { action(); });
 * Calls action when guard goes out of scope.
 *
 * @param function      Function to call.
 */
template <typename Function>
ScopeGuard<Function> makeScopeGuard(Function function) {
    return ScopeGuard<Function>(std::move(function));
}

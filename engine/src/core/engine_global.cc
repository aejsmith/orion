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

#include "core/engine_global.h"

/** List of initialized globals in destruction order. */
std::list<EngineGlobalBase *> EngineGlobalBase::m_globals;

/** Destructor, verifies the object has been destroyed. */
EngineGlobalBase::~EngineGlobalBase() {
    check(!m_initialized);
}

/** Destroy all global objects in correct order. */
void EngineGlobalBase::destroyAll() {
    /* Destruction order is last first, iterate in reverse. */
    while (!m_globals.empty()) {
        EngineGlobalBase *global = m_globals.back();
        global->destroy();
        global->m_initialized = false;
        m_globals.pop_back();
    }
}

/** Register a global in the list. */
void EngineGlobalBase::init() {
    /* Check we haven't already been initialized. FIXME: thread safety. */
    check(!m_initialized);
    m_initialized = true;

    m_globals.push_back(this);
}

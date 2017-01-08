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

#include "engine/global_resource.h"

#include <list>

/** @return             List of all resources. */
static auto &globalResources() {
    static std::list<GlobalResourceBase *> list;
    return list;
}

/** Register a resource. */
void GlobalResourceBase::registerResource() {
    globalResources().push_back(this);
}

/** Destroy all global resources. */
void GlobalResourceBase::destroyAll() {
    auto &list = globalResources();

    while (!list.empty()) {
        GlobalResourceBase *resource = list.back();
        list.pop_back();
        resource->destroy();
    }
}

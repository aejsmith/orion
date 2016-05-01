/*
 * Copyright (C) 2016 Alex Smith
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
 * @brief               Meta-object system.
 */

#include "core/object.h"

#include <map>

/** @return             Map of globally declared meta-classes. */
static auto &metaClassMap() {
    static std::map<std::string, const MetaClass *> map;
    return map;
}

/** Initialise a meta-class.
 * @param name          Name of the meta-class.
 * @param parent        Parent meta-class. */
MetaClass::MetaClass(const char *name, const MetaClass *parent) :
    m_name(name),
    m_parent(parent)
{
    auto ret = metaClassMap().insert(std::make_pair(m_name, this));
    checkMsg(ret.second, "Registering meta-class '%s' that already exists", m_name);
}

/** Destroy a metaclass. */
MetaClass::~MetaClass() {
    metaClassMap().erase(m_name);
}

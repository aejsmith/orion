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
 *
 * Notes:
 *  - Currently we do not globally track registered names for all MetaTypes
 *    like we do for MetaClasses. This is for two reasons: firstly, because
 *    MetaTypes are registered dynamically a given type may not be registered
 *    at the time it is looked up, and secondly because I can't think of a need
 *    to be able to look up a non-Object type by name.
 */

#include "core/object.h"

#include <map>

/** Initialise a meta-type.
 * @param name          Name of the meta-type. */
MetaType::MetaType(const char *name, uint32_t traits) :
    m_name(name),
    m_traits(traits)
{}

/** Allocate a new meta-type.
 * @param signature     Signature for lookup function to extract type name from.
 * @return              Pointer to allocated meta-type. */
const MetaType *MetaType::allocate(const char *signature) {
    /*
     * Derive the type name from the function signature (see LookupImpl).
     * Currently works for GCC/clang only, object.h will error if the compiler
     * is not recognised to remind that support needs to be added here.
     */
    std::string name(signature);
    size_t start = name.rfind("LookupT = ") + 10;
    size_t end = name.rfind(", LookupEnable") - start;
    name = name.substr(start, end);

    return new MetaType(strdup(name.c_str()), 0);
}

/** @return             Map of globally declared meta-classes. */
static auto &metaClassMap() {
    /* Only used from global constructors, no synchronisation is needed. */
    static std::map<std::string, const MetaClass *> map;
    return map;
}

/** Initialise a meta-class.
 * @param name          Name of the meta-class.
 * @param parent        Parent meta-class. */
MetaClass::MetaClass(const char *name, const MetaClass *parent) :
    MetaType(name, MetaType::kIsObject),
    m_parent(parent)
{
    auto ret = metaClassMap().insert(std::make_pair(name, this));
    checkMsg(ret.second, "Registering meta-class '%s' that already exists", name);
}

/** Destroy a meta-class. */
MetaClass::~MetaClass() {
    metaClassMap().erase(name());
}

/** Determine if this class is the base or the same as another.
 * @param other         Class to check.
 * @return              Whether this class is the base of the specified class. */
bool MetaClass::isBaseOf(const MetaClass &other) const {
    const MetaClass *current = &other;

    while (current) {
        if (current == this)
            return true;

        current = current->m_parent;
    }

    return false;
}

/** Look up a meta-class by name.
 * @param name          Name of the meta-class.
 * @return              Pointer to meta-class if found, or null if not. */
const MetaClass *MetaClass::lookup(const std::string &name) {
    auto &map = metaClassMap();
    auto ret = map.find(name);
    return (ret != map.end()) ? ret->second : nullptr;
}

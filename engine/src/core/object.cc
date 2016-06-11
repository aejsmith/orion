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
MetaClass::MetaClass(
    const char *name,
    const MetaClass *parent,
    const PropertyArray &properties)
    :
    MetaType(name, MetaType::kIsObject),
    m_parent(parent),
    m_properties(properties)
{
    auto ret = metaClassMap().insert(std::make_pair(name, this));
    checkMsg(ret.second, "Registering meta-class '%s' that already exists", name);

    /* Add properties to a map for fast lookup. */
    for (const auto &property : properties) {
        auto ret = m_propertyMap.insert(std::make_pair(property.name(), &property));
        checkMsg(ret.second, "Meta-class '%s' has duplicate property '%s'", name, property.name());
    }
}

/** Destroy a meta-class. */
MetaClass::~MetaClass() {
    metaClassMap().erase(name());
}

/** Determine if this class is the base of or the same as another.
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

/** Look up a property by name on the class.
 * @param name          Name of the property to look up.
 * @return              Pointer to property if found, null if not. */
const MetaProperty *MetaClass::lookupProperty(const char *name) const {
    const MetaClass *current = this;

    while (current) {
        auto ret = current->m_propertyMap.find(name);
        if (ret != current->m_propertyMap.end())
            return ret->second;

        current = current->m_parent;
    }

    return nullptr;
}

/** Look up a meta-class by name.
 * @param name          Name of the meta-class.
 * @return              Pointer to meta-class if found, null if not. */
const MetaClass *MetaClass::lookup(const std::string &name) {
    auto &map = metaClassMap();
    auto ret = map.find(name);
    return (ret != map.end()) ? ret->second : nullptr;
}

/**
 * Visit all known meta-classes and execute a function for each.
 *
 * For every known meta-class, executes the specified function on it. This can
 * be used, for example, to build up a list of meta-classes fulfilling certain
 * criteria.
 *
 * @param function      Function to execute.
 */
void MetaClass::visit(const std::function<void (const MetaClass &)> &function) {
    for (auto it : metaClassMap())
        function(*it.second);
}

/** Initialise a property.
 * @param name          Name of the property.
 * @param type          Type of the property.
 * @param getFunction   Function to get the property value.
 * @param setFunction   Function to set the property value. */
MetaProperty::MetaProperty(
    const char *name,
    const MetaType &type,
    GetFunction getFunction,
    SetFunction setFunction)
    :
    m_name(name),
    m_type(type),
    m_getFunction(getFunction),
    m_setFunction(setFunction)
{}

static const MetaProperty *lookupAndCheckProperty(
    const MetaClass &metaClass,
    const char *name,
    const MetaType &type)
{
    const MetaProperty *property = metaClass.lookupProperty(name);
    if (!property) {
        logError("Attempt to access non-existant property '%s' on class '%s'", name, metaClass.name());
        return nullptr;
    }

    if (&type != &property->type()) {
        logError(
            "Type mismatch accessing property '%s' on class '%s', requested '%s', actual '%s'",
            name, metaClass.name(), type.name(), property->type().name());
        return nullptr;
    }

    return property;
}

/** Get a property value.
 * @param name          Name of the property to get.
 * @param type          Type of the property to get.
 * @param value         Where to store property value.
 * @return              Whether the property could be found. */
bool Object::getProperty(const char *name, const MetaType &type, void *value) const {
    const MetaProperty *property = lookupAndCheckProperty(metaClass(), name, type);
    if (!property)
        return false;

    property->get(this, value);
    return true;
}

/** Set a property value.
 * @param name          Name of the property to set.
 * @param type          Type of the property to set.
 * @param value         Buffer containing new property value.
 * @return              Whether the property could be found. */
bool Object::setProperty(const char *name, const MetaType &type, const void *value) {
    const MetaProperty *property = lookupAndCheckProperty(metaClass(), name, type);
    if (!property)
        return false;

    property->set(this, value);
    return true;
}

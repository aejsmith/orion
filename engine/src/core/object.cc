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
 *
 * TODO:
 *  - Can we enforce at compile time that properties must be a supported type,
 *    to ensure we don't run into issues with serialisation (SerialisationBuffer
 *    for example needs to handle the type properly). Perhaps inject something
 *    into the objgen-generated code, similar to HasSerialise, to check it.
 *  - A Variant class could be used instead of SerialisationBuffer?
 */

#include "core/object.h"
#include "core/serialiser.h"

#include <map>
#include <new>

/** Initialise a meta-type.
 * @param name          Name of the meta-type.
 * @param size          Size of an instance of the type.
 * @param traits        Traits for the type.
 * @param parent        Parent type (for pointers and Object-derived classes). */
MetaType::MetaType(const char *name, size_t size, uint32_t traits, const MetaType *parent) :
    m_name(name),
    m_size(size),
    m_traits(traits),
    m_parent(parent),
    m_enumConstants(nullptr)
{}

/** Allocate a new meta-type.
 * @param signature     Signature for lookup function to extract type name from.
 * @param size          Size of an instance of the type.
 * @param traits        Traits for the type.
 * @param parent        Parent type (for pointers).
 * @return              Pointer to allocated meta-type. */
const MetaType *MetaType::allocate(const char *signature, size_t size, uint32_t traits, const MetaType *parent) {
    /* Derive the type name from the function signature (see LookupImpl).
     * Currently works for GCC/clang only, object.h will error if the compiler
     * is not recognised to remind that support needs to be added here. */
    std::string name(signature);
    size_t start = name.rfind("LookupT = ") + 10;
    size_t end = name.rfind(", LookupEnable") - start;
    name = name.substr(start, end);

    return new MetaType(strdup(name.c_str()), size, traits, parent);
}

/** @return             Map of globally declared meta-classes. */
static auto &metaClassMap() {
    /* Only used from global constructors, no synchronisation is needed. */
    static std::map<std::string, const MetaClass *> map;
    return map;
}

/** Initialise a meta-class.
 * @param name          Name of the meta-class.
 * @param size          Size of an instance of the class.
 * @param traits        Traits of the class.
 * @param parent        Parent meta-class.
 * @param constructor   Constructor function (if constructable).
 * @param properties    Array of properties. */
MetaClass::MetaClass(
    const char *name,
    size_t size,
    uint32_t traits,
    const MetaClass *parent,
    ConstructorFunction constructor,
    const PropertyArray &properties)
    :
    MetaType(name, size, traits | MetaType::kIsObject, parent),
    m_constructor(std::move(constructor)),
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

        current = current->parent();
    }

    return false;
}

/**
 * Construct an object of this class.
 *
 * Constructs an object of this class using its default constructor. The class
 * must be publically constructable, as indicated by isConstructable().
 *
 * @return              Pointer to constructed object.
 */
Object *MetaClass::construct() const {
    checkMsg(
        m_traits & kIsPublicConstructable,
        "Attempt to construct object of class '%s' which is not publically constructable",
        m_name);
    return m_constructor();
}

/**
 * Construct an object of this class.
 *
 * Constructs an object of this class using its default constructor. This
 * version allows construction even if the constructor is not public. The
 * primary use for this is deserialisation.
 *
 * @return              Pointer to constructed object.
 */
Object *MetaClass::constructPrivate() const {
    checkMsg(
        m_traits & kIsConstructable,
        "Attempt to construct object of class '%s' which is not constructable",
        m_name);
    return m_constructor();
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

        current = current->parent();
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

/** Look up a property and check that it is the given type.
 * @param metaClass     Class of the object.
 * @param name          Name of the property to look up.
 * @param type          Requested type.
 * @return              Pointer to property if found and correct type, null
 *                      otherwise. */
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

/**
 * Class providing a temporary buffer for (de)serialisation.
 *
 * We need some temporary storage when (de)serialising properties. The trouble
 * is that for non-POD types we must ensure that the constructor/destructor is
 * called on the buffer, as property get functions and Serialiser::read()
 * assume that the buffer is constructed. This class allocates a buffer and
 * calls the constructor/destructor as necessary. We only need to handle types
 * that are supported as properties here.
 */
struct SerialisationBuffer {
    const MetaType *type;
    uint8_t *data;

    SerialisationBuffer(const MetaType &inType) :
        type(&inType),
        data(new uint8_t[type->size()])
    {
        if (this->type == &MetaType::lookup<std::string>()) {
            new (data) std::string();
        } else if (this->type->isPointer() && this->type->isRefcounted()) {
            new (data) ReferencePtr<Refcounted>();
        }
    }

    ~SerialisationBuffer() {
        if (this->type == &MetaType::lookup<std::string>()) {
            /* Workaround clang bug. The standard allows:
             *   reinterpret_cast<std::string *>(data)->~string();
             * but clang does not. */
            using namespace std;
            reinterpret_cast<string *>(data)->~string();
        } else if (this->type->isPointer() && this->type->isRefcounted()) {
            reinterpret_cast<ReferencePtr<Refcounted> *>(data)->~ReferencePtr();
        }
    }
};

/**
 * Serialise the object.
 *
 * Serialises the object. The default implementation of this method will
 * automatically serialise all of the object's properties. Additional data can
 * be serialised by overridding this method to serialise it, as well as
 * deserialise() to restore it. Overridden implementations *must* call their
 * parent class' implementation.
 *
 * @param serialiser    Serialiser to serialise to.
 */
void Object::serialise(Serialiser &serialiser) const {
    /* Serialise properties into a separate group. */
    serialiser.beginGroup("objectProperties");

    /* We should serialise base class properties first. It may be that, for
     * example, the set method of a derived class property depends on the value
     * of a base class property. */
    std::function<void (const MetaClass *)> serialiseProperties =
        [&] (const MetaClass *metaClass) {
            if (metaClass->parent())
                serialiseProperties(metaClass->parent());

            for (const MetaProperty &property : metaClass->properties()) {
                SerialisationBuffer buf(property.type());
                property.get(this, buf.data);
                serialiser.write(property.name(), property.type(), buf.data);
            }
        };
    serialiseProperties(&metaClass());

    serialiser.endGroup();
}

/**
 * Deserialise the object.
 *
 * Deserialises the object. For a class to be deserialisable, it must be
 * constructable (does not need to be publically), i.e. it must have a zero
 * argument constructor. When an object is being created from a serialised
 * data file, an instance of the class is first constructed using the zero-
 * argument constructor. It is the responsibility of this constructor to
 * initialise default values of all properties. Then, this method is called to
 * restore serialised data. The default implementation of this method will
 * automatically deserialise all of the object's properties. Additional data
 * that was serialised by serialise() can be restored by overriding this method.
 * Overridden implementations *must* call their parent class' implementation.
 *
 * @param serialiser    Serialiser to deserialise from.
 */
void Object::deserialise(Serialiser &serialiser) {
    if (serialiser.beginGroup("objectProperties")) {
        std::function<void (const MetaClass *)> deserialiseProperties =
            [&] (const MetaClass *metaClass) {
                if (metaClass->parent())
                    deserialiseProperties(metaClass->parent());

                for (const MetaProperty &property : metaClass->properties()) {
                    SerialisationBuffer buf(property.type());
                    if (serialiser.read(property.name(), property.type(), buf.data))
                        property.set(this, buf.data);
                }
            };
        deserialiseProperties(&metaClass());

        serialiser.endGroup();
    }
}

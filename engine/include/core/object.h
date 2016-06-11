/*
 * Copyright (C) 2015-2016 Alex Smith
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
 * @brief               Object system.
 */

#pragma once

#include "core/hash_table.h"
#include "core/refcounted.h"

#include <type_traits>
#include <vector>

/**
 * Annotation macros.
 */

/** Macro to define an annotation attribute for objgen. */
#ifdef ORION_OBJGEN
    #define META_ATTRIBUTE(type, ...) \
        __attribute__((annotate("orion:" type ":" #__VA_ARGS__)))
#else
    #define META_ATTRIBUTE(type, ...)
#endif

/** Helper for CLASS() and base Object definition. */
#define DECLARE_STATIC_METACLASS(...) \
    static const META_ATTRIBUTE("class", __VA_ARGS__) MetaClass staticMetaClass

/**
 * Metadata annotation for Object-derived classes.
 *
 * This macro must be placed on every class which derives from Object (directly
 * or indirectly) in order to add and generate definitions for type metadata.
 * It is expected to be placed in the public section of the class, and does not
 * alter visibility in any way.
 *
 * Any header or source file using this macro must have code generated for it
 * using objgen. Source files should include the objgen output manually after
 * the class definition.
 *
 * The parameters to this macro can specify attributes influencing the class
 * behaviour, with the syntax "attribute1": "string", "attribute2": true, ...
 * The following attributes are supported:
 *
 *  - constructable: Set to false to disallow construction of this class through
 *    the object system. This also disables serialization for the class.
 *
 * Example:
 *
 *   class Foobar : public Object {
 *   public:
 *       CLASS();
 *       ...
 *   };
 *
 * @param ...           Attributes of the class, parsed by objgen.
 */
#define CLASS(...) \
    DECLARE_STATIC_METACLASS(__VA_ARGS__); \
    virtual const MetaClass &metaClass() const override;

/**
 * Annotation for object properties.
 *
 * This macro marks a class member as a property. This means it can be accessed
 * through the dynamic property interface in Object, and information about it
 * can be retrieved at runtime.
 *
 * Public members can be marked as a property with no additional support
 * required. Private members require a getter and setter method to be specified,
 * this is typically used when the property requires validation on the value set
 * for it, or when something must be done when the property value changes.
 *
 * Example:
 *
 *   class Foo {
 *   public:
 *       CLASS();
 *
 *       PROPERTY() int wop;
 *       ...
 *   private:
 *       PROPERTY("get": "foop", "set": "setFoop")
 *       glm::vec3 m_foop;
 *   };
 *
 * @param ...           Attributes of the property, parsed by objgen.
 */
#define PROPERTY(...) \
    META_ATTRIBUTE("property", __VA_ARGS__)

/**
 * Macro to declare a virtual object property.
 *
 * This macro can be used to declare a "virtual property" on a class. Such
 * properties are not directly associated with a member variable, rather getter
 * and setter methods to modify them. This is useful, for instance, to expose a
 * property as a certain type for editing while using a different internal
 * implementation.
 *
 * The getter and settter methods to use can be specified with the "get" and
 * "set" attributes. If these are not specified, then defaults are used - it is
 * assumed that the getter function has the same name as the property, and the
 * setter is named set<PropertyName> (camel case). For instance, for a property
 * named "position", the default getter method is "position" and the setter is
 * "setPosition".
 *
 * This functionality is implemented by declaring a static member variable in
 * the class named "vprop_<name>", which is picked up by objgen. These variables
 * should never be used (using them should result in link errors because they
 * have no definition). The variable is still declared when compiling outside
 * of objgen so that errors (e.g. bad types, name conflicts) are still reported
 * by the main compiler.
 *
 * Example:
 *
 *   class Foo {
 *   public:
 *       VPROPERTY(glm::vec3, position);
 *
 *       const glm::vec3 &position() const;
 *       void setPosition(const glm::vec3 &position);
 *       ...
 *   };
 *
 * @param type          Type of the property.
 * @param name          Name of the property.
 * @param ...           Attributes of the property, parsed by objgen. At least
 *                      a "get" and "set" attribute must be specified.
 */
#define VPROPERTY(type, name, ...) \
    static META_ATTRIBUTE("property", __VA_ARGS__) type vprop_##name

/**
 * Metadata classes.
 */

class Object;

/**
 * Base type metadata class.
 *
 * This provides basic information about a type (currently only a name). For
 * types outside of the object system, it just provides a means of doing type
 * comparisons on such types inside the object system, for dynamic property
 * accesses and method invocations. Metadata is generated dynamically the first
 * time it is required. For Object-derived types, this class forms the base of
 * MetaClass, and for these metadata is generated at build time.
 */
class MetaType {
public:
    /** Type trait flags. */
    enum : uint32_t {
        /** Is an Object-derived class. */
        kIsObject = (1 << 0),
        /** Type is constructable through the Object system. */
        kIsConstructable = (1 << 1),
        /** Type is publically constructable. */
        kIsPublicConstructable = (1 << 2),
    };

    /** @return             Name of the type. */
    const char *name() const { return m_name; }

    /** @return             Whether the type is an Object-derived class. */
    bool isObject() const { return m_traits & kIsObject; }

    /** Look up the meta-type for a given type.
     * @tparam T            Type to get for.
     * @return              Meta-type for that type. */
    template <typename T>
    static inline const MetaType &lookup() {
        return LookupImpl<T>::get();
    }
protected:
    MetaType(const char *name, uint32_t traits);

    static const MetaType *allocate(const char *signature);

    /** Helper to get the MetaType for a type. */
    template <typename LookupT, typename LookupEnable = void>
    struct LookupImpl {
        /*
         * The type name is determined using a compiler-provided macro to get
         * the name of the get() function, from which we can extract the
         * template parameter type. This is thoroughly evil, I love it!
         * Remember to modify allocate() when adding new compiler support.
         */
        #ifdef __GNUC__
            #define LOOKUP_FUNCTION_SIGNATURE __PRETTY_FUNCTION__
        #else
            #error "Unsupported compiler"
        #endif

        static NOINLINE const MetaType &get() {
            /*
             * In the generic case, we dynamically allocate a MetaType the first
             * time it is requested for that type. Store a pointer to the
             * MetaType as a static local variable. The allocation will take
             * place the first time this function is called for a given type,
             * and all calls after that will return the same. Although a copy
             * of this function will be generated for every translation unit
             * it is used in, they will all be merged into one by the linker.
             * We force this to be not inline so the compiler doesn't duplicate
             * guard variable stuff all over the place.
             */
            static const MetaType *type = allocate(LOOKUP_FUNCTION_SIGNATURE);
            return *type;
        }

        #undef LOOKUP_FUNCTION_SIGNATURE
    };

    /** Specialization for Object-derived classes to use the static MetaClass. */
    template <typename T>
    struct LookupImpl<T, typename std::enable_if<std::is_base_of<Object, T>::value>::type> {
        static FORCEINLINE const MetaType &get() {
            return T::staticMetaClass;
        }
    };

    const char *m_name;                 /**< Name of the type. */
    uint32_t m_traits;                  /**< Traits for the type. */
};

/** Class providing metadata about a property. */
class MetaProperty {
public:
    /** Type of the get function defined by objgen. */
    using GetFunction = void (*)(const Object *object, void *value);
    /** Type of the set function defined by objgen. */
    using SetFunction = void (*)(Object *object, const void *value);

    MetaProperty(
        const char *name,
        const MetaType &type,
        GetFunction getFunction,
        SetFunction setFunction);

    /** @return             Name of the property. */
    const char *name() const { return m_name; }
    /** @return             Type of the property. */
    const MetaType &type() const { return m_type; }
private:
    /** Get the property value.
     * @param object        Object to get property from.
     * @param value         Where to store property value. */
    void get(const Object *object, void *value) const {
        m_getFunction(object, value);
    }

    /** Set the property value.
     * @param object        Object to set property on.
     * @param value         Buffer containing new property value. */
    void set(Object *object, const void *value) const {
        m_setFunction(object, value);
    }

    const char *m_name;                 /**< Name of the property. */
    const MetaType &m_type;             /**< Type of the property. */
    GetFunction m_getFunction;          /**< Get function defined by objgen. */
    SetFunction m_setFunction;          /**< Set function defined by objgen. */

    friend class Object;
};

/** Metadata for an Object-derived class. */
class MetaClass : public MetaType {
public:
    /** Type of an array of properties. */
    using PropertyArray = std::vector<MetaProperty>;

    /** Type of the constructor function generated by objgen. */
    using ConstructorFunction = std::function<Object *()>;

    MetaClass(
        const char *name,
        const MetaClass *parent,
        uint32_t traits,
        ConstructorFunction constructor,
        const PropertyArray &properties);
    ~MetaClass();

    /** @return             Metadata for parent class. */
    const MetaClass *parent() const { return m_parent; }
    /** @return             Array of properties in the class. */
    const PropertyArray &properties() const { return m_properties; }

    bool isConstructable() const;
    bool isBaseOf(const MetaClass &other) const;

    Object *construct() const;

    const MetaProperty *lookupProperty(const char *name) const;

    static const MetaClass *lookup(const std::string &name);
    static void visit(const std::function<void (const MetaClass &)> &function);
private:
    const MetaClass *m_parent;          /**< Metadata for parent class. */
    ConstructorFunction m_constructor;  /**< Constructor function object. */
    const PropertyArray &m_properties;  /**< Array of properties. */

    /** Map of properties for fast lookup. */
    HashMap<std::string, const MetaProperty *> m_propertyMap;
};

/** @return             Whether the class is constructable with construct(). */
inline bool MetaClass::isConstructable() const {
    /* To the outside world, we only return true if the class is publically
     * constructable. The public construct() method only works for classes for
     * which this is the case. Private construction is only used during
     * deserialization, which is done internally. */
    return m_traits & kIsPublicConstructable;
}

/**
 * Object class.
 */

/**
 * Base class of all classes using the object system.
 *
 * The object system provides additional functionality on top of regular C++
 * classes. The primary feature is reflection, which is used for both automatic
 * (de)serialisation of properties, and for editing of properties. It also
 * allows the creation of new instances of Object-derived classes from the
 * reflection information, allowing for instance an object to be constructed
 * given a string containing the class name. In addition, all Objects are
 * reference counted.
 *
 * All classes which derive (directly or indirectly) from Object must be
 * annotated with the CLASS() macro. Class properties can be specified with the
 * PROPERTY() and VPROPERTY() macros. For further details, see the documentation
 * of those macros.
 *
 * In order for a class to be constructable through the object system (and
 * therefore able to be deserialized or created through the editor), it must
 * have a default constructor, i.e. one with no parameters. Other constructors
 * may exist as well, however these are not usable by the object system. To be
 * constructable by MetaClass::construct(), this default constructor must also
 * be public. A non-public constructor can still be used to deserialize an
 * object, however.
 */
class Object : public Refcounted, Noncopyable {
public:
    DECLARE_STATIC_METACLASS("constructable": false);

    /**
     * Get the meta-class of this Object.
     *
     * Gets the meta-class of this specific Object instance. This can be used
     * to identify the type of this object.
     *
     * @return              Meta-class of this object.
     */
    virtual const MetaClass &metaClass() const;

    /** Get a property value.
     * @tparam T            Type of the property to get.
     * @param name          Name of the property to get.
     * @param value         Where to store property value.
     * @return              Whether the property could be found. */
    template <typename T>
    bool getProperty(const char *name, T &value) const {
        return getProperty(name, MetaType::lookup<T>(), &value);
    }

    /** Set a property value.
     * @tparam T            Type of the property to get.
     * @param name          Name of the property to get.
     * @param value         New property value.
     * @return              Whether the property could be found. */
    template <typename T>
    bool setProperty(const char *name, const T &value) {
        return setProperty(name, MetaType::lookup<T>(), &value);
    }
protected:
    Object() {}
private:
    bool getProperty(const char *name, const MetaType &type, void *value) const;
    bool setProperty(const char *name, const MetaType &type, const void *value);
};

/**
 * Object-specific wrapper for ReferencePtr.
 *
 * No functional difference between the two, just to clarify intention and to
 * allow for additional Object-specific behaviour to be added later without
 * having to change any other code.
 */
template <typename T>
using ObjectPtr = ReferencePtr<T>;

/**
 * Cast an Object pointer down the inheritance hierarchy.
 *
 * This is similar to dynamic_cast, making use of the object system's type
 * information instead. Only down-casts are allowed, up-casts should just be
 * explicit conversions.
 *
 * @tparam TargetPtr    Target pointer type.
 * @param object        Object pointer to cast.
 *
 * @return              Pointer to object if cast was successful, nullptr if not.
 */
template <typename TargetPtr, typename Source>
inline TargetPtr object_cast(Source *object) {
    static_assert(
        std::is_pointer<TargetPtr>::value,
        "target type must be a pointer");

    using Target = typename std::remove_pointer<TargetPtr>::type;

    static_assert(
        std::is_base_of<Object, Source>::value,
        "source type must be derived from Object");
    static_assert(
        std::is_base_of<Source, Target>::value,
        "target type must be derived from source");
    static_assert(
        std::is_const<Target>::value == std::is_const<Source>::value &&
            std::is_volatile<Target>::value == std::is_volatile<Source>::value,
        "target and source cv-qualifiers must be the same");

    return (Target::staticMetaClass.isBaseOf(object->metaClass()))
        ? static_cast<TargetPtr>(object)
        : nullptr;
}

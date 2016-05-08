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

#include "core/refcounted.h"

#include <type_traits>

/**
 * Annotation macros.
 */

/** Macro to define an annotation attribute for objgen. */
#ifdef ORION_OBJGEN
#   define META_ATTRIBUTE(type, ...) \
        __attribute__((annotate("orion:" type ":" #__VA_ARGS__)))
#else
#   define META_ATTRIBUTE(type, ...)
#endif

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
 * Example:
 *
 *   class Foobar : public Object {
 *   public:
 *       CLASS();
 *       ...
 *   };
 *
 * @param ...           Directives for objgen indicating traits of the class.
 */
#define CLASS(...) \
    static const META_ATTRIBUTE("class", __VA_ARGS__) MetaClass staticMetaClass; \
    virtual const MetaClass &metaClass() const;

/**
 * Annotation for object properties.
 *
 * This macro marks a class member as a property. This means it can be accessed
 * through the dynamic property interface in Object, and information about it
 * can be retrieved at runtime.
 *
 * Public members can be marked as a property with no additional support
 * required. Private members require a setter method to be specified, this
 * is typically used when the property requires validation on the value set for
 * it.
 *
 * Example:
 *
 *   public:
 *       PROPERTY() int wop;
 *   private:
 *       PROPERTY(set = setFoop)
 *       glm::vec3 foop;
 *
 * @param ...           Directives for objgen indicating traits of the property.
 */
#define PROPERTY(...) \
    META_ATTRIBUTE("property", __VA_ARGS__)

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
    /** @return             Name of the type. */
    const char *name() const { return m_name; }

    /** @return             Whether the type is an Object-derived class. */
    bool isObject() const { return m_traits & kIsObject; }

    /** Look up the meta-type for a given type.
     * @tparam T            Type to get for.
     * @return              Meta-type for that type. */
    template <typename T>
    static inline const MetaType *lookup() {
        return LookupImpl<T>::get();
    }
protected:
    /** Type trait flags. */
    enum : uint32_t {
        kIsObject = (1 << 0),           /**< Is an Object-derived class. */
    };

    MetaType(const char *name, uint32_t traits);
private:
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

        static NOINLINE const MetaType *get() {
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
            return type;
        }

        #undef LOOKUP_FUNCTION_SIGNATURE
    };

    /** Specialization for Object-derived classes to use the static MetaClass. */
    template <typename T>
    struct LookupImpl<T, typename std::enable_if<std::is_base_of<Object, T>::value>::type> {
        static FORCEINLINE const MetaType *get() {
            return &T::staticMetaClass;
        }
    };

    const char *m_name;                 /**< Name of the type. */
    uint32_t m_traits;                  /**< Traits for the type. */
};

/** Metadata for an Object-derived class. */
class MetaClass : public MetaType {
public:
    explicit MetaClass(const char *name, const MetaClass *parent = nullptr);
    ~MetaClass();

    /** @return             Metadata for parent class. */
    const MetaClass *parent() const { return m_parent; }

    bool isBaseOf(const MetaClass &other) const;

    static const MetaClass *lookup(const std::string &name);
private:
    const MetaClass *m_parent;          /**< Metadata for parent class. */
};

/** Class providing metadata about a property. */
class MetaProperty {
public:

};

/**
 * Object class.
 */

/** Base class of all meta-objects. */
class Object : public Refcounted {
public:
    CLASS();
};

/**
 * Object-specific wrapper for ReferencePtr.
 *
 * No functional difference between the two, just to clarify intention and to
 * allow for additional Object-specific behaviour to be added later without
 * having to change any other code.
 */
template <typename T>
class ObjectPtr : public ReferencePtr<T> {
public:
    static_assert(std::is_base_of<Object, T>::value, "type must be derived from Object");
    using ReferencePtr<T>::ReferencePtr;
};

/**
 * Cast an Object pointer down the inheritance hierarchy.
 *
 * This is similar to dynamic_cast, making use of the meta-object system's
 * type information instead. Only down-casts are allowed, up-casts should just
 * be explicit conversions.
 *
 * @tparam TargetPtr    Target pointer type.
 * @param object        Object pointer to cast.
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

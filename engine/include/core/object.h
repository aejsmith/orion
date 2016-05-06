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
 * @brief               Meta-object system.
 */

#pragma once

#include "core/refcounted.h"

#include <type_traits>

/** Macro to define an annotation attribute for objgen. */
#ifdef ORION_OBJGEN
#   define META_ATTRIBUTE(type, ...) \
        __attribute__((annotate("orion:" type ":" #__VA_ARGS__)))
#else
#   define META_ATTRIBUTE(type, ...)
#endif

/**
 * Annotation for meta-object classes.
 *
 * This macro marks a class as a meta-object class, i.e. one which derives from
 * Object. All such classes must include this annotation in order to add
 * definitions for class metadata objects, and to mark the class for objgen.
 * Note that this leaves the current class access specifier as private, as it
 * is intended to be placed right at the top of a class declaration.
 *
 * Example:
 *
 *   class Foobar : public Object {
 *       CLASS()
 *   public:
 *       ...
 *   };
 *
 * @param ...           Directives for objgen indicating traits of the class.
 */
#define CLASS(...) \
    public: \
        static const META_ATTRIBUTE("class", __VA_ARGS__) MetaClass staticMetaClass; \
        virtual const MetaClass &metaClass() const; \
    private:

/**
 * Annotation for meta-object structures.
 *
 * This macro is equivalent to CLASS() except for a struct. The only difference
 * between the two is that this leaves the access specifier as public.
 *
 * Example:
 *
 *   struct Foobar : Object {
 *       STRUCT()
 *
 *       ...
 *   };
 *
 * @param ...           Directives for objgen indicating traits of the class.
 */
#define STRUCT(...) \
        CLASS(__VA_ARGS__) \
    public:

/**
 * Annotation for class properties.
 *
 * This macro marks a class member as a property. This means it can be accessed
 * through the generic property interface in Object, and information about it
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

/** Class providing metadata about a class. */
class MetaClass {
public:
    MetaClass(const char *name, const MetaClass *parent = nullptr);
    ~MetaClass();

    /** @return             Name of the class. */
    const char *name() const { return m_name; }
    /** @return             Metadata for parent class. */
    const MetaClass *parent() const { return m_parent; }

    bool isBaseOf(const MetaClass &other) const;

    static const MetaClass *lookup(const std::string &name);
private:
    const char *m_name;                 /**< Name of the class. */
    const MetaClass *m_parent;          /**< Metadata for parent class. */
};

/** Class providing metadata about a property. */
class MetaProperty {
public:

};

/** Base class of all meta-objects. */
class Object : public Refcounted {
    CLASS()
public:

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
    static_assert(std::is_pointer<TargetPtr>::value, "target type must be a pointer");

    using Target = typename std::remove_pointer<TargetPtr>::type;

    static_assert(std::is_base_of<Object, Source>::value, "source type must be derived from Object");
    static_assert(std::is_base_of<Source, Target>::value, "target type must be derived from source");
    static_assert(
        std::is_const<Target>::value == std::is_const<Source>::value &&
            std::is_volatile<Target>::value == std::is_volatile<Source>::value,
        "target and source cv-qualifiers must be the same");

    return (Target::staticMetaClass.isBaseOf(object->metaClass()))
        ? static_cast<TargetPtr>(object)
        : nullptr;
}

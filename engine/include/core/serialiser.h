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
 * @brief               Object serialisation.
 *
 * TODO:
 *  - Add version numbers to serialised files. This would allow us to handle
 *    changes in the serialised data. Need both an engine and a game version
 *    number, so that changes in both engine classes and game-specific classes
 *    can be handled separately.
 */

#pragma once

#include "core/object.h"

namespace Detail {
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wundefined-inline"

    /** Helper to identify a type which has a serialise method. */
    template <typename T>
    class HasSerialise {
    private:
        template <typename U>
        static constexpr auto test(int) ->
            typename std::is_same<
                decltype(std::declval<U>().serialise(std::declval<Serialiser &>())),
                void
            >::type;

        template <typename U>
        static constexpr std::false_type test(...);
    public:
        static constexpr bool value = decltype(test<T>(0))::value;
    };

    /** Helper to identify a type which has a deserialise method. */
    template <typename T>
    class HasDeserialise {
    private:
        template <typename U>
        static constexpr auto test(int) ->
            typename std::is_same<
                decltype(std::declval<U>().deserialise(std::declval<Serialiser &>())),
                void
            >::type;

        template <typename U>
        static constexpr std::false_type test(...);
    public:
        static constexpr bool value = decltype(test<T>(0))::value;
    };

    #pragma clang diagnostic pop
}

/**
 * Base class for performing object (de)serialisation.
 *
 * This class is the base interface for classes which can serialise and
 * deserialise Object-derived classes. There are multiple implementations of
 * this class for different serialised file formats.
 *
 * The basic usage for serialisation is as follows:
 *
 *     JSONSerialiser serialiser;
 *     std::vector<uint8_t> data = serialiser.serialise(object);
 *
 * For deserialisation:
 *
 *     JSONSerialiser serialiser;
 *     ObjectPtr<MyClass> object = serialiser.deserialise<MyClass>(data);
 *
 * Internally, this uses Object::serialise() and Object::deserialise() to
 * (de)serialise the data. The base Object implementations of these methods
 * automatically (de)serialise all class properties. If any additional data
 * which is not stored in properties needs to be serialised, these methods can
 * be overridden to implement such behaviour (see the documentation of those
 * methods for further details).
 *
 * A serialised data file can contain multiple objects. This is for objects
 * which refer to some "child" objects. For example, a serialised Entity also
 * stores all of its Components. When this is done, the first object in the
 * file is the "primary" object, i.e. the object passed to serialise() and the
 * one returned by deserialise(). Each object in the file has an index given by
 * the order they are defined in the file. Serialising a reference to an object
 * causes the object to be serialised, and the reference is stored as the ID of
 * the object in the file. A single object will only be serialised once within
 * the same file, i.e. adding 2 references to the same object (checked by
 * address) will only serialise 1 copy of it.
 *
 * An exception to this behaviour is for managed assets. Despite being just
 * objects, if a reference to an object derived from Asset is serialised and
 * the asset is managed, the asset path will be stored. Unmanaged assets will
 * be serialised to the file.
 */
class Serialiser {
public:
    virtual ~Serialiser() {}

    /**
     * Serialise an object.
     *
     * Serialises the object into the file format implemented by this serialiser
     * instance. The return value is a binary data array which can be fed into
     * deserialise() to reconstruct the object, written to a file to deserialise
     * later, etc.
     *
     * @param object        Object to serialise.
     *
     * @return              Binary data array containing serialised object.
     */
    virtual std::vector<uint8_t> serialise(const Object *object) = 0;

    /**
     * Deserialise an object.
     *
     * Deserialises an object previously serialised in the format implemented
     * by this serialiser instance.
     *
     * @param data          Serialised data array.
     * @param metaClass     Expected type of the object.
     *
     * @return              Pointer to deserialised object, or null on failure.
     */
    virtual ObjectPtr<Object> deserialise(const std::vector<uint8_t> &data, const MetaClass &metaClass) = 0;

    /**
     * Deserialise an object.
     *
     * Deserialises an object previously serialised in the format implemented
     * by this serialiser instance.
     *
     * @tparam T            Expected type of the object.
     * @param data          Serialised data array.
     *
     * @return              Pointer to deserialised object, or null on failure.
     */
    template <typename T>
    ObjectPtr<T> deserialise(const std::vector<uint8_t> &data) {
        ObjectPtr<Object> object = deserialise(data, T::staticMetaClass);
        return object.staticCast<T>();
    }

    /**
     * Interface used by (de)serialisation methods.
     */

    /**
     * Begin a value group within the current scope.
     *
     * This can be used to create a group of named values inside the current
     * scope. This is useful for example for nested structures (and is used by
     * the implementations of read()/write() for arbitrary structures). When
     * serialising, this function creates a new group and makes it the current
     * scope. When deserialising, it looks for the specified group and makes it
     * the current scope. Each call to this function must be matched with a call
     * to endGroup() at the end. As an example, using JSON serialisation, the
     * following code:
     *
     *     serialiser.beginGroup("foo");
     *     serialiser.write("bar", m_foo.bar);
     *     serialiser.endGroup();
     *
     * Gives the following:
     *
     *     "foo": {
     *         "bar": ...
     *     }
     *
     * @param name          Name of the group.
     *
     * @return              Whether a group was found (can only return false
     *                      during deserialisation). If this returns false,
     *                      endGroup() should not be called.
     */
    virtual bool beginGroup(const char *name = nullptr) = 0;

    /** End a value group. */
    virtual void endGroup() = 0;

    /**
     * Begin a value array within the current scope.
     *
     * This can be used to create a group of unnamed values inside the current
     * scope. This can be used to represent containers such as lists an arrays.
     * When serialising, this function creates a new array and makes it the
     * current scope. When deserialising, it looks for the specified array and
     * makes it the current scope. Each call to this function must be matched
     * with a call to endArray() at the end. Values should be read and written
     * using pop()/push() rather than read()/write(). Order is preserved, so
     * items will be deserialised in the order they were serialised. pop()
     * returns false to indicate that the end of the array has been reached.
     *
     * @param name          Name of the array.
     *
     * @return              Whether an array was found (can only return false
     *                      during deserialisation). If this returns false,
     *                      endArray() should not be called.
     */
    virtual bool beginArray(const char *name = nullptr) = 0;

    /** End a value group. */
    virtual void endArray() = 0;

    void write(const char *name, const bool &value);
    void write(const char *name, const int8_t &value);
    void write(const char *name, const uint8_t &value);
    void write(const char *name, const int16_t &value);
    void write(const char *name, const uint16_t &value);
    void write(const char *name, const int32_t &value);
    void write(const char *name, const uint32_t &value);
    void write(const char *name, const int64_t &value);
    void write(const char *name, const uint64_t &value);
    void write(const char *name, const float &value);
    void write(const char *name, const double &value);
    void write(const char *name, const std::string &value);
    void write(const char *name, const glm::vec2 &value);
    void write(const char *name, const glm::vec3 &value);
    void write(const char *name, const glm::vec4 &value);
    void write(const char *name, const glm::quat &value);

    /** Write an enum.
     * @tparam T            Type of the enum.
     * @param name          Name for the value.
     * @param value         Value to write. */
    template <typename T, typename std::enable_if<std::is_enum<T>::value>::type * = nullptr>
    void write(const char *name, const T &value) {
        write(name, MetaType::lookup<T>(), &value);
    }

    /**
     * Write an object reference.
     *
     * Serialises the object referred to by the given pointer if it has not
     * already been serialised in this file, and writes a reference to the
     * object within the serialised file. If the pointer refers to a managed
     * asset, then only a reference to that asset will be saved, rather than
     * including a serialised copy of the asset.
     *
     * @tparam T            Type of the object reference.
     * @param name          Name for the value.
     * @param object        Pointer to object to write.
     */
    template <typename T, typename std::enable_if<std::is_base_of<Object, T>::value>::type * = nullptr>
    void write(const char *name, const ObjectPtr<T> &object) {
        write(name, MetaType::lookup<const ObjectPtr<T>>(), &object);
    }

    /**
     * Write an object pointer.
     *
     * Serialises the object referred to by the given pointer if it has not
     * already been serialised in this file, and writes a reference to the
     * object within the serialised file. If the pointer refers to a managed
     * asset, then only a reference to that asset will be saved, rather than
     * including a serialised copy of the asset.
     *
     * @tparam T            Type of the object pointer.
     * @param name          Name for the value.
     * @param object        Pointer to object to write.
     */
    template <typename T, typename std::enable_if<std::is_base_of<Object, T>::value>::type * = nullptr>
    void write(const char *name, const T *object) {
        write(name, MetaType::lookup<const T *>(), &object);
    }

    /**
     * Write a structure/class.
     *
     * This method can be used to write any type which provides a serialise
     * method of the form:
     *
     *     void serialise(Serialiser &serialiser) const;
     *
     * This method will begin a group with the given name, call the type's
     * serialise() method, and end the group.
     *
     * @param name          Name for the value.
     * @param value         Value to write.
     */
    template <typename T, typename std::enable_if<Detail::HasSerialise<T>::value>::type * = nullptr>
    void write(const char *name, const T &value) {
        beginGroup(name);
        value.serialise(*this);
        endGroup();
    }

    /** Push an entry onto the current array.
     * @param value         Value to push. */
    template <typename T>
    void push(T &&value) {
        write(nullptr, std::forward<T>(value));
    }

    bool read(const char *name, bool &value);
    bool read(const char *name, int8_t &value);
    bool read(const char *name, uint8_t &value);
    bool read(const char *name, int16_t &value);
    bool read(const char *name, uint16_t &value);
    bool read(const char *name, int32_t &value);
    bool read(const char *name, uint32_t &value);
    bool read(const char *name, int64_t &value);
    bool read(const char *name, uint64_t &value);
    bool read(const char *name, float &value);
    bool read(const char *name, double &value);
    bool read(const char *name, std::string &value);
    bool read(const char *name, glm::vec2 &value);
    bool read(const char *name, glm::vec3 &value);
    bool read(const char *name, glm::vec4 &value);
    bool read(const char *name, glm::quat &value);

    /** Read an enum.
     * @tparam T            Type of the enum.
     * @param name          Name for the value.
     * @param value         Value to write. */
    template <typename T, typename std::enable_if<std::is_enum<T>::value>::type * = nullptr>
    bool read(const char *name, T &value) {
        return read(name, MetaType::lookup<T>(), &value);
    }

    /**
     * Read an object reference.
     *
     * Deserialises the specified object if it has not already been deserialised
     * from this file, and returns a reference to the object.
     *
     * @tparam T            Expected type of the object.
     * @param name          Name for the value.
     * @param object        Where to store reference to object (not modified if
     *                      value could not be found).
     *
     * @return              Whether the value was found.
     */
    template <typename T, typename std::enable_if<std::is_base_of<Object, T>::value>::type * = nullptr>
    bool read(const char *name, ObjectPtr<T> &object) {
        return read(name, MetaType::lookup<ObjectPtr<T>>(), &object);
    }

    /**
     * Read an object pointer.
     *
     * Deserialises the specified object if it has not already been deserialised
     * from this file, and returns a pointer to the object. This is a non-
     * reference counting pointer, so users should be sure that the object will
     * have a proper reference elsewhere. The object will be kept alive until
     * the end of the deserialise() call, if no references remain after that
     * then the object will be freed and this pointer will be invalid.
     *
     * @tparam T            Expected type of the object.
     * @param name          Name for the value.
     * @param object        Where to store pointer to object (not modified if
     *                      value could not be found).
     *
     * @return              Whether the value was found.
     */
    template <typename T, typename std::enable_if<std::is_base_of<Object, T>::value>::type * = nullptr>
    bool read(const char *name, T *&object) {
        return read(name, MetaType::lookup<T *>(), &object);
    }

    /**
     * Read a structure/class.
     *
     * This method can be used to read any type which provides a deserialise
     * method of the form:
     *
     *     void deserialise(Serialiser &serialiser);
     *
     * This method will begin a group with the given name, call the type's
     * deserialise() method, and end the group.
     *
     * @param name          Name for the value.
     * @param value         Value to write.
     *
     * @return              Whether the value was found.
     */
    template <typename T, typename std::enable_if<Detail::HasDeserialise<T>::value>::type * = nullptr>
    bool read(const char *name, T &value) {
        if (beginGroup(name)) {
            value.deserialise(*this);
            endGroup();
            return true;
        } else {
            return false;
        }
    }

    /** Pop an entry from the current array.
     * @param value         Value to push.
     * @return              Whether the value was found. */
    template <typename T>
    bool pop(T &&value) {
        return read(nullptr, std::forward<T>(value));
    }
protected:
    Serialiser() {}

    /** Write a value.
     * @param name          Name for the value (null if array).
     * @param type          Type of the value.
     * @param value         Pointer to value. */
    virtual void write(const char *name, const MetaType &type, const void *value) = 0;

    /** Read a value.
     * @param name          Name for the value (null if array).
     * @param type          Type of the value.
     * @param value         Pointer to value.
     * @return              Whether the value was found. */
    virtual bool read(const char *name, const MetaType &type, void *value) = 0;

    void serialiseObject(const Object *object);
    bool deserialiseObject(const char *className, const MetaClass &metaClass, ObjectPtr<Object> &object);
private:
    friend class Object;
};

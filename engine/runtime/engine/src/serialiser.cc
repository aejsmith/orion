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
 */

#include "engine/serialiser.h"

/** Serialise an object to the current scope.
 * @param object        Object to serialise. */
void Serialiser::serialiseObject(const Object *object) {
    /* We are a friend of Object, we can call this. */
    object->serialise(*this);
}

/** Deserialise an object from the current scope.
 * @param className     Class name given in the serialised file.
 * @param metaClass     Expected class of the object.
 * @param object        Where to store pointer to object. Set after construction
 *                      but before Object::deserialise() is called.
 * @return              Whether the object was successfully deserialised. */
bool Serialiser::deserialiseObject(const char *className,
                                   const MetaClass &metaClass,
                                   bool isPrimary,
                                   ObjectPtr<Object> &object)
{
    const MetaClass *givenMetaClass = MetaClass::lookup(className);
    if (!givenMetaClass) {
        logError("Serialised data contains unknown class '%s'", className);
        return false;
    }

    if (!metaClass.isBaseOf(*givenMetaClass)) {
        logError(
            "Class mismatch in serialised data (expected '%s', have '%s')",
            metaClass.name(), className);
        return false;
    }

    /* We allow deserialisation of classes that do not have a public constructor. */
    object = givenMetaClass->constructPrivate();

    if (isPrimary && this->postConstructFunction)
        this->postConstructFunction(object);

    object->deserialise(*this);
    return true;
}

/** Write a boolean value.
 * @param name          Name for the value.
 * @param value         Value to write. */
void Serialiser::write(const char *name, const bool &value) {
    write(name, MetaType::lookup<bool>(), &value);
}

/** Write an 8-bit signed integer value.
 * @param name          Name for the value.
 * @param value         Value to write. */
void Serialiser::write(const char *name, const int8_t &value) {
    write(name, MetaType::lookup<int8_t>(), &value);
}

/** Write an 8-bit unsigned integer value.
 * @param name          Name for the value.
 * @param value         Value to write. */
void Serialiser::write(const char *name, const uint8_t &value) {
    write(name, MetaType::lookup<uint8_t>(), &value);
}

/** Write a 16-bit signed integer value.
 * @param name          Name for the value.
 * @param value         Value to write. */
void Serialiser::write(const char *name, const int16_t &value) {
    write(name, MetaType::lookup<int16_t>(), &value);
}

/** Write a 16-bit unsigned integer value.
 * @param name          Name for the value.
 * @param value         Value to write. */
void Serialiser::write(const char *name, const uint16_t &value) {
    write(name, MetaType::lookup<uint16_t>(), &value);
}

/** Write a 32-bit signed integer value.
 * @param name          Name for the value.
 * @param value         Value to write. */
void Serialiser::write(const char *name, const int32_t &value) {
    write(name, MetaType::lookup<int32_t>(), &value);
}

/** Write a 32-bit unsigned integer value.
 * @param name          Name for the value.
 * @param value         Value to write. */
void Serialiser::write(const char *name, const uint32_t &value) {
    write(name, MetaType::lookup<uint32_t>(), &value);
}

/** Write a 64-bit signed integer value.
 * @param name          Name for the value.
 * @param value         Value to write. */
void Serialiser::write(const char *name, const int64_t &value) {
    write(name, MetaType::lookup<int64_t>(), &value);
}

/** Write a 64-bit unsigned integer value.
 * @param name          Name for the value.
 * @param value         Value to write. */
void Serialiser::write(const char *name, const uint64_t &value) {
    write(name, MetaType::lookup<uint64_t>(), &value);
}

/** Write a single-precision floating point value.
 * @param name          Name for the value.
 * @param value         Value to write. */
void Serialiser::write(const char *name, const float &value) {
    write(name, MetaType::lookup<float>(), &value);
}

/** Write a double-precision floating point value.
 * @param name          Name for the value.
 * @param value         Value to write. */
void Serialiser::write(const char *name, const double &value) {
    write(name, MetaType::lookup<double>(), &value);
}

/** Write a string value.
 * @param name          Name for the value.
 * @param value         Value to write. */
void Serialiser::write(const char *name, const std::string &value) {
    write(name, MetaType::lookup<std::string>(), &value);
}

/** Write a 2-component vector value.
 * @param name          Name for the value.
 * @param value         Value to write. */
void Serialiser::write(const char *name, const glm::vec2 &value) {
    write(name, MetaType::lookup<glm::vec2>(), &value);
}

/** Write a 3-component vector value.
 * @param name          Name for the value.
 * @param value         Value to write. */
void Serialiser::write(const char *name, const glm::vec3 &value) {
    write(name, MetaType::lookup<glm::vec3>(), &value);
}

/** Write a 4-component vector value.
 * @param name          Name for the value.
 * @param value         Value to write. */
void Serialiser::write(const char *name, const glm::vec4 &value) {
    write(name, MetaType::lookup<glm::vec4>(), &value);
}

/** Write a quaternion value.
 * @param name          Name for the value.
 * @param value         Value to write. */
void Serialiser::write(const char *name, const glm::quat &value) {
    write(name, MetaType::lookup<glm::quat>(), &value);
}

/** Read a boolean value.
 * @param name          Name for the value.
 * @param value         Where to store value (not modified if not found).
 * @return              Whether the value was found. */
bool Serialiser::read(const char *name, bool &value) {
    return read(name, MetaType::lookup<bool>(), &value);
}

/** Read an 8-bit signed integer value.
 * @param name          Name for the value.
 * @param value         Where to store value (not modified if not found).
 * @return              Whether the value was found. */
bool Serialiser::read(const char *name, int8_t &value) {
    return read(name, MetaType::lookup<int8_t>(), &value);
}

/** Read an 8-bit unsigned integer value.
 * @param name          Name for the value.
 * @param value         Where to store value (not modified if not found).
 * @return              Whether the value was found. */
bool Serialiser::read(const char *name, uint8_t &value) {
    return read(name, MetaType::lookup<uint8_t>(), &value);
}

/** Read a 16-bit signed integer value.
 * @param name          Name for the value.
 * @param value         Where to store value (not modified if not found).
 * @return              Whether the value was found. */
bool Serialiser::read(const char *name, int16_t &value) {
    return read(name, MetaType::lookup<int16_t>(), &value);
}

/** Read a 16-bit unsigned integer value.
 * @param name          Name for the value.
 * @param value         Where to store value (not modified if not found).
 * @return              Whether the value was found. */
bool Serialiser::read(const char *name, uint16_t &value) {
    return read(name, MetaType::lookup<uint16_t>(), &value);
}

/** Read a 32-bit signed integer value.
 * @param name          Name for the value.
 * @param value         Where to store value (not modified if not found).
 * @return              Whether the value was found. */
bool Serialiser::read(const char *name, int32_t &value) {
    return read(name, MetaType::lookup<int32_t>(), &value);
}

/** Read a 32-bit unsigned integer value.
 * @param name          Name for the value.
 * @param value         Where to store value (not modified if not found).
 * @return              Whether the value was found. */
bool Serialiser::read(const char *name, uint32_t &value) {
    return read(name, MetaType::lookup<uint32_t>(), &value);
}

/** Read a 64-bit signed integer value.
 * @param name          Name for the value.
 * @param value         Where to store value (not modified if not found).
 * @return              Whether the value was found. */
bool Serialiser::read(const char *name, int64_t &value) {
    return read(name, MetaType::lookup<int64_t>(), &value);
}

/** Read a 64-bit unsigned integer value.
 * @param name          Name for the value.
 * @param value         Where to store value (not modified if not found).
 * @return              Whether the value was found. */
bool Serialiser::read(const char *name, uint64_t &value) {
    return read(name, MetaType::lookup<uint64_t>(), &value);
}

/** Read a single-precision floating point value.
 * @param name          Name for the value.
 * @param value         Where to store value (not modified if not found).
 * @return              Whether the value was found. */
bool Serialiser::read(const char *name, float &value) {
    return read(name, MetaType::lookup<float>(), &value);
}

/** Read a double-precision floating point value.
 * @param name          Name for the value.
 * @param value         Where to store value (not modified if not found).
 * @return              Whether the value was found. */
bool Serialiser::read(const char *name, double &value) {
    return read(name, MetaType::lookup<double>(), &value);
}

/** Read a string value.
 * @param name          Name for the value.
 * @param value         Where to store value (not modified if not found).
 * @return              Whether the value was found. */
bool Serialiser::read(const char *name, std::string &value) {
    return read(name, MetaType::lookup<std::string>(), &value);
}

/** Read a 2-component vector value.
 * @param name          Name for the value.
 * @param value         Where to store value (not modified if not found).
 * @return              Whether the value was found. */
bool Serialiser::read(const char *name, glm::vec2 &value) {
    return read(name, MetaType::lookup<glm::vec2>(), &value);
}

/** Read a 3-component vector value.
 * @param name          Name for the value.
 * @param value         Where to store value (not modified if not found).
 * @return              Whether the value was found. */
bool Serialiser::read(const char *name, glm::vec3 &value) {
    return read(name, MetaType::lookup<glm::vec3>(), &value);
}

/** Read a 4-component vector value.
 * @param name          Name for the value.
 * @param value         Where to store value (not modified if not found).
 * @return              Whether the value was found. */
bool Serialiser::read(const char *name, glm::vec4 &value) {
    return read(name, MetaType::lookup<glm::vec4>(), &value);
}

/** Read a quaternion value.
 * @param name          Name for the value.
 * @param value         Where to store value (not modified if not found).
 * @return              Whether the value was found. */
bool Serialiser::read(const char *name, glm::quat &value) {
    return read(name, MetaType::lookup<glm::quat>(), &value);
}

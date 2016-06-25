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
 * @brief               JSON serialisation class.
 */

#include "core/json_serialiser.h"

#include "engine/asset_manager.h"

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

#include <list>

/** Internal state used during (de)serialisation. */
struct JSONSerialiser::State {
    bool writing;                           /**< Whether we are currently writing or reading. */
    rapidjson::Document document;           /**< Current document. */

    /** Map of object addresses to pre-existing IDs (serialising). */
    HashMap<const Object *, uint32_t> objectToIDMap;

    /** Map of IDs to pre-existing objects (deserialising). */
    HashMap<uint32_t, ObjectPtr<Object>> idToObjectMap;

    /** Structure representing a scope. */
    struct Scope {
        enum Type {
            kObject,
            kGroup,
            kArray,
        };

        Type type;                          /**< Type of the scope. */
        rapidjson::Value &value;            /**< Value that it refers to. */
        size_t nextIndex;                   /**< Next array index. */

        Scope(Type inType, rapidjson::Value &inValue) :
            type(inType),
            value(inValue),
            nextIndex(0)
        {}
    };

    /**
     * Scope stack.
     *
     * This is used to keep track of which value we are currently reading from
     * or writing to. Each (add|find)Object(), beginChild() and beginArray()
     * call pushes a new scope on to the end. read() and write() operate on the
     * scope at the end of the list.
     */
    std::list<Scope> scopes;

    /** Get the current scope and check its type.
     * @param name          Name of the member to be added.
     * @return              Reference to current scope. */
    Scope &currentScope(const char *name) {
        Scope &scope = this->scopes.back();

        if (name) {
            check(scope.type != Scope::kArray);
        } else {
            check(scope.type == Scope::kArray);
        }

        return scope;
    }

    /** Begin a new scope.
     * @param name          Name of the scope.
     * @param type          Type of the scope.
     * @return              Whether the scope was found (for deserialisation). */
    bool beginScope(const char *name, Scope::Type type) {
        rapidjson::Type jsonType = (type == Scope::kArray)
            ? rapidjson::kArrayType
            : rapidjson::kObjectType;

        Scope &scope = currentScope(name);

        rapidjson::Value *value;

        if (this->writing) {
            rapidjson::Value initValue(jsonType);
            value = &addMember(scope, name, initValue);
        } else {
            value = getMember(scope, name);
            if (!value || value->GetType() != jsonType)
                return false;
        }

        this->scopes.emplace_back(type, *value);
        return true;
    }

    /** Add a member to a scope.
     * @param scope         Scope to add to.
     * @param name          Name of the member to be added (null if array).
     * @param value         Value to add.
     * @return              Reference to member added. */
    rapidjson::Value &addMember(Scope &scope, const char *name, rapidjson::Value &value) {
        if (scope.type == Scope::kArray) {
            scope.value.PushBack(value, this->document.GetAllocator());
            rapidjson::Value &ret = scope.value[scope.value.Size() - 1];
            return ret;
        } else {
            check(!scope.value.HasMember(name));
            scope.value.AddMember(rapidjson::StringRef(name), value, this->document.GetAllocator());
            rapidjson::Value &ret = scope.value[name];
            return ret;
        }
    }

    /** Get a member from a scope.
     * @param scope         Scope to get from.
     * @param name          Name of the member to get (null if array).
     * @return              Pointer to member added, or null if not found. */
    rapidjson::Value *getMember(Scope &scope, const char *name) {
        if (scope.type == Scope::kArray) {
            if (scope.nextIndex >= scope.value.Size())
                return nullptr;

            return &scope.value[scope.nextIndex++];
        } else {
            if (!scope.value.HasMember(name))
                return nullptr;

            return &scope.value[name];
        }
    }
};

JSONSerialiser::JSONSerialiser() :
    m_state(nullptr)
{}

/** Serialise an object.
 * @param object        Object to serialise.
 * @return              Binary data array containing serialised object. */
std::vector<uint8_t> JSONSerialiser::serialise(const Object *object) {
    State state;
    m_state = &state;
    m_state->writing = true;
    m_state->document.SetArray();

    /* Serialise the object. */
    addObject(object);

    /* Write out the JSON stream. */
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    m_state->document.Accept(writer);
    std::vector<uint8_t> data(buffer.GetSize());
    memcpy(&data[0], buffer.GetString(), buffer.GetSize());

    m_state = nullptr;
    return data;
}

/** Serialise an object.
 * @param object        Object to serialise. Must not already be added.
 * @return              ID of object within file. */
uint32_t JSONSerialiser::addObject(const Object *object) {
    /* Create a new object. */
    uint32_t id = m_state->document.Size();
    m_state->document.PushBack(rapidjson::kObjectType, m_state->document.GetAllocator());
    rapidjson::Value &value = m_state->document[m_state->document.Size() - 1];

    /* Record it in the object map so we don't serialise it again. */
    m_state->objectToIDMap.insert(std::make_pair(object, id));

    /* Write out the type of the object, as well as its ID. The ID is not used
     * in deserialisation (it's done based on order of appearance in the array),
     * but we write it anyway because JSON is meant to be a human readable
     * format, and having the ID helps to understand it. */
    value.AddMember("objectClass", rapidjson::StringRef(object->metaClass().name()), m_state->document.GetAllocator());
    value.AddMember("objectID", id, m_state->document.GetAllocator());

    /* Serialise the object in a new scope. */
    m_state->scopes.emplace_back(State::Scope::kObject, value);
    serialiseObject(object);
    m_state->scopes.pop_back();

    return id;
}

/** Deserialise an object.
 * @param data          Serialised data array.
 * @param metaClass     Expected type of the object.
 * @return              Pointer to deserialised object, or null on failure. */
ObjectPtr<Object> JSONSerialiser::deserialise(const std::vector<uint8_t> &data, const MetaClass &metaClass) {
    State state;
    m_state = &state;
    m_state->writing = false;

    /* Parse the JSON stream. */
    m_state->document.Parse(reinterpret_cast<const char *>(&data[0]), data.size());
    if (m_state->document.HasParseError()) {
        const char *msg = rapidjson::GetParseError_En(m_state->document.GetParseError());
        logError("Parse error in serialised data (at %zu): %s", m_state->document.GetErrorOffset(), msg);
        return nullptr;
    }

    /* The object to return is the first object in the file. */
    ObjectPtr<Object> object = findObject(0, metaClass);

    m_state = nullptr;
    return object;
}

/** Deserialise an object or return an already existing object.
 * @param id            ID of the object.
 * @param metaClass     Expected type of the object.
 * @return              Pointer to object, or null on failure. */
ObjectPtr<Object> JSONSerialiser::findObject(uint32_t id, const MetaClass &metaClass) {
    /* Check if it is already deserialised. */
    auto existing = m_state->idToObjectMap.find(id);
    if (existing != m_state->idToObjectMap.end())
        return existing->second;

    if (id >= m_state->document.Size()) {
        logError(
            "Invalid serialised object ID %zu (only %zu objects available)",
            id, m_state->document.Size());
        return nullptr;
    }

    rapidjson::Value &value = m_state->document[id];

    if (!value.HasMember("objectClass")) {
        logError("Serialised object %zu does not have an 'objectClass' value", id);
        return nullptr;
    }

    const char *className = value["objectClass"].GetString();

    /* The serialised object or any objects it refers to may contain references
     * back to itself. Therefore, to ensure that we don't deserialise the object
     * multiple times, we must store it in our map before we call its
     * deserialise() method. Serialiser::deserialiseObject() sets the object
     * pointer referred to as soon it has constructed the object, before calling
     * deserialise(). This ensures that deserialised references to the object
     * will point to the correct object. */
    auto inserted = m_state->idToObjectMap.insert(std::make_pair(id, nullptr));
    check(inserted.second);

    m_state->scopes.emplace_back(State::Scope::kObject, value);
    bool success = deserialiseObject(className, metaClass, inserted.first->second);
    m_state->scopes.pop_back();

    if (success) {
        return inserted.first->second;
    } else {
        m_state->idToObjectMap.erase(inserted.first);
        return nullptr;
    }
}

/** Begin a value group within the current scope.
 * @param name          Name of the group.
 * @return              Whether a group was found. */
bool JSONSerialiser::beginGroup(const char *name) {
    check(m_state);
    return m_state->beginScope(name, State::Scope::kGroup);
}

/** End a value group. */
void JSONSerialiser::endGroup() {
    check(m_state);
    check(m_state->scopes.back().type == State::Scope::kGroup);

    m_state->scopes.pop_back();
}

/** Begin a value array within the current scope.
 * @param name          Name of the array.
 * @return              Whether a group was found. */
bool JSONSerialiser::beginArray(const char *name) {
    check(m_state);
    return m_state->beginScope(name, State::Scope::kArray);
}

/** End a value array. */
void JSONSerialiser::endArray() {
    check(m_state);
    check(m_state->scopes.back().type == State::Scope::kArray);

    m_state->scopes.pop_back();
}

/** Write a value.
 * @param name          Name for the value (null if array).
 * @param type          Type of the value.
 * @param value         Pointer to value. */
void JSONSerialiser::write(const char *name, const MetaType &type, const void *value) {
    check(m_state);
    check(m_state->writing);

    if (type.isPointer() && type.pointeeType().isObject()) {
        /* Object references require special handling. We serialise these as a
         * JSON object containing details of where to find the object. If the
         * reference is null, the JSON object is empty. If the reference refers
         * to a managed asset, it contains an "asset" member containing the
         * asset path. Otherwise, we serialise the object if it has not already
         * been added to the file, and store an "objectID" member referring to
         * it. */
        beginGroup(name);

        const Object *object = *reinterpret_cast<const Object *const *>(value);
        if (object) {
            const Asset *asset;

            /* Check if it is already serialised. We check this before handling
             * assets, because if we are serialising an asset and that contains
             * any child objects, we want any references they contain back to
             * the asset itself to point to the object within the serialised
             * file rather than using an asset path reference. */
            auto existing = m_state->objectToIDMap.find(object);
            if (existing != m_state->objectToIDMap.end()) {
                Serialiser::write("objectID", existing->second);
            } else if ((asset = object_cast<const Asset *>(object)) && asset->managed()) {
                std::string path = asset->path();
                Serialiser::write("asset", path);
            } else {
                uint32_t id = addObject(object);
                Serialiser::write("objectID", id);
            }
        }

        endGroup();
        return;
    }

    State::Scope &scope = m_state->currentScope(name);

    auto &allocator = m_state->document.GetAllocator();
    rapidjson::Value jsonValue;

    /* Determine the value to write. */
    if (&type == &MetaType::lookup<bool>()) {
        jsonValue.SetBool(*reinterpret_cast<const bool *>(value));
    } else if (&type == &MetaType::lookup<int8_t>()) {
        jsonValue.SetInt(*reinterpret_cast<const int8_t *>(value));
    } else if (&type == &MetaType::lookup<uint8_t>()) {
        jsonValue.SetUint(*reinterpret_cast<const uint8_t *>(value));
    } else if (&type == &MetaType::lookup<int16_t>()) {
        jsonValue.SetInt(*reinterpret_cast<const int16_t *>(value));
    } else if (&type == &MetaType::lookup<uint16_t>()) {
        jsonValue.SetUint(*reinterpret_cast<const uint16_t *>(value));
    } else if (&type == &MetaType::lookup<int32_t>()) {
        jsonValue.SetInt(*reinterpret_cast<const int32_t *>(value));
    } else if (&type == &MetaType::lookup<uint32_t>()) {
        jsonValue.SetUint(*reinterpret_cast<const uint32_t *>(value));
    } else if (&type == &MetaType::lookup<int64_t>()) {
        jsonValue.SetInt64(*reinterpret_cast<const int64_t *>(value));
    } else if (&type == &MetaType::lookup<uint64_t>()) {
        jsonValue.SetUint64(*reinterpret_cast<const uint64_t *>(value));
    } else if (&type == &MetaType::lookup<float>()) {
        jsonValue.SetFloat(*reinterpret_cast<const float *>(value));
    } else if (&type == &MetaType::lookup<double>()) {
        jsonValue.SetDouble(*reinterpret_cast<const double *>(value));
    } else if (&type == &MetaType::lookup<std::string>()) {
        auto str = reinterpret_cast<const std::string *>(value);
        jsonValue.SetString(str->c_str(), allocator);
    } else if (&type == &MetaType::lookup<glm::vec2>()) {
        auto vec = reinterpret_cast<const glm::vec2 *>(value);
        jsonValue.SetArray();
        jsonValue.PushBack(vec->x, allocator);
        jsonValue.PushBack(vec->y, allocator);
    } else if (&type == &MetaType::lookup<glm::vec3>()) {
        auto vec = reinterpret_cast<const glm::vec3 *>(value);
        jsonValue.SetArray();
        jsonValue.PushBack(vec->x, allocator);
        jsonValue.PushBack(vec->y, allocator);
        jsonValue.PushBack(vec->z, allocator);
    } else if (&type == &MetaType::lookup<glm::vec4>()) {
        auto vec = reinterpret_cast<const glm::vec4 *>(value);
        jsonValue.SetArray();
        jsonValue.PushBack(vec->x, allocator);
        jsonValue.PushBack(vec->y, allocator);
        jsonValue.PushBack(vec->z, allocator);
        jsonValue.PushBack(vec->w, allocator);
    } else if (&type == &MetaType::lookup<glm::quat>()) {
        auto quat = reinterpret_cast<const glm::quat *>(value);
        jsonValue.SetArray();
        jsonValue.PushBack(quat->w, allocator);
        jsonValue.PushBack(quat->x, allocator);
        jsonValue.PushBack(quat->y, allocator);
        jsonValue.PushBack(quat->z, allocator);
    } else if (type.isEnum()) {
        // FIXME: Potentially handle cases where we don't have enum metadata?
        // FIXME: Incorrect where enum size is not an int.
        const MetaType::EnumConstantArray &constants = type.enumConstants();
        for (const MetaType::EnumConstant &constant : constants) {
            if (*reinterpret_cast<const int *>(value) == constant.second) {
                jsonValue.SetString(rapidjson::StringRef(constant.first));
                break;
            }
        }

        check(!jsonValue.IsNull());
    } else {
        fatal("Type '%s' is unsupported for serialisation", type.name());
    }

    m_state->addMember(scope, name, jsonValue);
}

/** Read a value.
 * @param name          Name for the value (null if array).
 * @param type          Type of the value.
 * @param value         Pointer to value.
 * @return              Whether the value was found. */
bool JSONSerialiser::read(const char *name, const MetaType &type, void *value) {
    check(m_state);
    check(!m_state->writing);

    if (type.isPointer() && type.pointeeType().isObject()) {
        /* See write() for details on how we handle object references. */
        if (!beginGroup(name))
            return false;

        /* An empty object indicates a null reference. */
        if (m_state->scopes.back().value.MemberCount() == 0) {
            *reinterpret_cast<Object **>(value) = nullptr;
            endGroup();
            return true;
        }

        const MetaClass &metaClass = static_cast<const MetaClass &>(type.pointeeType());

        ObjectPtr<Object> ret;

        /* Check if we have an asset path. */
        std::string path;
        if (Serialiser::read("asset", path)) {
            ret = g_assetManager->load(path);
            if (ret && !metaClass.isBaseOf(ret->metaClass())) {
                logError(
                    "Class mismatch in serialised data (expected '%s', have '%s')",
                    metaClass.name(), ret->metaClass().name());
                ret.reset();
            }
        } else {
            /* Must be serialised within the file. */
            uint32_t id;
            if (Serialiser::read("objectID", id))
                ret = findObject(id, metaClass);
        }

        endGroup();

        if (ret) {
            if (type.isRefcounted()) {
                *reinterpret_cast<ObjectPtr<Object> *>(value) = std::move(ret);
            } else {
                *reinterpret_cast<Object **>(value) = ret;
            }

            return true;
        } else {
            return false;
        }
    }

    State::Scope &scope = m_state->currentScope(name);

    rapidjson::Value *member = m_state->getMember(scope, name);
    if (!member)
        return false;

    /* Get a reference to make the [] operator usage nicer below. */
    rapidjson::Value &jsonValue = *member;

    if (&type == &MetaType::lookup<bool>()) {
        if (!jsonValue.IsBool())
            return false;

        *reinterpret_cast<bool *>(value) = jsonValue.GetBool();
    } else if (&type == &MetaType::lookup<int8_t>()) {
        if (!jsonValue.IsInt())
            return false;

        *reinterpret_cast<int8_t *>(value) = jsonValue.GetInt();
    } else if (&type == &MetaType::lookup<uint8_t>()) {
        if (!jsonValue.IsUint())
            return false;

        *reinterpret_cast<uint8_t *>(value) = jsonValue.GetUint();
    } else if (&type == &MetaType::lookup<int16_t>()) {
        if (!jsonValue.IsInt())
            return false;

        *reinterpret_cast<int16_t *>(value) = jsonValue.GetInt();
    } else if (&type == &MetaType::lookup<uint16_t>()) {
        if (!jsonValue.IsUint())
            return false;

        *reinterpret_cast<uint16_t *>(value) = jsonValue.GetUint();
    } else if (&type == &MetaType::lookup<int32_t>()) {
        if (!jsonValue.IsInt())
            return false;

        *reinterpret_cast<int32_t *>(value) = jsonValue.GetInt();
    } else if (&type == &MetaType::lookup<uint32_t>()) {
        if (!jsonValue.IsUint())
            return false;

        *reinterpret_cast<uint32_t *>(value) = jsonValue.GetUint();
    } else if (&type == &MetaType::lookup<int64_t>()) {
        if (!jsonValue.IsInt64())
            return false;

        *reinterpret_cast<int64_t *>(value) = jsonValue.GetInt64();
    } else if (&type == &MetaType::lookup<uint64_t>()) {
        if (!jsonValue.IsUint64())
            return false;

        *reinterpret_cast<uint64_t *>(value) = jsonValue.GetUint64();
    } else if (&type == &MetaType::lookup<float>()) {
        if (!jsonValue.IsFloat())
            return false;

        *reinterpret_cast<float *>(value) = jsonValue.GetFloat();
    } else if (&type == &MetaType::lookup<double>()) {
        if (!jsonValue.IsDouble())
            return false;

        *reinterpret_cast<double *>(value) = jsonValue.GetDouble();
    } else if (&type == &MetaType::lookup<std::string>()) {
        if (!jsonValue.IsString())
            return false;

        *reinterpret_cast<std::string *>(value) = jsonValue.GetString();
    } else if (&type == &MetaType::lookup<glm::vec2>()) {
        if (!jsonValue.IsArray() ||
            jsonValue.Size() != 2 ||
            !jsonValue[0u].IsNumber() ||
            !jsonValue[1u].IsNumber())
        {
            return false;
        }

        *reinterpret_cast<glm::vec2 *>(value) = glm::vec2(
            jsonValue[0u].GetFloat(),
            jsonValue[1u].GetFloat());
    } else if (&type == &MetaType::lookup<glm::vec3>()) {
        if (!jsonValue.IsArray() ||
            jsonValue.Size() != 3 ||
            !jsonValue[0u].IsNumber() ||
            !jsonValue[1u].IsNumber() ||
            !jsonValue[2u].IsNumber())
        {
            return false;
        }

        *reinterpret_cast<glm::vec3 *>(value) = glm::vec3(
            jsonValue[0u].GetFloat(),
            jsonValue[1u].GetFloat(),
            jsonValue[2u].GetFloat());
    } else if (&type == &MetaType::lookup<glm::vec4>()) {
        if (!jsonValue.IsArray() ||
            jsonValue.Size() != 4 ||
            !jsonValue[0u].IsNumber() ||
            !jsonValue[1u].IsNumber() ||
            !jsonValue[2u].IsNumber() ||
            !jsonValue[3u].IsNumber())
        {
            return false;
        }

        *reinterpret_cast<glm::vec4 *>(value) = glm::vec4(
            jsonValue[0u].GetFloat(),
            jsonValue[1u].GetFloat(),
            jsonValue[2u].GetFloat(),
            jsonValue[3u].GetFloat());
    } else if (&type == &MetaType::lookup<glm::quat>()) {
        if (!jsonValue.IsArray() ||
            jsonValue.Size() != 4 ||
            !jsonValue[0u].IsNumber() ||
            !jsonValue[1u].IsNumber() ||
            !jsonValue[2u].IsNumber() ||
            !jsonValue[3u].IsNumber())
        {
            return false;
        }

        *reinterpret_cast<glm::quat *>(value) = glm::quat(
            jsonValue[0u].GetFloat(),
            jsonValue[1u].GetFloat(),
            jsonValue[2u].GetFloat(),
            jsonValue[3u].GetFloat());
    } else if (type.isEnum()) {
        if (!jsonValue.IsString())
            return false;

        /* Match the string against a value. */
        const MetaType::EnumConstantArray &constants = type.enumConstants();
        auto constant = constants.begin();
        while (constant != constants.end()) {
            if (std::strcmp(jsonValue.GetString(), constant->first) == 0) {
                // FIXME: enum size
                *reinterpret_cast<int *>(value) = constant->second;
                break;
            }

            ++constant;
        }

        if (constant == constants.end())
            return false;
    } else {
        fatal("Type '%s' is unsupported for deserialisation", type.name());
    }

    return true;
}

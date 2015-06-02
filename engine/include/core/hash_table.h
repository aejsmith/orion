/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Hash table class.
 */

#pragma once

#include "core/hash.h"

#include <unordered_map>

/** Hash table.
 * @todo                Eventually we will implement this ourselves.
 * @tparam Key          Type of the key. The hashValue() function must be
 *                      implemented for this type.
 * @tparam Value        Type of the value. */
template <typename Key, typename Value> using HashTable = std::unordered_map<Key, Value, Hash<Key>>;

/** Hash table with non-unique keys.
 * @tparam Key          Type of the key. The hashValue() function must be
 *                      implemented for this type.
 * @tparam Value        Type of the value. */
template <typename Key, typename Value> using MultiHashTable = std::unordered_multimap<Key, Value, Hash<Key>>;

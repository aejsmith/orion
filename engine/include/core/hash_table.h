/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Hash table class.
 */

#pragma once

#include "core/hash.h"

#include <unordered_map>
#include <unordered_set>

/** Hash map.
 * @tparam Key          Type of the key. The hashValue() function must be
 *                      implemented for this type.
 * @tparam Value        Type of the value. */
template <typename Key, typename Value> using HashMap = std::unordered_map<Key, Value, Hash<Key>>;

/** Hash map with non-unique keys.
 * @tparam Key          Type of the key. The hashValue() function must be
 *                      implemented for this type.
 * @tparam Value        Type of the value. */
template <typename Key, typename Value> using MultiHashMap = std::unordered_multimap<Key, Value, Hash<Key>>;

/** Hash set (key and value are the same).
 * @tparam Key          Type of the key/value. The hashValue() function must be
 *                      implemented for this type. */
template <typename Key> using HashSet = std::unordered_set<Key, Hash<Key>>;

/** Hash set with non-unique keys.
 * @tparam Key          Type of the key/value. The hashValue() function must be
 *                      implemented for this type. */
template <typename Key> using MultiHashSet = std::unordered_multiset<Key, Hash<Key>>;

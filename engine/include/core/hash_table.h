/*
 * Copyright (C) 2015 Alex Smith
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

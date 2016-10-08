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
 * @brief               Path class.
 */

#pragma once

#include "core/hash.h"

#include <algorithm>

/**
 * Class representing a path.
 *
 * This class stores a path string in a standard format, using '/' as the path
 * separator. Paths stored by this class are always normalized, i.e. extraneous
 * separators are removed, as are components that are just '.'.
 */
class Path {
public:
    /**
     * Initializers.
     */

    /** Initialize the path to refer to the engine base directory. */
    Path() : m_path(".") {}

    /** Copy another path.
     * @param other         Path to copy. */
    Path(const Path &other) : m_path(other.m_path) {}

    /** Move another path.
     * @param other         Path to move. */
    Path(Path &&other) : m_path(std::move(other.m_path)) {}

    /** Convert a string to a path.
     * @param path          String to convert.
     * @param normalized    Whether the string is already normalized. */
    Path(const std::string &path, bool normalized = false) {
        if (normalized) {
            m_path = path;
        } else {
            normalize(path.c_str(), path.length(), m_path);
        }
    }

    /** Convert a string to a path.
     * @param path          String to convert.
     * @param normalized    Whether the string is already normalized. */
    Path(const char *path, bool normalized = false) {
        if (normalized) {
            m_path = path;
        } else {
            normalize(path, strlen(path), m_path);
        }
    }

    /** Override with the value of another string.
     * @param other         Path to copy. */
    Path &operator =(const Path &other) {
        m_path = other.m_path;
        return *this;
    }

    /** Override with the value of another string.
     * @param other         Path to move. */
    Path &operator =(Path &&other) {
        m_path = std::move(other.m_path);
        return *this;
    }

    /**
     * Accessors.
     */

    /** @return             Path string. */
    const std::string &str() const { return m_path; }
    /** @return             Path string. */
    const char *c_str() const { return m_path.c_str(); }

    size_t components() const;
    Path subset(size_t index, size_t count = std::numeric_limits<size_t>::max()) const;

    /**
     * Modifiers.
     */

    Path &operator /=(const Path &path);

    /** Concatenate two paths with a separator between them.
     * @param path          Path to concatenate. If this is an absolute
     *                      path, the returned path will just be this path.
     * @return              Path formed by concatenating the two paths.
     *                      Neither of the input path strings are modified. */
    Path operator /(const Path &path) const {
        if (path.isAbsolute()) {
            return path;
        } else {
            return Path(*this) /= path;
        }
    }

    /** Append a string to the path.
     * @param str           String to append. */
    Path &operator +=(const std::string &str) {
        m_path += str;
        return *this;
    }

    /** Concatenate two path strings.
     * @param str           String to concatenate.
     * @return              Path formed by concatenating this path and the string. */
    Path operator +(const std::string &str) const {
        return Path(*this) += str;
    }

    /**
     * Queries.
     */

    bool isRoot() const;
    bool isAbsoluteRoot() const;
    bool isRelative() const;
    bool isAbsolute() const;
    Path directoryName() const;
    Path fileName() const;
    std::string baseFileName() const;
    std::string extension(bool keepDot = false) const;

    /**
     * Operators.
     */

    /** Compare this path with another. */
    bool operator ==(const Path &other) const {
        return m_path == other.m_path;
    }

    /** Get a hash from a path. */
    friend size_t hashValue(const Path &path) {
        return hashValue(path.m_path);
    }
private:
    static void normalize(const char *path, size_t length, std::string &output);
private:
    std::string m_path;     /**< Path string. */
};

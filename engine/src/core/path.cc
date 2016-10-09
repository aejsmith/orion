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

#include "core/path.h"

/** Platform path separator. */
#ifdef ORION_PLATFORM_WIN32
    static const char kPlatformPathSeparator = '\\';
#else
    static const char kPlatformPathSeparator = '/';
#endif

/* @return              Number of components in the path. */
size_t Path::components() const {
    /* We always have at least one component. Each '/' adds another. */
    size_t count = 1;

    /* Start at position 1, we don't want to count an initial '/' (i.e.
     * absolute path) as another component. */
    for (size_t pos = 1; pos < m_path.length(); pos++) {
        if (m_path[pos] == '/')
            count++;
    }

    return count;
}

/** Get a subset of this path.
 * @param index         First component to include.
 * @param count         Number of components to include. If the subset is
 *                      greater than the total number of components, the
 *                      returned path will be trimmed.
 * @return              Path subset, or "." if range is completely outside the
 *                      path. */
Path Path::subset(size_t index, size_t count) const {
    if (count == 0)
        return Path();

    size_t current = 0;
    size_t start = 0;

    for (size_t pos = 1; pos < m_path.length(); pos++) {
        if (m_path[pos] == '/') {
            current++;

            if (current == index)
                start = pos + 1;

            /* Don't allow it to overflow. */
            if (index + count > index && current == index + count)
                return Path(m_path.substr(start, pos - start), kNormalized);
        }
    }

    return (current >= index) ? Path(m_path.substr(start), kNormalized) : Path();
}

/** Convert to a platform-specific path.
 * @return              Platform-specific path. */
std::string Path::toPlatform() const {
    std::string ret = m_path;
    std::replace(ret.begin(), ret.end(), '/', kPlatformPathSeparator);
    return ret;
}

/** Append a path, adding a separator between them.
 * @param path          Path to append. If this is an absolute path, it
 *                      will entirely replace the current path. */
Path &Path::operator /=(const Path &path) {
    if (path.isAbsolute()) {
        m_path = path.m_path;
        return *this;
    } else if (path.isRoot()) {
        return *this;
    }

    if (&path == this) {
        std::string copy(path.str());
        m_path += '/';
        m_path += copy;
    } else {
        if (isRoot()) {
            m_path = path.m_path;
        } else {
            if (!isAbsoluteRoot())
                m_path += '/';
            m_path += path.m_path;
        }
    }

    return *this;
}

/** @return             Whether the path refers to the engine root. */
bool Path::isRoot() const {
    return m_path.length() == 1 && m_path[0] == '.';
}

/** @return             Whether the path refers to the absolute FS root. */
bool Path::isAbsoluteRoot() const {
    #ifdef ORION_PLATFORM_WIN32
        return m_path.length() == 3 && m_path[1] == ':' && m_path[2] == '/';
    #else
        return m_path.length() == 1 && m_path[0] == '/';
    #endif
}

/** @return             Whether the path is a relative path. */
bool Path::isRelative() const {
    return !isAbsolute();
}

/** @return             Whether the path is an absolute path. */
bool Path::isAbsolute() const {
    #ifdef ORION_PLATFORM_WIN32
        return m_path.length() >= 3 && m_path[1] == ':' && m_path[2] == '/';
    #else
        return m_path[0] == '/';
    #endif
}

/**
 * Return the directory name portion of the path.
 *
 * Returns the directory name portion of the path string. This is everything
 * preceding the last separator in the path, or, if there is no seperator, the
 * engine base directory (".").
 *
 * @return              Directory name portion of the path.
 */
Path Path::directoryName() const {
    size_t pos = m_path.rfind('/');

    if (pos == std::string::npos) {
        return Path();
    } else if (pos == 0) {
        return Path("/", kNormalized);
    } else {
        return Path(m_path.substr(0, pos), kNormalized);
    }
}

/**
 * Return the file name portion of the path.
 *
 * Returns the file name portion of the path string. This is everything after
 * the final separator in the path, or, if there is no separator in the path,
 * the whole path.
 *
 * @return              File name portion of the path.
 */
Path Path::fileName() const {
    size_t pos = m_path.rfind('/');

    if (pos == std::string::npos || (pos == 0 && m_path.length() == 1)) {
        return *this;
    } else {
        return Path(m_path.substr(pos + 1), kNormalized);
    }
}

/**
 * Return the base file name of the path.
 *
 * Returns the base file name of the path. This is the string returned by
 * fileName() with any extension stripped off. File names with only one '.'
 * at the start are treated as having no extension.
 *
 * @return              Base file name.
 */
std::string Path::baseFileName() const {
    Path file = fileName();
    size_t pos = file.m_path.rfind('.');

    if (pos == 0 || pos == std::string::npos) {
        return file.m_path;
    } else {
        return file.m_path.substr(0, pos);
    }
}

/** Return the extension of the file name, if any.
 * @param keepDot       Whether to keep the '.' at the start of the extension.
 * @return              File name extension, or an empty string if no extension. */
std::string Path::extension(bool keepDot) const {
    Path file = fileName();
    size_t pos = file.m_path.rfind('.');

    if (pos == 0 || pos == std::string::npos) {
        return std::string();
    } else {
        return file.m_path.substr((keepDot) ? pos : pos + 1);
    }
}

/**
 * Normalize a path string.
 *
 * Normalizes a path string. Duplicate separators, trailing separators and
 * redundant '.'s are removed. An empty string is turned into '.'.
 *
 * @param path          Path to normalize.
 * @param length        Length of path.
 * @param state         Current normalization state of the string.
 * @param output        String to output normalized path to.
 */
void Path::normalize(const char *path, size_t length, NormalizationState state, std::string &output) {
    if (length == 0 || (length == 1 && path[0] == '.')) {
        output = '.';
        return;
    }

    output.reserve(length);

    bool seenSlash = false;
    bool seenDot = false;

    for (size_t pos = 0; pos < length; pos++) {
        char ch = path[pos];

        if (state == kUnnormalizedPlatform) {
            /* Convert platform-specific path separators. */
            if (ch == kPlatformPathSeparator)
                ch = '/';
        }

        if (ch == '/') {
            if (seenDot) {
                seenDot = false;
            } else if (!seenSlash) {
                output.push_back('/');
            }

            seenSlash = true;
        } else if (ch == '.' && (seenSlash || pos == 0)) {
            seenDot = true;
            seenSlash = false;
        } else {
            if (seenDot) {
                output.push_back('.');
                seenDot = false;
            }

            seenSlash = false;

            output.push_back(ch);
        }
    }

    /* Ensure we haven't written an extraneous '/' at the end. */
    if (output.length() > 1 && output[output.length() - 1] == '/')
        output.pop_back();

    output.shrink_to_fit();
}

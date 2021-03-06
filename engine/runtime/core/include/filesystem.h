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
 * @brief               Filesystem API.
 *
 * Right now this is just a wrapper for a platform-dependent filesystem
 * implementation. Relative paths are relative to the game base directory.
 *
 * In future, when we support some sort of data packages, we will implement a
 * layered system where package files will be layered on top of the base FS.
 * This would cause relative paths to be resolved into the package files, but
 * absolute paths (for example for user data) would be passed down to the
 * underlying platform FS. Multiple packages will be able to be layered on top
 * of each other, so for example patches could be distributed as a package that
 * only changes the necessary files which would be layered onto the base
 * package.
 */

#pragma once

#include "core/data_stream.h"
#include "core/path.h"

#include <algorithm>

/** Filesystem entry type enumeration. */
enum class FileType {
    kFile,                      /**< Regular file. */
    kDirectory,                 /**< Directory. */
    kOther,                     /**< Other. */
};

/** A handle to a regular file allowing I/O on the file. */
class File : public DataStream {
public:
    /** Access modes. */
    enum Mode {
        /** Open for reading. */
        kRead = (1 << 0),
        /** Open for writing. */
        kWrite = (1 << 1),
        /** Create the file if it doesn't exist (use with kWrite). */
        kCreate = (1 << 2),
        /** Truncate the file if it already exists. */
        kTruncate = (1 << 3),
    };
protected:
    File() {}
};

/** A handle to a directory allowing the directory contents to be iterated. */
class Directory : Noncopyable {
public:
    /** A structure describing a directory entry. */
    struct Entry {
        Path name;              /**< Name of the entry. */
        FileType type;          /**< Type of the entry. */
    };
public:
    virtual ~Directory() {}

    /** Reset the directory to the beginning. */
    virtual void reset() = 0;

    /** Get the next directory entry.
     * @note                This API ignores '.' and '..' entries.
     * @param entry         Entry to fill in.
     * @return              True if entry read, false if the end of the
     *                      directory has been reached or an error occurred. */
    virtual bool next(Entry &entry) = 0;
protected:
    Directory() {}
};

/**
 * Interface for accessing the filesystem.
 *
 * These functions provides an interface to access the filesystem. We use a
 * standard path format across all platforms, using '/' as the path separator.
 * Absolute paths always refer to the underlying system FS. However, relative
 * paths into the engine base directory can resolve into package files instead.
 * Whether a package file or the system FS is used is transparent to users of
 * this class.
 */
namespace Filesystem {
    /** Open a file.
     * @param path          Path to file to open.
     * @param mode          Mode to open file with (combination of File::Mode
     *                      flags, defaults to kRead).
     * @return              Pointer to opened file, or null on failure. */
    extern File *openFile(const Path &path, unsigned mode = File::kRead);

    /** Open a directory.
     * @param path          Path to directory to open.
     * @return              Pointer to opened directory, or null on failure. */
    extern Directory *openDirectory(const Path &path);

    /** Check if a path exists.
     * @param path          Path to check.
     * @return              Whether the path exists. */
    extern bool exists(const Path &path);

    /** Check if a path exists and is a certain type.
     * @param path          Path to check.
     * @param type          Type to check for.
     * @return              Whether the path exists and is the specified type. */
    extern bool isType(const Path &path, FileType type);

    /** Set the current working directory.
     * @param path          Path to set to.
     * @return              Whether successful. */
    extern bool setWorkingDirectory(const Path &path);

    /** Get the full path name from a path.
     * @param path          Path to get full path to.
     * @param fullPath      Where to return corresponding absolute path string.
     * @return              Whether successful. */
    extern bool getFullPath(const Path &path, Path &fullPath);
}

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
 * @brief               Win32 filesystem implementation.
 */

#include "core/filesystem.h"

#define NOMINMAX
#include <windows.h>

#include <string.h>

/** Win32 file implementation. */
class Win32File : public File {
public:
    Win32File(HANDLE handle);
    ~Win32File();

    uint64_t size() const override;

    bool read(void *buf, size_t size) override;
    bool write(const void *buf, size_t size) override;
    bool seek(SeekMode mode, int64_t offset) override;
    uint64_t offset() const override;

    bool read(void *buf, size_t size, uint64_t offset) override;
    bool write(const void *buf, size_t size, uint64_t offset) override;
private:
    HANDLE m_handle;                /**< File handle. */
};

/** Win32 directory implementation. */
class Win32Directory : public Directory {
public:
    Win32Directory(const Path &path);
    ~Win32Directory();

    void reset();
    bool next(Entry &entry);
private:
    std::string m_path;             /**< Directory path. */
    HANDLE m_find;                  /**< Find handle. */
};

/** Initialize the file.
 * @param handle        Opened file handle. */
Win32File::Win32File(HANDLE handle) :
    m_handle(handle)
{}

/** Destroy the file. */
Win32File::~Win32File() {
    CloseHandle(m_handle);
}

/** @return             Total file size. */
uint64_t Win32File::size() const {
    LARGE_INTEGER size;
    if (!GetFileSizeEx(m_handle, &size))
        return 0;
    return size.QuadPart;
}

/** Read from the file at the current offset.
 * @param buf           Buffer to read into.
 * @param size          Number of bytes to read.
 * @return              Whether the read was successful. */
bool Win32File::read(void *buf, size_t size) {
    check(size <= std::numeric_limits<DWORD>::max());

    DWORD bytesRead;
    bool ret = ReadFile(m_handle, buf, size, &bytesRead, nullptr);
    return ret && bytesRead == size;
}

/** Write to the file at the current offset.
 * @param buf           Buffer containing data to write.
 * @param size          Number of bytes to write.
 * @return              Whether the write was successful. */
bool Win32File::write(const void *buf, size_t size) {
    check(size <= std::numeric_limits<DWORD>::max());

    DWORD bytesWritten;
    bool ret = WriteFile(m_handle, buf, size, &bytesWritten, nullptr);
    return ret && bytesWritten == size;
}

/** Set the file offset.
 * @param mode          Seek mode.
 * @param offset        Offset value.
 * @return              Whether the seek was successful. */
bool Win32File::seek(SeekMode mode, int64_t offset) {
    DWORD method;

    switch (mode) {
        case SeekMode::kSeekSet:
            method = FILE_BEGIN;
            break;
        case SeekMode::kSeekCurrent:
            method = FILE_CURRENT;
            break;
        case SeekMode::kSeekEnd:
            method = FILE_END;
            break;
        default:
            return false;
    }

    LARGE_INTEGER distance;
    distance.QuadPart = offset;
    return SetFilePointerEx(m_handle, distance, nullptr, method) == 0;
}

/** @return             Current file offset. */
uint64_t Win32File::offset() const {
    LARGE_INTEGER distance, current;
    distance.QuadPart = 0;
    if (!SetFilePointerEx(m_handle, distance, &current, FILE_CURRENT))
        return 0;

    return current.QuadPart;
}

/** Read from the file at the specified offset.
 * @param buf           Buffer to read into.
 * @param size          Number of bytes to read.
 * @param offset        Offset to read from.
 * @return              Whether the read was successful. */
bool Win32File::read(void *buf, size_t size, uint64_t offset) {
    check(size <= std::numeric_limits<DWORD>::max());

    OVERLAPPED overlapped = {};
    overlapped.Offset = static_cast<DWORD>(offset);
    overlapped.OffsetHigh = static_cast<DWORD>(offset >> 32);

    DWORD bytesRead;
    bool ret = ReadFile(m_handle, buf, size, &bytesRead, &overlapped);
    return ret && bytesRead == size;
}

/** Write to the file at the specified offset.
 * @param buf           Buffer containing data to write.
 * @param size          Number of bytes to write.
 * @param offset        Offset to write to.
 * @return              Whether the write was successful. */
bool Win32File::write(const void *buf, size_t size, uint64_t offset) {
    check(size <= std::numeric_limits<DWORD>::max());

    OVERLAPPED overlapped = {};
    overlapped.Offset = static_cast<DWORD>(offset);
    overlapped.OffsetHigh = static_cast<DWORD>(offset >> 32);

    DWORD bytesWritten;
    bool ret = WriteFile(m_handle, buf, size, &bytesWritten, nullptr);
    return ret && bytesWritten == size;
}

/** Initialize the directory.
 * @param path          Path to the directory. */
Win32Directory::Win32Directory(const Path &path) :
    m_find(INVALID_HANDLE_VALUE)
{
    /* To match the entire directory contents we need a wildcard. */
    m_path = path.toPlatform() + "\\*";
}

/** Close the directory. */
Win32Directory::~Win32Directory() {
    reset();
}

/** Reset the directory to the beginning. */
void Win32Directory::reset() {
    if (m_find != INVALID_HANDLE_VALUE) {
        FindClose(m_find);
        m_find = INVALID_HANDLE_VALUE;
    }
}

/** Get the next directory entry.
 * @param entry         Entry to fill in.
 * @return              True if entry read, false if the end of the directory
 *                      has been reached or an error occurred. */
bool Win32Directory::next(Entry &entry) {
    WIN32_FIND_DATA findData;

    while (true) {
        if (m_find == INVALID_HANDLE_VALUE) {
            m_find = FindFirstFile(m_path.c_str(), &findData);
            if (m_find == INVALID_HANDLE_VALUE)
                return false;
        } else {
            if (!FindNextFile(m_find, &findData)) {
                reset();
                return false;
            }
        }

        if (strcmp(findData.cFileName, ".") && strcmp(findData.cFileName, ".."))
            break;
    }

    entry.name = findData.cFileName;

    if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        entry.type = FileType::kDirectory;
    } else {
        entry.type = FileType::kFile;
    }

    return true;
}

/** Open a regular file.
 * @param path          Path to file to open.
 * @param mode          Mode to open file with (combination of File::Mode flags).
 * @return              Pointer to opened file, or null on failure. */
File *Filesystem::openFile(const Path &path, unsigned mode) {
    std::string winPath = path.toPlatform();

    DWORD desiredAccess = 0;
    if (mode & File::kRead)
        desiredAccess = GENERIC_READ;
    if (mode & File::kWrite)
        desiredAccess = GENERIC_WRITE;

    DWORD creationDisposition = OPEN_EXISTING;
    if (mode & File::kCreate) {
        check(mode & File::kWrite);
        if (mode & File::kTruncate) {
            creationDisposition = CREATE_ALWAYS;
        } else {
            creationDisposition = OPEN_ALWAYS;
        }
    } else if (mode & File::kTruncate) {
        creationDisposition = TRUNCATE_EXISTING;
    }

    HANDLE handle = CreateFile(
        winPath.c_str(),
        desiredAccess,
        0,
        nullptr,
        creationDisposition,
        0,
        nullptr);
    if (handle == INVALID_HANDLE_VALUE) {
        logError("Failed to open file '%s': 0x%x", winPath.c_str(), GetLastError());
        return nullptr;
    }

    return new Win32File(handle);
}

/** Open a directory.
 * @param path          Path to directory.
 * @return              Pointer to opened directory, or null on failure. */
Directory *Filesystem::openDirectory(const Path &path) {
    if (!isType(path, FileType::kDirectory))
        return nullptr;

    return new Win32Directory(path);
}

/** Check if a path exists.
 * @param path          Path to check.
 * @return              Whether the path exists. */
bool Filesystem::exists(const Path &path) {
    std::string winPath = path.toPlatform();

    WIN32_FIND_DATA findFileData;
    HANDLE handle = FindFirstFile(winPath.c_str(), &findFileData);
    bool found = handle != INVALID_HANDLE_VALUE;
    if (found)
        FindClose(handle);
    return found;
}

/** Check if a path exists and is a certain type.
 * @param path          Path to check.
 * @param type          Type to check for.
 * @return              Whether the path exists and is the specified type. */
bool Filesystem::isType(const Path &path, FileType type) {
    std::string winPath = path.toPlatform();

    DWORD attributes = GetFileAttributes(winPath.c_str());
    if (attributes == INVALID_FILE_ATTRIBUTES)
        return false;

    switch (type) {
        case FileType::kFile:
            return !(attributes & FILE_ATTRIBUTE_DIRECTORY);
        case FileType::kDirectory:
            return attributes & FILE_ATTRIBUTE_DIRECTORY;
        default:
            return true;
    }
}

/** Set the current working directory.
 * @param path          Path to set to.
 * @return              Whether successful. */
bool Filesystem::setWorkingDirectory(const Path &path) {
    return SetCurrentDirectory(path.c_str());
}

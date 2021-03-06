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
 * @brief               POSIX filesystem implementation.
 */

#include "core/filesystem.h"

#include <sys/stat.h>

#include <dirent.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

/** POSIX file implementation. */
class POSIXFile : public File {
public:
    POSIXFile(int fd);
    ~POSIXFile();

    uint64_t size() const override;

    bool read(void *buf, size_t size) override;
    bool write(const void *buf, size_t size) override;
    bool seek(SeekMode mode, int64_t offset) override;
    uint64_t offset() const override;

    bool read(void *buf, size_t size, uint64_t offset) override;
    bool write(const void *buf, size_t size, uint64_t offset) override;
private:
    int m_fd;                       /**< File descriptor. */
};

/** POSIX directory implementation. */
class POSIXDirectory : public Directory {
public:
    POSIXDirectory(DIR *dir);
    ~POSIXDirectory();

    void reset();
    bool next(Entry &entry);
private:
    DIR *m_dir;                     /**< Directory handle. */
};

/** Initialize the file.
 * @param fd            Opened file descriptor. */
POSIXFile::POSIXFile(int fd) : m_fd(fd) {}

/** Destroy the file. */
POSIXFile::~POSIXFile() {
    close(m_fd);
}

/** @return             Total file size. */
uint64_t POSIXFile::size() const {
    struct stat st;
    int ret = fstat(m_fd, &st);
    if (ret != 0)
        return 0;

    return st.st_size;
}

/** Read from the file at the current offset.
 * @param buf           Buffer to read into.
 * @param size          Number of bytes to read.
 * @return              Whether the read was successful. */
bool POSIXFile::read(void *buf, size_t size) {
    return ::read(m_fd, buf, size) == static_cast<ssize_t>(size);
}

/** Write to the file at the current offset.
 * @param buf           Buffer containing data to write.
 * @param size          Number of bytes to write.
 * @return              Whether the write was successful. */
bool POSIXFile::write(const void *buf, size_t size) {
    return ::write(m_fd, buf, size) == static_cast<ssize_t>(size);
}

/** Set the file offset.
 * @param mode          Seek mode.
 * @param offset        Offset value.
 * @return              Whether the seek was successful. */
bool POSIXFile::seek(SeekMode mode, int64_t offset) {
    int whence;

    switch (mode) {
        case SeekMode::kSeekSet:
            whence = SEEK_SET;
            break;
        case SeekMode::kSeekCurrent:
            whence = SEEK_CUR;
            break;
        case SeekMode::kSeekEnd:
            whence = SEEK_END;
            break;
        default:
            return false;
    }

    return lseek(m_fd, offset, whence) == 0;
}

/** @return             Current file offset. */
uint64_t POSIXFile::offset() const {
    return lseek(m_fd, SEEK_CUR, 0);
}

/** Read from the file at the specified offset.
 * @param buf           Buffer to read into.
 * @param size          Number of bytes to read.
 * @param offset        Offset to read from.
 * @return              Whether the read was successful. */
bool POSIXFile::read(void *buf, size_t size, uint64_t offset) {
    return pread(m_fd, buf, size, offset) == static_cast<ssize_t>(size);
}

/** Write to the file at the specified offset.
 * @param buf           Buffer containing data to write.
 * @param size          Number of bytes to write.
 * @param offset        Offset to write to.
 * @return              Whether the write was successful. */
bool POSIXFile::write(const void *buf, size_t size, uint64_t offset) {
    return pwrite(m_fd, buf, size, offset) == static_cast<ssize_t>(size);
}

/** Initialize the directory.
 * @param dir           Opened directory handle. */
POSIXDirectory::POSIXDirectory(DIR *dir) : m_dir(dir) {}

/** Close the directory. */
POSIXDirectory::~POSIXDirectory() {
    closedir(m_dir);
}

/** Reset the directory to the beginning. */
void POSIXDirectory::reset() {
    rewinddir(m_dir);
}

/** Get the next directory entry.
 * @param entry         Entry to fill in.
 * @return              True if entry read, false if the end of the directory
 *                      has been reached or an error occurred. */
bool POSIXDirectory::next(Entry &entry) {
    struct dirent *dent;

    while (true) {
        dent = readdir(m_dir);
        if (!dent)
            return false;

        if (strcmp(dent->d_name, ".") && strcmp(dent->d_name, ".."))
            break;
    }

    entry.name = dent->d_name;
    switch (dent->d_type) {
        case DT_REG:
            entry.type = FileType::kFile;
            break;
        case DT_DIR:
            entry.type = FileType::kDirectory;
            break;
        default:
            entry.type = FileType::kOther;
            break;
    }

    return true;
}

/** Open a regular file.
 * @param path          Path to file to open.
 * @param mode          Mode to open file with (combination of File::Mode flags).
 * @return              Pointer to opened file, or null on failure. */
File *Filesystem::openFile(const Path &path, unsigned mode) {
    int flags = 0;

    if (mode & File::kRead)
        flags |= O_RDONLY;
    if (mode & File::kWrite)
        flags |= O_WRONLY;
    if (mode & File::kCreate) {
        check(mode & File::kWrite);
        flags |= O_CREAT;
    }
    if (mode & File::kTruncate)
        flags |= O_TRUNC;

    int fd = open(path.c_str(), flags, 0644);
    if (fd < 0)
        return nullptr;

    return new POSIXFile(fd);
}

/** Open a directory.
 * @param path          Path to directory.
 * @return              Pointer to opened directory, or null on failure. */
Directory *Filesystem::openDirectory(const Path &path) {
    DIR *dir = opendir(path.c_str());
    if (!dir)
        return nullptr;

    return new POSIXDirectory(dir);
}

/** Check if a path exists.
 * @param path          Path to check.
 * @return              Whether the path exists. */
bool Filesystem::exists(const Path &path) {
    struct stat st;
    return stat(path.c_str(), &st) == 0;
}

/** Check if a path exists and is a certain type.
 * @param path          Path to check.
 * @param type          Type to check for.
 * @return              Whether the path exists and is the specified type. */
bool Filesystem::isType(const Path &path, FileType type) {
    struct stat st;
    if (stat(path.c_str(), &st) != 0)
        return false;

    switch (type) {
        case FileType::kFile:
            return S_ISREG(st.st_mode);
        case FileType::kDirectory:
            return S_ISDIR(st.st_mode);
        default:
            return true;
    }
}

/** Set the current working directory.
 * @param path          Path to set to.
 * @return              Whether successful. */
bool Filesystem::setWorkingDirectory(const Path &path) {
    return chdir(path.c_str()) == 0;
}

/** Get the full path name from a path.
 * @param path          Path to get full path to.
 * @param fullPath      Where to return corresponding absolute path string.
 * @return              Whether successful. */
bool Filesystem::getFullPath(const Path &path, Path &fullPath) {
    char *str = realpath(path.c_str(), nullptr);
    if (!str)
        return false;

    fullPath = Path(str, Path::kNormalized);
    free(str);
    return true;
}

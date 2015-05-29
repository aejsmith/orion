/**
 * @file
 * @copyright           2014 Alex Smith
 * @brief               POSIX filesystem implementation.
 */

#include "core/filesystem.h"

#include <SDL.h>

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

/** POSIX filesystem interface. */
class POSIXFilesystem : public Filesystem {
public:
    File *openFile(const Path &path, unsigned mode = File::kRead) override;
    Directory *openDirectory(const Path &path) override;

    bool exists(const Path &path) override;
    bool isType(const Path &path, FileType type) override;
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
    struct dirent dent;
    struct dirent *result;

    while (true) {
        int ret = readdir_r(m_dir, &dent, &result);
        if (ret != 0 || !result)
            return false;

        if (strcmp(dent.d_name, ".") && strcmp(dent.d_name, ".."))
            break;
    }

    entry.name = dent.d_name;
    switch (dent.d_type) {
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
File *POSIXFilesystem::openFile(const Path &path, unsigned mode) {
    int flags = 0;

    if (mode & File::kRead)
        flags |= O_RDONLY;
    if (mode & File::kWrite)
        flags |= O_WRONLY;

    int fd = open(path.c_str(), flags);
    if (fd < 0)
        return nullptr;

    return new POSIXFile(fd);
}

/** Open a directory.
 * @param path          Path to directory.
 * @return              Pointer to opened directory, or null on failure. */
Directory *POSIXFilesystem::openDirectory(const Path &path) {
    DIR *dir = opendir(path.c_str());
    if (!dir)
        return nullptr;

    return new POSIXDirectory(dir);
}

/** Check if a path exists.
 * @param path          Path to check.
 * @return              Whether the path exists. */
bool POSIXFilesystem::exists(const Path &path) {
    struct stat st;
    return stat(path.c_str(), &st) == 0;
}

/** Check if a path exists and is a certain type.
 * @param path          Path to check.
 * @param type          Type to check for.
 * @return              Whether the path exists and is the specified type. */
bool POSIXFilesystem::isType(const Path &path, FileType type) {
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

/** Initialize the platform filesystem interface.
 * @return              Pointer to Filesystem object. */
Filesystem *Platform::createFilesystem() {
    /* Switch to the engine base directory. SDL_GetBasePath returns the
     * binary directory, the base directory is above that. */
    char *basePath = SDL_GetBasePath();
    std::string path(basePath);
    path += "..";

    int ret = chdir(path.c_str());
    if (ret != 0)
        fatal("Failed to change to engine directory '%s'", path.c_str());
    SDL_free(basePath);

    return new POSIXFilesystem;
}

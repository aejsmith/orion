/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Platform-independent filesystem API.
 */

#ifndef ORION_LIB_FS_H
#define ORION_LIB_FS_H

#include "lib/data_stream.h"

#include <string>

/** Filesystem entry type enumeration. */
enum class FileType {
	kFile,				/**< Regular file. */
	kDirectory,			/**< Directory. */
	kOther,				/**< Other. */
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
	};
protected:
	File() {}
};

/** A handle to a directory allowing the directory contents to be iterated. */
class Directory : Noncopyable {
public:
	/** A structure describing a directory entry. */
	struct Entry {
		std::string name;	/**< Name of the entry. */
		FileType type;		/**< Type of the entry. */
	};
public:
	virtual ~Directory() {}

	/** Reset the directory to the beginning. */
	virtual void reset() = 0;

	/** Get the next directory entry.
	 * @note		This API ignores '.' and '..' entries.
	 * @param entry		Entry to fill in.
	 * @return		True if entry read, false if the end of the
	 *			directory has been reached or an error occurred. */
	virtual bool next(Entry &entry) = 0;
protected:
	Directory() {}
};

/**
 * Global FS API, implemented by the platform.
 */

namespace fs {

/** Open a regular file.
 * @param path		Path to file to open.
 * @param mode		Mode to open file with (combination of File::Mode flags).
 * @return		Pointer to opened file, or null on failure. */
extern File *open_file(const std::string &path, unsigned mode = File::kRead);

/** Open a directory.
 * @param path		Path to directory.
 * @return		Pointer to opened directory, or null on failure. */
extern Directory *open_directory(const std::string &path);

/** Check if a path exists.
 * @param path		Path to check.
 * @return		Whether the path exists. */
extern bool exists(const std::string &path);

/** Check if a path exists and is a certain type.
 * @param path		Path to check.
 * @param type		Type to check for.
 * @return		Whether the path exists and is the specified type. */
extern bool is_type(const std::string &path, FileType type);

/* TODO: App data/user data path functions. */

}

#endif /* ORION_LIB_FS_H */

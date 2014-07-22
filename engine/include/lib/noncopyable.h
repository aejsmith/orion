/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Noncopyable base class.
 */

#ifndef ORION_LIB_NONCOPYABLE_H
#define ORION_LIB_NONCOPYABLE_H

/** Base class ensuring derived classes are not copyable. */
struct Noncopyable {
	Noncopyable &operator=(const Noncopyable &) = delete;
	Noncopyable(const Noncopyable &) = delete;
	Noncopyable() = default;
};

#endif /* ORION_LIB_NONCOPYABLE_H */

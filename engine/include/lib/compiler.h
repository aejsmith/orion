/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Compiler-specific macros/definitions.
 */

#ifndef ORION_LIB_COMPILER_H
#define ORION_LIB_COMPILER_H

#ifdef __GNUC__
# define PACKED			__attribute__((packed))
# define ALIGNED(a)		__attribute__((aligned(a)))
# define NORETURN		__attribute__((noreturn))
# define likely(x)		__builtin_expect(!!(x), 1)
# define unlikely(x)		__builtin_expect(!!(x), 0)
#else
# error "Compiler is not supported"
#endif

#endif /* ORION_LIB_COMPILER_H */

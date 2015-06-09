/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Input information definitions.
 */

#pragma once

#include "input/defs.h"

/** Information about a single input. */
struct InputInfo {
    InputCode code;                 /**< Code for the input. */
    const char *name;               /**< Name of the input. */
    InputType type;                 /**< Type of the input. */
public:
    InputInfo(InputCode inCode, const char *inName, InputType inType) :
        code(inCode),
        name(inName),
        type(inType)
    {}

    static const InputInfo *lookup(InputCode code);
    static const InputInfo *lookup(const char *name);
};

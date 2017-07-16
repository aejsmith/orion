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
        code (inCode),
        name (inName),
        type (inType)
    {}

    static const InputInfo *lookup(InputCode code);
    static const InputInfo *lookup(const char *name);
};

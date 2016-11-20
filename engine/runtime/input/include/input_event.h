/*
 * Copyright (C) 2015-2016 Alex Smith
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
 * @brief               Input event structure.
 */

#pragma once

#include "input/input_info.h"

/** Base input event structure. */
struct InputEvent {
    InputCode code;                 /**< Input that is being performed. */
    const InputInfo *info;          /**< Information about the input. */
    uint32_t modifiers;             /**< Current modifier state (bitmap of InputModifier values). */
public:
    InputEvent(const InputInfo *inInfo, uint32_t inModifiers) :
        code(inInfo->code),
        info(inInfo),
        modifiers(inModifiers)
    {}
};

/** Details of a button up/down event. */
struct ButtonEvent : InputEvent {
    /**
     * Character corresponding to the button pressed.
     *
     * This gives a textual representation, if any, of a button pressed. While
     * the raw input codes correspond to physical key positions, irrespective
     * of layout, this gives the representation of the key for the user's
     * keyboard layout. If a key has no textual representation, this will be 0.
     */
    char character;
public:
    ButtonEvent(const InputInfo *inInfo, uint32_t inModifiers, char inCharacter) :
        InputEvent(inInfo, inModifiers),
        character(inCharacter)
    {}
};

/** Details of a axis movement event. */
struct AxisEvent : InputEvent {
    /**
     * Movement delta.
     *
     * This gives the delta change on the axis. Scale of this value depends on
     * the type of axis. For mouse movement, it gives the delta change in
     * pixels. For mouse scrolling, it gives the number of positions scrolled
     * (positive is up, negative is down).
     */
    float delta;
public:
    AxisEvent(const InputInfo *inInfo, uint32_t inModifiers, float inDelta) :
        InputEvent(inInfo, inModifiers),
        delta(inDelta)
    {}
};

/** Details of a text input event. */
struct TextInputEvent {
    /** Text that was input. */
    std::string text;
public:
    TextInputEvent(std::string inText) :
        text(std::move(inText))
    {}
};

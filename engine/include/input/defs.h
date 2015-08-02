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
 * @brief               Input basic definitions.
 */

#pragma once

#include "core/core.h"

/** Input type enumeration. */
enum class InputType {
    /** Discrete button (e.g. keyboard/mouse/gamepad button). */
    kButton,

    /** Continuous axis (e.g. mouse movement/gamepad analog stick). */
    kAxis,
};

/**
 * Input code enumeration.
 *
 * This enumeration is used to identify a physical input from an input device.
 * Input codes for keyboard keys are independent of whatever keyboard layout the
 * user has set, it is a fixed layout based on a standard US keyboard.
 */
enum class InputCode {
    /**
     * Keyboard codes.
     *
     * The values here are based on the USB keyboard usage page standard,
     * the same as what SDL uses. This simplifies conversion from SDL
     * definitions to internal ones.
     */
    kA = 4,
    kB = 5,
    kC = 6,
    kD = 7,
    kE = 8,
    kF = 9,
    kG = 10,
    kH = 11,
    kI = 12,
    kJ = 13,
    kK = 14,
    kL = 15,
    kM = 16,
    kN = 17,
    kO = 18,
    kP = 19,
    kQ = 20,
    kR = 21,
    kS = 22,
    kT = 23,
    kU = 24,
    kV = 25,
    kW = 26,
    kX = 27,
    kY = 28,
    kZ = 29,
    k1 = 30,
    k2 = 31,
    k3 = 32,
    k4 = 33,
    k5 = 34,
    k6 = 35,
    k7 = 36,
    k8 = 37,
    k9 = 38,
    k0 = 39,
    kReturn = 40,
    kEscape = 41,
    kBackspace = 42,
    kTab = 43,
    kSpace = 44,
    kMinus = 45,
    kEquals = 46,
    kLeftBracket = 47,
    kRightBracket = 48,
    kBackslash = 49,
    kSemicolon = 51,
    kApostrophe = 52,
    kGrave = 53,
    kComma = 54,
    kPeriod = 55,
    kSlash = 56,
    kCapsLock = 57,
    kF1 = 58,
    kF2 = 59,
    kF3 = 60,
    kF4 = 61,
    kF5 = 62,
    kF6 = 63,
    kF7 = 64,
    kF8 = 65,
    kF9 = 66,
    kF10 = 67,
    kF11 = 68,
    kF12 = 69,
    kPrintScreen = 70,
    kScrollLock = 71,
    kPause = 72,
    kInsert = 73,
    kHome = 74,
    kPageUp = 75,
    kDelete = 76,
    kEnd = 77,
    kPageDown = 78,
    kRight = 79,
    kLeft = 80,
    kDown = 81,
    kUp = 82,
    kNumLock = 83,
    kKPDivide = 84,
    kKPMultiply = 85,
    kKPMinus = 86,
    kKPPlus = 87,
    kKPEnter = 88,
    kKP1 = 89,
    kKP2 = 90,
    kKP3 = 91,
    kKP4 = 92,
    kKP5 = 93,
    kKP6 = 94,
    kKP7 = 95,
    kKP8 = 96,
    kKP9 = 97,
    kKP0 = 98,
    kKPPeriod = 99,
    kNonUSBackslash = 100,
    kApplication = 101,
    kKPEquals = 103,
    kLeftCtrl = 224,
    kLeftShift = 225,
    kLeftAlt = 226,
    kLeftSuper = 227,
    kRightCtrl = 228,
    kRightShift = 229,
    kRightAlt = 230,
    kRightSuper = 231,
    kNumKeyboardCodes,

    /**
     * Mouse codes.
     */
    kMouseX,
    kMouseY,
    kMouseScroll,
    kMouseLeft,
    kMouseRight,
    kMouseMiddle,
    kNumMouseCodes,

    kNumInputCodes,
};

/**
 * Keyboard modifier enumeration.
 *
 * Enumeration of possible keyboard modifiers bitmasks. These can be OR'd
 * together.
 */
namespace InputModifier {
    enum : uint32_t {
        kNone = 0,

        kLeftShift = (1 << 0),
        kRightShift = (1 << 1),
        kShift = (kLeftShift | kRightShift),

        kLeftCtrl = (1 << 2),
        kRightCtrl = (1 << 3),
        kCtrl = (kLeftCtrl | kRightCtrl),

        kLeftAlt = (1 << 4),
        kRightAlt = (1 << 5),
        kAlt = (kLeftAlt | kRightAlt),

        kLeftSuper = (1 << 6),
        kRightSuper = (1 << 7),
        kSuper = (kLeftSuper | kRightSuper),

        kNumLock = (1 << 8),
        kCapsLock = (1 << 9),
    };
}

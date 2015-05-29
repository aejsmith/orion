/**
 * @file
 * @copyright           2014 Alex Smith
 * @brief               String utility functions.
 */

#pragma once

#include "core/defs.h"

namespace String {
    /** Split a string into tokens.
     * @param str           String to split.
     * @param tokens        Vector to fill with tokens (existing content left intact).
     * @param delimiters    Delimiter characters (defaults to " ").
     * @param trimEmpty     Whether to ignore empty tokens (defaults to true). */
    template <typename Container>
    void tokenize(const std::string &str, Container &tokens, const char *delimiters = " ", bool trimEmpty = true) {
        size_t last = 0;
        size_t pos = 0;

        while (pos != std::string::npos) {
            pos = str.find_first_of(delimiters, last);
            if (!trimEmpty || last != ((pos == std::string::npos) ? str.length() : pos))
                tokens.emplace_back(str, last, pos - last);
            last = pos + 1;
        }
    }

    extern std::string format(const char *fmt, va_list args);
    extern std::string format(const char *fmt, ...);
}

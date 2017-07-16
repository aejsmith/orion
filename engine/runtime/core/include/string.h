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
 * @brief               String utility functions.
 */

#pragma once

#include "core/defs.h"

namespace String {
    /** Split a string into tokens.
     * @param str           String to split.
     * @param tokens        Container to fill with tokens (existing content left
     *                      intact).
     * @param delimiters    Delimiter characters (defaults to " ").
     * @param maxTokens     Maximum number of tokens (-1 for no limit, the default).
     *                      After this limit is reached the remainder of the string
     *                      is added to the last token.
     * @param trimEmpty     Whether to ignore empty tokens (defaults to true). */
    template <typename Container>
    void tokenize(const std::string &str,
                  Container &tokens,
                  const char *delimiters = " ",
                  int maxTokens = -1,
                  bool trimEmpty = true)
    {
        size_t last = 0;
        size_t pos = 0;
        int numTokens = 0;

        while (pos != std::string::npos) {
            if (maxTokens > 0 && numTokens == maxTokens - 1) {
                tokens.emplace_back(str, last);
                break;
            } else {
                pos = str.find_first_of(delimiters, last);

                if (!trimEmpty || last != ((pos == std::string::npos) ? str.length() : pos))
                    tokens.emplace_back(str, last, pos - last);

                last = pos + 1;
                numTokens++;
            }
        }
    }

    extern std::string vformat(const char *fmt, va_list args);
    extern std::string format(const char *fmt, ...);
}

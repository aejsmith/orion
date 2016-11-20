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
 * @brief               Data stream utility functions.
 */

#include "core/data_stream.h"

/** Read from the stream until the next line break.
 * @param line          String to fill with line content. The actual line
 *                      terminator will not be written.
 * @return              Whether the line was successfully read. */
bool DataStream::readLine(std::string &line) {
    line.clear();

    /* Reserve space in the string so we're not repeatedly reallocating. */
    line.reserve(256);

    char ch;
    while (read(&ch, 1) && ch != '\n')
        line.push_back(ch);

    /* Shrink down to actual size. */
    line.reserve();
    return ch == '\n' || line.length() != 0;
}

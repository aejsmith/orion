/*
 * Copyright (C) 2016 Alex Smith
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
 * @brief               Linux helper functions.
 */

#include "core/path.h"
#include "core/platform.h"

#include <errno.h>
#include <unistd.h>

/** Get the program executable name (without extensions).
 * @return              Program executable name. */
std::string Platform::getProgramName() {
    std::string str;
    str.resize(PATH_MAX);

    int ret = readlink("/proc/self/exe", &str[0], PATH_MAX - 1);
    if (ret == -1)
        fatal("Failed to get program name: %s", strerror(errno));

    str[ret] = 0;
    str.resize(ret);
    return Path(str).baseFileName();
}

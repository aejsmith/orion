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
 * @brief               Data stream interface.
 */

#pragma once

#include "core/core.h"

/** Interface to a data stream. */
class DataStream : Noncopyable {
public:
    /** Seek modes. */
    enum SeekMode {
        kSeekSet,               /**< Set the offset to the specified value. */
        kSeekCurrent,           /**< Set the offset relative to the current offset. */
        kSeekEnd,               /**< Set the offset relative to the end of the file. */
    };
public:
    /** Close the stream. */
    virtual ~DataStream() {}

    /**
     * Stream properties.
     */

    /** @return             Total stream size. */
    virtual uint64_t size() const = 0;

    /**
     * Stored offset I/O.
     */

    /** Read from the stream at the current offset.
     * @param buf           Buffer to read into.
     * @param size          Number of bytes to read.
     * @return              Whether the read was successful. */
    virtual bool read(void *buf, size_t size) = 0;

    /** Write to the stream at the current offset.
     * @param buf           Buffer containing data to write.
     * @param size          Number of bytes to write.
     * @return              Whether the write was successful. */
    virtual bool write(const void *buf, size_t size) = 0;

    /** Set the stream offset.
     * @param mode          Seek mode.
     * @param offset        Offset value.
     * @return              Whether the seek was successful. */
    virtual bool seek(SeekMode mode, int64_t offset) = 0;

    /** @return             Current stream offset. */
    virtual uint64_t offset() const = 0;

    bool readLine(std::string &line);

    /**
     * Specific offset I/O.
     */

    /** Read from the stream at the specified offset.
     * @param buf           Buffer to read into.
     * @param size          Number of bytes to read.
     * @param offset        Offset to read from.
     * @return              Whether the read was successful. */
    virtual bool read(void *buf, size_t size, uint64_t offset) = 0;

    /** Write to the stream at the specified offset.
     * @param buf           Buffer containing data to write.
     * @param size          Number of bytes to write.
     * @param offset        Offset to write to.
     * @return              Whether the write was successful. */
    virtual bool write(const void *buf, size_t size, uint64_t offset) = 0;
protected:
    DataStream() {}
};

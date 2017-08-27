/*
 * Copyright (C) 2017 Alex Smith
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
 * @brief               GPU query pool class.
 */

#pragma once

#include "gpu/defs.h"

struct GPUQueryPoolDesc;

/** Class implementing a pool of GPU queries. */
class GPUQueryPool : public GPUObject {
public:
    /** Type of a query. */
    enum Type {
        kTimestampQuery,                /**< Timestamp query. */
    };
public:
    /** Reset a range of queries.
     * @param start         Start of the range.
     * @param count         Number of queries to reset. */
    virtual void reset(uint32_t start, uint32_t count) = 0;

    /**
     * Get results from submitted queries.
     *
     * Waits to get results from a range of submitted queries. If specified,
     * the current command stream will be submitted to the device before
     * attempting to get the query results. This is needed if any of the queries
     * were submitted within the current frame, otherwise this function will
     * hang indefinitely.
     *
     * @param start         Start of the range.
     * @param count         Number of queries to get results for.
     * @param data          Array to store results in.
     * @param flush         Whether to flush the command stream.
     */
    virtual void getResults(uint32_t start,
                            uint32_t count,
                            uint64_t *data,
                            bool flush = false) = 0;
protected:
    explicit GPUQueryPool(const GPUQueryPoolDesc &desc);
    ~GPUQueryPool() {}
protected:
    GPUQueryPool::Type m_type;          /**< Type of the queries. */
    uint32_t m_count;                   /**< Number of queries. */
};

/** Type of a pointer to a GPUQueryPool. */
using GPUQueryPoolPtr = GPUObjectPtr<GPUQueryPool>;

/** Descriptor for a GPUQueryPool. */
struct GPUQueryPoolDesc {
    GPUQueryPool::Type type;            /**< Type of the queries. */
    uint32_t count;                     /**< Number of queries. */

    SET_DESC_PARAMETER(setType, GPUQueryPool::Type, type);
    SET_DESC_PARAMETER(setCount, uint32_t, count);
};

/** Initialise the pool.
 * @param desc          Descriptor for the pool. */
inline GPUQueryPool::GPUQueryPool(const GPUQueryPoolDesc &desc) :
    m_type  (desc.type),
    m_count (desc.count)
{}

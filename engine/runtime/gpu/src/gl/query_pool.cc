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
 * @brief               GL query pool class.
 */

#include "query_pool.h"

/** Initialise the query pool.
 * @param desc          Descriptor for the pool. */
GLQueryPool::GLQueryPool(const GPUQueryPoolDesc &desc) :
    GPUQueryPool (desc),
    m_queries    (desc.count)
{
    check(m_type == kTimestampQuery);

    glGenQueries(desc.count, &m_queries[0]);
}

/** Destroy the pool. */
GLQueryPool::~GLQueryPool() {
    glDeleteQueries(m_count, &m_queries[0]);
}

/** Reset a range of queries.
 * @param start         Start of the range.
 * @param count         Number of queries to reset. */
void GLQueryPool::reset(uint32_t start, uint32_t count) {
    /* Nothing happens. */
}

/** Get results from submitted queries.
 * @param start         Start of the range.
 * @param count         Number of queries to get results for.
 * @param data          Array to store results in.
 * @param flush         Whether to flush the command stream. */
void GLQueryPool::getResults(uint32_t start,
                             uint32_t count,
                             uint64_t *data,
                             bool flush)
{
    for (uint32_t i = 0; i < count; i++)
        glGetQueryObjectui64v(m_queries[start + i], GL_QUERY_RESULT, &data[i]);
}

/** End a query.
 * @param index         Index of the query. */
void GLQueryPool::end(uint32_t index) {
    switch (m_type) {
        case kTimestampQuery:
            glQueryCounter(m_queries[index], GL_TIMESTAMP);
            break;

        default:
            unreachable();

    }
}

/** Create a query pool.
 * @param desc          Descriptor for the query pool.
 * @return              Pointer to created pool. */
GPUQueryPoolPtr GLGPUManager::createQueryPool(const GPUQueryPoolDesc &desc) {
    return new GLQueryPool(desc);
}

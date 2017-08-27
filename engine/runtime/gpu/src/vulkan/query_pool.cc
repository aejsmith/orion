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
 * @brief               Vulkan query pool class.
 */

#include "manager.h"
#include "query_pool.h"

/** Initialise the query pool.
 * @param manager       Manager owning this pool.
 * @param desc          Descriptor for the pool. */
VulkanQueryPool::VulkanQueryPool(VulkanGPUManager *manager,
                                 const GPUQueryPoolDesc &desc) :
    GPUQueryPool (desc),
    VulkanHandle (manager)
{
    check(m_type == kTimestampQuery);

    VkQueryPoolCreateInfo createInfo = {};
    createInfo.sType      = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    createInfo.queryType  = VK_QUERY_TYPE_TIMESTAMP;
    createInfo.queryCount = m_count;

    checkVk(vkCreateQueryPool(manager->device()->handle(),
                              &createInfo,
                              nullptr,
                              &m_handle));

    /* Queries are initially in undefined state. */
    reset(0, m_count);
}

/** Destroy the pool. */
VulkanQueryPool::~VulkanQueryPool() {
    vkDestroyQueryPool(manager()->device()->handle(), m_handle, nullptr);
}

/** Reset a range of queries.
 * @param start         Start of the range.
 * @param count         Number of queries to reset. */
void VulkanQueryPool::reset(uint32_t start, uint32_t count) {
    vkCmdResetQueryPool(manager()->currentFrame().primaryCmdBuf->handle(),
                        m_handle,
                        start, count);
}

/** Get results from submitted queries.
 * @param start         Start of the range.
 * @param count         Number of queries to get results for.
 * @param data          Array to store results in.
 * @param flush         Whether to flush the command stream. */
void VulkanQueryPool::getResults(uint32_t start,
                                 uint32_t count,
                                 uint64_t *data,
                                 bool flush)
{
    if (flush)
        manager()->flush();

    // FIXME: After a reset we should ensure that we wait until the reset has
    // been executed before trying to wait on the query result. For MicroProfile
    // this is sufficient because it waits long enough before trying to get
    // results.
    checkVk(vkGetQueryPoolResults(manager()->device()->handle(),
                                  m_handle,
                                  start, count,
                                  sizeof(*data) * count,
                                  data,
                                  sizeof(*data),
                                  VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT));

    /* Convert timestamps to nanoseconds. */
    const auto &limits = manager()->device()->limits();
    if (m_type == kTimestampQuery && limits.timestampPeriod != 1.0f) {
        for (uint32_t i = 0; i < count; i++)
            data[i] = static_cast<double>(data[i]) * limits.timestampPeriod;
    }
}

/** End a query.
 * @param index         Index of the query.
 * @param cmdBuf        Command buffer to end on. */
void VulkanQueryPool::end(uint32_t index, VulkanCommandBuffer *cmdBuf) {
    switch (m_type) {
        case kTimestampQuery:
            vkCmdWriteTimestamp(cmdBuf->handle(),
                                VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                m_handle,
                                index);
            break;

        default:
            unreachable();

    }
}

/** Create a query pool.
 * @param desc          Descriptor for the query pool.
 * @return              Pointer to created pool. */
GPUQueryPoolPtr VulkanGPUManager::createQueryPool(const GPUQueryPoolDesc &desc) {
    return new VulkanQueryPool(this, desc);
}

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
 * @brief               Vulkan queue class.
 */

#include "manager.h"
#include "queue.h"

/** Create a queue object managing a device queue.
 * @param manager       Manager that owns this queue.
 * @param queueFamily   Queue family that queue belongs to.
 * @param index         Index of the queue. */
VulkanQueue::VulkanQueue(VulkanGPUManager *manager, uint32_t queueFamily, uint32_t index) :
    VulkanHandle(manager)
{
    vkGetDeviceQueue(manager->device()->handle(), queueFamily, index, &m_handle);
}

/** Submit a command buffer.
 * @param cmdBuf        Command buffer to submit.
 * @param wait          Optional semaphore to wait on before executing.
 * @param waitStages    Pipeline stages wait should take place at.
 * @param signal        Optional semaphore to signal after execution is complete.
 * @param fence         Optional fence to signal upon completion. */
void VulkanQueue::submit(VulkanCommandBuffer *cmdBuf,
                         VulkanSemaphore *wait,
                         VkPipelineStageFlags waitStages,
                         VulkanSemaphore *signal,
                         VulkanFence *fence)
{
    check(cmdBuf->m_state == VulkanCommandBuffer::State::kRecorded);

    VkCommandBuffer handle = cmdBuf->handle();

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &handle;
    submitInfo.waitSemaphoreCount = (wait) ? 1 : 0;
    VkSemaphore waitHandle = (wait) ? wait->handle() : VK_NULL_HANDLE;
    submitInfo.pWaitSemaphores = &waitHandle;
    submitInfo.pWaitDstStageMask = &waitStages;
    submitInfo.signalSemaphoreCount = (signal) ? 1 : 0;
    VkSemaphore signalHandle = (signal) ? signal->handle() : VK_NULL_HANDLE;
    submitInfo.pSignalSemaphores = &signalHandle;

    checkVk(vkQueueSubmit(m_handle,
                          1, &submitInfo,
                          (fence) ? fence->handle() : VK_NULL_HANDLE));

    cmdBuf->m_state = VulkanCommandBuffer::State::kSubmitted;
}

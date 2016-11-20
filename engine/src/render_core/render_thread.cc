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
 * @brief               Rendering thread class.
 */

#include "render_core/render_thread.h"

/** Start the render thread. */
RenderThread::RenderThread() :
    m_sync(0)
{
    /* Start the thread. */
    m_thread = std::thread([this] () { this->run(); });
}

/** Destroy the render thread. */
RenderThread::~RenderThread() {
    /* This tells run() to exit. */
    std::unique_lock<std::mutex> lock(m_lock);
    m_sync = 2;
    m_conditions[0].notify_one();
    lock.unlock();

    m_thread.join();
}

/**
 * Submit work to the render thread.
 *
 * Called by the game thread at the end of a frame to tell it to process all the
 * messages that have been queued to it. This also synchronises with the thread
 * to ensure that it always stays at most 1 frame behind the game thread. After
 * this returns, the submitted messages will have been taken by the render
 * thread and it will continue independently, so the game thread is free to
 * start queueing up new messages.
 */
void RenderThread::submit() {
    /* Indicate to the render thread that we have work available. */
    std::unique_lock<std::mutex> lock(m_lock);
    m_sync = 1;
    m_conditions[0].notify_one();

    /* Wait for it to finish its current work and take over the message buffers.
     * When this returns we are free to continue. */
    m_conditions[1].wait(lock, [this] { return m_sync == 0; });
}

/** Allocate space for a message.
 * @param size          Size required for the message.
 * @return              Pointer to the allocated buffer space. */
void *RenderThread::allocateMessage(size_t size) {
    size_t alignedSize = Math::roundUp(size, 16);

    if (!m_messageBuffers.empty()) {
        MessageBuffer &buffer = m_messageBuffers.back();
        if (size <= MessageBuffer::kMaxSize - buffer.nextOffset) {
            void *ret = &buffer.data[buffer.nextOffset];

            const size_t maxSize = MessageBuffer::kMaxSize;
            buffer.nextOffset = std::min(buffer.nextOffset + alignedSize, maxSize);

            return ret;
        }
    }

    m_messageBuffers.emplace_back();
    MessageBuffer &buffer = m_messageBuffers.back();
    buffer.nextOffset = alignedSize;
    return &buffer.data[0];
}

/** Main function of the render thread. */
void RenderThread::run() {
    while (true) {
        /* Wait for the game thread to indicate that it has work for us. */
        std::unique_lock<std::mutex> lock(m_lock);
        m_conditions[0].wait(lock, [this] { return m_sync != 0; });

        /* This is set by the destructor to indicate that we should exit. */
        if (m_sync == 2)
            break;

        /* Take ownership of the submitted message buffers. */
        std::list<MessageBuffer> messageBuffers = std::move(m_messageBuffers);

        /* Wake the game thread back up. */
        m_sync = 0;
        m_conditions[1].notify_one();
        lock.unlock();

        /* Process the messages. */
        for (MessageBuffer &buffer : messageBuffers) {
            size_t offset = 0;

            while (offset < buffer.nextOffset) {
                MessageBase *message = reinterpret_cast<MessageBase *>(&buffer.data[offset]);
                offset += Math::roundUp(message->size, 16);
                message->invoke(message);
            }
        }
    }
}

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

#pragma once

#include "core/core.h"

#include <atomic>
#include <condition_variable>
#include <functional>
#include <list>
#include <memory>
#include <new>
#include <thread>

/**
 * Class managing the rendering thread.
 *
 * The main game logic and rendering are run in parallel. While the game is
 * updating frame N, the render thread is updating frame N-1. To allow them to
 * run independently, we maintain separate representations of the world state
 * for each thread. Each game object that requires some render thread state has
 * one or more associated RenderObject-derived classes which contains the state.
 *
 * The render thread state cannot be updated directly by the main thread, since
 * it will still be using it. Therefore, updates are performed by queueing
 * "messages" to the render thread. Messages are implemented as lambdas which
 * capture the new state from the game thread and actually perform the update
 * on the render state when called. These messages are stored in a buffer, and
 * at the beginning of a new frame the render thread processes all messages in
 * order.
 */
class RenderThread {
public:
    template <typename T>
    void queueMessage(T &&function);

    void submit();

    /** @return             ID of the render thread. */
    std::thread::id id() const { return m_thread.get_id(); }
private:
    /** Base message structure. */
    struct MessageBase {
        size_t size;                    /**< Size of the whole message. */
        void (*invoke)(MessageBase *);  /**< Function which invokes the message lambda. */
    };

    /** Implementation for a specific message type. */
    template <typename T>
    class Message : public MessageBase {
    public:
        Message(T &&function) :
            MessageBase{ sizeof(*this), invokeImpl },
            m_function(std::move(function))
        {}
    private:
        static void invokeImpl(MessageBase *ptr) {
            auto message = static_cast<Message *>(ptr);
            message->m_function();

            /* We destroy messages as soon as they're processed. This is
             * necessary to destroy any values captured in the lambda. */
            message->~Message();
        }

        T m_function;
    };

    /**
     * Message buffer class.
     *
     * We don't want to be performing memory allocations for each message, so
     * instead we pack messages into a large buffer. If there is no space in
     * a buffer for a message, then a new one is allocated.
     */
    struct alignas(16) MessageBuffer {
        /** Size of a buffer (minus the nextOffset member). */
        static constexpr size_t kMaxSize = 32768 - sizeof(size_t);

        std::array<uint8_t, kMaxSize> data;
        size_t nextOffset;
    };

    RenderThread();
    ~RenderThread();

    void *allocateMessage(size_t size);

    void run();

    std::thread m_thread;               /**< Render thread. */

    /** Synchronisation state. */
    std::mutex m_lock;
    std::atomic<uint8_t> m_sync;
    std::condition_variable m_conditions[2];

    /**
     * Message buffers being written by the game thread.
     *
     * This is the set of message buffers currently being written by the game
     * thread. It does not require any locking because the render thread does
     * not read from this: it only ever takes the filled out message buffers at
     * the start of a frame, which is synchronised with the game thread, and
     * does not otherwise touch this list.
     */
    std::list<MessageBuffer> m_messageBuffers;

    friend class RenderManager;
};

/**
 * Queue a message to the render thread.
 *
 * Queues a message to the render thread. The given lambda will be called when
 * the render thread processes the message. Lambda captures (by value) can be
 * used to include data in the message, this will all be saved along with the
 * message. Messages are processed by the render thread in the order in which
 * they are sent.
 *
 * @param function      Lambda containing the message action and data.
 */
template <typename T>
inline void RenderThread::queueMessage(T &&function) {
    static_assert(
        sizeof(Message<T>) <= MessageBuffer::kMaxSize,
        "Render thread message is too large to fit in a buffer");

    /* Allocate buffer space. */
    void *buffer = allocateMessage(sizeof(Message<T>));

    /* Construct the message. We move construct the lambda here. Since we're
     * inlined, this can allow the compiler to construct the lambda including
     * captured values in place and avoid copying. */
    new (buffer) Message<T>(std::move(function));
}

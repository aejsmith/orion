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
 * @brief               Temporary render target pool.
 */

#pragma once

#include "core/hash_table.h"

#include "engine/engine.h"
#include "engine/global_resource.h"

#include "gpu/gpu_manager.h"

/** Class for allocating temporary render targets. */
class RenderTargetPool : public Engine::FrameListener {
private:
    /** Structure containing details of a render target. */
    struct Target : Refcounted {
        GPUTexturePtr texture;              /**< Texture. */
        unsigned unusedFrames;              /**< Number of frames that the target has gone unused. */
    };
public:
    /**
     * Handle to a render target.
     *
     * This is a handle to a temporary render target, which behaves like a
     * GPUTexture pointer. While it is held, the render target will not be
     * re-allocated elsehwere. Once it is destroyed, the target is elligble to
     * be re-allocated.
     */
    class Handle : Noncopyable {
    public:
        Handle() {}

        Handle(ReferencePtr<Target> target) :
            m_target(std::move(target))
        {}

        Handle(const Handle &other) :
            m_target(other.m_target)
        {}

        Handle(Handle &&other) :
            m_target(std::move(other.m_target))
        {}

        Handle &operator =(const Handle &other) {
            m_target = other.m_target;
            return *this;
        }

        Handle &operator =(Handle &&other) {
            m_target = std::move(other.m_target);
            return *this;
        }

        bool operator ==(const Handle &other) const{
            return m_target == other.m_target;
        }

        bool operator !=(const Handle &other) const{
            return !(*this == other);
        }

        /** @return             Whether the handle is valid. */
        explicit operator bool() const { return !!m_target; }

        /** @return             Raw pointer to the texture. */
        operator GPUTexture *() const { return (m_target) ? m_target->texture : nullptr; }

        /** @return             Reference to texture. */
        GPUTexture &operator *() const { return *m_target->texture; }

        /** @return             Pointer to texture. */
        GPUTexture *operator ->() const { return m_target->texture; }

        /** @return             Raw pointer to the texture. */
        GPUTexture *get() const { return (m_target) ? m_target->texture : nullptr; }

    private:
        ReferencePtr<Target> m_target;
    };

    RenderTargetPool();
    virtual ~RenderTargetPool();

    Handle allocate(const GPUTextureDesc &desc);

    void frameStarted() override;
private:
    /** Pool of temporary render target textures. */
    MultiHashMap<GPUTextureDesc, ReferencePtr<Target>> m_pool;
};

extern GlobalResource<RenderTargetPool> g_renderTargetPool;

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

#include "render_core/render_target_pool.h"

/** Number of frames that a target can go unused for before being freed. */
static const unsigned kNumUnusedFramesBeforeFree = 3;

/** Global render target pool. */
GlobalResource<RenderTargetPool> g_renderTargetPool;

/** Initialise the render target pool. */
RenderTargetPool::RenderTargetPool() {
    g_engine->addFrameListener(this);
}

/** Destroy the render target pool. */
RenderTargetPool::~RenderTargetPool() {}

/**
 * Allocate a temporary render target.
 *
 * Allocates a texture matching the given parameters from the temporary render
 * target pool. These are to be used for things which are only needed within a
 * single SceneRenderer pass, such as shadow maps. All targets allocated from
 * the pool are marked as free for re-use at the next call to
 * allocRenderTargets().
 *
 * @param desc          Texture descriptor.
 *
 * @return              Pointer to allocated render target (not reference
 *                      counted, the texture is guaranteed to exist until the
 *                      next call to allocRenderTargets()).
 */
RenderTargetPool::Handle RenderTargetPool::allocate(const GPUTextureDesc &desc) {
    /* See if we have a matching target spare in the pool. */
    auto range = m_pool.equal_range(desc);
    for (auto it = range.first; it != range.second; ++it) {
        if (it->second->refcount() == 1) {
            it->second->unusedFrames = 0;
            return Handle(it->second);
        }
    }

    logDebug(
        "Allocating new %ux%ux%u temporary render target of type %d",
        desc.width, desc.height,
        (desc.type == GPUTexture::kTexture2DArray || desc.type == GPUTexture::kTexture3D) ? desc.depth : 0,
        desc.type);

    /* Nothing found, create a new texture. */
    ReferencePtr<Target> target = new Target;
    target->texture = g_gpuManager->createTexture(desc);
    target->unusedFrames = 0;

    m_pool.insert(std::make_pair(desc, target));

    return Handle(std::move(target));
}

/** Clean up unused render targets. */
void RenderTargetPool::frameStarted() {
    for (auto it = m_pool.begin(); it != m_pool.end(); ) {
        ReferencePtr<Target> &target = it->second;
        if (target->refcount() == 1) {
            if (target->unusedFrames == kNumUnusedFramesBeforeFree) {
                const GPUTextureDesc &desc = it->first;
                logDebug(
                    "Releasing unused %ux%ux%u temporary render target of type %d",
                    desc.width, desc.height,
                    (desc.type == GPUTexture::kTexture2DArray || desc.type == GPUTexture::kTexture3D) ? desc.depth : 0,
                    desc.type);

                it = m_pool.erase(it);
                continue;
            }

            target->unusedFrames++;
        }

        ++it;
    }
}

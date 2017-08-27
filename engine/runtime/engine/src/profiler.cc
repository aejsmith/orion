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
 * @brief               MicroProfile-based profiler.
 */

#define MICROPROFILE_IMPL 1

#include "engine/profiler.h"

#include "gpu/command_list.h"
#include "gpu/gpu_manager.h"

#include <thread>

/** Global profiler instance. */
Profiler *g_profiler;

/** Initialise the profiler. */
Profiler::Profiler() {
    MicroProfileOnThreadCreate("Main");
    MicroProfileWebServerStart();

    MicroProfileSetForceEnable(true);
    MicroProfileSetEnableAllGroups(true);
    MicroProfileSetForceMetaCounters(true);

    g_MicroProfile.GPU.Shutdown          = gpuShutdown;
    g_MicroProfile.GPU.Flip              = gpuFlip;
    g_MicroProfile.GPU.InsertTimer       = gpuInsertTimer;
    g_MicroProfile.GPU.GetTimeStamp      = gpuGetTimeStamp;
    g_MicroProfile.GPU.GetTicksPerSecond = gpuTicksPerSecond;
    g_MicroProfile.GPU.GetTickReference  = gpuTickReference;
}

/** End the current frame. */
void Profiler::endFrame() {
    MicroProfileFlip();
}

/** Initialise the GPU profiler. */
void Profiler::gpuInit() {
    GPUState &state = m_gpuState;

    auto desc = GPUQueryPoolDesc().
        setType  (GPUQueryPool::kTimestampQuery).
        setCount (MICROPROFILE_GPU_MAX_QUERIES);
    state.queryPool = g_gpuManager->createQueryPool(desc);

    state.mainThreadID = std::this_thread::get_id();
}

/** Shut down the GPU profiler. */
void Profiler::gpuShutdown() {}

/** End the current GPU frame. */
uint32_t Profiler::gpuFlip() {
    GPUState &state = g_profiler->m_gpuState;

    const uint32_t frameQueries  = MICROPROFILE_GPU_MAX_QUERIES / MICROPROFILE_GPU_FRAMES;
    const uint32_t frameIndex    = state.frame % MICROPROFILE_GPU_FRAMES;

    const uint32_t frameTimeStamp = gpuInsertTimer(nullptr);
    const uint32_t framePut       = std::min(state.framePut.load(), frameQueries);

    state.submitted[frameIndex] = framePut;
    state.framePut.store(0);
    state.frame++;

    if (state.frame >= MICROPROFILE_GPU_FRAMES) {
        const uint64_t pendingFrame      = state.frame - MICROPROFILE_GPU_FRAMES;
        const uint32_t pendingFrameIndex = pendingFrame % MICROPROFILE_GPU_FRAMES;

        const uint32_t pendingFrameStart = pendingFrameIndex * frameQueries;
        const uint32_t pendingFrameCount = state.submitted[pendingFrameIndex];

        if (pendingFrameCount) {
            state.queryPool->getResults(pendingFrameStart,
                                        pendingFrameCount,
                                        &state.results[pendingFrameStart]);
            state.queryPool->reset(pendingFrameStart,
                                   pendingFrameCount);
        }
    }

    return frameTimeStamp;
}

/** Insert a timer.
 * @param context       GPUCommandList pointer, or null for global scope. */
uint32_t Profiler::gpuInsertTimer(void *context) {
    GPUState &state = g_profiler->m_gpuState;

    /* Haven't set up multithreaded support in MicroProfile yet. */
    check(std::this_thread::get_id() == state.mainThreadID);

    const uint32_t frameQueries = MICROPROFILE_GPU_MAX_QUERIES / MICROPROFILE_GPU_FRAMES;
    const uint32_t index        = state.framePut.fetch_add(1);

    if (index >= frameQueries)
        return static_cast<uint32_t>(-1);

    const uint32_t queryIndex = ((state.frame % MICROPROFILE_GPU_FRAMES) * frameQueries) + index;

    if (context) {
        auto cmdList = reinterpret_cast<GPUCommandList *>(context);
        cmdList->endQuery(state.queryPool, queryIndex);
    } else {
        g_gpuManager->endQuery(state.queryPool, queryIndex);
    }

    return queryIndex;
}

/** Get the value of a timestamp.
 * @param index         Index of the query as returned by gpuInsertTimer(). */
uint64_t Profiler::gpuGetTimeStamp(uint32_t index) {
    GPUState &state = g_profiler->m_gpuState;
    return state.results[index];
}

/** @return             The number of GPU ticks (query value) per second. */
uint64_t Profiler::gpuTicksPerSecond() {
    /* GPU backend always uses nanoseconds. */
    return 1000000000ull;
}

/** Get the current CPU and GPU tick.
 * @param outCPU        Where to store current CPU tick.
 * @param outGPU        Where to store current GPU tick. */
bool Profiler::gpuTickReference(int64_t *outCPU, int64_t *outGPU) {
    /* MicroProfile doesn't call this at the moment. */
    check(false);
    return true;
}

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

#pragma once

#if ORION_MICROPROFILE

/** Set to 1 to enable GPU profiling of debug groups. */
#define ORION_PROFILE_GPU_GROUPS                1

/** MicroProfile settings. */
#define MICROPROFILE_PER_THREAD_BUFFER_SIZE     (2 * 1024 * 1024)
#define MICROPROFILE_PER_THREAD_GPU_BUFFER_SIZE (1 * 1024 * 1024)
#define MICROPROFILE_MAX_FRAME_HISTORY          22
#define MICROPROFILE_LABEL_BUFFER_SIZE          (1 * 1024 * 1024)
#define MICROPROFILE_GPU_MAX_QUERIES            8192

#include "microprofile.h"

#include "gpu/query_pool.h"

/** Wrapper class around MicroProfile. */
class Profiler {
public:
    Profiler();

    void gpuInit();
    void endFrame();

private:
    /** GPU profiling state. */
    struct GPUState {
        GPUQueryPoolPtr queryPool;

        uint64_t frame = 0;
        std::atomic<uint32_t> framePut{0};

        uint32_t submitted[MICROPROFILE_GPU_FRAMES] = {};
        uint64_t results[MICROPROFILE_GPU_MAX_QUERIES] = {};

        std::thread::id mainThreadID;
    };

private:
    static void gpuShutdown();
    static uint32_t gpuFlip();
    static uint32_t gpuInsertTimer(void *context);
    static uint64_t gpuGetTimeStamp(uint32_t index);
    static uint64_t gpuTicksPerSecond();
    static bool gpuTickReference(int64_t *outCPU, int64_t *outGPU);

private:
    GPUState m_gpuState;
};

extern Profiler *g_profiler;

#define PROFILE_SCOPE(group, timer, colour) \
    MICROPROFILE_SCOPEI(group, timer, colour)

#define PROFILE_FUNCTION_SCOPE(group, colour) \
    MICROPROFILE_SCOPEI(group, __func__, colour)

#else // ORION_MICROPROFILE

#define PROFILE_SCOPE(group, timer, colour)
#define PROFILE_FUNCTION_SCOPE(group, colour)

#endif // ORION_MICROPROFILE

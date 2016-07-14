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
 * @brief               GPU resource definitions.
 */

#pragma once

#include "gpu/state.h"

class GPUBuffer;
class GPUTexture;

/** Possible types of a shader resource. */
enum class GPUResourceType {
    kNone,
    kUniformBuffer,
    kTexture,
};

/** Descriptor for a GPU resource set layout. */
struct GPUResourceSetLayoutDesc {
    /** Details of a slot in a resource set. */
    struct Slot {
        GPUResourceType type;           /**< Type of the resource for this slot. */
    };

    /** Array of slot descriptors. */
    std::vector<Slot> slots;

    /** Initialise an empty layout. */
    GPUResourceSetLayoutDesc() {}

    /** Initialise with pre-allocated arrays.
     * @param numSlots      Number of slots to allocate. */
    GPUResourceSetLayoutDesc(size_t numSlots) :
        slots(numSlots)
    {}
};

/**
 * Layout information for a resource set.
 *
 * This class defines the layout of a resource set, i.e. the details of the
 * type of resource that will be bound at each slot.
 *
 * Although this is based on GPUState, it's not quite the same as the other
 * state objects in that we don't cache it. This is because we don't create
 * arbitrary layouts, rather we have a finite set of layouts (the global ones,
 * and one for each shader).
 */
using GPUResourceSetLayout = GPUState<GPUResourceSetLayoutDesc>;

/** Type of a pointer to a GPU resource set layout. */
using GPUResourceSetLayoutPtr = GPUObjectPtr<GPUResourceSetLayout>;

/** Array of resource set layouts for a pipeline. */
using GPUResourceSetLayoutArray = std::vector<GPUResourceSetLayoutPtr>;

/**
 * A set of resources for a shader.
 *
 * Resources used by shaders (uniform buffers, textures, etc.) are organised
 * into groups known as resource sets. Each set has a set of slots for resources
 * in a layout defined by a GPUResourceSetLayout object. Resource sets maintain
 * the bindings in each slot until they are changed.
 *
 * Resource sets map directly onto modern APIs such as Vulkan. On other APIs
 * they are emulated by applying the bindings at draw time.
 */
class GPUResourceSet : public GPUObject {
public:
    /** Structure containing bindings for a slot. */
    struct Slot {
        const GPUResourceSetLayoutDesc::Slot &desc;

        /** Primary object (buffer or texture). */
        GPUObjectPtr<GPUObject> object;

        /** Sampler for a texture. */
        GPUSamplerStatePtr sampler;

        Slot(const GPUResourceSetLayoutDesc::Slot &inDesc) :
            desc(inDesc)
        {}
    };

    void bindUniformBuffer(size_t index, GPUBuffer *buffer);
    void bindTexture(size_t index, GPUTexture *texture, GPUSamplerState *sampler);

    /** @return             Layout of the resource set. */
    GPUResourceSetLayout *layout() const { return m_layout; }
    /** @return             Array of bindings for each slot. */
    const std::vector<Slot> &slots() const { return m_slots; }
protected:
    GPUResourceSet(GPUResourceSetLayout *layout);
    ~GPUResourceSet();

    /** Update a slot's binding.
     * @param index         Index of the slot that was changed. */
    virtual void updateSlot(size_t index) {}

    GPUResourceSetLayoutPtr m_layout;   /**< Layout of the resource set. */

    /** Array of bindings for each slot. */
    std::vector<Slot> m_slots;

    /* For default creation method in GPUManager. */
    friend class GPUManager;
};

/** Type of a pointer to a GPU resource set. */
using GPUResourceSetPtr = GPUObjectPtr<GPUResourceSet>;

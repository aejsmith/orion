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

#include "gpu/buffer.h"
#include "gpu/resource.h"
#include "gpu/texture.h"

/** Create a GPU resource set.
 * @param layout        Layout to use for the set. */
GPUResourceSet::GPUResourceSet(GPUResourceSetLayout *layout) :
    m_layout (layout)
{
    const GPUResourceSetLayoutDesc &desc = m_layout->desc();
    m_slots.reserve(desc.slots.size());
    for (const GPUResourceSetLayoutDesc::Slot &slotDesc : desc.slots)
        m_slots.emplace_back(slotDesc);
}

/** Destroy the resource set. */
GPUResourceSet::~GPUResourceSet() {}

/** Bind a uniform buffer.
 * @param index         Slot index to bind to (must be a uniform buffer slot).
 * @param buffer        Buffer to bind. */
void GPUResourceSet::bindUniformBuffer(size_t index, GPUBuffer *buffer) {
    check(index < m_slots.size());
    Slot &slot = m_slots[index];
    check(slot.desc.type == GPUResourceType::kUniformBuffer);

    if (slot.object != buffer) {
        slot.object = buffer;
        slot.sampler = nullptr;

        updateSlot(index);
    }
}

/** Bind a texture.
 * @param index         Slot index to bind to (must be a texture slot).
 * @param texture       Texture to bind.
 * @param sampler       Sampler state to use. */
void GPUResourceSet::bindTexture(size_t index, GPUTexture *texture, GPUSamplerState *sampler) {
    check(index < m_slots.size());
    Slot &slot = m_slots[index];
    check(slot.desc.type == GPUResourceType::kTexture);

    if (slot.object != texture || slot.sampler != sampler) {
        slot.object = texture;
        slot.sampler = sampler;

        updateSlot(index);
    }
}

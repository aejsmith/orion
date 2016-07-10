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
 * @brief               OpenGL resource set implementation.
 *
 * The resource set model uses a single namespace for all resource types. In
 * contrast, GL's binding model has a separate set of binding points for each
 * resource type. Therefore, in order to map resource sets onto GL, we need to
 * remap the set/slot numbers into these spaces.
 *
 * An obvious solution would be to use a fixed mapping where the GL binding
 * point number is the set number multiplied by a constant, plus the slot
 * number, regardless of the resource type. For instance, set 2 slot 3 becomes
 * (2 * 16) + 3 = 35 (imposing an artificial limit on the maximum number of
 * resources per set of 16). The issue with this is that the GL binding spaces
 * have limited sizes. On GL 3.3, our minimum, an implementation must provide
 * at least 36 uniform buffer binding points and at least 48 texture units.
 * Because of this, using a fixed mapping is impractical as it would impose
 * severe restrictions on the number of resource sets or resources per set.
 *
 * Instead, we build a mapping dynamically for each set layout taking resource
 * type into account. Each resource type's binding space has a fixed division
 * based on set number, then we dynamically map the slots in a set layout into
 * those spaces.
 *
 * Since shaders have fixed set/binding numbers hardcoded into them, when we
 * receive shader source we record the set/binding number for each name and
 * then strip them out before handing the source to the driver. At pipeline
 * creation time, we bind the uniforms in the shader to the correct binding
 * points based on the supplied layouts.
 *
 * TODO:
 *  - Could improve over a fixed division of the binding spaces based on set
 *    number, as currently this limits us to 8 textures per set, which could
 *    become a problem. Since we know what's in the global sets, we can do
 *    better.
 */

#include "resource.h"

/** Maximum number of uniform buffers per set. */
static const size_t kGLMaxUniformBuffersPerSet = 36 / kGLMaxResourceSets;

/** Maximum number of textures per set. */
static const size_t kGLMaxTexturesPerSet = 48 / kGLMaxResourceSets;

/** Create a resource set layout.
 * @param desc          Descriptor for the layout. */
GLResourceSetLayout::GLResourceSetLayout(GPUResourceSetLayoutDesc &&desc) :
    GPUResourceSetLayout(std::move(desc))
{
    m_mapping.resize(m_desc.slots.size());

    size_t nextUniformBuffer = 0;
    size_t nextTexture = 0;

    for (size_t i = 0; i < m_desc.slots.size(); i++) {
        switch (m_desc.slots[i].type) {
            case GPUResourceType::kUniformBuffer:
                checkMsg(
                    nextUniformBuffer < kGLMaxUniformBuffersPerSet,
                    "Exceeded maximum number of uniform buffers per resource set");

                m_mapping[i] = nextUniformBuffer++;
                break;
            case GPUResourceType::kTexture:
                checkMsg(
                    nextTexture < kGLMaxTexturesPerSet,
                    "Exceeded maximum number of textures per resource set");

                m_mapping[i] = nextTexture++;
                break;
            default:
                m_mapping[i] = -1ul;
                continue;
        }
    }
}

/**
 * Map a set/slot index to a binding point.
 *
 * Maps the given set/slot index to a binding point number specific to the
 * type of resource in the slot (if the slot is for a texture, the returned
 * value is a texture unit index, if it's a uniform buffer then the return
 * value is a uniform buffer binding point index, etc.)
 *
 * @param set           Set number where this layout is being used.
 * @param slot          Slot number to map.
 */
size_t GLResourceSetLayout::mapSlot(size_t set, size_t slot) const {
    check(set < kGLMaxResourceSets);
    check(slot < m_desc.slots.size());

    size_t multiplier = 0;
    switch (m_desc.slots[slot].type) {
        case GPUResourceType::kUniformBuffer:
            multiplier = kGLMaxUniformBuffersPerSet;
            break;
        case GPUResourceType::kTexture:
            multiplier = kGLMaxTexturesPerSet;
            break;
        default:
            checkMsg(false, "Invalid resource slot type");
            break;
    }

    return (set * multiplier) + m_mapping[slot];
}

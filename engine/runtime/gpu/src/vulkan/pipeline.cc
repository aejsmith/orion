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
 * @brief               Vulkan pipeline implementation.
 */

#include "commands.h"
#include "manager.h"
#include "pipeline.h"

/** Create a pipeline object.
 * @param manager       Manager that the pipeline is for.
 * @param desc          Descriptor for the pipeline. */
VulkanPipeline::VulkanPipeline(VulkanGPUManager *manager, GPUPipelineDesc &&desc) :
    GPUPipeline(std::move(desc)),
    VulkanObject(manager)
{
    /* Create a pipeline layout. */
    std::vector<VkDescriptorSetLayout> setLayouts;
    setLayouts.reserve(m_resourceLayout.size());
    for (const GPUResourceSetLayout *it : m_resourceLayout) {
        auto resourceSetLayout = static_cast<const VulkanResourceSetLayout *>(it);
        setLayouts.push_back(resourceSetLayout->handle());
    }

    VkPipelineLayoutCreateInfo layoutCreateInfo = {};
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutCreateInfo.setLayoutCount = setLayouts.size();
    layoutCreateInfo.pSetLayouts = &setLayouts[0];

    checkVk(vkCreatePipelineLayout(manager->device()->handle(), &layoutCreateInfo, nullptr, &m_layout));

    /* Fill out stage information ready for creation calls. */
    for (size_t i = 0; i < m_programs.size(); i++) {
        if (!m_programs[i])
            continue;

        auto program = static_cast<VulkanProgram *>(m_programs[i].get());

        m_stageInfos.emplace_back();
        auto &stageInfo = m_stageInfos.back();
        stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

        switch (program->stage()) {
            case ShaderStage::kVertex:
                stageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
                break;
            case ShaderStage::kFragment:
                stageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
                break;
        }

        stageInfo.module = program->handle();
        stageInfo.pName = "main";
    }
}

/** Destroy the pipeline. */
VulkanPipeline::~VulkanPipeline() {
    for (const auto &it : m_pipelines)
        vkDestroyPipeline(manager()->device()->handle(), it.second, nullptr);

    vkDestroyPipelineLayout(manager()->device()->handle(), m_layout, nullptr);
}

/** Construct a state key given rendering state.
 * @param state         Current command state.
 * @param primType      Primitive type being rendered.
 * @param vertices      Vertex data. */
VulkanPipeline::StateKey::StateKey(
    const VulkanCommandState &state,
    PrimitiveType primType,
    const GPUVertexData *vertices)
    :
    primitiveType(primType),
    renderPass(state.renderPass),
    rasterizerState(state.pending.rasterizerState),
    depthStencilState(state.pending.depthStencilState),
    blendState(state.pending.blendState),
    vertexDataLayout(vertices->layout())
{}

/** Compare this key with another. */
bool VulkanPipeline::StateKey::operator ==(const StateKey &other) const {
    return
        primitiveType == other.primitiveType &&
        renderPass == other.renderPass &&
        rasterizerState == other.rasterizerState &&
        depthStencilState == other.depthStencilState &&
        blendState == other.blendState &&
        vertexDataLayout == other.vertexDataLayout;
}

/** Get a hash from a render pass compatibility key. */
size_t hashValue(const VulkanPipeline::StateKey &key) {
    size_t hash = hashValue(key.primitiveType);
    hash = hashCombine(hash, key.renderPass);
    hash = hashCombine(hash, key.rasterizerState);
    hash = hashCombine(hash, key.depthStencilState);
    hash = hashCombine(hash, key.blendState);
    hash = hashCombine(hash, key.vertexDataLayout);
    return hash;
}

/** Bind a pipeline object for given rendering state.
 * @param state         Current command state.
 * @param primType      Primitive type being rendered.
 * @param vertices      Vertex data. */
void VulkanPipeline::bind(VulkanCommandState &state, PrimitiveType primType, const GPUVertexData *vertices) {
    /* Look to see if we have one already. */
    StateKey key(state, primType, vertices);
    auto ret = m_pipelines.find(key);
    VkPipeline pipeline = (ret != m_pipelines.end())
        ? ret->second
        : create(state, primType, vertices, std::move(key));

    if (pipeline != state.pipelineObject) {
        vkCmdBindPipeline(state.cmdBuf->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
        state.pipelineObject = pipeline;

        /* Reference the object (will already have been done if already bound). */
        state.cmdBuf->addReference(state.pipeline);
    }
}

/** Create a new pipeline object.
 * @param state         Current command state.
 * @param primType      Primitive type being rendered.
 * @param vertices      Vertex data.
 * @param key           Pipeline state key.
 * @return              Created pipeline. */
VkPipeline VulkanPipeline::create(
    const VulkanCommandState &state,
    PrimitiveType primType,
    const GPUVertexData *vertices,
    StateKey &&key)
{
    VkGraphicsPipelineCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    createInfo.layout = m_layout;
    createInfo.renderPass = state.renderPass->handle();
    createInfo.subpass = 0;

    /* If we have not got any pipelines cached yet, we create this as the
     * "initial pipeline" and set the allow derivatives bit on it. Any we
     * create after this is created as a derivative of the initial pipeline.
     * This might make it more efficient both to create the pipeline, and to
     * switch between the derivative pipelines. All pipelines we create within
     * this object will share the same shader stages, therefore there is a good
     * chance that there is optimization opportunity for the driver. */
    if (m_pipelines.empty()) {
        createInfo.flags |= VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT;
    } else {
        createInfo.flags |= VK_PIPELINE_CREATE_DERIVATIVE_BIT;
        createInfo.basePipelineHandle = m_initialPipeline;
        createInfo.basePipelineIndex = -1;
    }

    /* Add shader stages. */
    createInfo.stageCount = m_stageInfos.size();
    createInfo.pStages = &m_stageInfos[0];

    /* Vertex input state. */
    auto vertexDataLayout = static_cast<const VulkanVertexDataLayout *>(vertices->layout());
    createInfo.pVertexInputState = &vertexDataLayout->createInfo();

    /* Input assembly state. */
    VkPipelineInputAssemblyStateCreateInfo assemblyStateInfo = {};
    assemblyStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    assemblyStateInfo.primitiveRestartEnable = false;

    switch (primType) {
        case PrimitiveType::kTriangleList:
            assemblyStateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            break;
        case PrimitiveType::kTriangleStrip:
            assemblyStateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
            break;
        case PrimitiveType::kTriangleFan:
            assemblyStateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
            break;
        case PrimitiveType::kPointList:
            assemblyStateInfo.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
            break;
        case PrimitiveType::kLineList:
            assemblyStateInfo.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
            break;
    }

    createInfo.pInputAssemblyState = &assemblyStateInfo;

    /* Viewport count must be set in the pipeline, actual viewport is dynamic. */
    VkPipelineViewportStateCreateInfo viewportStateInfo = {};
    viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportStateInfo.viewportCount = 1;
    viewportStateInfo.scissorCount = 1;
    createInfo.pViewportState = &viewportStateInfo;

    /* Rasterizer state. */
    auto rasterizerState = static_cast<const VulkanRasterizerState *>(state.pending.rasterizerState.get());
    createInfo.pRasterizationState = &rasterizerState->createInfo();

    /* Multisample state. */
    VkPipelineMultisampleStateCreateInfo multisampleStateInfo = {};
    multisampleStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleStateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    createInfo.pMultisampleState = &multisampleStateInfo;

    /* Depth/stencil state. */
    auto depthStencilState = static_cast<const VulkanDepthStencilState *>(state.pending.depthStencilState.get());
    createInfo.pDepthStencilState = &depthStencilState->createInfo();

    /* Blend state is a little awkward in that the spec requires that the
     * attachment count matches the subpass' attachment count. In
     * VulkanBlendState we maintain the state for the maximum number of
     * attachments. Therefore we copy the generated state structure here and
     * modify the count. */
    auto blendState = static_cast<const VulkanBlendState *>(state.pending.blendState.get());
    VkPipelineColorBlendStateCreateInfo blendStateInfo = blendState->createInfo();
    blendStateInfo.attachmentCount = state.renderPass->desc().colourAttachments.size();
    createInfo.pColorBlendState = &blendStateInfo;

    /* Set up dynamic states. */
    const VkDynamicState kDynamicStates[2] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };
    VkPipelineDynamicStateCreateInfo dynamicStateInfo = {};
    dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateInfo.dynamicStateCount = arraySize(kDynamicStates);
    dynamicStateInfo.pDynamicStates = kDynamicStates;
    createInfo.pDynamicState = &dynamicStateInfo;

    /* Create the pipeline. TODO: Pipeline caching. */
    VkPipeline pipeline;
    checkVk(vkCreateGraphicsPipelines(
        manager()->device()->handle(),
        VK_NULL_HANDLE,
        1, &createInfo,
        nullptr,
        &pipeline));

    /* Set the initial pipeline if this is it. */
    if (m_pipelines.empty())
        m_initialPipeline = pipeline;

    /* Cache the pipeline. */
    m_pipelines.emplace(std::move(key), pipeline);

    return pipeline;
}

/**
 * Determine if two pipeline layouts are compatible for a resource set.
 *
 * The Vulkan specification details rules for pipeline layout compatiblity.
 * Two pipeline layouts are compatible for set N if they were created with
 * matching (the same, or identically defined) descriptor set layouts for sets
 * 0 through N, and they were created with identical push constant ranges.
 * This function checks that compatibility between two pipelines.
 *
 * @param other         Other pipeline to check.
 * @param set           Set number to check.
 *
 * @return              Whether the pipelines are compatible for the given set.
 */
bool VulkanPipeline::isCompatibleForSet(VulkanPipeline *other, size_t set) const {
    for (size_t i = 0; i <= set; i++) {
        if (i >= m_resourceLayout.size() || i >= other->m_resourceLayout.size())
            return false;

        // TODO: Could check layout definition as well, but I'm not sure how
        // much benefit we'll get from this.
        if (m_resourceLayout[i] != other->m_resourceLayout[i])
            return false;
    }

    // TODO: If we ever use push constants we will need to check that here.
    return true;
}

/** Create a pipeline object.
 * @param desc          Descriptor for the pipeline.
 * @return              Pointer to created pipeline. */
GPUPipelinePtr VulkanGPUManager::createPipeline(GPUPipelineDesc &&desc) {
    return new VulkanPipeline(this, std::move(desc));
}

/** Vertex attribute format conversion table. */
static const VkFormat kAttributeFormats[VertexAttribute::kNumTypes][4][2] = {
    /* kByteType */
    {
        { VK_FORMAT_R8_SINT, VK_FORMAT_R8_SNORM },
        { VK_FORMAT_R8G8_SINT, VK_FORMAT_R8G8_SNORM },
        { VK_FORMAT_R8G8B8_SINT, VK_FORMAT_R8G8B8_SNORM },
        { VK_FORMAT_R8G8B8A8_SINT, VK_FORMAT_R8G8B8A8_SNORM },
    },
    /* kUnsignedByteType */
    {
        { VK_FORMAT_R8_UINT, VK_FORMAT_R8_UNORM },
        { VK_FORMAT_R8G8_UINT, VK_FORMAT_R8G8_UNORM },
        { VK_FORMAT_R8G8B8_UINT, VK_FORMAT_R8G8B8_UNORM },
        { VK_FORMAT_R8G8B8A8_UINT, VK_FORMAT_R8G8B8A8_UNORM },
    },
    /* kShortType */
    {
        { VK_FORMAT_R16_SINT, VK_FORMAT_R16_SNORM },
        { VK_FORMAT_R16G16_SINT, VK_FORMAT_R16G16_SNORM },
        { VK_FORMAT_R16G16B16_SINT, VK_FORMAT_R16G16B16_SNORM },
        { VK_FORMAT_R16G16B16A16_SINT, VK_FORMAT_R16G16B16A16_SNORM },
    },
    /* kUnsignedShortType */
    {
        { VK_FORMAT_R16_UINT, VK_FORMAT_R16_UNORM },
        { VK_FORMAT_R16G16_UINT, VK_FORMAT_R16G16_UNORM },
        { VK_FORMAT_R16G16B16_UINT, VK_FORMAT_R16G16B16_UNORM },
        { VK_FORMAT_R16G16B16A16_UINT, VK_FORMAT_R16G16B16A16_UNORM },
    },
    /* kIntType */
    {
        { VK_FORMAT_R32_SINT, VK_FORMAT_UNDEFINED },
        { VK_FORMAT_R32G32_SINT, VK_FORMAT_UNDEFINED },
        { VK_FORMAT_R32G32B32_SINT, VK_FORMAT_UNDEFINED },
        { VK_FORMAT_R32G32B32A32_SINT, VK_FORMAT_UNDEFINED },
    },
    /* kUnsignedIntType */
    {
        { VK_FORMAT_R32_UINT, VK_FORMAT_UNDEFINED },
        { VK_FORMAT_R32G32_UINT, VK_FORMAT_UNDEFINED },
        { VK_FORMAT_R32G32B32_UINT, VK_FORMAT_UNDEFINED },
        { VK_FORMAT_R32G32B32A32_UINT, VK_FORMAT_UNDEFINED },
    },
    /* kFloatType */
    {
        { VK_FORMAT_R32_SFLOAT, VK_FORMAT_UNDEFINED },
        { VK_FORMAT_R32G32_SFLOAT, VK_FORMAT_UNDEFINED },
        { VK_FORMAT_R32G32B32_SFLOAT, VK_FORMAT_UNDEFINED },
        { VK_FORMAT_R32G32B32A32_SFLOAT, VK_FORMAT_UNDEFINED },
    },
    /* kDoubleType */
    {
        { VK_FORMAT_R64_SFLOAT, VK_FORMAT_UNDEFINED },
        { VK_FORMAT_R64G64_SFLOAT, VK_FORMAT_UNDEFINED },
        { VK_FORMAT_R64G64B64_SFLOAT, VK_FORMAT_UNDEFINED },
        { VK_FORMAT_R64G64B64A64_SFLOAT, VK_FORMAT_UNDEFINED },
    },
};

/** Initialise the vertex data layout.
 * @param desc          Layout descriptor. */
VulkanVertexDataLayout::VulkanVertexDataLayout(const GPUVertexDataLayoutDesc &desc) :
    GPUVertexDataLayout(desc),
    m_createInfo()
{
    m_bindings.resize(m_desc.bindings.size());
    for (size_t i = 0; i < m_bindings.size(); i++) {
        m_bindings[i].binding = i;
        m_bindings[i].stride = m_desc.bindings[i].stride;
        // TODO: instancing
        m_bindings[i].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    }

    m_attributes.resize(m_desc.attributes.size());
    for (size_t i = 0; i < m_attributes.size(); i++) {
        const auto &attribDesc = m_desc.attributes[i];
        m_attributes[i].location = attribDesc.glslIndex();
        m_attributes[i].binding = attribDesc.binding;
        m_attributes[i].offset = attribDesc.offset;

        check(attribDesc.type < arraySize(kAttributeFormats));
        check(attribDesc.components && attribDesc.components <= arraySize(kAttributeFormats[0]));

        m_attributes[i].format =
            kAttributeFormats[attribDesc.type][attribDesc.components - 1][attribDesc.normalised];

        // FIXME: Check format support.
        check(m_attributes[i].format != VK_FORMAT_UNDEFINED);
    }

    m_createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    m_createInfo.vertexBindingDescriptionCount = m_bindings.size();
    m_createInfo.pVertexBindingDescriptions = &m_bindings[0];
    m_createInfo.vertexAttributeDescriptionCount = m_attributes.size();
    m_createInfo.pVertexAttributeDescriptions = &m_attributes[0];
}

/** Create a vertex data layout object.
 * @param desc          Descriptor for vertex data layout.
 * @return              Pointer to created vertex data layout object. */
GPUVertexDataLayoutPtr VulkanGPUManager::createVertexDataLayout(const GPUVertexDataLayoutDesc &desc) {
    return new VulkanVertexDataLayout(desc);
}

/** Initialise the blend state.
 * @param desc          Descriptor for blend state. */
VulkanBlendState::VulkanBlendState(const GPUBlendStateDesc &desc) :
    GPUBlendState(desc),
    m_createInfo()
{
    auto convertBlendFactor =
        [] (BlendFactor factor) -> VkBlendFactor {
            switch (factor) {
                case BlendFactor::kZero:
                    return VK_BLEND_FACTOR_ZERO;
                case BlendFactor::kOne:
                    return VK_BLEND_FACTOR_ONE;
                case BlendFactor::kSourceColour:
                    return VK_BLEND_FACTOR_SRC_COLOR;
                case BlendFactor::kDestColour:
                    return VK_BLEND_FACTOR_DST_COLOR;
                case BlendFactor::kOneMinusSourceColour:
                    return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
                case BlendFactor::kOneMinusDestColour:
                    return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
                case BlendFactor::kSourceAlpha:
                    return VK_BLEND_FACTOR_SRC_ALPHA;
                case BlendFactor::kDestAlpha:
                    return VK_BLEND_FACTOR_DST_ALPHA;
                case BlendFactor::kOneMinusSourceAlpha:
                    return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                case BlendFactor::kOneMinusDestAlpha:
                    return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
            }
        };

    auto convertBlendFunc =
        [] (BlendFunc func) -> VkBlendOp {
            switch (func) {
                case BlendFunc::kAdd:
                    return VK_BLEND_OP_ADD;
                case BlendFunc::kSubtract:
                    return VK_BLEND_OP_SUBTRACT;
                case BlendFunc::kReverseSubtract:
                    return VK_BLEND_OP_REVERSE_SUBTRACT;
                case BlendFunc::kMin:
                    return VK_BLEND_OP_MIN;
                case BlendFunc::kMax:
                    return VK_BLEND_OP_MAX;
            }
        };

    /* We don't support per-RT state yet, just set identically for each. Note
     * that when creating a pipeline, it is required that the attachment count
     * matches the subpass' attachment count. Therefore, when creating the
     * pipeline we create a copy of this structure and set the attachment count
     * to the correct value there. */
    m_attachments.resize(kMaxColourRenderTargets);
    for (auto &attachment : m_attachments) {
        attachment.blendEnable =
            desc.func != BlendFunc::kAdd ||
            desc.sourceFactor != BlendFactor::kOne ||
            desc.destFactor != BlendFactor::kZero;
        attachment.srcColorBlendFactor = convertBlendFactor(desc.sourceFactor);
        attachment.srcAlphaBlendFactor = attachment.srcColorBlendFactor;
        attachment.dstColorBlendFactor = convertBlendFactor(desc.destFactor);
        attachment.dstAlphaBlendFactor = attachment.dstColorBlendFactor;
        attachment.colorBlendOp = convertBlendFunc(desc.func);
        attachment.alphaBlendOp = attachment.colorBlendOp;
        attachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT |
            VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT;
    }

    m_createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    m_createInfo.attachmentCount = m_attachments.size();
    m_createInfo.pAttachments = &m_attachments[0];
}

/** Create a blend state object.
 * @param desc          Descriptor for blend state.
 * @return              Created blend state object. */
GPUBlendStatePtr VulkanGPUManager::createBlendState(const GPUBlendStateDesc &desc) {
    return new VulkanBlendState(desc);
}

/** Initialise the depth/stencil state.
 * @param desc          Descriptor for depth/stencil state. */
VulkanDepthStencilState::VulkanDepthStencilState(const GPUDepthStencilStateDesc &desc) :
    GPUDepthStencilState(desc),
    m_createInfo()
{
    m_createInfo.sType            = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    m_createInfo.depthTestEnable  = desc.depthFunc != ComparisonFunc::kAlways;
    m_createInfo.depthWriteEnable = desc.depthWrite;
    m_createInfo.depthCompareOp   = VulkanUtil::convertComparisonFunc(desc.depthFunc);
}

/** Create a depth/stencil state object.
 * @param desc          Descriptor for depth/stencil state.
 * @return              Created depth/stencil state object. */
GPUDepthStencilStatePtr VulkanGPUManager::createDepthStencilState(const GPUDepthStencilStateDesc &desc) {
    return new VulkanDepthStencilState(desc);
}

/** Initialise the rasterizer state.
 * @param desc          Descriptor for rasterizer state. */
VulkanRasterizerState::VulkanRasterizerState(const GPURasterizerStateDesc &desc) :
    GPURasterizerState(desc),
    m_createInfo()
{
    m_createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    m_createInfo.depthClampEnable = desc.depthClamp;
    m_createInfo.polygonMode = VK_POLYGON_MODE_FILL;

    switch (desc.cullMode) {
        case CullMode::kDisabled:
            m_createInfo.cullMode = VK_CULL_MODE_NONE;
            break;
        case CullMode::kBack:
            m_createInfo.cullMode = VK_CULL_MODE_BACK_BIT;
            break;
        case CullMode::kFront:
            m_createInfo.cullMode = VK_CULL_MODE_FRONT_BIT;
            break;
    }

    /* Standard engine front face order is counter-clockwise. However, to
     * compensate for the differences between GL and Vulkan clip spaces (Y is
     * up in GL but down in Vulkan), we render upside down in Vulkan and flip
     * at the end of the frame. As a side effect of that, we have to reverse
     * the front face order. */
    m_createInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;

    m_createInfo.lineWidth = 1.0f;
}

/** Create a rasterizer state object.
 * @param desc          Descriptor for rasterizer state.
 * @return              Created rasterizer state object. */
GPURasterizerStatePtr VulkanGPUManager::createRasterizerState(const GPURasterizerStateDesc &desc) {
    return new VulkanRasterizerState(desc);
}

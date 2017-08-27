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
 * @brief               GPU command list interface.
 */

#include "gpu/command_list.h"
#include "gpu/gpu_manager.h"

/** Create a new command list for a render pass.
 * @param passInstance  Render pass instance pointer. */
GPUCommandList::GPUCommandList(GPURenderPassInstance *passInstance) :
    m_passInstance      (passInstance),
    m_parent            (nullptr),
    m_dirtyState        (kAllState),
    m_dirtyResourceSets (0)
{
    /* Set up default state. */
    m_state.blendState        = g_gpuManager->getBlendState();
    m_state.depthStencilState = g_gpuManager->getDepthStencilState();
    m_state.rasterizerState   = g_gpuManager->getRasterizerState();
    m_state.viewport          = m_passInstance->desc().renderArea;
    m_state.scissorEnabled    = false;
}

/** Create a new child command list.
 * @param parent        Parent command list.
 * @param inherit       Flags indicating which state to inherit. */
GPUCommandList::GPUCommandList(GPUCommandList *parent, uint32_t inherit) :
    m_passInstance      (parent->m_passInstance),
    m_parent            (parent),
    m_dirtyState        (kAllState),
    m_dirtyResourceSets (0)
{
    if (inherit & kPipelineState)
        m_state.pipeline = parent->m_state.pipeline;

    if (inherit & kResourceSetState) {
        for (size_t i = 0; i < m_state.resourceSets.size(); i++) {
            m_state.resourceSets[i] = parent->m_state.resourceSets[i];
            if (m_state.resourceSets[i])
                m_dirtyResourceSets |= 1 << i;
        }
    }

    m_state.blendState        = (inherit & kBlendState)
                                    ? parent->m_state.blendState
                                    : g_gpuManager->getBlendState();
    m_state.depthStencilState = (inherit & kDepthStencilState)
                                    ? parent->m_state.depthStencilState
                                    : g_gpuManager->getDepthStencilState();
    m_state.rasterizerState   = (inherit & kRasterizerState)
                                    ? parent->m_state.rasterizerState
                                    : g_gpuManager->getRasterizerState();
    m_state.viewport          = (inherit & kViewportState)
                                    ? parent->m_state.viewport
                                    : m_passInstance->desc().renderArea;

    if (inherit & kScissorState) {
        m_state.scissorEnabled = parent->m_state.scissorEnabled;
        m_state.scissor = parent->m_state.scissor;
    } else {
        m_state.scissorEnabled = false;
    }
}

/**
 * Delete the command list.
 *
 * Command lists are not reference counted like other GPU objects, since they
 * are transient objects used within a frame. Typically, they will be
 * automatically deleted as a result of calling either submitChild(), or
 * GPUManager::submitRenderPass(). However, should a command list need to be
 * discarded, it can be deleted manually.
 */
GPUCommandList::~GPUCommandList() {
    if (!m_parent)
        delete m_passInstance;
}

/** Bind a pipeline for rendering.
 * @param pipeline      Pipeline to use. */
void GPUCommandList::bindPipeline(GPUPipeline *pipeline) {
    check(pipeline);

    if (m_state.pipeline != pipeline) {
        m_state.pipeline = pipeline;
        m_dirtyState |= kPipelineState;
    }
}

/**
 * Bind a resource set.
 *
 * Binds the specified resource set to a set index for upcoming draws. Note
 * that after binding a resource set with this function, it must not be
 * changed for the remainder of the frame.
 *
 * @param index         Resource set index to bind to.
 * @param resources     Resource set to bind.
 */
void GPUCommandList::bindResourceSet(unsigned index, GPUResourceSet *resources) {
    check(index < m_state.resourceSets.size());
    check(resources);

    if (m_state.resourceSets[index] != resources) {
        m_state.resourceSets[index] = resources;
        m_dirtyResourceSets |= 1 << index;
        m_dirtyState |= kResourceSetState;
    }
}

/** Set the blend state.
 * @param state         Blend state to set. */
void GPUCommandList::setBlendState(GPUBlendState *state) {
    check(state);

    if (m_state.blendState != state) {
        m_state.blendState = state;
        m_dirtyState |= kBlendState;
    }
}

/** Set the blend state.
 * @param desc          Descriptor for the blend state to set. */
void GPUCommandList::setBlendState(const GPUBlendStateDesc &desc) {
    setBlendState(g_gpuManager->getBlendState(desc));
}

/** Set the depth/stencil state.
 * @param state         Depth/stencil state to set. */
void GPUCommandList::setDepthStencilState(GPUDepthStencilState *state) {
    check(state);

    if (m_state.depthStencilState != state) {
        m_state.depthStencilState = state;
        m_dirtyState |= kDepthStencilState;
    }
}

/** Set the depth/stencil state.
 * @param desc          Descriptor for the depth/stencil state to set. */
void GPUCommandList::setDepthStencilState(const GPUDepthStencilStateDesc &desc) {
    setDepthStencilState(g_gpuManager->getDepthStencilState(desc));
}

/** Set the rasterizer state.
 * @param state         Rasterizer state to set. */
void GPUCommandList::setRasterizerState(GPURasterizerState *state) {
    check(state);

    if (m_state.rasterizerState != state) {
        m_state.rasterizerState = state;
        m_dirtyState |= kRasterizerState;
    }
}

/** Set the rasterizer state.
 * @param desc          Descriptor for the rasterizer state to set. */
void GPUCommandList::setRasterizerState(const GPURasterizerStateDesc &desc) {
    setRasterizerState(g_gpuManager->getRasterizerState(desc));
}

/** Set the viewport.
 * @param viewport      Viewport rectangle in pixels. Must be <= size of
 *                      the current render target. */
void GPUCommandList::setViewport(const IntRect &viewport) {
    if (m_state.viewport != viewport) {
        check(m_passInstance->desc().renderArea.contains(viewport));

        m_state.viewport = viewport;
        m_dirtyState |= kViewportState;
    }
}

/** Set the scissor test parameters.
 * @param enable        Whether to enable scissor testing.
 * @param scissor       Scissor rectangle. */
void GPUCommandList::setScissor(bool enable, const IntRect &scissor) {
    if (m_state.scissorEnabled != enable || m_state.scissor != scissor) {
        check(!enable || m_passInstance->desc().renderArea.contains(scissor));

        m_state.scissorEnabled = enable;
        m_state.scissor = scissor;
        m_dirtyState |= kScissorState;
    }
}

/**
 * Save part of the current state.
 *
 * Pushes the parts of the current state indicated by the given flags onto the
 * state stack, to be restored by a later call to popState().
 *
 * @param state         Flags indicating the state parts to push.
 */
void GPUCommandList::pushState(uint32_t state) {
    m_stateStack.emplace_back();
    State &savedState = m_stateStack.back();
    savedState.pushed = state;

    if (state & kPipelineState)
        savedState.pipeline = m_state.pipeline;
    if (state & kResourceSetState)
        savedState.resourceSets = m_state.resourceSets;
    if (state & kBlendState)
        savedState.blendState = m_state.blendState;
    if (state & kDepthStencilState)
        savedState.depthStencilState = m_state.depthStencilState;
    if (state & kRasterizerState)
        savedState.rasterizerState = m_state.rasterizerState;
    if (state & kViewportState)
        savedState.viewport = m_state.viewport;
    if (state & kScissorState) {
        savedState.scissorEnabled = m_state.scissorEnabled;
        savedState.scissor = m_state.scissor;
    }
}

/** Restore state saved by the last call to pushState(). */
void GPUCommandList::popState() {
    const State &savedState = m_stateStack.back();

    if (savedState.pushed & kPipelineState)
        bindPipeline(savedState.pipeline);
    if (savedState.pushed & kResourceSetState) {
        for (size_t i = 0; i < savedState.resourceSets.size(); i++)
            bindResourceSet(i, savedState.resourceSets[i]);
    }
    if (savedState.pushed & kBlendState)
        setBlendState(savedState.blendState);
    if (savedState.pushed & kDepthStencilState)
        setDepthStencilState(savedState.depthStencilState);
    if (savedState.pushed & kRasterizerState)
        setRasterizerState(savedState.rasterizerState);
    if (savedState.pushed & kViewportState)
        setViewport(savedState.viewport);
    if (savedState.pushed & kScissorState)
        setScissor(savedState.scissorEnabled, savedState.scissor);

    m_stateStack.pop_back();
}

/**
 * Generic command list implementation.
 */

/** Create a new command list for a render pass.
 * @param passInstance  Render pass instance pointer. */
GPUGenericCommandList::GPUGenericCommandList(GPURenderPassInstance *passInstance) :
    GPUCommandList(passInstance)
{}

/** Create a new child command list.
 * @param parent        Parent command list.
 * @param inherit       Flags indicating which state to inherit. */
GPUGenericCommandList::GPUGenericCommandList(GPUCommandList *parent, uint32_t inherit) :
    GPUCommandList(parent, inherit)
{}

/** Destroy the command list. */
GPUGenericCommandList::~GPUGenericCommandList() {
    for (Command *command : m_commands) {
        /* We don't have virtual destructors as we don't want to have to have
         * a vtable on the command structure, must delete through a pointer to
         * the actual command type. */
        #define DELETE_TYPE(typeEnum, typeName) \
            case Command::typeEnum: delete static_cast<typeName *>(command); break

        switch (command->type) {
            DELETE_TYPE(kBindPipeline, CommandBindPipeline);
            DELETE_TYPE(kBindResourceSet, CommandBindResourceSet);
            DELETE_TYPE(kSetBlendState, CommandSetBlendState);
            DELETE_TYPE(kSetDepthStencilState, CommandSetDepthStencilState);
            DELETE_TYPE(kSetRasterizerState, CommandSetRasterizerState);
            DELETE_TYPE(kSetViewport, CommandSetViewport);
            DELETE_TYPE(kSetScissor, CommandSetScissor);
            DELETE_TYPE(kDraw, CommandDraw);
            DELETE_TYPE(kEndQuery, CommandEndQuery);
            DELETE_TYPE(kBeginDebugGroup, CommandBeginDebugGroup);
            DELETE_TYPE(kEndDebugGroup, CommandEndDebugGroup);
        }

        #undef DELETE_TYPE
    }
}

/** Create a child command list.
 * @param inherit       Flags indicating which state to inherit.
 * @return              Created command list. */
GPUCommandList *GPUGenericCommandList::createChild(uint32_t inherit) {
    return new GPUGenericCommandList(this, inherit);
}

/** Submit a child command list.
 * @param cmdList       Command list to submit. */
void GPUGenericCommandList::submitChild(GPUCommandList *cmdList) {
    /* Copy the command list onto the end of ours. */
    auto genericCmdList = static_cast<GPUGenericCommandList *>(cmdList);
    m_commands.splice(m_commands.end(), genericCmdList->m_commands);

    /* Our currently set state may have been invalidated by the child commands,
     * so we must flag things dirty to re-apply if necessary. */
    m_dirtyState |= kAllState;
    for (size_t i = 0; i < m_state.resourceSets.size(); i++) {
        if (m_state.resourceSets[i]) {
            m_dirtyResourceSets |= 1 << i;
            m_dirtyState |= kResourceSetState;
        }
    }

    delete cmdList;
}

/** Draw primitives.
 * @param type          Primitive type to render.
 * @param vertices      Vertex data to use.
 * @param indices       Index data to use (can be null). */
void GPUGenericCommandList::draw(PrimitiveType type, GPUVertexData *vertices, GPUIndexData *indices) {
    check(m_state.pipeline);

    /* Generate commands to apply state. This is delayed until it is actually
     * needed in order to avoid generating redundant commands if some state is
     * set and then replaced before a command actually requires it. */
    if (m_dirtyState & kPipelineState) {
        CommandBindPipeline *command = new CommandBindPipeline;
        command->pipeline = m_state.pipeline;
        m_commands.push_back(command);
        m_dirtyState &= ~kPipelineState;
    }

    if (m_dirtyState & kResourceSetState) {
        for (size_t i = 0; i < m_state.resourceSets.size(); i++) {
            if (m_dirtyResourceSets & (1 << i) && m_state.resourceSets[i]) {
                CommandBindResourceSet *command = new CommandBindResourceSet;
                command->index = i;
                command->resources = m_state.resourceSets[i];
                m_commands.push_back(command);
                m_dirtyResourceSets &= ~(1 << i);
            }
        }

        m_dirtyState &= ~kResourceSetState;
    }

    if (m_dirtyState & kBlendState) {
        CommandSetBlendState *command = new CommandSetBlendState;
        command->state = m_state.blendState;
        m_commands.push_back(command);
        m_dirtyState &= ~kBlendState;
    }

    if (m_dirtyState & kDepthStencilState) {
        CommandSetDepthStencilState *command = new CommandSetDepthStencilState;
        command->state = m_state.depthStencilState;
        m_commands.push_back(command);
        m_dirtyState &= ~kDepthStencilState;
    }

    if (m_dirtyState & kRasterizerState) {
        CommandSetRasterizerState *command = new CommandSetRasterizerState;
        command->state = m_state.rasterizerState;
        m_commands.push_back(command);
        m_dirtyState &= ~kRasterizerState;
    }

    if (m_dirtyState & kViewportState) {
        CommandSetViewport *command = new CommandSetViewport;
        command->viewport = m_state.viewport;
        m_commands.push_back(command);
        m_dirtyState &= ~kViewportState;
    }

    if (m_dirtyState & kScissorState) {
        CommandSetScissor *command = new CommandSetScissor;
        command->enable = m_state.scissorEnabled;
        command->scissor = m_state.scissor;
        m_commands.push_back(command);
        m_dirtyState &= ~kScissorState;
    }

    CommandDraw *command = new CommandDraw;
    command->type = type;
    command->vertices = vertices;
    command->indices = indices;
    m_commands.push_back(command);
}

/** End a query.
 * @param queryPool     Query pool the query is in.
 * @param index         Index of the query to end. */
void GPUGenericCommandList::endQuery(GPUQueryPool *queryPool, uint32_t index) {
    CommandEndQuery *command = new CommandEndQuery;
    command->queryPool = queryPool;
    command->index     = index;
    m_commands.push_back(command);
}

#ifdef ORION_BUILD_DEBUG

/** Begin a debug group.
 * @param str           Group string. */
void GPUGenericCommandList::beginDebugGroup(const std::string &str) {
    CommandBeginDebugGroup *command = new CommandBeginDebugGroup;
    command->str = str;
    m_commands.push_back(command);
}

/** End the current debug group. */
void GPUGenericCommandList::endDebugGroup() {
    CommandEndDebugGroup *command = new CommandEndDebugGroup;
    m_commands.push_back(command);
}

#endif

/** Execute then delete the command list.
 * @param context       Context to execute commands on. */
void GPUGenericCommandList::execute(Context *context) {
    for (Command *baseCommand : m_commands) {
        switch (baseCommand->type) {
            case Command::kBindPipeline:
            {
                auto command = static_cast<CommandBindPipeline *>(baseCommand);
                context->bindPipeline(command->pipeline);
                break;
            }

            case Command::kBindResourceSet:
            {
                auto command = static_cast<CommandBindResourceSet *>(baseCommand);
                context->bindResourceSet(command->index, command->resources);
                break;
            }

            case Command::kSetBlendState:
            {
                auto command = static_cast<CommandSetBlendState *>(baseCommand);
                context->setBlendState(command->state);
                break;
            }

            case Command::kSetDepthStencilState:
            {
                auto command = static_cast<CommandSetDepthStencilState *>(baseCommand);
                context->setDepthStencilState(command->state);
                break;
            }

            case Command::kSetRasterizerState:
            {
                auto command = static_cast<CommandSetRasterizerState *>(baseCommand);
                context->setRasterizerState(command->state);
                break;
            }

            case Command::kSetViewport:
            {
                auto command = static_cast<CommandSetViewport *>(baseCommand);
                context->setViewport(command->viewport);
                break;
            }

            case Command::kSetScissor:
            {
                auto command = static_cast<CommandSetScissor *>(baseCommand);
                context->setScissor(command->enable, command->scissor);
                break;
            }

            case Command::kDraw:
            {
                auto command = static_cast<CommandDraw *>(baseCommand);
                context->draw(command->type, command->vertices, command->indices);
                break;
            }

            case Command::kEndQuery:
            {
                auto command = static_cast<CommandEndQuery *>(baseCommand);
                context->endQuery(command->queryPool, command->index);
                break;
            }

            #ifdef ORION_BUILD_DEBUG

            case Command::kBeginDebugGroup:
            {
                auto command = static_cast<CommandBeginDebugGroup *>(baseCommand);
                context->beginDebugGroup(command->str);
                break;
            }

            case Command::kEndDebugGroup:
            {
                context->endDebugGroup();
                break;
            }

            #endif

            default:
                unreachable();
        }
    }

    delete this;
}

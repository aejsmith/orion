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

#pragma once

#include "gpu/index_data.h"
#include "gpu/pipeline.h"
#include "gpu/render_pass.h"
#include "gpu/resource.h"
#include "gpu/state.h"
#include "gpu/vertex_data.h"

#include "shader/resource.h"

#include <list>

/**
 * Class for recording GPU commands.
 *
 * This class is the primary interface for performing rendering commands.
 * Commands are recorded into command lists and later submitted through the GPU
 * manager. On modern APIs (e.g. Vulkan), this interface will directly translate
 * onto the API's command lists, while on traditional APIs (e.g. GL) commands
 * are recorded in a generic representation and later translated into API calls
 * when the command list is submitted.
 *
 * Command lists are recorded within a render pass: a command list is first
 * obtained from a call to GPUManager::beginRenderPass(). Commands can then be
 * recorded into the list, and submitted with GPUManager::submitRenderPass().
 *
 * This interface is intended to allow multithreaded usage. To support this,
 * command lists can be built in a hierarchy. An individual command list is not
 * thread-safe. Instead, to allow parallel recording of commands, child lists
 * can be created. These can be recorded in parallel, and then submitted
 * sequentially to the parent command list.
 */
class GPUCommandList : Noncopyable {
public:
    /** Structure storing rendering state. */
    struct State {
        GPUPipelinePtr pipeline;
        std::array<GPUResourceSetPtr, ResourceSets::kNumResourceSets> resourceSets;
        GPUBlendStatePtr blendState;
        GPUDepthStencilStatePtr depthStencilState;
        GPURasterizerStatePtr rasterizerState;
        IntRect viewport;
        bool scissorEnabled;
        IntRect scissor;

        /** In the state stack, the state bits pushed in this group. */
        uint32_t pushed;
    };

    /** Flags indicating which pieces of state to push/inherit. */
    enum : uint32_t {
        kNoState = 0,

        kPipelineState = (1 << 0),
        kResourceSetState = (1 << 1),
        kBlendState = (1 << 2),
        kDepthStencilState = (1 << 3),
        kRasterizerState = (1 << 4),
        kViewportState = (1 << 5),
        kScissorState = (1 << 6),

        kAllState =
            kPipelineState | kResourceSetState | kBlendState | kDepthStencilState |
            kRasterizerState | kViewportState | kScissorState,
    };

    virtual ~GPUCommandList();

    /**
     * Create a child command list.
     *
     * Creates a new command list as a child of this one. The new command list
     * will inherit the state indicated by the given flags from this one. This
     * has no effect on the state of this command list, nor will any changes
     * made to the child list.
     *
     * @param inherit       Flags indicating which state to inherit.
     *
     * @return              Created command list.
     */
    virtual GPUCommandList *createChild(uint32_t inherit = kAllState) = 0;

    /**
     * Submit a child command list.
     *
     * Submits the commands recorded in the given child command list into this
     * command list. Those commands will be performed after all commands
     * recorded in this list prior to this call, and before any recorded after
     * the call. The child list is deleted by this function, therefore must not
     * be used after calling it.
     *
     * @param cmdList       Command list to submit.
     */
    virtual void submitChild(GPUCommandList *cmdList) = 0;

    /**
     * State methods.
     */

    void bindPipeline(GPUPipeline *pipeline);
    void bindResourceSet(unsigned index, GPUResourceSet *resources);
    void setBlendState(GPUBlendState *state);
    void setBlendState(const GPUBlendStateDesc &desc = GPUBlendStateDesc());
    void setDepthStencilState(GPUDepthStencilState *state);
    void setDepthStencilState(const GPUDepthStencilStateDesc &desc = GPUDepthStencilStateDesc());
    void setRasterizerState(GPURasterizerState *state);
    void setRasterizerState(const GPURasterizerStateDesc &desc = GPURasterizerStateDesc());
    void setViewport(const IntRect &viewport);
    void setScissor(bool enable, const IntRect &scissor);

    void pushState(uint32_t state);
    void popState();

    /**
     * Commands.
     */

    /** Draw primitives.
     * @param type          Primitive type to render.
     * @param vertices      Vertex data to use.
     * @param indices       Index data to use (can be null). */
    virtual void draw(PrimitiveType type, GPUVertexData *vertices, GPUIndexData *indices) = 0;

    /**
     * Debug methods.
     */

    #ifdef ORION_BUILD_DEBUG

    /** Begin a debug group.
     * @param str           Group string. */
    virtual void beginDebugGroup(const std::string &str) {}

    /** End the current debug group. */
    virtual void endDebugGroup() {}

    #endif

    /**
     * State query methods.
     */

    /** @return             Render pass this command list is for. */
    const GPURenderPass *pass() const { return m_passInstance->desc().pass; }
    /** @return             Render pass instance this command list is for. */
    const GPURenderPassInstance *passInstance() const { return m_passInstance; }
    /** @reteurn            Current rendering state. */
    const State &state() const { return m_state; }
protected:
    explicit GPUCommandList(GPURenderPassInstance *passInstance);
    GPUCommandList(GPUCommandList *parent, uint32_t inherit);

    /** Render pass instance. */
    GPURenderPassInstance *m_passInstance;

    GPUCommandList *m_parent;               /**< Parent command list. */
    State m_state;                          /**< Current state. */
    uint32_t m_dirtyState;                  /**< Dirty state flags. */
    uint32_t m_dirtyResourceSets;           /**< Dirty resource set bindings. */
    std::list<State> m_stateStack;          /**< State stack for push/pop. */
};

/**
 * Generic command list implementation.
 *
 * This is used by APIs which don't have native command list support. We create
 * our own list of commands which gets converted into API calls when the render
 * pass is submitted.
 */
class GPUGenericCommandList : public GPUCommandList {
public:
    explicit GPUGenericCommandList(GPURenderPassInstance *passInstance);
    GPUGenericCommandList(GPUCommandList *parent, uint32_t inherit);
    ~GPUGenericCommandList();

    GPUCommandList *createChild(uint32_t inherit) override;
    void submitChild(GPUCommandList *cmdList) override;

    void draw(PrimitiveType type, GPUVertexData *vertices, GPUIndexData *indices) override;

    #ifdef ORION_BUILD_DEBUG
    void beginDebugGroup(const std::string &str) override;
    void endDebugGroup() override;
    #endif

    /** Context to execute commands on. */
    class Context {
    public:
        virtual void bindPipeline(GPUPipeline *pipeline) = 0;
        virtual void bindResourceSet(unsigned index, GPUResourceSet *resources) = 0;
        virtual void setBlendState(GPUBlendState *state) = 0;
        virtual void setDepthStencilState(GPUDepthStencilState *state) = 0;
        virtual void setRasterizerState(GPURasterizerState *state) = 0;
        virtual void setViewport(const IntRect &viewport) = 0;
        virtual void setScissor(bool enable, const IntRect &scissor) = 0;
        virtual void draw(PrimitiveType type, GPUVertexData *vertices, GPUIndexData *indices) = 0;

        #ifdef ORION_BUILD_DEBUG
        virtual void beginDebugGroup(const std::string &str) = 0;
        virtual void endDebugGroup() = 0;
        #endif
    };

    void execute(Context *context);
private:
    /** Base command structure. */
    struct Command {
        /** Type of the command. */
        enum Type {
            kBindPipeline,
            kBindResourceSet,
            kSetBlendState,
            kSetDepthStencilState,
            kSetRasterizerState,
            kSetViewport,
            kSetScissor,
            kDraw,
            kBeginDebugGroup,
            kEndDebugGroup,
        };

        Type type;

        explicit Command(Type inType) : type(inType) {}
    };

    struct CommandBindPipeline : Command {
        GPUPipelinePtr pipeline;

        CommandBindPipeline() : Command(kBindPipeline) {}
    };

    struct CommandBindResourceSet : Command {
        unsigned index;
        GPUResourceSetPtr resources;

        CommandBindResourceSet() : Command(kBindResourceSet) {}
    };

    struct CommandSetBlendState : Command {
        GPUBlendStatePtr state;

        CommandSetBlendState() : Command(kSetBlendState) {}
    };

    struct CommandSetDepthStencilState : Command {
        GPUDepthStencilStatePtr state;

        CommandSetDepthStencilState() : Command(kSetDepthStencilState) {}
    };

    struct CommandSetRasterizerState : Command {
        GPURasterizerStatePtr state;

        CommandSetRasterizerState() : Command(kSetRasterizerState) {}
    };

    struct CommandSetViewport : Command {
        IntRect viewport;

        CommandSetViewport() : Command(kSetViewport) {}
    };

    struct CommandSetScissor : Command {
        bool enable;
        IntRect scissor;

        CommandSetScissor() : Command(kSetScissor) {}
    };

    struct CommandDraw : Command {
        PrimitiveType type;
        GPUVertexDataPtr vertices;
        GPUIndexDataPtr indices;

        CommandDraw() : Command(kDraw) {}
    };

    struct CommandBeginDebugGroup : Command {
        std::string str;

        CommandBeginDebugGroup() : Command(kBeginDebugGroup) {}
    };

    struct CommandEndDebugGroup : Command {
        CommandEndDebugGroup() : Command(kEndDebugGroup) {}
    };

    std::list<Command *> m_commands;
};

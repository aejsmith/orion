/*
 * Copyright (C) 2015 Alex Smith
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
 * @brief               Engine main class.
 */

#include "core/filesystem.h"
#include "core/string.h"

#include "engine/asset_manager.h"
#include "engine/debug_manager.h"
#include "engine/engine.h"
#include "engine/game.h"
#include "engine/window.h"
#include "engine/world.h"

#include "gpu/gpu_manager.h"

#include "input/input_manager.h"

#include "physics/physics_manager.h"

#include "render/render_manager.h"

#include "shader/shader_manager.h"

#include <SDL.h>

/** Global instance of the engine. */
Engine *g_engine = nullptr;

/** Initialize the engine.
 * @param config        Engine configuration structure. */
Engine::Engine(const EngineConfiguration &config) :
    m_config(config),
    m_world(nullptr),
    m_lastTick(0),
    m_lastFPS(0),
    m_frames(0)
{
    check(!g_engine);
    g_engine = this;

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
        fatal("Failed to initialize SDL: %s", SDL_GetError());

    /* Create the debug manager early to allow other systems to register things
     * with it. Rendering resources are initialised later. */
    g_debugManager = new DebugManager;

    /* Initialize the log. */
    g_logManager = new LogManager;
    logInfo("Orion revision %s built at %s", g_versionString, g_versionTimestamp);

    /* Initialize platform systems. */
    g_filesystem = Platform::createFilesystem();

    /* Create the GPU manager, create the main window, and finally properly
     * initialize the GPU interface. */
    g_gpuManager = GPUManager::create(config);
    g_mainWindow = new Window(config);
    g_gpuManager->init();

    /* Initialize other global systems. */
    g_inputManager = new InputManager;
    g_shaderManager = new ShaderManager;
    g_assetManager = new AssetManager;
    g_renderManager = new RenderManager;
    g_renderManager->init();
    g_debugManager->initResources();
    g_physicsManager = new PhysicsManager;

    /* Create the game instance. */
    m_game = game::createGame();
}

/** Shut down the engine. */
Engine::~Engine() {
    /* Unload the world. */
    m_world.reset();

    /* Shut down the game. */
    delete m_game;

    /* Shut down global systems. */
    delete g_physicsManager;
    delete g_renderManager;
    delete g_shaderManager;
    delete g_debugManager;
    delete g_assetManager;
    delete g_inputManager;
    delete g_gpuManager;
    delete g_mainWindow;
    delete g_filesystem;
    delete g_logManager;

    SDL_Quit();
    g_engine = nullptr;
}

/** Run the engine main loop. */
void Engine::run() {
    while (true) {
        uint32_t startTicks = SDL_GetTicks();

        if (!pollEvents())
            return;

        g_debugManager->startFrame();

        /* Display statistics from the previous frame. */
        g_debugManager->writeText(String::format("FPS: %.1f\n", m_stats.fps));
        g_debugManager->writeText(String::format("Frame time: %.0f ms\n", m_stats.frameTime * 1000.0f));
        g_debugManager->writeText(String::format("Draw calls: %u\n", m_stats.drawCalls));

        /* Reset frame statistics. */
        m_stats.drawCalls = 0;

        m_game->startFrame();

        tick();
        renderAllTargets();

        /* Present the final rendered frame. */
        g_gpuManager->endFrame(m_config.displayVsync);

        /* Clear out debug primitives from this frame. */
        g_debugManager->endFrame();

        m_game->endFrame();

        /* Update statistics. */
        m_frames++;
        uint32_t endTicks = SDL_GetTicks();
        uint32_t frameTicks = endTicks - startTicks;
        m_stats.frameTime = static_cast<float>(frameTicks) / 1000.0f;
    }
}

/** Poll for pending SDL events.
 * @return              Whether to continue executing. */
bool Engine::pollEvents() {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        // FIXME: Need an Engine::quit() method
        if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE)
            return false;

        if (g_inputManager->handleEvent(&event))
            continue;

        switch (event.type) {
            case SDL_QUIT:
                return false;
        }
    }

    return true;
}

/** Tick the world. */
void Engine::tick() {
    uint32_t tick = SDL_GetTicks();

    if (m_lastTick && tick != m_lastTick) {
        /* Update the world. */
        float dt = static_cast<float>(tick - m_lastTick) / 1000.0f;
        m_world->tick(dt);
    }

    m_lastTick = tick;

    /* Update FPS counter. */
    if (!m_lastFPS || (tick - m_lastFPS) > 1000) {
        m_stats.fps = (m_lastFPS)
            ? static_cast<float>(m_frames) / (static_cast<float>(tick - m_lastFPS) / 1000.0f)
            : 0;

        g_mainWindow->setTitle(
            m_config.title + " [FPS: " + std::to_string(static_cast<unsigned>(m_stats.fps)) + "]");

        m_lastFPS = tick;
        m_frames = 0;
    }
}

/** Render all render targets. */
void Engine::renderAllTargets() {
    for (RenderTarget *target : m_renderTargets)
        target->render();
}

/** Create a world and make it the active world.
 * @return              Created world. */
World *Engine::createWorld() {
    m_world.reset();

    m_world = new World;
    return m_world;
}

/**
 * Add a render target to the main rendering loop.
 *
 * Adds an active render target to the main rendering loop to be updated. This
 * should not be called manually - a render target will automatically be added
 * here when a view is added to it.
 *
 * @param target        Render target to add.
 */
void Engine::addRenderTarget(RenderTarget *target) {
    /* List is sorted by priority. */
    for (auto it = m_renderTargets.begin(); it != m_renderTargets.end(); ++it) {
        RenderTarget *exist = *it;
        if (target->priority() < exist->priority()) {
            m_renderTargets.insert(it, target);
            return;
        }
    }

    /* Insertion point not found, add at end. */
    m_renderTargets.push_back(target);
}

/** Remove a render target from the main rendering loop.
 * @param target        Render target to remove. */
void Engine::removeRenderTarget(RenderTarget *target) {
    m_renderTargets.remove(target);
}

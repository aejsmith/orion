/*
 * Copyright (C) 2015-2017 Alex Smith
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

#pragma once

#include "core/listener.h"

#include "engine/object.h"

#include <list>

class Game;
class RenderTarget;
class World;

/**
 * Engine configuration.
 *
 * TODO:
 *  - Eventually this will only contain static configuration for the application,
 *    e.g. the title. Dynamic settings like screen resolution will move to some
 *    preferences class that will save and restore settings.
 */
struct EngineConfiguration {
    /** Title of the game. */
    std::string title;

    /** Display parameters. */
    uint32_t displayWidth;          /**< Screen width. */
    uint32_t displayHeight;         /**< Screen height. */
    bool displayFullscreen;         /**< Whether the window should be fullscreen. */
    bool displayVsync;              /**< Whether to synchronize updates with vertical retrace. */
};

/** Engine statistics. */
struct EngineStats {
    float fps;                      /**< Number of frames per second. */
    float frameTime;                /**< Last frame time in seconds. */
    unsigned drawCalls;             /**< Number of draw calls in the last frame. */
public:
    EngineStats() :
        fps       (0),
        frameTime (0),
        drawCalls (0)
    {}
};

/** Main class of the engine. */
class Engine : Noncopyable {
public:
    /** Frame listener class. */
    class FrameListener : public Listener<FrameListener> {
    public:
        /** Called at the start of a new frame. */
        virtual void frameStarted() = 0;
    };

    Engine(int argc, char **argv);
    ~Engine();

    void run();

    /** @return             Engine configuration. */
    const EngineConfiguration &config() const { return m_config; }
    /** @return             Game instance. */
    Game *game() const { return m_game; }
    /** @return             Engine statistics. */
    EngineStats &stats() { return m_stats; }
    /** @return             Engine command line arguments. */
    const std::vector<std::string> &arguments() const { return m_arguments; }

    /**
     * World management.
     */

    World *createWorld();
    World *loadWorld(const std::string &path);

    /** @return             Active game world. */
    World *world() const { return m_world; }

    /**
     * Rendering loop.
     */

    void addRenderTarget(RenderTarget *target);
    void removeRenderTarget(RenderTarget *target);

    /**
     * Event handling.
     */

    void addFrameListener(FrameListener *listener);
private:
    /**
     * Main loop functions.
     */

    bool pollEvents();
    void tick();
    void renderAllTargets();
private:
    EngineConfiguration m_config;   /**< Engine configuration. */
    ObjectPtr<Game> m_game;         /**< Game instance. */
    ObjectPtr<World> m_world;       /**< Active game world. */

    /** List of render targets. */
    std::list<RenderTarget *> m_renderTargets;

    /** Event notification. */
    Notifier<FrameListener> m_frameNotifier;

    /** Timing information. */
    uint32_t m_lastTick;            /**< Last tick time. */
    uint32_t m_lastFPS;             /**< Last FPS value. */
    uint32_t m_frames;              /**< Number of frames rendered since last FPS update. */

    /** Engine statistics. */
    EngineStats m_stats;

    /** Command line arguments. */
    std::vector<std::string> m_arguments;
};

extern Engine *g_engine;

extern const char *g_versionString;
extern const char *g_versionTimestamp;

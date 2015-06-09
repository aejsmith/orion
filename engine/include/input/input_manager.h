/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Input manager.
 */

#pragma once

#include "input/defs.h"

#include <list>

class InputHandler;
union SDL_Event;

/** Global input manager class. */
class InputManager : Noncopyable {
public:
    InputManager();

    bool handleEvent(SDL_Event *event);
private:
    void registerHandler(InputHandler *handler);
    void unregisterHandler(InputHandler *handler);
private:
    /** List of handlers, sorted by priority. */
    std::list<InputHandler *> m_handlers;

    friend class InputHandler;
};

extern EngineGlobal<InputManager> g_inputManager;

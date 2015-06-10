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

    uint32_t getModifiers();
    bool getButtonState(InputCode code);
    glm::ivec2 getCursorPosition();

    void setMouseCaptured(bool captured);

    /** @return             Whether the mouse is captured. */
    bool mouseCaptured() const { return m_mouseCaptured; }

    bool handleEvent(SDL_Event *event);
private:
    void registerHandler(InputHandler *handler);
    void unregisterHandler(InputHandler *handler);
private:
    bool m_mouseCaptured;               /**< Whether the mouse is captured. */

    /** List of handlers, sorted by priority. */
    std::list<InputHandler *> m_handlers;

    friend class InputHandler;
};

extern EngineGlobal<InputManager> g_inputManager;

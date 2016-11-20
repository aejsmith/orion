/*
 * Copyright (C) 2015-2016 Alex Smith
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
    void beginTextInput(InputHandler *handler);
    void endTextInput(InputHandler *handler);
private:
    bool m_mouseCaptured;               /**< Whether the mouse is captured. */

    /** List of handlers, sorted by priority. */
    std::list<InputHandler *> m_handlers;

    InputHandler *m_textInputHandler;   /**< Current text input handler. */

    friend class InputHandler;
};

extern InputManager *g_inputManager;

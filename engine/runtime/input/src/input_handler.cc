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
 * @brief               Input handler class.
 */

#include "input/input_handler.h"
#include "input/input_manager.h"

/** Initialise the input handler.
 * @param priority      Input handling priority. */
InputHandler::InputHandler(Priority priority) :
    m_priority   (priority),
    m_registered (false)
{}

/** Destroy the input handler. */
InputHandler::~InputHandler() {
    if (m_registered)
        unregisterInputHandler();
}

/** Set the input handling priority.
 * @param priority      New input handling priority. */
void InputHandler::setInputPriority(Priority priority) {
    if (m_registered) {
        unregisterInputHandler();
        m_priority = priority;
        registerInputHandler();
    } else {
        m_priority = priority;
    }
}

/** Register the handler with the input manager. */
void InputHandler::registerInputHandler() {
    check(!m_registered);

    g_inputManager->registerHandler(this);
    m_registered = true;
}

/** Unregister the handler from the input manager. */
void InputHandler::unregisterInputHandler() {
    check(m_registered);

    g_inputManager->unregisterHandler(this);
    m_registered = false;
}

/**
 * Begin text input.
 *
 * This function starts collecting text input and delivers the input to this
 * handler via handleTextInput(). When text input is no longer required,
 * endTextInput() should be called to stop collecting input.
 */
void InputHandler::beginTextInput() {
    g_inputManager->beginTextInput(this);
}

/** End text input. */
void InputHandler::endTextInput() {
    g_inputManager->endTextInput(this);
}

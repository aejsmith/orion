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

#pragma once

#include "input/input_event.h"

/**
 * Input handler base class.
 *
 * Classes which wish to handle input should derive from this class and
 * implement the handler methods. When requested, the handler will be added to
 * the input manager.
 */
class InputHandler {
public:
    /** Input handling priority definitions. */
    enum Priority {
        kDebugOverlayPriority,      /**< Debug overlay. */
        kGUIPriority,               /**< GUI. */
        kWorldPriority,             /**< World entities. */
    };
public:
    virtual ~InputHandler();

    /** @return             Input handling priority. */
    Priority inputPriority() const { return m_priority; }
protected:
    explicit InputHandler(Priority priority = kWorldPriority);

    void setInputPriority(Priority priority);

    void registerInputHandler();
    void unregisterInputHandler();

    void beginTextInput();
    void endTextInput();

    /** Handle a button down event.
     * @param event         Button event.
     * @return              Whether the event was handled. If false, the event
     *                      will be passed to the next highest priority input
     *                      handler. */
    virtual bool handleButtonDown(const ButtonEvent &event) { return false; }

    /** Handle a button up event.
     * @param event         Button event.
     * @return              Whether the event was handled. If false, the event
     *                      will be passed to the next highest priority input
     *                      handler. */
    virtual bool handleButtonUp(const ButtonEvent &event) { return false; }

    /** Handle an axis event.
     * @param event         Axis event.
     * @return              Whether the event was handled. If false, the event
     *                      will be passed to the next highest priority input
     *                      handler. */
    virtual bool handleAxis(const AxisEvent &event) { return false; }

    /** Handle a text input event.
     * @param event         Text input event. */
    virtual void handleTextInput(const TextInputEvent &event) { }
private:
    Priority m_priority;            /**< Input handling priority. */
    bool m_registered;              /**< Whether the handler is registered with the input manager. */

    friend class InputManager;
};

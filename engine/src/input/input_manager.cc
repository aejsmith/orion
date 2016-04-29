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
 * @brief               Input manager.
 */

#include "input/input_handler.h"
#include "input/input_manager.h"

#include <functional>

#include <SDL.h>

/** Global input manager. */
InputManager *g_inputManager;

/** Initialise the input manager. */
InputManager::InputManager() :
    m_mouseCaptured(false)
{}

/** Get the current input modifier state.
 * @return              Bitmask of currently held modifier keys. */
uint32_t InputManager::getModifiers() {
    SDL_Keymod sdlModifiers = SDL_GetModState();

    uint32_t modifiers = 0;
    if (sdlModifiers & KMOD_LSHIFT)
        modifiers |= InputModifier::kLeftShift;
    if (sdlModifiers & KMOD_RSHIFT)
        modifiers |= InputModifier::kRightShift;
    if (sdlModifiers & KMOD_LCTRL)
        modifiers |= InputModifier::kLeftCtrl;
    if (sdlModifiers & KMOD_RCTRL)
        modifiers |= InputModifier::kRightCtrl;
    if (sdlModifiers & KMOD_LALT)
        modifiers |= InputModifier::kLeftAlt;
    if (sdlModifiers & KMOD_RALT)
        modifiers |= InputModifier::kRightAlt;
    if (sdlModifiers & KMOD_LGUI)
        modifiers |= InputModifier::kLeftSuper;
    if (sdlModifiers & KMOD_RGUI)
        modifiers |= InputModifier::kRightSuper;
    if (sdlModifiers & KMOD_NUM)
        modifiers |= InputModifier::kNumLock;
    if (sdlModifiers & KMOD_CAPS)
        modifiers |= InputModifier::kCapsLock;

    return modifiers;
}

/** Get the state of a button input.
 * @param code          Input code for button to get state of (must refer to a
 *                      button input).
 * @return              Whether the button is currently pressed. */
bool InputManager::getButtonState(InputCode code) {
    const InputInfo *inputInfo = InputInfo::lookup(code);
    checkMsg(inputInfo, "Input code %d is invalid", code);
    checkMsg(inputInfo->type == InputType::kButton, "Input %d is not a button", code);

    if (code < InputCode::kNumKeyboardCodes) {
        const uint8_t *keyboardState = SDL_GetKeyboardState(NULL);
        return keyboardState[static_cast<size_t>(code)];
    } else if (code < InputCode::kNumMouseCodes) {
        uint32_t mouseState = SDL_GetMouseState(nullptr, nullptr);

        switch (code) {
            case InputCode::kMouseLeft:
                return mouseState & SDL_BUTTON(SDL_BUTTON_LEFT);
            case InputCode::kMouseMiddle:
                return mouseState & SDL_BUTTON(SDL_BUTTON_MIDDLE);
            case InputCode::kMouseRight:
                return mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT);
            default:
                unreachable();
        }
    } else {
        unreachable();
    }
}

/** Get the current mouse cursor position.
 * @return              Vector containing X/Y position of the mouse cursor. */
glm::ivec2 InputManager::getCursorPosition() {
    int x, y;
    SDL_GetMouseState(&x, &y);
    return glm::ivec2(x, y);
}

/** Set whether the mouse is captured.
 * @param captured      Whether the mouse should be captured. */
void InputManager::setMouseCaptured(bool captured) {
    if (m_mouseCaptured != captured) {
        SDL_SetRelativeMouseMode((captured) ? SDL_TRUE : SDL_FALSE);
        m_mouseCaptured = captured;
    }
}

/** Handle an SDL event.
 * @param event         SDL event structure.
 * @return              Whether the event was handled. */
bool InputManager::handleEvent(SDL_Event *event) {
    uint32_t modifiers = getModifiers();

    /** Dispatch an event to a handler.
     * @param function      Function to call. */
    auto dispatchInputEvent = [&] (auto function) {
        for (InputHandler *handler : m_handlers) {
            if (function(handler))
                break;
        }
    };

    /* Process the event. */
    switch (event->type) {
        case SDL_KEYDOWN:
            /* Ignore repeats for now. FIXME: Want to handle this in future for
             * text input, etc. */
            if (event->key.repeat)
                return false;

        case SDL_KEYUP:
        {
            /* Map the scan code to an input code. */
            InputCode inputCode = static_cast<InputCode>(event->key.keysym.scancode);
            const InputInfo *inputInfo = InputInfo::lookup(inputCode);
            if (!inputInfo) {
                logWarning("Unrecognised scan code %u", event->key.keysym.scancode);
                return false;
            }

            /* Get the character representation, if any, of this code. SDL's
             * keycodes are defined to ASCII values if they have a printable
             * representation, or the scancode with bit 30 set otherwise. */
            char character = (!(event->key.keysym.sym & SDLK_SCANCODE_MASK))
                ? event->key.keysym.sym
                : 0;

            ButtonEvent buttonEvent(inputInfo, modifiers, character);

            dispatchInputEvent([&] (InputHandler *handler) {
                if (event->type == SDL_KEYDOWN) {
                    return handler->handleButtonDown(buttonEvent);
                } else {
                    return handler->handleButtonUp(buttonEvent);
                }
            });

            return true;
        }

        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
        {
            /* Convert SDL's button to our own. */
            InputCode inputCode;
            switch (event->button.button) {
                case SDL_BUTTON_LEFT:
                    inputCode = InputCode::kMouseLeft;
                    break;
                case SDL_BUTTON_RIGHT:
                    inputCode = InputCode::kMouseRight;
                    break;
                case SDL_BUTTON_MIDDLE:
                    inputCode = InputCode::kMouseMiddle;
                    break;
                default:
                    logWarning("Unrecognised SDL button code %u", event->button.button);
                    return false;
            }

            const InputInfo *inputInfo = InputInfo::lookup(inputCode);

            ButtonEvent buttonEvent(inputInfo, modifiers, 0);

            dispatchInputEvent([&] (InputHandler *handler) {
                if (event->type == SDL_MOUSEBUTTONDOWN) {
                    return handler->handleButtonDown(buttonEvent);
                } else {
                    return handler->handleButtonUp(buttonEvent);
                }
            });

            return true;
        }

        case SDL_MOUSEMOTION:
        {
            if (event->motion.xrel) {
                const InputInfo *inputInfo = InputInfo::lookup(InputCode::kMouseX);

                AxisEvent axisEvent(inputInfo, modifiers, event->motion.xrel);

                dispatchInputEvent([&] (InputHandler *handler) {
                    return handler->handleAxis(axisEvent);
                });
            }

            if (event->motion.yrel) {
                const InputInfo *inputInfo = InputInfo::lookup(InputCode::kMouseY);

                AxisEvent axisEvent(inputInfo, modifiers, event->motion.yrel);

                dispatchInputEvent([&] (InputHandler *handler) {
                    return handler->handleAxis(axisEvent);
                });
            }

            return true;
        }

        case SDL_MOUSEWHEEL:
        {
            const InputInfo *inputInfo = InputInfo::lookup(InputCode::kMouseScroll);

            AxisEvent axisEvent(inputInfo, modifiers, event->wheel.y);

            dispatchInputEvent([&] (InputHandler *handler) {
                return handler->handleAxis(axisEvent);
            });

            return true;
        }
    }

    return false;
}

/** Register an input handler.
 * @param handler       Handler to register. */
void InputManager::registerHandler(InputHandler *handler) {
    /* List is sorted by priority. */
    for (auto it = m_handlers.begin(); it != m_handlers.end(); ++it) {
        InputHandler *exist = *it;
        if (handler->inputPriority() < exist->inputPriority()) {
            m_handlers.insert(it, handler);
            return;
        }
    }

    /* Insertion point not found, add at end. */
    m_handlers.push_back(handler);
}

/** Unregister an input handler.
 * @param handler       Handler to unregister. */
void InputManager::unregisterHandler(InputHandler *handler) {
    m_handlers.remove(handler);
}

/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Input handler class.
 */

#include "input/input_handler.h"
#include "input/input_manager.h"

/** Initialise the input handler.
 * @param priority      Input handling priority. */
InputHandler::InputHandler(Priority priority) :
    m_priority(priority),
    m_registered(false)
{}

/** Destroy the input handler. */
InputHandler::~InputHandler() {
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
    if (!m_registered) {
        g_inputManager->registerHandler(this);
        m_registered = true;
    }
}

/** Unregister the handler from the input manager. */
void InputHandler::unregisterInputHandler() {
    if (m_registered) {
        g_inputManager->unregisterHandler(this);
        m_registered = false;
    }
}

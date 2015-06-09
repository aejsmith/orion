/**
 * @file
 * @copyright           2015 Alex Smith
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
        kConsolePriority,           /**< Console. */
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
private:
    Priority m_priority;            /**< Input handling priority. */
    bool m_registered;              /**< Whether the handler is registered with the input manager. */

    friend class InputManager;
};

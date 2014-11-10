/**
 * @file
 * @copyright           2014 Alex Smith
 * @brief               Game interface.
 */

#pragma once

#include "core/core.h"

/** Global game class. */
class Game : Noncopyable {
public:
    Game() {}
    ~Game() {}
};

/** Interface to game code. */
namespace game {
    /** Get the engine configuration.
     * @param config        Engine configuration to fill in. */
    void engineConfiguration(EngineConfiguration &config);

    /** Create the Game instance.
     * @return              Created Game instance. */
    Game *createGame();
}

/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Main engine startup function.
 */

#include "engine/engine.h"
#include "engine/game.h"

#include <SDL.h>

/** Engine main entry point.
 * @param argc          Argument count.
 * @param argv          Argument array. */
int main(int argc, char **argv) {
    EngineConfiguration config;
    game::engineConfiguration(config);

    Engine engine(config);
    engine.run();
    return 0;
}

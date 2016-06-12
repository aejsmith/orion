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
 * @brief               Game interface.
 */

#pragma once

#include "engine/engine.h"

/**
 * Global game class.
 *
 * This is the class that is responsible for configuring the engine and
 * setting up the game once the engine has been initialised. Game code must
 * define a single class which is derived from this. It will be looked up by
 * the engine and an instance of it will be constructed early in initialisation.
 * Once the engine is initialised, the init() method will be called to set the
 * game up.
 */
class Game : public Object {
public:
    CLASS();

    /** Get the engine configuration.
     * @param config        Engine configuration to fill in. */
    virtual void engineConfiguration(EngineConfiguration &config) = 0;

    /** Initialise the game. */
    virtual void init() = 0;

    /** Called at the beginning of each frame. */
    virtual void startFrame() {}

    /** Called at the end of each frame. */
    virtual void endFrame() {}
protected:
    Game() {}
};

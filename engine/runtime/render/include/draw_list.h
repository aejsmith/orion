/*
 * Copyright (C) 2017 Alex Smith
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
 * @brief               Draw list class.
 */

#pragma once

#include "gpu/command_list.h"

#include <deque>

class Pass;
class RenderEntity;

/**
 * Class maintaining a list of draws.
 *
 * This class builds up a list of draw calls to perform for a set of entities.
 * For now it is just a simple list, but later it will handle sorting of draw
 * calls.
 */
class DrawList {
public:
    DrawList();
    ~DrawList();

    void add(RenderEntity *entity, const std::string &passType);

    void draw(GPUCommandList *cmdList, const ShaderKeywordSet &variation);
private:
    /** Structure containing details of a single draw. */
    struct Draw {
        RenderEntity *entity;
        const Pass *pass;
    };

    std::deque<Draw> m_draws;           /**< List of draws. */
};

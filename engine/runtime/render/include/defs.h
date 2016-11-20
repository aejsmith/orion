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
 * @brief               Renderer definitions.
 */

#pragma once

#include "core/pixel_format.h"

/** Rendering path enumeration. */
enum class RenderPath {
    kForward,                       /**< Forward rendering. */
    kDeferred,                      /**< Deferred lighting. */
};

/** Screen buffer pixel formats. */
static const PixelFormat kScreenColourBufferFormat = PixelFormat::kR8G8B8A8;
static const PixelFormat kScreenDepthBufferFormat = PixelFormat::kDepth24Stencil8;

/** Shadow map pixel format. */
static const PixelFormat kShadowMapFormat = PixelFormat::kDepth24Stencil8;

/**
 * G-Buffer pixel formats. The buffer layout is as follows:
 *
 *     | Format      | R          | G          | B          | A
 *  ---|-------------|------------|------------|------------|------------
 *   A | R10G10B10A2 | Normal.x   | Normal.y   | Normal.z   | -
 *  ---|-------------|------------|------------|------------|------------
 *   B | R8G8B8A8    | Diffuse.r  | Diffuse.g  | Diffuse.b  | -
 *  ---|-------------|------------|------------|------------|------------
 *   C | R8G8B8A8    | Specular.r | Specular.g | Specular.b | 1/Shininess
 *  ---|-------------|------------|------------|------------|------------
 *   D | D24S8       | Depth      | -          | -          | -
 *
 * These are all unsigned normalized textures, therefore the normals are
 * scaled to fit into the [0, 1] range, and the shininess is stored as
 * its reciprocal. Position is reconstructed from the depth buffer.
 */
static const PixelFormat kDeferredBufferAFormat = PixelFormat::kR10G10B10A2;
static const PixelFormat kDeferredBufferBFormat = PixelFormat::kR8G8B8A8;
static const PixelFormat kDeferredBufferCFormat = PixelFormat::kR8G8B8A8;
static const PixelFormat kDeferredBufferDFormat = PixelFormat::kDepth24Stencil8;

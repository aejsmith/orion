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
 * @brief               Skybox component.
 */

#pragma once

#include "engine/texture.h"

#include "graphics/renderer.h"

#include "shader/material.h"

/**
 * Component which renders a skybox.
 *
 * This component renders a skybox. A skybox is a textured box which is drawn
 * around the entire world to represent what is in the distance. It is drawn on
 * the far plane, so behind anything else rendered in the scene.
 */
class Skybox : public Renderer {
public:
    CLASS();

    Skybox();

    VPROPERTY(TextureCubePtr, texture);

    /** @return             Texture that this skybox uses. */
    TextureCube *texture() const { return m_texture; }

    void setTexture(TextureCube *texture);
protected:
    ~Skybox() {}

    virtual void createSceneEntities(SceneEntityList &entities) override;
private:
    TextureCubePtr m_texture;       /**< Skybox texture. */
    MaterialPtr m_material;         /**< Skybox material. */

    friend class SkyboxSceneEntity;
};

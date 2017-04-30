/*
 * Copyright (C) 2016-2017 Alex Smith
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
 * @brief               World explorer debug window.
 */

#pragma once

#include "engine/entity.h"
#include "engine/debug_window.h"

#include <map>

class MeshRenderer;

/** World explorer debug overlay window. */
class WorldExplorerWindow : public DebugWindow {
public:
    WorldExplorerWindow();
    void render() override;
private:
    using ClassList = std::list<const MetaClass *>;

    void displayOptions();
    void displayEntityTree();

    void displayEnumPropertyEditor(Object *object, const MetaProperty &property);
    bool displayAssetEditor(AssetPtr &asset, const MetaClass &metaClass);
    void displayAssetPropertyEditor(Object *object, const MetaProperty &property);
    void displayObjectPropertyEditor(Object *object, const MetaProperty &property);
    void displayPropertyEditors(Object *object, const MetaClass *metaClass);
    bool displayObjectEditor(Object *object, bool canDestroy);
    void displayEntityEditor();

    void displayMeshRendererEditor(MeshRenderer *renderer);

    const ClassList &getDerivedClasses(const MetaClass &metaClass);

    // FIXME: This is an ideal use for a weak pointer.
    EntityPtr m_currentEntity;          /**< Currently selected entity. */
    Entity *m_entityToOpen;             /**< Entity to force to be open. */

    /** List of child objects to display editors for after the current. */
    std::list<ObjectPtr<Object>> m_childObjects;

    /** Map of known derived classes of a given class. */
    std::map<const MetaClass *, ClassList   > m_derivedClasses;
};

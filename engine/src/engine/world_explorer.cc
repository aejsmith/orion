/*
 * Copyright (C) 2016 Alex Smith
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

#include "engine/component.h"
#include "engine/engine.h"
#include "engine/entity.h"
#include "engine/world_explorer.h"
#include "engine/world.h"

/** Initialise the world explorer. */
WorldExplorerWindow::WorldExplorerWindow() :
    DebugWindow("World Explorer")
{}

/** Render the world explorer. */
void WorldExplorerWindow::render() {
    // FIXME: Ideally we need a weak pointer here. This could hold on to an
    // Entity long after it is freed if the window is not opened.
    if (m_currentEntity && m_currentEntity->refcount() == 1)
        m_currentEntity = nullptr;

    ImGui::SetNextWindowSize(ImVec2(500, 600), ImGuiSetCond_Once);
    ImGui::SetNextWindowPosCenter(ImGuiSetCond_Once);

    if (!begin() || !g_engine->world()) {
        ImGui::End();
        return;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));

    displayEntityTree();
    ImGui::Separator();
    ImGui::Spacing();
    displayEntityEditor();

    ImGui::PopStyleVar();
    ImGui::End();
}

/** Display the entity tree. */
void WorldExplorerWindow::displayEntityTree() {
    ImGui::BeginChild(
        "entityTree",
        ImVec2(0, ImGui::GetContentRegionAvail().y * 0.33f),
        false);

    World *world = g_engine->world();
    Entity *nextEntity = nullptr;

    std::function<void (Entity *)> addEntity =
        [&] (Entity *entity) {
            ImGuiTreeNodeFlags nodeFlags =
                ImGuiTreeNodeFlags_OpenOnArrow |
                ImGuiTreeNodeFlags_OpenOnDoubleClick;
            if (entity == m_currentEntity)
                nodeFlags |= ImGuiTreeNodeFlags_Selected;
            if (entity->children().empty())
                nodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

            bool nodeOpen = ImGui::TreeNodeEx(entity, nodeFlags, "%s", entity->name().c_str());
            if (ImGui::IsItemClicked())
                nextEntity = entity;
            if (nodeOpen) {
                for (Entity *child : entity->children())
                    addEntity(child);

                if (!entity->children().empty())
                    ImGui::TreePop();
            }
        };
    for (Entity *child : world->root()->children())
        addEntity(child);

    /* Change outside loop to avoid visual inconsistency if selection changes. */
    if (nextEntity)
        m_currentEntity = nextEntity;

    ImGui::EndChild();
}

/** Helper for property editor functions. */
template <typename T, typename Func>
void editProperty(Object *object, const MetaProperty &property, Func display) {
    T value;
    object->getProperty<T>(property.name(), value);
    if (display(&value))
        object->setProperty<T>(property.name(), value);
}

/** Display editors for a specific class' properties. */
static void displayPropertyEditors(Object *object, const MetaClass *metaClass) {
    /* Display base class properties first. */
    if (metaClass->parent())
        displayPropertyEditors(object, metaClass->parent());

    for (const MetaProperty &property : metaClass->properties()) {
        ImGui::PushID(&property);

        ImGui::Text(property.name());
        ImGui::NextColumn();

        ImGui::PushItemWidth(-1);

        if (&property.type() == &MetaType::lookup<bool>()) {
            editProperty<bool>(
                object, property,
                [&] (bool *value) { return ImGui::Checkbox("", value); });
        } else if (&property.type() == &MetaType::lookup<glm::vec3>()) {
            editProperty<glm::vec3>(
                object, property,
                [&] (glm::vec3 *value) { return ImGui::InputFloat3("", &value->x); });
        } else if (&property.type() == &MetaType::lookup<glm::quat>()) {
            editProperty<glm::quat>(
                object, property,
                [&] (glm::quat *value) {
                    glm::vec3 eulerAngles = glm::eulerAngles(*value);
                    eulerAngles = glm::vec3(
                        glm::degrees(eulerAngles.x),
                        glm::degrees(eulerAngles.y),
                        glm::degrees(eulerAngles.z));
                    if (ImGui::InputFloat3("", &eulerAngles.x)) {
                        eulerAngles = glm::vec3(
                            glm::radians(eulerAngles.x),
                            glm::radians(eulerAngles.y),
                            glm::radians(eulerAngles.z));
                        *value = glm::quat(eulerAngles);
                        return true;
                    } else {
                        return false;
                    }
                });
        }

        ImGui::PopItemWidth();

        ImGui::NextColumn();

        ImGui::PopID();
    }
}

/** Display an editor for an object's properties. */
static void displayObjectEditor(Object *object) {
    if (!ImGui::CollapsingHeader(object->metaClass().name(), ImGuiTreeNodeFlags_DefaultOpen))
        return;

    ImGui::PushID(object);
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnOffset(1, ImGui::GetWindowContentRegionWidth() * 0.3f);

    displayPropertyEditors(object, &object->metaClass());

    ImGui::Columns(1);
    ImGui::PopID();
}

/** Display the entity editor for the current entity. */
void WorldExplorerWindow::displayEntityEditor() {
    if (!m_currentEntity)
        return;

    ImGui::BeginChild("entityEditor", ImVec2(0, 0), false);

    /* Editor for entity properties. */
    displayObjectEditor(m_currentEntity);

    /* Editor for each component's properties. */
    for (Component *component : m_currentEntity->components())
        displayObjectEditor(component);

    ImGui::EndChild();
}

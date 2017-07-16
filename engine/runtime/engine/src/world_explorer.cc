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

#include "core/filesystem.h"

#include "engine/asset_manager.h"
#include "engine/component.h"
#include "engine/engine.h"
#include "engine/entity.h"
#include "engine/json_serialiser.h"
#include "engine/world_explorer.h"
#include "engine/world.h"

#include "graphics/mesh_renderer.h"

/** Initialise the world explorer. */
WorldExplorerWindow::WorldExplorerWindow() :
    DebugWindow   ( "World Explorer"),
    m_entityToOpen (nullptr)
{}

/** Render the world explorer. */
void WorldExplorerWindow::render() {
    // FIXME: Ideally we need a weak pointer here. This could hold on to an
    // Entity long after it is freed if the window is not opened.
    if (!m_currentEntity || m_currentEntity->refcount() == 1)
        m_currentEntity = g_engine->world()->root();

    ImGuiIO &io = ImGui::GetIO();
    ImGui::SetNextWindowSize(ImVec2(450, io.DisplaySize.y - 60), ImGuiSetCond_Once);
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 450 - 20, 40), ImGuiSetCond_Once);

    if (!begin() || !g_engine->world()) {
        ImGui::End();
        return;
    }

    displayOptions();
    ImGui::Separator();
    ImGui::Spacing();
    displayEntityTree();
    ImGui::Separator();
    ImGui::Spacing();
    displayEntityEditor();

    ImGui::End();
}

/** Display the option buttons. */
void WorldExplorerWindow::displayOptions() {
    if (ImGui::Button("New Entity")) {
        /* Want to open the tree node that we're creating under. ImGUI only
         * offers an API to open the next tree node specified, so save this to
         * later and then we will open it. */
        m_entityToOpen = m_currentEntity;
        m_currentEntity = m_currentEntity->createChild("entity");
    }

    ImGui::SameLine();

    if (ImGui::Button("New Component"))
        ImGui::OpenPopup("newComponent");
    if (ImGui::BeginPopup("newComponent")) {
        static ImGuiTextFilter filter;
        ImGui::PushItemWidth(-1);
        filter.Draw("");
        ImGui::PopItemWidth();

        ImGui::BeginChild("newComponentList", ImVec2(250, 250), false);

        const ClassList &componentClasses = getDerivedClasses(Component::staticMetaClass);

        for (const MetaClass *metaClass : componentClasses) {
            if (filter.PassFilter(metaClass->name())) {
                if (ImGui::MenuItem(metaClass->name())) {
                    ImGui::CloseCurrentPopup();
                    m_currentEntity->createComponent(*metaClass);
                }
            }
        }

        ImGui::EndChild();
        ImGui::EndPopup();
    }

    ImGui::SameLine();

    static std::string path(128, 0);
    bool hadError = false;

    if (ImGui::Button("Save"))
        ImGui::OpenPopup("Save World");
    if (ImGui::BeginPopupModal("Save World", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Please enter a file name:");

        ImGui::PushItemWidth(-1);
        ImGui::InputText("", &path[0], 128);
        ImGui::PopItemWidth();

        ImGui::Spacing();

        if (ImGui::Button("OK", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();

            JSONSerialiser serialiser;
            std::vector<uint8_t> data = serialiser.serialise(g_engine->world());
            std::unique_ptr<File> file(
                Filesystem::openFile(path,
                                     File::kWrite | File::kCreate | File::kTruncate));
            if (file) {
                file->write(&data[0], data.size());
            } else {
                hadError = true;
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel", ImVec2(120, 0)))
            ImGui::CloseCurrentPopup();

        ImGui::EndPopup();
    }

    if (hadError)
        ImGui::OpenPopup("Save Error");
    if (ImGui::BeginPopupModal("Save Error", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Unable to create '%s'", path.c_str());
        ImGui::Spacing();
        if (ImGui::Button("OK", ImVec2(120, 0)))
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
}

/** Display the entity tree. */
void WorldExplorerWindow::displayEntityTree() {
    ImGui::BeginChild(
        "entityTree",
        ImVec2(0, ImGui::GetContentRegionAvail().y * 0.3f),
        false);

    World *world = g_engine->world();
    Entity *nextEntity = nullptr;

    std::function<void (Entity *)> addEntity =
        [&] (Entity *entity) {
            ImGuiTreeNodeFlags nodeFlags =
                ImGuiTreeNodeFlags_OpenOnArrow |
                ImGuiTreeNodeFlags_OpenOnDoubleClick;
            if (entity == world->root())
                nodeFlags |= ImGuiTreeNodeFlags_DefaultOpen;
            if (entity == m_currentEntity)
                nodeFlags |= ImGuiTreeNodeFlags_Selected;
            if (entity->children().empty())
                nodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

            if (entity == m_entityToOpen) {
                ImGui::SetNextTreeNodeOpen(true);
                m_entityToOpen = nullptr;
            }

            bool nodeOpen = ImGui::TreeNodeEx(entity, nodeFlags, "%s", entity->name.c_str());
            if (ImGui::IsItemClicked())
                nextEntity = entity;
            if (nodeOpen) {
                for (Entity *child : entity->children())
                    addEntity(child);

                if (!entity->children().empty())
                    ImGui::TreePop();
            }
        };
    addEntity(world->root());

    /* Change outside loop to avoid visual inconsistency if selection changes. */
    if (nextEntity)
        m_currentEntity = nextEntity;

    ImGui::EndChild();
}

/** Helper for property editor functions. */
template <typename T, typename Func>
void displayPropertyEditor(Object *object, const MetaProperty &property, Func display) {
    if (&property.type() != &MetaType::lookup<T>())
        return;

    ImGui::PushID(&property);

    ImGui::Text(property.name());
    ImGui::NextColumn();
    ImGui::PushItemWidth(-1);

    T value;
    object->getProperty<T>(property.name(), value);
    if (display(&value))
        object->setProperty<T>(property.name(), value);

    ImGui::PopItemWidth();
    ImGui::NextColumn();
    ImGui::PopID();
}

/** Helper to get an enum value name. */
static bool enumItemGetter(void *data, int index, const char **outText) {
    auto constants = *reinterpret_cast<const MetaType::EnumConstantArray *>(data);
    if (outText)
        *outText = constants[index].first;
    return true;
}

/** Edit enum properties. */
void WorldExplorerWindow::displayEnumPropertyEditor(Object *object, const MetaProperty &property) {
    if (!property.type().isEnum())
        return;

    ImGui::PushID(&property);

    ImGui::Text(property.name());
    ImGui::NextColumn();
    ImGui::PushItemWidth(-1);

    const MetaType::EnumConstantArray &constants = property.type().enumConstants();

    /* Get current value and match it against a constant. FIXME: int is not
     * always right, might be a different size... */
    int value;
    object->getProperty(property.name(), property.type(), &value);
    int index;
    for (index = 0; static_cast<size_t>(index) < constants.size(); index++) {
        if (value == constants[index].second)
            break;
    }

    if (ImGui::Combo("", &index, enumItemGetter,
                     const_cast<MetaType::EnumConstantArray *>(&constants),
                     constants.size()))
    {
        value = constants[index].second;
        object->setProperty(property.name(), property.type(), &value);
    }

    ImGui::PushItemWidth(-1);
    ImGui::NextColumn();
    ImGui::PopID();
}

/** Display an asset editor.
 * @param asset         Asset pointer to modify.
 * @param metaClass     Expected class of the asset.
 * @return              Whether the asset was changed. */
bool WorldExplorerWindow::displayAssetEditor(AssetPtr &asset, const MetaClass &metaClass) {
    bool ret = false;

    /* Edit the asset path. Only update when enter is pressed. It's OK that we
     * allocate this string each time we're called, ImGui buffers internally
     * while a textbox is being edited. */
    static std::string errorPath;
    static std::string errorIncorrectType;
    std::string path = (asset) ? asset->path() : "";
    path.resize(128);
    if (ImGui::InputText(metaClass.name(), &path[0], 128, ImGuiInputTextFlags_EnterReturnsTrue)) {
        path.resize(std::strlen(&path[0]));

        /* Try to load the new asset. */
        asset = g_assetManager->load(path);
        if (!asset) {
            errorPath = path;
            errorIncorrectType.clear();
            ImGui::OpenPopup("Invalid Asset");
        } else if (!metaClass.isBaseOf(asset->metaClass())) {
            errorPath = path;
            errorIncorrectType = asset->metaClass().name();
            ImGui::OpenPopup("Invalid Asset");
        } else {
            ret = true;
        }
    }

    if (ImGui::BeginPopupModal("Invalid Asset", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        if (errorIncorrectType.empty()) {
            ImGui::Text("Asset '%s' could not be found", errorPath.c_str());
        } else {
            ImGui::Text("Asset '%s' is incorrect type '%s'", errorPath.c_str(), errorIncorrectType.c_str());
        }

        ImGui::Spacing();
        if (ImGui::Button("OK", ImVec2(120,0)))
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    return ret;
}

/** Edit properties which reference an asset. */
void WorldExplorerWindow::displayAssetPropertyEditor(Object *object, const MetaProperty &property) {
    if (!property.type().isPointer())
        return;
    if (!property.type().pointeeType().isObject())
        return;

    const MetaClass &pointeeClass = static_cast<const MetaClass &>(property.type().pointeeType());

    if (!Asset::staticMetaClass.isBaseOf(pointeeClass))
        return;

    ImGui::PushID(&property);

    ImGui::Text(property.name());
    ImGui::NextColumn();

    AssetPtr asset;
    object->getProperty(property.name(), property.type(), &asset);
    if (displayAssetEditor(asset, pointeeClass))
        object->setProperty(property.name(), property.type(), &asset);

    ImGui::NextColumn();
    ImGui::PopID();
}

/** Edit properties which reference a non-asset object. */
void WorldExplorerWindow::displayObjectPropertyEditor(Object *object, const MetaProperty &property) {
    if (!property.type().isPointer())
        return;
    if (!property.type().pointeeType().isObject())
        return;

    const MetaClass &pointeeClass = static_cast<const MetaClass &>(property.type().pointeeType());

    if (Asset::staticMetaClass.isBaseOf(pointeeClass) ||
        Entity::staticMetaClass.isBaseOf(pointeeClass) ||
        Component::staticMetaClass.isBaseOf(pointeeClass))
    {
        return;
    }

    ImGui::PushID(&property);

    ImGui::Text(property.name());
    ImGui::NextColumn();

    ObjectPtr<Object> target;
    object->getProperty(property.name(), property.type(), &target);

    ImGui::BeginGroup();

    ImGui::AlignFirstTextHeightToWidgets();
    ImGui::Text(target->metaClass().name());

    ImGui::SameLine();

    if (ImGui::Button("New"))
        ImGui::OpenPopup("new");
    if (ImGui::BeginPopup("new")) {
        static ImGuiTextFilter filter;
        ImGui::PushItemWidth(-1);
        filter.Draw("");
        ImGui::PopItemWidth();

        ImGui::BeginChild("newObjectList", ImVec2(250, 250), false);

        const ClassList &derivedClasses = getDerivedClasses(pointeeClass);

        for (const MetaClass *metaClass : derivedClasses) {
            if (filter.PassFilter(metaClass->name())) {
                if (ImGui::MenuItem(metaClass->name())) {
                    ImGui::CloseCurrentPopup();
                    target = metaClass->construct();
                    object->setProperty(property.name(), property.type(), &target);
                }
            }
        }

        ImGui::EndChild();
        ImGui::EndPopup();
    }

    ImGui::EndGroup();

    ImGui::NextColumn();
    ImGui::PopID();

    /* Want to add an editor for this object. */
    m_childObjects.emplace_back(std::move(target));
}

/** Display editors for a specific class' properties. */
void WorldExplorerWindow::displayPropertyEditors(Object *object, const MetaClass *metaClass) {
    /* Display base class properties first. */
    if (metaClass->parent())
        displayPropertyEditors(object, metaClass->parent());

    for (const MetaProperty &property : metaClass->properties()) {
        /* These all do nothing if the type does not match. */

        ImGui::AlignFirstTextHeightToWidgets();

        displayPropertyEditor<bool>(
            object, property,
            [&] (bool *value) {
                return ImGui::Checkbox("", value);
            });

        #define DISPLAY_INT_EDITOR(type) \
            displayPropertyEditor<type>( \
                object, property, \
                [&] (type *value) { \
                    int tmp = *value; \
                    bool ret = ImGui::InputInt("", &tmp, 0, 0, ImGuiInputTextFlags_EnterReturnsTrue); \
                    if (ret) \
                        *value = static_cast<type>(tmp); \
                    return ret; \
                });

        DISPLAY_INT_EDITOR(int8_t);
        DISPLAY_INT_EDITOR(uint8_t);
        DISPLAY_INT_EDITOR(int16_t);
        DISPLAY_INT_EDITOR(uint16_t);
        DISPLAY_INT_EDITOR(int32_t);
        DISPLAY_INT_EDITOR(uint32_t);

        #undef DISPLAY_INT_EDITOR

        displayPropertyEditor<float>(
            object, property,
            [&] (float *value) {
                return ImGui::InputFloat("", value, 0.0f, 0.0f, -1, ImGuiInputTextFlags_EnterReturnsTrue);
            });

        displayPropertyEditor<std::string>(
            object, property,
            [&] (std::string *value) {
                std::string &str = *value;
                str.resize(128);
                if (ImGui::InputText("", &str[0], 128, ImGuiInputTextFlags_EnterReturnsTrue)) {
                    str.resize(std::strlen(&str[0]));
                    return true;
                } else {
                    return false;
                }
            });

        displayPropertyEditor<glm::vec3>(
            object, property,
            [&] (glm::vec3 *value) {
                return ImGui::InputFloat3("", &value->x, -1, ImGuiInputTextFlags_EnterReturnsTrue);
            });

        displayPropertyEditor<glm::quat>(
            object, property,
            [&] (glm::quat *value) {
                glm::vec3 eulerAngles = glm::eulerAngles(*value);
                eulerAngles = glm::vec3(glm::degrees(eulerAngles.x),
                                        glm::degrees(eulerAngles.y),
                                        glm::degrees(eulerAngles.z));
                if (ImGui::SliderFloat3("", &eulerAngles.x, -180.0f, 180.0f)) {
                    eulerAngles = glm::vec3(glm::radians(eulerAngles.x),
                                            glm::radians(eulerAngles.y),
                                            glm::radians(eulerAngles.z));
                    *value = glm::quat(eulerAngles);
                    return true;
                } else {
                    return false;
                }
            });

        displayEnumPropertyEditor(object, property);
        displayAssetPropertyEditor(object, property);
        displayObjectPropertyEditor(object, property);
    }
}

/** Custom editor for a MeshRenderer.
 * @param renderer      Renderer to edit. */
void WorldExplorerWindow::displayMeshRendererEditor(MeshRenderer *renderer) {
    const Mesh *mesh = renderer->mesh();
    if (!mesh)
        return;

    ImGui::Text("materials"); ImGui::Spacing();
    ImGui::NextColumn(); ImGui::NextColumn();

    for (const std::pair<std::string, size_t> &materialPair : mesh->materials()) {
        ImGui::Indent();
        ImGui::AlignFirstTextHeightToWidgets();
        ImGui::Text(materialPair.first.c_str());
        ImGui::Unindent();
        ImGui::NextColumn();

        AssetPtr material = renderer->material(materialPair.second);
        if (displayAssetEditor(material, Material::staticMetaClass))
            renderer->setMaterial(materialPair.second, static_cast<Material *>(material.get()));

        ImGui::NextColumn();
    }
}

/** Display an editor for an object's properties.
 * @param object        Object to display for.
 * @param canDestroy    Whether the object can be destroyed.
 * @return              Whether to destroy the object. */
bool WorldExplorerWindow::displayObjectEditor(Object *object, bool canDestroy) {
    bool open = true;

    if (!ImGui::CollapsingHeader(object->metaClass().name(), (canDestroy) ? &open : nullptr, ImGuiTreeNodeFlags_DefaultOpen))
        return false;

    ImGui::PushID(object);
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnOffset(1, ImGui::GetWindowContentRegionWidth() * 0.3f);

    /* Generic editors based on class properties. */
    displayPropertyEditors(object, &object->metaClass());

    /* Custom editors beyond what can be done with the property system. */
    if (&object->metaClass() == &MeshRenderer::staticMetaClass)
        displayMeshRendererEditor(static_cast<MeshRenderer *>(object));

    ImGui::Columns(1);

    /* Display child object editors. */
    if (open && !m_childObjects.empty()) {
        auto children = std::move(m_childObjects);

        for (const ObjectPtr<Object> &child : children) {
            ImGui::Indent();
            ImGui::BeginChild(child->metaClass().name());
            displayObjectEditor(child, false);
            ImGui::EndChild();
        }
    }

    ImGui::PopID();

    return !open;
}

/** Display the entity editor for the current entity. */
void WorldExplorerWindow::displayEntityEditor() {
    if (!m_currentEntity)
        return;

    ImGui::BeginChild("entityEditor", ImVec2(0, 0), false);

    /* Editor for entity properties. */
    if (m_currentEntity->parent()) {
        if (displayObjectEditor(m_currentEntity, true)) {
            EntityPtr next = m_currentEntity->parent();
            m_currentEntity->destroy();
            m_currentEntity = std::move(next);

            ImGui::EndChild();
            return;
        }
    }

    Component *toDestroy = nullptr;

    /* Editor for each component's properties. */
    for (Component *component : m_currentEntity->components()) {
        if (displayObjectEditor(component, true))
            toDestroy = component;
    }

    /* Must destroy after the loop, otherwise we modify the list while it is in
     * use and break things. */
    if (toDestroy)
        toDestroy->destroy();

    ImGui::EndChild();
}

/** Get a list of constructable classes derived from the given class.
 * @param metaClass     Class to get derived classes of.
 * @return              Sorted list of derived classes. */
const WorldExplorerWindow::ClassList &WorldExplorerWindow::getDerivedClasses(const MetaClass &metaClass) {
    auto it = m_derivedClasses.find(&metaClass);
    if (it != m_derivedClasses.end())
        return it->second;

    ClassList classList;

    MetaClass::visit(
        [&] (const MetaClass &otherClass) {
            if (metaClass.isBaseOf(otherClass) && otherClass.isConstructable())
                classList.emplace_back(&otherClass);
        });

    classList.sort(
        [] (const MetaClass *a, const MetaClass *b) {
            return strcmp(a->name(), b->name()) < 0;
        });

    auto ret = m_derivedClasses.emplace(&metaClass, std::move(classList));
    return ret.first->second;
}

#include "ProjectBrowser.hpp"
#include <filesystem>
#include <cstring>
#include <imgui.h>
#include "ProjectManager.hpp"
#include "ThumbnailCache.hpp"
#ifdef _WIN32
#include "platform/win/FileDialog.hpp"
#endif

void ProjectBrowser::Init()
{
    auto cur = std::filesystem::current_path().string();
    strncpy(m_location, cur.c_str(), sizeof(m_location)-1);
}

void ProjectBrowser::Show(bool* pOpen)
{
    if (!ImGui::Begin("Home", pOpen)) { ImGui::End(); return; }

    ImGui::Columns(3, nullptr, false);
    // Left categories
    ImGui::BeginChild("categories", ImVec2(220, 0), true);
    if (ImGui::Selectable("Recent", m_tab==0)) m_tab=0;
    if (ImGui::Selectable("Templates", m_tab==1)) m_tab=1;
    if (ImGui::Selectable("Samples", m_tab==2)) m_tab=2;
    ImGui::EndChild();
    ImGui::NextColumn();

    // Middle content
    ImGui::BeginChild("content", ImVec2(0, 0), true);
    if (m_tab==0) DrawRecent();
    else if (m_tab==1) DrawTemplates();
    else DrawSamples();
    ImGui::EndChild();
    ImGui::NextColumn();

    // Right details
    ImGui::BeginChild("details", ImVec2(0, 0), true);
    ImGui::Text("Project Defaults");
    const char* langs[] = {"Blueprint", "C++"};
    ImGui::Combo("Type", &m_language, langs, IM_ARRAYSIZE(langs));
    ImGui::InputText("Project Name", m_projName, sizeof(m_projName));
    ImGui::InputText("Location", m_location, sizeof(m_location));
    ImGui::SameLine(); if (ImGui::Button("...")) {
#ifdef _WIN32
        extern std::string WinPickFolder();
#endif
#ifdef _WIN32
        // Use native folder picker (non-modal ImGui)
        if (auto dir = WinFileDialog::PickFolder(); !dir.empty())
            strncpy(m_location, dir.c_str(), sizeof(m_location)-1);
#endif
    }
    if (ImGui::Button("Create"))
    {
        gProjectManager.CreateProject(m_projName, m_location, m_selectedTemplate, m_language);
    }
    ImGui::EndChild();
    ImGui::Columns(1);

    // No modal pickers; all selections happen via native dialog above.

    ImGui::End();
}

void ProjectBrowser::DrawRecent()
{
    ImGui::TextDisabled("Recent Projects");
    ImGui::Separator();
    auto recents = gProjectManager.Recents();
    if (recents.empty()) { ImGui::Text("No recent projects"); return; }
    float avail = ImGui::GetContentRegionAvail().x;
    int cols = (int)(avail / 260.0f); if (cols < 1) cols = 1; ImGui::Columns(cols, nullptr, false);
    for (auto& r : recents)
    {
        ImGui::PushID(r.path.c_str());
        ImGui::BeginChild("card", ImVec2(250, 180), true);
        std::string thumb = (std::filesystem::path(r.path) / "thumbnail.png").string();
        GLuint tex = ThumbnailCache::Get(thumb);
        ImGui::Image((ImTextureID)(intptr_t)tex, ImVec2(240, 120));
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::Text("%s", r.name.c_str());
            ImGui::TextDisabled("%s", r.path.c_str());
            ImGui::TextDisabled("Last opened: %s", r.lastOpened.c_str());
            ImGui::EndTooltip();
        }
        if (ImGui::Button(r.name.c_str(), ImVec2(240, 0)))
        {
            gProjectManager.OpenProject(r.path);
        }
        ImGui::EndChild();
        ImGui::PopID();
        ImGui::NextColumn();
    }
    ImGui::Columns(1);
}

void ProjectBrowser::DrawTemplates()
{
    ImGui::TextDisabled("Choose a Template");
    ImGui::Separator();
    const char* names[] = {"Blank", "First Person", "Third Person", "Top Down", "Vehicle", "Virtual Reality"};
    int count = IM_ARRAYSIZE(names);
    float avail = ImGui::GetContentRegionAvail().x;
    int cols = (int)(avail / 140.0f); if (cols < 1) cols = 1; ImGui::Columns(cols, nullptr, false);
    for (int i = 0; i < count; ++i)
    {
        ImGui::PushID(i);
        ImGui::BeginChild("card", ImVec2(130, 140), true);
        // Try to load a template thumbnail if present under assets/editor/templates
        std::string thumb = std::string("assets/editor/templates/") + names[i] + ".png";
        GLuint tex = ThumbnailCache::Get(thumb);
        ImGui::Image((ImTextureID)(intptr_t)tex, ImVec2(120, 80));
        if (ImGui::Selectable(names[i], m_selectedTemplate==i, 0, ImVec2(120, 0)))
            m_selectedTemplate = i;
        ImGui::EndChild();
        ImGui::PopID();
        ImGui::NextColumn();
    }
    ImGui::Columns(1);
}

void ProjectBrowser::DrawSamples()
{
    ImGui::TextDisabled("Sample Scenes");
    ImGui::Separator();
    const char* samples[] = {"Terrain", "City", "Vehicle Test", "Simple Shooter"};
    for (auto* s : samples)
    {
        if (ImGui::Selectable(s))
        {
            // Create project with sample name under Location
            strncpy(m_projName, s, sizeof(m_projName)-1);
            gProjectManager.CreateProject(m_projName, m_location, /*templateId*/0, m_language);
        }
    }
}

// Removed modal picker in favor of native OS dialog

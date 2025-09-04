#include "Toasts.hpp"
#include <imgui.h>
#include <vector>
#include <chrono>

namespace {
std::vector<Toasts::Item> gItems;
auto gLast = std::chrono::high_resolution_clock::now();
}

namespace Toasts
{
void Push(const std::string& text, double seconds)
{
    gItems.push_back({text, seconds});
}

void UpdateAndDraw()
{
    auto now = std::chrono::high_resolution_clock::now();
    double dt = std::chrono::duration<double>(now - gLast).count();
    gLast = now;
    for (auto& it : gItems) it.ttl -= dt;
    gItems.erase(std::remove_if(gItems.begin(), gItems.end(), [](const Item& i){ return i.ttl <= 0.0; }), gItems.end());

    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImVec2 pos(vp->Pos.x + vp->Size.x - 260.0f, vp->Pos.y + 60.0f);
    for (auto& it : gItems)
    {
        ImGui::SetNextWindowPos(pos, ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(0.85f);
        ImGui::Begin("##toast", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);
        ImGui::TextUnformatted(it.text.c_str());
        ImGui::End();
        pos.y += 50.0f;
    }
}
}


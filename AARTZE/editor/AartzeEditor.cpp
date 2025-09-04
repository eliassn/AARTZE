#include "AartzeEditor.h"
#include <cmath>
#include <vector>
#include <cstdio>
#include "imgui_internal.h"
#include "../../thirdparty/imgui/misc/cpp/imgui_stdlib.h"

#ifdef _WIN32
#  include <windows.h>
#endif

using namespace aartze;

static const char* ToStr(Workspace w) {
    switch (w) {
        case Workspace::Layout: return "Layout";
        case Workspace::Modeling: return "Modeling";
        case Workspace::Sculpting: return "Sculpting";
        case Workspace::UVEditing: return "UV Editing";
        case Workspace::TexturePaint: return "Texture Paint";
        case Workspace::Shading: return "Shading";
        case Workspace::Animation: return "Animation";
        case Workspace::Rendering: return "Rendering";
        case Workspace::Compositing: return "Compositing";
        case Workspace::GeometryNodes: return "Geometry Nodes";
        case Workspace::Scripting: return "Scripting";
    }
    return "";
}

static const char* ImportLabel(ImportTab t) {
    switch (t) {
        case ImportTab::Blend: return "Blend (.blend)";
        case ImportTab::FBX:   return "FBX (.fbx)";
        case ImportTab::GLTF:  return "glTF (.gltf/.glb)";
        case ImportTab::OBJ:   return "OBJ (.obj)";
        case ImportTab::USD:   return "USD (.usd/.usdz)";
    }
    return "";
}

EditorUI::EditorUI() {
    StartSplash();
#ifdef USE_TEXTEDITOR
    textEd.SetLanguageDefinition(TextEditor::LanguageDefinition::CPlusPlus());
    textEd.SetPalette(TextEditor::GetDarkPalette());
    textEd.SetText(
R"(#include <cstdio>

int main(){
    // Hello from AARTZE C++ mode (ImGui integrated)
    printf("Hello AARTZE\n");
    return 0;
}
)"
);
#else
    code =
R"(#include <cstdio>

int main(){
    // Hello from AARTZE C++ mode (ImGui integrated)
    printf("Hello AARTZE\n");
    return 0;
}
)";
#endif
}

void EditorUI::ApplyTheme(ImGuiStyle* st) {
    ImGuiStyle& style = st ? *st : ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    colors[ImGuiCol_WindowBg]        = ImVec4(0.07f,0.07f,0.08f,1.00f);
    colors[ImGuiCol_ChildBg]         = ImVec4(0.08f,0.08f,0.09f,0.70f);
    colors[ImGuiCol_PopupBg]         = ImVec4(0.10f,0.10f,0.11f,0.98f);
    colors[ImGuiCol_Border]          = ImVec4(0.18f,0.18f,0.20f,1.00f);
    colors[ImGuiCol_FrameBg]         = ImVec4(0.13f,0.13f,0.15f,1.00f);
    colors[ImGuiCol_FrameBgHovered]  = ImVec4(0.18f,0.18f,0.22f,1.00f);
    colors[ImGuiCol_FrameBgActive]   = ImVec4(0.22f,0.20f,0.12f,1.00f); // amber tint
    colors[ImGuiCol_Button]          = ImVec4(0.13f,0.13f,0.15f,1.00f);
    colors[ImGuiCol_ButtonHovered]   = ImVec4(0.20f,0.20f,0.22f,1.00f);
    colors[ImGuiCol_ButtonActive]    = ImVec4(0.35f,0.28f,0.10f,1.00f);
    colors[ImGuiCol_Header]          = ImVec4(0.18f,0.18f,0.22f,1.00f);
    colors[ImGuiCol_HeaderHovered]   = ImVec4(0.24f,0.24f,0.28f,1.00f);
    colors[ImGuiCol_HeaderActive]    = ImVec4(0.30f,0.26f,0.12f,1.00f);
    colors[ImGuiCol_Tab]             = ImVec4(0.14f,0.14f,0.16f,1.00f);
    colors[ImGuiCol_TabHovered]      = ImVec4(0.22f,0.22f,0.24f,1.00f);
    colors[ImGuiCol_TabActive]       = ImVec4(0.26f,0.24f,0.12f,1.00f);
    colors[ImGuiCol_Separator]       = ImVec4(0.18f,0.18f,0.20f,1.00f);

    style.FrameRounding = 6.f;
    style.GrabRounding  = 6.f;
    style.TabRounding   = 6.f;
    style.WindowRounding= 12.f;
    style.ChildRounding = 12.f;
    style.ScrollbarSize = 14.f;
}

void EditorUI::StartSplash() {
    booting = true;
    splashProgress = 0.f;
    splashStage = 0;
    snprintf(splashText, sizeof(splashText), "Initializing engine modules…");
    splashStart = ImGui::GetTime();
}

void EditorUI::Draw() {
    // Always show a small top-left overlay so we can confirm rendering
    ImGui::SetNextWindowPos(ImVec2(12,12), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.35f);
    if (ImGui::Begin("##AARTZE_Overlay", nullptr,
        ImGuiWindowFlags_NoDecoration|ImGuiWindowFlags_NoInputs|
        ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoNav))
    {
        ImGui::TextColored(ImVec4(0.98f,0.69f,0.22f,1.f), "AARTZE Editor ACTIVE");
    }
    ImGui::End();

    if (booting) { DrawSplash(); return; }

    DrawTopBar();

    // Second row with workspace + mode dropdowns lives in the same top bar in web; here keep it compact:
    // (We draw header/tool rows + main layout + timeline)
    DrawHeaderTools();

    if (viewMode == ViewMode::Editor) {
        DrawMain3Col();
        DrawTimeline();
    } else if (viewMode == ViewMode::Auto) {
        DrawAutoMode();
    } else {
        DrawCodeMode();
    }

    DrawStatusBar();
}

/* ================= SPLASH ================= */

void EditorUI::DrawSplash() {
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(0,0));
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
    ImGui::Begin("##Splash", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                 ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus);

    // Center “A” disc + ring
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 c = ImGui::GetWindowPos() + io.DisplaySize * 0.5f;
    float R  = 140.f;
    dl->AddCircle(c, R+18.f, IM_COL32(34,211,238,160), 128, 4.f);
    dl->AddCircle(c, R,      IM_COL32(34,211,238,110), 128, 6.f);
    dl->AddCircleFilled(c, R-40.f, IM_COL32(15,15,17,200), 64);

    // Big A
    ImGui::SetCursorScreenPos(c - ImVec2(24, 42));
    ImGui::PushFont(ImGui::GetFont()); // your font size may vary
    ImGui::TextColored(ImVec4(0.35f,0.9f,1.f,1.f), "A");
    ImGui::PopFont();

    // Bottom bar (progress)
    ImVec2 pos = ImGui::GetWindowPos();
    ImVec2 sz  = ImGui::GetWindowSize();
    ImVec2 barPos = pos + ImVec2(0, sz.y - 96);
    ImRect  barR  = ImRect(barPos, barPos + ImVec2(sz.x, 96));
    dl->AddRectFilled(barR.Min, barR.Max, IM_COL32(0,0,0,180));

    ImGui::SetCursorScreenPos(barR.Min + ImVec2(24, 16));
    ImGui::Text("AARTZE Editor – learning");
    ImGui::SetCursorPosY(ImGui::GetCursorPosY()+2);
    ImGui::TextColored(ImVec4(0.7f,0.7f,0.7f,1.f), "AARTZE Editor 0.1.0");

    float dt = float(ImGui::GetTime() - splashStart);
    // progress ease
    if (splashProgress < 100.f) {
        float step = (splashProgress < 80.f) ? 60.f : (splashProgress < 97.f ? 20.f : 6.f);
        splashProgress = ImMin(100.f, splashProgress + step * ImGui::GetIO().DeltaTime);
        // stage text
        const char* stages[] = {
            "Bootstrapping runtime…",
            "Loading core plugins…",
            "Initializing renderer…",
            "Compiling shaders (cached)…",
            "Mounting content…",
            "Setting up world…",
            "Spawning editor systems…",
            "Preparing viewport…",
            "Loading Startup Map: /Engine/Maps/Templates/OpenWorld…"
        };
        int nstage = int(sizeof(stages)/sizeof(stages[0]));
        int idx = (int)(splashProgress / (100.f / nstage));
        idx = (idx < nstage ? idx : nstage-1);
        if (idx != splashStage) {
            splashStage = idx;
            snprintf(splashText, sizeof(splashText), "%s", stages[idx]);
        }
    } else {
        // done -> small delay to show 100%
        if (dt > 1.2f) booting = false;
    }

    // progress bar
    ImGui::SetCursorScreenPos(barR.Min + ImVec2(24, 52));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.f);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.16f,0.16f,0.17f,1));
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.98f,0.69f,0.22f,1));
    ImGui::ProgressBar(splashProgress/100.f, ImVec2(sz.x-24*2-84, 16));
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar();

    ImGui::SameLine();
    ImGui::Text("%d%%  —  %s", (int)std::floor(splashProgress+0.5f), splashText);

    ImGui::End();
    ImGui::PopStyleVar();
}

/* ============== TOP BAR & HEADER ============== */

void EditorUI::DrawTopBar() {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
    ImGui::Begin("##TopBar", nullptr, ImGuiWindowFlags_NoDecoration|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoSavedSettings);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.10f,0.10f,0.11f,0.95f));
    ImGui::BeginChild("TopMenubar", ImVec2(0, 36), true);

    ImGui::TextColored(ImVec4(0.98f,0.69f,0.22f,1), "AARTZE"); ImGui::SameLine();

    const char* menus[] = {"File","Edit","Window","Tools","Build","Select","Actor","Help"};
    for (auto& m: menus) { ImGui::SameLine(); ImGui::TextUnformatted(m); }

    ImGui::SameLine(ImGui::GetWindowWidth()-120);
    ImGui::TextColored(ImVec4(0.6f,0.8f,0.7f,1.f), "learning  ●");

    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::End();
    ImGui::PopStyleVar();
}

bool EditorUI::TabButton(const char* label, bool active) {
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10,5));
    ImGui::PushStyleColor(ImGuiCol_Button, active ? ImVec4(0.20f,0.18f,0.10f,1) : ImVec4(0.12f,0.12f,0.13f,1));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.18f,0.18f,0.22f,1));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.26f,0.24f,0.12f,1));
    bool clicked = ImGui::Button(label);
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(2);
    return clicked;
}

void EditorUI::DrawHeaderTools() {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
    ImGui::Begin("##HeaderTools", nullptr, ImGuiWindowFlags_NoDecoration|ImGuiWindowFlags_NoSavedSettings);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.10f,0.10f,0.11f,1));
    ImGui::BeginChild("Hdr", ImVec2(0, 40), true);

    // Play controls
    if (ImGui::Button("▶")) {}
    ImGui::SameLine(); if (ImGui::Button("⏸")) {}
    ImGui::SameLine(); if (ImGui::Button("■")) {}
    ImGui::SameLine(); ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);

    // Object Mode dropdown
    ImGui::SameLine();
    if (ImGui::BeginCombo("##objmode", "Object Mode")) {
        const char* items[] = {"Object Mode","Edit Mode","Sculpt Mode","Vertex Paint","Weight Paint","Texture Paint"};
        for (auto& it: items) { if (ImGui::Selectable(it)) {} }
        ImGui::EndCombo();
    }

    ImGui::SameLine(); ImGui::Button("Global");
    ImGui::SameLine(); ImGui::Button("Local");
    ImGui::SameLine(); ImGui::Button("Pivot ▾");
    ImGui::SameLine(); ImGui::Button("🧲"); // snap
    ImGui::SameLine(); ImGui::Button("Increment ▾");
    ImGui::SameLine(); ImGui::Button("Proportional Editing");

    ImGui::SameLine(); ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
    ImGui::SameLine(); ImGui::Button("Perspective ▾");
    ImGui::SameLine(); ImGui::Button("Shading ▾");
    ImGui::SameLine(); ImGui::Button("Overlays");

    // Right: Workspace & Mode dropdowns
    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 340);
    // Workspace
    ImGui::TextUnformatted("Workspace:");
    ImGui::SameLine();
    if (ImGui::BeginCombo("##workspace", ToStr(workspace))) {
        for (int i=0;i<= (int)Workspace::Scripting;i++) {
            bool sel = (workspace==(Workspace)i);
            if (ImGui::Selectable(ToStr((Workspace)i), sel)) workspace = (Workspace)i;
        }
        ImGui::EndCombo();
    }
    ImGui::SameLine();
    // Mode
    const char* modeLabel = (viewMode==ViewMode::Editor?"Editor":(viewMode==ViewMode::Auto?"Auto-mode":"C++-mode"));
    if (ImGui::BeginCombo("##mode", modeLabel)) {
        if (ImGui::Selectable("Editor", viewMode==ViewMode::Editor)) viewMode = ViewMode::Editor;
        if (ImGui::Selectable("Auto-mode", viewMode==ViewMode::Auto)) viewMode = ViewMode::Auto;
        if (ImGui::Selectable("C++-mode", viewMode==ViewMode::Code)) viewMode = ViewMode::Code;
        ImGui::EndCombo();
    }

    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::End();
    ImGui::PopStyleVar();
}

/* ============== MAIN 3-COLUMN LAYOUT (Editor mode) ============== */

void EditorUI::DrawMain3Col() {
    ImGui::Begin("##MainEditor", nullptr,
        ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_NoSavedSettings);
    ImVec2 avail = ImGui::GetContentRegionAvail();

    // 3 columns: Viewport (8/12), Outliner (2/12), Details (2/12)
    float colW = avail.x;
    float vpW  = colW * 8.f/12.f - 6.f;
    float side = colW * 2.f/12.f - 6.f;
    float h    = avail.y - 8.f;

    // Viewport
    ImGui::BeginChild("Viewport", ImVec2(vpW, h), true);
    DrawViewport(ImGui::GetContentRegionAvail());
    ImGui::EndChild();

    ImGui::SameLine();

    // Outliner
    ImGui::BeginChild("Outliner", ImVec2(side, h), true);
    DrawOutliner(ImGui::GetContentRegionAvail());
    ImGui::EndChild();

    ImGui::SameLine();

    // Details
    ImGui::BeginChild("Details", ImVec2(side, h), true);
    DrawDetails(ImGui::GetContentRegionAvail());
    ImGui::EndChild();

    ImGui::End();
}

void EditorUI::DrawViewport(ImVec2 size) {
    ImGui::BeginChild("##vpbg", size, true);
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImRect r(ImGui::GetCursorScreenPos(), ImGui::GetCursorScreenPos()+size);
    // Subtle gradient fill
    dl->AddRectFilled(r.Min, r.Max, IM_COL32(25,26,30,255));
    // inner rounded rect
    ImRect inner(r.Min+ImVec2(18,18), r.Max-ImVec2(18,18));
    dl->AddRectFilled(inner.Min, inner.Max, IM_COL32(40,41,48,150), 24.f);

    // Left tool shelf (buttons)
    ImGui::SetCursorScreenPos(inner.Min + ImVec2(6, inner.GetHeight()*0.5f - 140));
    ImGui::BeginChild("##tools", ImVec2(40, 280), true);
    const char* tools[] = {"Sel","Cur","Move","Rot","Scale","Paint","Measure","Add","Cam","Light","Coll"};
    for (auto& t: tools) { if (ImGui::Button(t, ImVec2(28,24))) {} }
    ImGui::EndChild();

    // Axis gizmo + spinner (top-right)
    ImGui::SetCursorScreenPos(inner.Max - ImVec2(120, 64));
    ImGui::BeginChild("##gizmo", ImVec2(110, 52), true);
    ImGui::TextColored(ImVec4(1,0.3f,0.3f,1), "X");
    ImGui::TextColored(ImVec4(0.3f,1,0.5f,1), "Y");
    ImGui::TextColored(ImVec4(0.4f,0.6f,1,1), "Z");
    ImGui::SameLine();
    ImGui::Dummy(ImVec2(8,1)); ImGui::SameLine();
    ImGui::BeginGroup();
    Spinner(8.f, 2.f, IM_COL32(250,200,100,255));
    ImGui::TextDisabled("Loading");
    ImGui::EndGroup();
    ImGui::EndChild();

    ImGui::EndChild();
}

void EditorUI::Spinner(float radius, float thickness, ImU32 col, float speed) {
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    float t = float(ImGui::GetTime()*speed);
    float a0 = t;
    float a1 = t + IM_PI*1.2f;
    dl->PathClear();
    int seg = 64;
    for (int i=0;i<=seg;i++){
        float a = a0 + (a1-a0)*(i/(float)seg);
        dl->PathLineTo(pos + ImVec2(radius + std::cos(a)*radius, radius + std::sin(a)*radius));
    }
    dl->PathStroke(col, 0, thickness);
}

void EditorUI::DrawOutliner(ImVec2) {
    ImGui::TextDisabled("Outliner");
    ImGui::Separator();
    if (ImGui::TreeNodeEx("World", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Selectable("Camera"); ImGui::SameLine(); ImGui::TextDisabled("👁 🔒");
        ImGui::Selectable("Cube");   ImGui::SameLine(); ImGui::TextDisabled("👁 🔒");
        ImGui::Selectable("Light");  ImGui::SameLine(); ImGui::TextDisabled("👁 🔒");
        ImGui::TreePop();
    }
}

void EditorUI::DrawDetails(ImVec2) {
    ImGui::TextDisabled("Details");
    ImGui::Separator();

    // Import “tabs” (wrapping pills)
    {
        const struct { ImportTab t; const char* label; } tabs[] = {
            {ImportTab::Blend, "Blend (.blend)"},
            {ImportTab::FBX,   "FBX (.fbx)"},
            {ImportTab::GLTF,  "glTF (.gltf/.glb)"},
            {ImportTab::OBJ,   "OBJ (.obj)"},
            {ImportTab::USD,   "USD (.usd/.usdz)"},
        };
        for (auto& it: tabs) {
            bool act = (importTab==it.t);
            if (TabButton(it.label, act)) importTab = it.t;
            ImGui::SameLine();
        }
        ImGui::NewLine();

        ImGui::BeginChild("##importBox", ImVec2(0, 92), true);
        ImGui::Text("Import %s", ImportLabel(importTab));
        ImGui::Spacing();
        if (ImGui::Button("Choose File…")) { /* hook your file dialog */ }
        ImGui::SameLine(); ImGui::TextDisabled("(Options will appear here)");
        ImGui::EndChild();
    }

    ImGui::Spacing();
    ImGui::TextDisabled("Transform");
    ImGui::Separator();
    DrawTransformBlock("Location", tLocation, false);
    DrawTransformBlock("Rotation", tRotation, false);
    DrawTransformBlock("Scale",    tScale,    true);

    // Extra sections
    const char* secs[] = {"Delta Transform","Relations","Collections","Instancing","Motion Paths"};
    for (auto* s: secs) {
        if (ImGui::CollapsingHeader(s)) ImGui::TextDisabled("(placeholder)");
    }
}

void EditorUI::DrawTransformBlock(const char* label, Transform3& t, bool isScale) {
    ImGui::PushID(label);
    ImGui::BeginChild("##block", ImVec2(0, 120), true);
    ImGui::TextDisabled("%s", label);
    if (std::string(label) == "Rotation") {
        ImGui::SameLine(ImGui::GetContentRegionAvail().x - 110);
        if (ImGui::BeginCombo("##rot", "XYZ Euler")) {
            ImGui::Selectable("XYZ Euler", true);
            ImGui::Selectable("ZXY Euler");
            ImGui::Selectable("Quaternion");
            ImGui::EndCombo();
        }
    }
    ImGui::Spacing();

    auto row = [&](const char* axis, float* v){
        ImGui::PushID(axis);
        ImGui::TextDisabled("%s", axis); ImGui::SameLine();
        ImGui::SetNextItemWidth(140);
        ImGui::InputFloat("##v", v, 0,0,"%.3f");
        ImGui::PopID();
    };
    row("X", &t.x);
    row("Y", &t.y);
    row("Z", &t.z);

    ImGui::EndChild();
    ImGui::PopID();
}

/* ============== TIMELINE ============== */

void EditorUI::DrawTimeline() {
    ImGui::Begin("##Timeline", nullptr, ImGuiWindowFlags_NoDecoration|ImGuiWindowFlags_NoSavedSettings);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.10f,0.10f,0.11f,1));
    ImGui::BeginChild("tl", ImVec2(0, 44), true);

    if (ImGui::Button("■")) {}
    ImGui::SameLine(); if (ImGui::Button("▶")) {}
    ImGui::SameLine(); if (ImGui::Button("⏸")) {}
    ImGui::SameLine(); ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);

    static int start=1, end=250, frame=1;
    ImGui::SameLine(); ImGui::TextDisabled("Start");
    ImGui::SameLine(); ImGui::SetNextItemWidth(64); ImGui::InputInt("##s", &start);
    ImGui::SameLine(); ImGui::TextDisabled("End");
    ImGui::SameLine(); ImGui::SetNextItemWidth(64); ImGui::InputInt("##e", &end);
    ImGui::SameLine(); ImGui::Dummy(ImVec2(12,1)); ImGui::SameLine();
    ImGui::ProgressBar(0.3f, ImVec2(ImGui::GetContentRegionAvail().x-160, 6));
    ImGui::SameLine(); ImGui::TextDisabled("Frame");
    ImGui::SameLine(); ImGui::SetNextItemWidth(64); ImGui::InputInt("##f", &frame);

    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::End();
}

/* ============== AUTO MODE ============== */

void EditorUI::DrawNodeGrid(const ImRect& r, float step, ImU32 col) {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    for (float x = r.Min.x; x < r.Max.x; x += step) dl->AddLine(ImVec2(x, r.Min.y), ImVec2(x, r.Max.y), col);
    for (float y = r.Min.y; y < r.Max.y; y += step) dl->AddLine(ImVec2(r.Min.x, y), ImVec2(r.Max.x, y), col);
}

void EditorUI::DrawAutoMode() {
    ImGui::Begin("##AutoMode", nullptr, ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoSavedSettings);
    ImVec2 avail = ImGui::GetContentRegionAvail();
    float left = avail.x * 3.f/12.f - 6.f;
    float center = avail.x * 6.f/12.f - 6.f;
    float right = avail.x * 3.f/12.f - 6.f;
    float h = avail.y - 6.f;

    // Palette (scrollable)
    ImGui::BeginChild("Palette", ImVec2(left, h), true);
    ImGui::TextDisabled("Palette"); ImGui::Separator();
    ImGui::BeginChild("palScroll", ImVec2(0,0), false, ImGuiWindowFlags_AlwaysVerticalScrollbar);
    const char* cats[]={("Events"),("Input"),("Actors"),("Math"),("Utilities"),("Rendering"),("Audio"),("Particles"),("Physics"),("AI"),("Flow"),("Custom"),("Plugins"),("Scripting"),("Animation"),("Cinematics")};
    for (auto* c: cats) {
        if (ImGui::TreeNodeEx(c, ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Button((std::string("Add ") + std::string(c) + " Node").c_str());
            ImGui::Button((std::string("Sample ") + std::string(c) + " Node").c_str());
            ImGui::TreePop();
        }
    }
    ImGui::EndChild();
    ImGui::EndChild();

    ImGui::SameLine();

    // Graph
    ImGui::BeginChild("Graph", ImVec2(center, h), true);
    ImGui::TextDisabled("Auto-mode Graph"); ImGui::Separator();
    ImRect r(ImGui::GetCursorScreenPos(), ImGui::GetCursorScreenPos()+ImGui::GetContentRegionAvail());
    ImGui::InvisibleButton("##graphbg", r.GetSize());
    DrawNodeGrid(r, 24.f, IM_COL32(255,255,255,18));

    // Mock nodes
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 a = r.Min + ImVec2(24,24);
    ImVec2 b = a + ImVec2(140, 36);
    dl->AddRectFilled(a, b, IM_COL32(32,32,36,220), 6.f);
    dl->AddRect(a, b, IM_COL32(250,200,90,180), 6.f);
    dl->AddText(a + ImVec2(8,10), IM_COL32(250,200,90,220), "Event Begin Play");

    ImVec2 a2 = a + ImVec2(200, 0);
    ImVec2 b2 = a2 + ImVec2(120, 36);
    dl->AddRectFilled(a2, b2, IM_COL32(36,36,40,220), 6.f);
    dl->AddRect(a2, b2, IM_COL32(130,130,140,160), 6.f);
    dl->AddText(a2 + ImVec2(8,10), IM_COL32(220,220,230,220), "Print String");

    ImGui::EndChild();

    ImGui::SameLine();

    // Details
    ImGui::BeginChild("NodeDetails", ImVec2(right, h), true);
    ImGui::TextDisabled("Details"); ImGui::Separator();
    ImGui::Text("Node Properties (placeholder)");
    ImGui::Separator();
    ImGui::Text("Variables (placeholder)");
    ImGui::EndChild();

    // Compiler / Output
    ImGui::BeginChild("Compiler", ImVec2(0, 140), true);
    ImGui::TextDisabled("Compiler / Output"); ImGui::Separator();
    ImGui::TextColored(ImVec4(0.7f,0.7f,0.7f,1.f), "(Logs will appear here)");
    ImGui::EndChild();

    ImGui::End();
}

/* ============== CODE MODE ============== */

void EditorUI::DrawCodeMode() {
    ImGui::Begin("##CodeMode", nullptr, ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoSavedSettings);
    ImVec2 avail = ImGui::GetContentRegionAvail();
    float left = avail.x * 2.f/12.f - 6.f;
    float center = avail.x * 7.f/12.f - 6.f;
    float right = avail.x * 3.f/12.f - 6.f;
    float h = avail.y - 6.f;

    // Explorer
    ImGui::BeginChild("Explorer", ImVec2(left, h), true);
    ImGui::TextDisabled("Explorer"); ImGui::Separator();
    ImGui::Selectable("main.cpp", true);
    ImGui::Selectable("engine.cpp");
    ImGui::Selectable("imgui_layer.cpp");
    ImGui::EndChild();

    ImGui::SameLine();

    // Editor area + toolbar
    ImGui::BeginChild("CodeCenter", ImVec2(center, h), true);
    ImGui::BeginChild("tabs", ImVec2(0, 30), true);
    ImGui::Text("📄 main.cpp    📄 engine.cpp");
    ImGui::SameLine(ImGui::GetContentRegionAvail().x-300);
    if (ImGui::Button("Build & Run")) {}
    ImGui::SameLine(); if (ImGui::Button("Debug")) {}
    ImGui::SameLine(); ImGui::Checkbox("Use Monaco", &monacoToggle); // placeholder to match web
    ImGui::EndChild();

#ifdef USE_TEXTEDITOR
    {
        ImVec2 edSz = ImGui::GetContentRegionAvail();
        textEd.Render("TextEditor", edSz, true);
    }
#else
    {
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.12f,0.12f,0.13f,1));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.f);
        ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput |
                                    ImGuiInputTextFlags_CallbackAlways;
        ImGui::InputTextMultiline("##code", &code, ImGui::GetContentRegionAvail(), flags);
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
    }
#endif
    ImGui::EndChild();

    ImGui::SameLine();

    // Right panel
    ImGui::BeginChild("Problems", ImVec2(right, h), true);
    ImGui::TextDisabled("Problems / Outline"); ImGui::Separator();
    ImGui::Text("No problems detected.");
    ImGui::Separator();
    ImGui::TextDisabled("Terminal"); ImGui::Separator();
    ImGui::Text("$ _");
    ImGui::EndChild();

    ImGui::End();
}

/* ============== STATUS BAR ============== */

void EditorUI::DrawStatusBar() {
    ImGui::Begin("##Status", nullptr, ImGuiWindowFlags_NoDecoration|ImGuiWindowFlags_NoSavedSettings);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.10f,0.10f,0.11f,1));
    ImGui::BeginChild("status", ImVec2(0, 36), true);

    ImGui::Text("Content Drawer"); ImGui::SameLine(); ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
    ImGui::SameLine(); ImGui::Text("Output Log"); ImGui::SameLine(); ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
    ImGui::SameLine(); ImGui::Text("Cmd");

    ImGui::SameLine(ImGui::GetWindowWidth()-240);
    ImGui::Text("All Saved   |   Revision Control");

    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::End();
}

#pragma once
#include "imgui.h"
#include <string>
#include <array>
#include "imgui_internal.h" // for ImRect and internal helpers

#ifdef USE_TEXTEDITOR
#  include "TextEditor.h" // https://github.com/BalazsJako/ImGuiColorTextEdit (or similar)
#endif

namespace aartze {

enum class ImportTab { Blend, FBX, GLTF, OBJ, USD };
enum class Workspace {
    Layout, Modeling, Sculpting, UVEditing, TexturePaint,
    Shading, Animation, Rendering, Compositing, GeometryNodes, Scripting
};
enum class ViewMode { Editor, Auto, Code };

struct Transform3 {
    float x=0.f, y=0.f, z=0.f;
};

struct EditorStyle {
    ImVec4 accent = ImVec4(0.98f, 0.69f, 0.22f, 1.f); // amber-ish
    ImVec4 panel  = ImVec4(0.09f, 0.09f, 0.10f, 1.f);
    ImVec4 panel2 = ImVec4(0.08f, 0.08f, 0.09f, 0.7f);
    ImVec4 border = ImVec4(0.18f, 0.18f, 0.2f, 1.f);
};

class EditorUI {
public:
    EditorUI();

    // Call once if you want the “AARTZE dark” theme automatically:
    static void ApplyTheme(ImGuiStyle* style = nullptr);

    // Call every frame inside an active ImGui frame.
    void Draw();

    // Accessors if you want to hook buttons later:
    ViewMode GetViewMode() const { return viewMode; }
    Workspace GetWorkspace() const { return workspace; }

    // Splash control
    void StartSplash();          // (re)start splash timer
    bool IsSplashDone() const { return !booting; }

private:
    // Frame parts
    void DrawSplash();
    void DrawTopBar();
    void DrawHeaderTools();
    void DrawMain3Col();
    void DrawOutliner(ImVec2 size);
    void DrawDetails(ImVec2 size);
    void DrawViewport(ImVec2 size);
    void DrawTimeline();
    void DrawAutoMode();
    void DrawCodeMode();
    void DrawStatusBar();

    // Helpers
    void DrawTransformBlock(const char* label, Transform3& t, bool isScale=false);
    void DrawAxisGizmoAndSpinner();
    void DrawLeftToolShelf();
    void DrawNodeGrid(const ImRect& r, float step=24.f, ImU32 color=IM_COL32(255,255,255,25));
    void Spinner(float radius, float thickness, ImU32 col, float speed=2.0f);
    bool TabButton(const char* label, bool active); // pill-like tabs (wrap friendly)

private:
    // State
    bool      booting = true;
    float     splashProgress = 0.f;
    double    splashStart = 0.0;
    int       splashStage = 0;
    char      splashText[160] = "Initializing engine modules…";

    // Menus/state
    Workspace workspace = Workspace::Layout;
    bool      workspaceMenuOpen = false;

    ViewMode  viewMode = ViewMode::Editor;
    bool      modeMenuOpen = false;
    bool      objectModeOpen = false;

    // Details panel
    ImportTab importTab = ImportTab::Blend;

    // Transform
    Transform3 tLocation {0,0,0};
    Transform3 tRotation {0,0,0};
    Transform3 tScale    {1,1,1};

    // Code editor buffer / optional TextEditor
#ifdef USE_TEXTEDITOR
    TextEditor textEd;
#else
    std::string code;
#endif
    bool       monacoToggle = true; // placeholder (kept for parity with web)

    // Style
    EditorStyle style;
};

} // namespace aartze

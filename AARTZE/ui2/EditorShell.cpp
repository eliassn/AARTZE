#include "EditorShell.hpp"
#include "StyleSet.hpp"
#include "Panels.hpp"
#include "Viewport.hpp"
#include <GLFW/glfw3.h>

namespace ui2 {

void EditorShell::Initialize(GLFWwindow* window)
{
    m_window = window;
    SetStyle(GetStyle());
    m_renderer.init();

    m_dock = std::make_shared<DockSpace>();
    // Center uses a real Viewport chrome (3D scene is already drawn by RenderingSystem)
    auto viewport = std::make_shared<ViewportPanel>();
    m_left = std::make_shared<OutlinerPanel>();
    m_right = std::make_shared<DetailsPanel>();
    m_bottom = std::make_shared<ContentPanel>();
    m_dock->setCenter(viewport);
    m_left->setWindow(window); m_dock->setLeft(m_left, 0.23f);
    m_right->setWindow(window); m_dock->setRight(m_right, 0.27f);
    m_bottom->setWindow(window); m_dock->setBottom(m_bottom, 0.26f);
}

void EditorShell::Resize(int w,int h) { m_w=w; m_h=h; }

void EditorShell::Tick(float dt)
{
    (void)dt;
    if(m_left) m_left->tick(); if(m_bottom) m_bottom->tick();
    m_dock->layout({0,0,(float)m_w,(float)m_h});
}

void EditorShell::Render()
{
    m_draw.clear();
    auto s = GetStyle();
    unsigned bg = ((unsigned)(s.WindowBg.a*255)<<24)|((unsigned)(s.WindowBg.b*255)<<16)|((unsigned)(s.WindowBg.g*255)<<8)|((unsigned)(s.WindowBg.r*255));
    m_draw.rectFilled(0,0,(float)m_w,(float)m_h, bg);
    // Menu/Toolbar/Status rows
    unsigned bar = ((unsigned)(s.PanelBg.a*255)<<24)|((unsigned)(s.PanelBg.b*255)<<16)|((unsigned)(s.PanelBg.g*255)<<8)|((unsigned)(s.PanelBg.r*255));
    const float menuH=28.0f, toolH=36.0f, statusH=22.0f;
    m_draw.rectFilled(0,0,(float)m_w,menuH, bar);
    m_draw.rectFilled(0,menuH,(float)m_w,toolH, bar);
    // Toolbar placeholders
    float bx=8.0f, by=menuH+6.0f; float bw=28.0f, bh=24.0f; unsigned accent=((unsigned)(s.Accent.a*255)<<24)|((unsigned)(s.Accent.b*255)<<16)|((unsigned)(s.Accent.g*255)<<8)|((unsigned)(s.Accent.r*255));
    for(int i=0;i<10;++i){ m_draw.rectFilled(bx,by,bw,bh,accent); bx+=bw+6.0f; }
    // Panels in remaining area
    m_dock->layout({0,menuH+toolH,(float)m_w,(float)m_h-(menuH+toolH+statusH)});
    m_dock->paint(m_draw);
    // Status bar
    m_draw.rectFilled(0,(float)m_h-statusH,(float)m_w,statusH, bar);
    m_renderer.render(m_draw, m_w, m_h);
}

void EditorShell::Shutdown()
{
    m_renderer.shutdown();
}

} // namespace ui2



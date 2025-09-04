#include "Panels.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <string>
#include "core/Coordinator.hpp"
#include "components/TransformComponent.hpp"
#include "components/MaterialComponent.hpp"
#include "editor/EditorState.hpp"

namespace ui2 {

static unsigned rgba(const Color& c){ return ((unsigned)(c.a*255)<<24)|((unsigned)(c.b*255)<<16)|((unsigned)(c.g*255)<<8)|((unsigned)(c.r*255)); }
static bool inside(float x,float y,const Rect& r){ return x>=r.x && x<=r.x+r.w && y>=r.y && y<=r.y+r.h; }

void PanelBase::paintHeader(DrawList& dl)
{
    auto s = GetStyle();
    unsigned bar = rgba(s.PanelBg);
    unsigned text = rgba(s.Text);
    dl.rectFilled(m_bounds.x, m_bounds.y, m_bounds.w, headerH(), bar);
    dl.text(m_bounds.x + 8.0f, m_bounds.y + 6.0f, m_title, text);
}

void PanelBase::paint(DrawList& dl)
{
    auto s = GetStyle();
    unsigned wbg = rgba(s.WindowBg);
    paintHeader(dl);
    dl.rectFilled(m_bounds.x, m_bounds.y + headerH(), m_bounds.w, m_bounds.h - headerH(), wbg);
}

// ---------------- Outliner ----------------
void OutlinerPanel::paint(DrawList& dl)
{
    PanelBase::paint(dl);
    auto s = GetStyle(); unsigned text = rgba(s.Text); unsigned item = rgba(s.PanelBg);
    const float bodyY = m_bounds.y + headerH();
    dl.rectFilled(m_bounds.x + 8, bodyY + 8, m_bounds.w - 16, 20, item);
    dl.text(m_bounds.x + 12, bodyY + 12, "Search...", s.Text);

    const float listTop = bodyY + 36; const float listH = m_bounds.h - headerH() - 44; const float rowH = 22.0f;
    int total = 0; for (unsigned e=0;e<MAX_ENTITIES;++e) if (gCoordinator.IsEntityAlive(e)) ++total;
    int first = std::max(0, (int)(m_scroll/rowH)); int visible = (int)(listH/rowH)+2; int last = std::min(total, first+visible);

    int idx=0;
    for (unsigned e=0;e<MAX_ENTITIES;++e)
    {
        if (!gCoordinator.IsEntityAlive(e)) continue;
        if (idx>=first && idx<last)
        {
            float y = listTop + (idx*rowH - m_scroll);
            Rect r{ m_bounds.x + 8, y, m_bounds.w - 16, rowH-4 };
            unsigned col = item; if ((int)e == m_selected) col = rgba(Color{ s.Accent.r, s.Accent.g, s.Accent.b, 0.35f });
            dl.rectFilled(r.x, r.y, r.w, r.h, col);
            dl.text(r.x + 6, r.y + 4, std::string("Entity ")+std::to_string(e), s.Text);
            float box=12, pad=6; Rect vis{ r.x + r.w - pad - 2*box - 4, r.y + 3, box, box };
            Rect lock{ r.x + r.w - pad - box,     r.y + 3, box, box };
            dl.rectFilled(vis.x, vis.y, vis.w, vis.h, EditorState::IsVisible(e)?rgba(s.Accent):item);
            dl.rectFilled(lock.x, lock.y, lock.w, lock.h, EditorState::IsLocked(e)?rgba(s.Accent):item);
        }
        ++idx;
    }
}

void OutlinerPanel::tick()
{
    if (!m_window) return;
    if (glfwGetKey(m_window, GLFW_KEY_PAGE_UP) == GLFW_PRESS)   m_scroll = std::max(0.0f, m_scroll - 10.0f);
    if (glfwGetKey(m_window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS) m_scroll = m_scroll + 10.0f;

    double mx,my; glfwGetCursorPos(m_window,&mx,&my);
    const float bodyY = m_bounds.y + headerH(); const float listTop = bodyY + 36; const float rowH = 22.0f;
    Rect click{ m_bounds.x + 8, listTop, m_bounds.w - 16, m_bounds.h - headerH() - 44 };
    if (inside((float)mx,(float)my, click) && glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT)==GLFW_PRESS)
    {
        int row = (int)((my - listTop + m_scroll)/rowH);
        int cur=0; unsigned hit=(unsigned)-1; for(unsigned e=0;e<MAX_ENTITIES;++e) if (gCoordinator.IsEntityAlive(e)){ if(cur==row){hit=e;break;} ++cur; }
        if (hit!=(unsigned)-1)
        {
            Rect r{ m_bounds.x + 8, (float)(listTop + (row*rowH - m_scroll)), m_bounds.w - 16, rowH-4 };
            float box=12, pad=6; Rect vis{ r.x + r.w - pad - 2*box - 4, r.y + 3, box, box }; Rect lock{ r.x + r.w - pad - box, r.y + 3, box, box };
            if (inside((float)mx,(float)my, vis)) EditorState::SetVisible(hit, !EditorState::IsVisible(hit));
            else if (inside((float)mx,(float)my, lock)) EditorState::SetLocked(hit, !EditorState::IsLocked(hit));
            else { m_selected=(int)hit; EditorState::SetSelected((int)hit); }
        }
    }
}

// -------- Details (editable skeleton) --------
static void drawStepper3(DrawList& dl, const Rect& base, const char* labels[3], float* v, const Color& panel, const Color& text)
{
    unsigned cPanel=rgba(panel), cText=rgba(text);
    float x=base.x, y=base.y, w=base.w, h=base.h; float colW=(w-16)/3.0f;
    for(int i=0;i<3;++i){ Rect r{ x + i*(colW+8), y, colW, h }; dl.rectFilled(r.x,r.y,r.w,r.h,cPanel); dl.text(r.x+6, r.y+4, labels?labels[i]:"", text); dl.text(r.x+34, r.y+4, std::to_string(v[i]).substr(0,5), text); }
}

void DetailsPanel::paint(DrawList& dl)
{
    PanelBase::paint(dl);
    auto s=GetStyle(); int sel=EditorState::GetSelected(); float y=m_bounds.y+headerH()+8; if(sel<0||!gCoordinator.IsEntityAlive((unsigned)sel)){ dl.text(m_bounds.x+12,y,"Select an object to view details.", s.Text); return; }
    float rowH=22.0f; float col=m_bounds.x+12; float w=m_bounds.w-24; const char* xyz[3]={"X","Y","Z"};
    // Transform
    dl.text(col, y, "Transform", s.Text); y+=18;
    if (gCoordinator.HasComponent<TransformComponent>((unsigned)sel))
    {
        auto& tr=gCoordinator.GetComponent<TransformComponent>((unsigned)sel);
        dl.text(col,y+4,"Position", s.Text); drawStepper3(dl, Rect{col+90,y,w-100,rowH}, xyz, tr.position.data(), s.PanelBg, s.Text); y+=rowH+6;
        dl.text(col,y+4,"Rotation", s.Text); drawStepper3(dl, Rect{col+90,y,w-100,rowH}, xyz, tr.rotation.data(), s.PanelBg, s.Text); y+=rowH+6;
        dl.text(col,y+4,"Scale", s.Text);    drawStepper3(dl, Rect{col+90,y,w-100,rowH}, xyz, tr.scale.data(),    s.PanelBg, s.Text); y+=rowH+10;
    }
    // Material
    dl.text(col, y, "Material", s.Text); y+=18;
    if (gCoordinator.HasComponent<MaterialComponent>((unsigned)sel))
    {
        auto& mc=gCoordinator.GetComponent<MaterialComponent>((unsigned)sel);
        const char* rgb[3]={"R","G","B"};
        dl.text(col,y+4,"BaseColor", s.Text); drawStepper3(dl, Rect{col+90,y,w-100,rowH}, rgb, mc.baseColor, s.PanelBg, s.Text); y+=rowH+6;
        float tmp[3]={mc.metallic,0,0}; const char* m[3]={"M","",""}; dl.text(col,y+4,"Metallic", s.Text); drawStepper3(dl, Rect{col+90,y,w-100,rowH}, m, tmp, s.PanelBg, s.Text); y+=rowH+6;
        tmp[0]=mc.roughness; dl.text(col,y+4,"Roughness", s.Text); drawStepper3(dl, Rect{col+90,y,w-100,rowH}, m, tmp, s.PanelBg, s.Text); y+=rowH+6;
    }
}

void DetailsPanel::tick()
{
    if (!m_window) return; int sel=EditorState::GetSelected(); if(sel<0||!gCoordinator.IsEntityAlive((unsigned)sel)) return; bool shift = glfwGetKey(m_window, GLFW_KEY_LEFT_SHIFT)==GLFW_PRESS || glfwGetKey(m_window, GLFW_KEY_RIGHT_SHIFT)==GLFW_PRESS; float step = shift?1.0f:0.1f;
    double mx,my; glfwGetCursorPos(m_window,&mx,&my);
    bool pressed = glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT)==GLFW_PRESS;
    auto s=GetStyle(); float rowH=22.0f; float col=m_bounds.x+12; float w=m_bounds.w-24; float y=m_bounds.y+headerH()+8+18; // after "Transform" title
    auto handleTriple=[&](float* v){ float x=col+90; float y0=y; float colW=(w-100-16)/3.0f; for(int i=0;i<3;++i){ Rect r{ x + i*(colW+8), y0, colW, rowH }; if(inside((float)mx,(float)my,r) && pressed && !m_prevPressed){ if(((float)mx - r.x) < r.w*0.5f) v[i]-=step; else v[i]+=step; } } y+=rowH+6; };
    if (gCoordinator.HasComponent<TransformComponent>((unsigned)sel))
    { auto& tr=gCoordinator.GetComponent<TransformComponent>((unsigned)sel); handleTriple(tr.position.data()); handleTriple(tr.rotation.data()); handleTriple(tr.scale.data()); y+=4; }
    y+=18; // after "Material" title
    if (gCoordinator.HasComponent<MaterialComponent>((unsigned)sel))
    { auto& mc=gCoordinator.GetComponent<MaterialComponent>((unsigned)sel); handleTriple(mc.baseColor); float tmp[3]={mc.metallic,0,0}; handleTriple(tmp); mc.metallic=tmp[0]; tmp[0]=mc.roughness; handleTriple(tmp); mc.roughness=tmp[0]; }
    m_prevPressed=pressed;
}

// -------- Content Browser --------
void ContentPanel::paint(DrawList& dl)
{
    PanelBase::paint(dl);
    auto s = GetStyle(); unsigned text = rgba(s.Text); unsigned item = rgba(s.PanelBg);
    float top = m_bounds.y + headerH(); float leftW = m_bounds.w * 0.25f;
    dl.rectFilled(m_bounds.x + 8, top + 8, leftW - 16, m_bounds.h - headerH() - 16, item);
    float gx = m_bounds.x + leftW + 8, gy = top + 8, gw = m_bounds.w - leftW - 16, gh = m_bounds.h - headerH() - 16;
    dl.rectFilled(gx, gy, gw, gh, item); dl.text(gx + 8, gy + 6, "Content/", s.Text);
    float x = gx + 8, y = gy + 28 - m_scroll; float tw=120, th=90; int cols = (int)((gw - 16) / (tw+12)); if (cols<1) cols=1;
    int tiles = 60; int firstRow = std::max(0,(int)(m_scroll/(th+16))); int rowsVisible=(int)((gh-28)/(th+16))+2; int lastTile=std::min(tiles,(firstRow+rowsVisible)*cols);
    for(int i=firstRow*cols;i<lastTile;++i){ float rx=x+(i%cols)*(tw+12); float ry=y+(i/cols)*(th+16); unsigned col = rgba(s.PanelBg); if(i==m_selected) col = rgba(Color{ s.Accent.r, s.Accent.g, s.Accent.b, 0.35f }); dl.rectFilled(rx, ry, tw, th, col); }
}

void ContentPanel::tick()
{
    if (!m_window) return; if (glfwGetKey(m_window, GLFW_KEY_PAGE_UP) == GLFW_PRESS)   m_scroll = std::max(0.0f, m_scroll - 20.0f); if (glfwGetKey(m_window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS) m_scroll = m_scroll + 20.0f; double mx,my; glfwGetCursorPos(m_window,&mx,&my); float top = m_bounds.y + headerH(); float leftW = m_bounds.w * 0.25f; float gx = m_bounds.x + leftW + 8, gy = top + 8, gw = m_bounds.w - leftW - 16, gh = m_bounds.h - headerH() - 16; Rect grid{gx, gy+28, gw-16, gh-28}; if (inside((float)mx,(float)my, grid) && glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT)==GLFW_PRESS){ float tw=120, th=90; int cols=(int)((gw-16)/(tw+12)); if(cols<1) cols=1; float localX=(float)mx-(gx+8); float localY=(float)my-(gy+28)+m_scroll; int col=(int)(localX/(tw+12)); int row=(int)(localY/(th+16)); if(col>=0&&col<cols&&row>=0){ int idx=row*cols+col; if(idx>=0&&idx<60) m_selected=idx; }}
}

} // namespace ui2

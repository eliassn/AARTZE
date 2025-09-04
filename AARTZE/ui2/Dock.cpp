#include "Dock.hpp"
#include "StyleSet.hpp"

namespace ui2 {

void TabBar::paint(DrawList& dl)
{
    auto s = GetStyle();
    unsigned bg = ((unsigned)(s.PanelBg.a*255)<<24)|((unsigned)(s.PanelBg.b*255)<<16)|((unsigned)(s.PanelBg.g*255)<<8)|((unsigned)(s.PanelBg.r*255));
    unsigned wbg= ((unsigned)(s.WindowBg.a*255)<<24)|((unsigned)(s.WindowBg.b*255)<<16)|((unsigned)(s.WindowBg.g*255)<<8)|((unsigned)(s.WindowBg.r*255));
    // Header
    dl.rectFilled(m_bounds.x, m_bounds.y, m_bounds.w, 24.0f, bg);
    // Body panel
    dl.rectFilled(m_bounds.x, m_bounds.y+24.0f, m_bounds.w, m_bounds.h-24.0f, wbg);
    // Titles (stub): draw first few titles as text; real layout will come later
    float x = m_bounds.x + 8.0f; float y = m_bounds.y + 4.0f;
    unsigned tx = ((unsigned)(s.Text.a*255)<<24)|((unsigned)(s.Text.b*255)<<16)|((unsigned)(s.Text.g*255)<<8)|((unsigned)(s.Text.r*255));
    int count = 0;
    for (auto& t : m_tabs) { if (++count>4) break; /* text later */ x += 120.0f; }
}

void DockSpace::layout(const Rect& b)
{
    Widget::layout(b);
    float x=b.x, y=b.y, w=b.w, h=b.h;
    float leftW = m_left ? w*m_leftRatio : 0.0f;
    float rightW = m_right ? w*m_rightRatio : 0.0f;
    float bottomH = m_bottom ? h*m_bottomRatio : 0.0f;
    if (m_left)  m_left->layout({x,y,w* m_leftRatio, h-bottomH});
    if (m_right) m_right->layout({x+w-rightW,y,rightW,h-bottomH});
    float cx = x + leftW; float cw = w - leftW - rightW;
    if (m_center) m_center->layout({cx,y,cw,h-bottomH});
    if (m_bottom) m_bottom->layout({x,y+h-bottomH,w,bottomH});
}

void DockSpace::paint(DrawList& dl)
{
    if (m_left) m_left->paint(dl);
    if (m_center) m_center->paint(dl);
    if (m_right) m_right->paint(dl);
    if (m_bottom) m_bottom->paint(dl);
}

}

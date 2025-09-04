#include "Viewport.hpp"

namespace ui2 {

static unsigned rgba(const Color& c){ return ((unsigned)(c.a*255)<<24)|((unsigned)(c.b*255)<<16)|((unsigned)(c.g*255)<<8)|((unsigned)(c.r*255)); }

void ViewportPanel::paint(DrawList& dl)
{
    PanelBase::paint(dl);
    // Draw a thin border to suggest the viewport region
    auto s = GetStyle();
    unsigned accent = rgba(s.Accent);
    float y0 = m_bounds.y + headerH();
    // Top border
    dl.rectFilled(m_bounds.x, y0, m_bounds.w, 1.0f, accent);
    // Left/right
    dl.rectFilled(m_bounds.x, y0, 1.0f, m_bounds.h - headerH(), accent);
    dl.rectFilled(m_bounds.x + m_bounds.w - 1.0f, y0, 1.0f, m_bounds.h - headerH(), accent);
    // Bottom
    dl.rectFilled(m_bounds.x, m_bounds.y + m_bounds.h - 1.0f, m_bounds.w, 1.0f, accent);
}

} // namespace ui2


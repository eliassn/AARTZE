#pragma once
#include "Panels.hpp"

namespace ui2 {

class ViewportPanel : public PanelBase {
public:
    ViewportPanel() : PanelBase("Viewport") {}
    void paint(DrawList& dl) override;
    void tick() override;
private:
    bool m_play{false};
};

} // namespace ui2


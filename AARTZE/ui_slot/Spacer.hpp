#pragma once
#include "Core.hpp"

class UiSpacer : public UiWidget {
public:
    explicit UiSpacer(glm::vec2 size) : m_size(size) {}
    glm::vec2 Measure(const UiMeasureCtx&) override { return m_size; }
    void Arrange(const UiGeometry& parent, UiArranged& out) override { out.Add(this, parent); }
    void Paint(UiPaintCtx&, const UiGeometry&) override {}
private:
    glm::vec2 m_size;
};

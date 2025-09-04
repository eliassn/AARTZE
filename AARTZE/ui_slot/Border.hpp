#pragma once
#include <memory>
#include <algorithm>
#include "Core.hpp"

// Single-child container with padding and alignment, like UE's SBorder (sans visuals).
class UiBorder : public UiWidget {
public:
    UiBrush brush{}; // not painted yet
    UiMargin padding{};
    UiHAlign hAlign{UiHAlign::Fill};
    UiVAlign vAlign{UiVAlign::Fill};

    void SetChild(std::shared_ptr<UiWidget> c) { child = std::move(c); }

    glm::vec2 Measure(const UiMeasureCtx& ctx) override
    {
        glm::vec2 d(0,0);
        if (child) d = child->Measure(ctx);
        d.x += padding.left + padding.right;
        d.y += padding.top  + padding.bottom;
        return d;
    }

    void Arrange(const UiGeometry& parent, UiArranged& out) override
    {
        if (!child) return;
        UiGeometry inner = parent;
        inner.rect.pos.x  += padding.left;
        inner.rect.pos.y  += padding.top;
        inner.rect.size.x -= (padding.left + padding.right);
        inner.rect.size.y -= (padding.top + padding.bottom);

        // Child desired
        glm::vec2 want = child->Measure(UiMeasureCtx{parent.dpiScale*96.0f});
        glm::vec2 size = inner.rect.size;
        if (hAlign != UiHAlign::Fill) size.x = std::min(size.x, want.x);
        if (vAlign != UiVAlign::Fill) size.y = std::min(size.y, want.y);

        glm::vec2 pos = inner.rect.pos;
        // Horizontal alignment
        if (hAlign == UiHAlign::Center) pos.x += (inner.rect.size.x - size.x) * 0.5f;
        else if (hAlign == UiHAlign::Right) pos.x += (inner.rect.size.x - size.x);
        // Vertical alignment
        if (vAlign == UiVAlign::Center) pos.y += (inner.rect.size.y - size.y) * 0.5f;
        else if (vAlign == UiVAlign::Bottom) pos.y += (inner.rect.size.y - size.y);

        UiGeometry g = parent; g.rect = UiRect{pos, size};
        out.Add(child.get(), g);
    }

    void Paint(UiPaintCtx&, const UiGeometry&) override {}

private:
    std::shared_ptr<UiWidget> child{};
};


#pragma once
#include <memory>
#include <optional>
#include <algorithm>
#include "Core.hpp"

// Size constraint wrapper similar to UE's SSizeBox.
class UiSizeBox : public UiWidget {
public:
    std::optional<float> widthOverride;
    std::optional<float> heightOverride;
    std::optional<float> minWidth, minHeight, maxWidth, maxHeight;

    void SetChild(std::shared_ptr<UiWidget> c) { child = std::move(c); }

    glm::vec2 Measure(const UiMeasureCtx& ctx) override
    {
        glm::vec2 d(0,0); if (child) d = child->Measure(ctx);
        d.x = applyAxis(d.x, widthOverride, minWidth, maxWidth);
        d.y = applyAxis(d.y, heightOverride, minHeight, maxHeight);
        return d;
    }

    void Arrange(const UiGeometry& parent, UiArranged& out) override
    {
        if (!child) return;
        glm::vec2 avail = parent.rect.size;
        glm::vec2 size;
        // Compute constrained size within available rect
        glm::vec2 want = child->Measure(UiMeasureCtx{parent.dpiScale*96.0f});
        size.x = applyAxis(want.x, widthOverride, minWidth, maxWidth);
        size.y = applyAxis(want.y, heightOverride, minHeight, maxHeight);
        size.x = std::min(size.x, avail.x);
        size.y = std::min(size.y, avail.y);

        // Center within parent rect
        glm::vec2 pos = parent.rect.pos;
        pos.x += (avail.x - size.x) * 0.5f;
        pos.y += (avail.y - size.y) * 0.5f;
        UiGeometry g = parent; g.rect = UiRect{pos, size};
        out.Add(child.get(), g);
    }

    void Paint(UiPaintCtx&, const UiGeometry&) override {}

private:
    std::shared_ptr<UiWidget> child{};

    static float applyAxis(float v, const std::optional<float>& overrideV,
                           const std::optional<float>& minV, const std::optional<float>& maxV)
    {
        float out = overrideV ? *overrideV : v;
        if (minV) out = std::max(out, *minV);
        if (maxV) out = std::min(out, *maxV);
        return out;
    }
};


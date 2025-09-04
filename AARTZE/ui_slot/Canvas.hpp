#pragma once
#include "Core.hpp"
#include <algorithm>

// Mirrors UE SConstraintCanvas::FSlot semantics (Offset/Anchors/Alignment/AutoSize/ZOrder)
struct UiCanvasSlot : public UiPanelSlot {
    UiMargin offset{0,0,1,1};   // L,T,R,B (R,B used as size when anchors are equal)
    UiAnchors anchors{ {0,0}, {0,0} };
    glm::vec2 alignment{0.5f, 0.5f};
    bool autoSize{false};
    int zOrder{0};
};

class UiCanvas : public UiPanel {
public:
    UiCanvasSlot& Add(const std::shared_ptr<UiWidget>& child)
    {
        m_children.push_back(child);
        m_slots.emplace_back();
        return m_slots.back();
    }

    glm::vec2 Measure(const UiMeasureCtx& ctx) override
    {
        // Canvas size is driven by parent; fallback to max desired of children
        glm::vec2 maxSize(0,0);
        for (size_t i=0;i<m_children.size();++i)
        {
            glm::vec2 d = m_children[i]->Measure(ctx);
            const UiMargin& p = m_slots[i].padding;
            d.x += p.left + p.right; d.y += p.top + p.bottom;
            maxSize.x = std::max(maxSize.x, d.x);
            maxSize.y = std::max(maxSize.y, d.y);
        }
        return maxSize;
    }

    void Arrange(const UiGeometry& parent, UiArranged& out) override
    {
        const UiRect& pr = parent.rect;
        // Stable sort by Z order for paint layering like UE SConstraintCanvas
        std::vector<size_t> order(m_children.size());
        for (size_t i=0;i<order.size();++i) order[i]=i;
        std::stable_sort(order.begin(), order.end(), [&](size_t a, size_t b){ return m_slots[a].zOrder < m_slots[b].zOrder; });

        for (size_t ord=0; ord<order.size(); ++ord)
        {
            size_t i = order[ord];
            auto& s = m_slots[i];
            glm::vec2 aMin = { pr.pos.x + pr.size.x * s.anchors.min.x,
                                pr.pos.y + pr.size.y * s.anchors.min.y };
            glm::vec2 aMax = { pr.pos.x + pr.size.x * s.anchors.max.x,
                                pr.pos.y + pr.size.y * s.anchors.max.y };

            // Compute size and position according to UE SConstraintCanvas rules
            glm::vec2 sizePx{0,0};
            glm::vec2 posPx{0,0};

            // X dimension
            if (s.anchors.min.x == s.anchors.max.x)
            {
                // absolute: offset.right is Width (unless autosize)
                sizePx.x = s.autoSize ? m_children[i]->Measure(UiMeasureCtx{parent.dpiScale*96.0f}).x : (float)s.offset.right;
                posPx.x  = aMin.x + (float)s.offset.left;
            }
            else
            {
                // anchored stretch: right is distance from max anchor
                float span = aMax.x - aMin.x;
                sizePx.x = span - (float)(s.offset.left + s.offset.right);
                posPx.x  = aMin.x + (float)s.offset.left;
            }

            // Y dimension
            if (s.anchors.min.y == s.anchors.max.y)
            {
                sizePx.y = s.autoSize ? m_children[i]->Measure(UiMeasureCtx{parent.dpiScale*96.0f}).y : (float)s.offset.bottom;
                posPx.y  = aMin.y + (float)s.offset.top;
            }
            else
            {
                float span = aMax.y - aMin.y;
                sizePx.y = span - (float)(s.offset.top + s.offset.bottom);
                posPx.y  = aMin.y + (float)s.offset.top;
            }

            // Alignment shifts the rect by pivot
            posPx.x -= s.alignment.x * sizePx.x;
            posPx.y -= s.alignment.y * sizePx.y;

            // Apply padding from UiPanelSlot
            posPx.x += s.padding.left;
            posPx.y += s.padding.top;
            sizePx.x -= (s.padding.left + s.padding.right);
            sizePx.y -= (s.padding.top + s.padding.bottom);

            UiGeometry g{}; g.dpiScale = parent.dpiScale; g.transform = parent.transform; g.rect = UiRect{posPx, sizePx};
            out.Add(m_children[i].get(), g);
        }
    }

    void Paint(UiPaintCtx&, const UiGeometry&) override {}
private:
    std::vector<UiCanvasSlot> m_slots;
};

#pragma once
#include "Core.hpp"
#include <algorithm>

struct UiOverlaySlot : public UiPanelSlot {
    int zOrder{0};
};

class UiOverlay : public UiPanel {
public:
    UiOverlaySlot& Add(const std::shared_ptr<UiWidget>& child)
    {
        m_children.push_back(child);
        m_slots.emplace_back();
        return m_slots.back();
    }

    glm::vec2 Measure(const UiMeasureCtx& ctx) override
    {
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
        // Sort by z-order like UE's SOverlay::FOverlaySlot
        std::vector<size_t> order(m_children.size());
        for (size_t i=0;i<order.size();++i) order[i]=i;
        std::stable_sort(order.begin(), order.end(), [&](size_t a, size_t b){ return m_slots[a].zOrder < m_slots[b].zOrder; });

        for (size_t ord=0; ord<order.size(); ++ord)
        {
            size_t i = order[ord];
            auto& slot = m_slots[i];
            UiGeometry g{}; g.dpiScale = parent.dpiScale; g.transform = parent.transform; g.rect = parent.rect;
            // Apply padding alignment (simple version)
            g.rect.pos.x += slot.padding.left;
            g.rect.pos.y += slot.padding.top;
            g.rect.size.x -= (slot.padding.left + slot.padding.right);
            g.rect.size.y -= (slot.padding.top + slot.padding.bottom);
            out.Add(m_children[i].get(), g);
        }
    }

    void Paint(UiPaintCtx& ctx, const UiGeometry& g) override
    {
        // Children paint handled by caller after Arrange â†’ Paint for each item
        (void)ctx; (void)g;
    }
private:
    std::vector<UiOverlaySlot> m_slots;
};

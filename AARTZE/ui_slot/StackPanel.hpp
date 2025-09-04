#pragma once
#include <vector>
#include <memory>
#include <algorithm>
#include "Core.hpp"

struct UiStackSlot : public UiPanelSlot {
    UiSizeRule sizeRule{UiSizeRule::Auto};
    float fill{0.0f}; // used when sizeRule==Fill
};

// Horizontal/Vertical box similar to UE's SBoxPanel (SVerticalBox/SHorizontalBox)
class UiStackPanel : public UiPanel {
public:
    explicit UiStackPanel(UiOrientation o = UiOrientation::Vertical) : m_orientation(o) {}

    UiStackSlot& Add(const std::shared_ptr<UiWidget>& child)
    {
        m_children.push_back(child);
        m_slots.emplace_back();
        return m_slots.back();
    }

    glm::vec2 Measure(const UiMeasureCtx& ctx) override
    {
        bool horizontal = (m_orientation == UiOrientation::Horizontal);
        float major = 0.0f; float cross = 0.0f;
        for (size_t i=0;i<m_children.size();++i)
        {
            glm::vec2 d = m_children[i]->Measure(ctx);
            const UiMargin& p = m_slots[i].padding;
            d.x += p.left + p.right; d.y += p.top + p.bottom;
            if (horizontal) { major += (m_slots[i].sizeRule==UiSizeRule::Auto ? d.x : 0.0f); cross = std::max(cross, d.y); }
            else            { major += (m_slots[i].sizeRule==UiSizeRule::Auto ? d.y : 0.0f); cross = std::max(cross, d.x); }
        }
        return horizontal ? glm::vec2(major, cross) : glm::vec2(cross, major);
    }

    void Arrange(const UiGeometry& parent, UiArranged& out) override
    {
        bool horizontal = (m_orientation == UiOrientation::Horizontal);
        float availMajor = horizontal ? parent.rect.size.x : parent.rect.size.y;
        float availCross = horizontal ? parent.rect.size.y : parent.rect.size.x;

        // First pass: measure autos and sum fill weights
        float usedByAuto = 0.0f; float totalFill = 0.0f;
        std::vector<glm::vec2> wants(m_children.size());
        for (size_t i=0;i<m_children.size();++i)
        {
            wants[i] = m_children[i]->Measure(UiMeasureCtx{parent.dpiScale*96.0f});
            const UiMargin& p = m_slots[i].padding;
            if (horizontal) { wants[i].x += p.left + p.right; wants[i].y += p.top + p.bottom; }
            else            { wants[i].x += p.left + p.right; wants[i].y += p.top + p.bottom; }
            if (m_slots[i].sizeRule == UiSizeRule::Auto)
                usedByAuto += (horizontal ? wants[i].x : wants[i].y);
            else totalFill += std::max(0.0f, m_slots[i].fill);
        }
        float remaining = std::max(0.0f, availMajor - usedByAuto);

        // Second pass: layout
        float cursor = horizontal ? parent.rect.pos.x : parent.rect.pos.y;
        for (size_t i=0;i<m_children.size();++i)
        {
            auto& slot = m_slots[i];
            glm::vec2 childSize = wants[i]; // includes padding currently
            const UiMargin& p = slot.padding;
            // Remove padding from size when assigning to child
            childSize.x = std::max(0.0f, childSize.x - (p.left + p.right));
            childSize.y = std::max(0.0f, childSize.y - (p.top + p.bottom));

            float thisMajor = (slot.sizeRule==UiSizeRule::Auto)
                              ? (horizontal ? wants[i].x : wants[i].y)
                              : (totalFill>0.0f ? remaining * (slot.fill/totalFill) : 0.0f);

            float crossSize = horizontal ? childSize.y : childSize.x;
            float maxCross  = availCross - (horizontal ? (p.top+p.bottom) : (p.left+p.right));
            float usedCross = std::min(crossSize, maxCross);
            // Fill cross if requested
            if ((horizontal && slot.vAlign==UiVAlign::Fill) || (!horizontal && slot.hAlign==UiHAlign::Fill))
                usedCross = maxCross;

            glm::vec2 pos = parent.rect.pos;
            glm::vec2 size = horizontal ? glm::vec2(thisMajor - (p.left+p.right), usedCross)
                                        : glm::vec2(usedCross, thisMajor - (p.top+p.bottom));

            if (horizontal)
            {
                pos.x = cursor + p.left; cursor += thisMajor; // advance cursor
                // vertical alignment within cross
                pos.y += p.top;
                if      (slot.vAlign==UiVAlign::Center) pos.y += (availCross - usedCross)*0.5f;
                else if (slot.vAlign==UiVAlign::Bottom) pos.y += (availCross - usedCross);
            }
            else
            {
                pos.y = cursor + p.top; cursor += thisMajor;
                pos.x += p.left;
                if      (slot.hAlign==UiHAlign::Center) pos.x += (availCross - usedCross)*0.5f;
                else if (slot.hAlign==UiHAlign::Right)  pos.x += (availCross - usedCross);
            }

            UiGeometry g = parent; g.rect = UiRect{pos, size};
            out.Add(m_children[i].get(), g);
        }
    }

    void Paint(UiPaintCtx&, const UiGeometry&) override {}

private:
    UiOrientation m_orientation{UiOrientation::Vertical};
    std::vector<UiStackSlot> m_slots;
};


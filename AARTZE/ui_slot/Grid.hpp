#pragma once
#include <vector>
#include <memory>
#include <algorithm>
#include "Core.hpp"

struct UiGridSlot : public UiPanelSlot {
    int row{0};
    int column{0};
    int rowSpan{1};
    int columnSpan{1};
};

// Simple grid similar to UE's SGridPanel (approximate spanning behavior)
class UiGrid : public UiPanel {
public:
    UiGridSlot& Add(const std::shared_ptr<UiWidget>& child)
    {
        m_children.push_back(child);
        m_slots.emplace_back();
        return m_slots.back();
    }

    glm::vec2 Measure(const UiMeasureCtx& ctx) override
    {
        computeGridMetrics(ctx);
        float totalW = 0.0f, totalH = 0.0f;
        for (float w : m_colWidths) totalW += w;
        for (float h : m_rowHeights) totalH += h;
        return {totalW, totalH};
    }

    void Arrange(const UiGeometry& parent, UiArranged& out) override
    {
        computeGridMetrics(UiMeasureCtx{parent.dpiScale*96.0f});
        // Compute cumulative offsets
        std::vector<float> xOff(m_colWidths.size()+1, 0.0f), yOff(m_rowHeights.size()+1, 0.0f);
        for (size_t i=1;i<xOff.size();++i) xOff[i] = xOff[i-1] + m_colWidths[i-1];
        for (size_t i=1;i<yOff.size();++i) yOff[i] = yOff[i-1] + m_rowHeights[i-1];

        for (size_t i=0;i<m_children.size();++i)
        {
            auto& s = m_slots[i];
            size_t c0 = (size_t)std::max(0, s.column);
            size_t r0 = (size_t)std::max(0, s.row);
            size_t c1 = std::min(xOff.size()-1, c0 + (size_t)std::max(1, s.columnSpan));
            size_t r1 = std::min(yOff.size()-1, r0 + (size_t)std::max(1, s.rowSpan));
            glm::vec2 cellPos = { parent.rect.pos.x + xOff[c0], parent.rect.pos.y + yOff[r0] };
            glm::vec2 cellSize = { xOff[c1] - xOff[c0], yOff[r1] - yOff[r0] };

            // Apply padding & alignment
            glm::vec2 pos = cellPos; glm::vec2 size = cellSize;
            pos.x += s.padding.left; pos.y += s.padding.top;
            size.x -= (s.padding.left + s.padding.right);
            size.y -= (s.padding.top + s.padding.bottom);
            glm::vec2 want = m_children[i]->Measure(UiMeasureCtx{parent.dpiScale*96.0f});
            if (s.hAlign != UiHAlign::Fill) size.x = std::min(size.x, want.x);
            if (s.vAlign != UiVAlign::Fill) size.y = std::min(size.y, want.y);
            if (s.hAlign == UiHAlign::Center) pos.x += (cellSize.x - s.padding.left - s.padding.right - size.x)*0.5f;
            else if (s.hAlign == UiHAlign::Right) pos.x += (cellSize.x - s.padding.left - s.padding.right - size.x);
            if (s.vAlign == UiVAlign::Center) pos.y += (cellSize.y - s.padding.top - s.padding.bottom - size.y)*0.5f;
            else if (s.vAlign == UiVAlign::Bottom) pos.y += (cellSize.y - s.padding.top - s.padding.bottom - size.y);

            UiGeometry g = parent; g.rect = UiRect{pos, size};
            out.Add(m_children[i].get(), g);
        }
    }

    void Paint(UiPaintCtx&, const UiGeometry&) override {}

private:
    std::vector<UiGridSlot> m_slots;
    std::vector<float> m_colWidths;
    std::vector<float> m_rowHeights;

    void computeGridMetrics(const UiMeasureCtx& ctx)
    {
        // Determine grid extents
        int maxCol = 0, maxRow = 0;
        for (auto& s : m_slots)
        {
            maxCol = std::max(maxCol, s.column + std::max(1, s.columnSpan));
            maxRow = std::max(maxRow, s.row    + std::max(1, s.rowSpan));
        }
        if (maxCol <= 0) maxCol = 1; if (maxRow <= 0) maxRow = 1;
        m_colWidths.assign((size_t)maxCol, 0.0f);
        m_rowHeights.assign((size_t)maxRow, 0.0f);

        // First pass: non-spanning items define min sizes
        for (size_t i=0;i<m_children.size();++i)
        {
            auto& s = m_slots[i];
            glm::vec2 d = m_children[i]->Measure(ctx);
            d.x += s.padding.left + s.padding.right;
            d.y += s.padding.top + s.padding.bottom;
            if (s.columnSpan == 1) m_colWidths[(size_t)s.column] = std::max(m_colWidths[(size_t)s.column], d.x);
            if (s.rowSpan == 1)    m_rowHeights[(size_t)s.row]   = std::max(m_rowHeights[(size_t)s.row],  d.y);
        }

        // Second pass: approximate spans (distribute extra evenly)
        for (size_t i=0;i<m_children.size();++i)
        {
            auto& s = m_slots[i];
            if (s.columnSpan > 1)
            {
                glm::vec2 d = m_children[i]->Measure(ctx);
                d.x += s.padding.left + s.padding.right;
                float curr = 0.0f; for (int c=0;c<s.columnSpan;++c) curr += m_colWidths[(size_t)(s.column+c)];
                if (d.x > curr)
                {
                    float add = (d.x - curr) / (float)s.columnSpan;
                    for (int c=0;c<s.columnSpan;++c) m_colWidths[(size_t)(s.column+c)] += add;
                }
            }
            if (s.rowSpan > 1)
            {
                glm::vec2 d = m_children[i]->Measure(ctx);
                d.y += s.padding.top + s.padding.bottom;
                float curr = 0.0f; for (int r=0;r<s.rowSpan;++r) curr += m_rowHeights[(size_t)(s.row+r)];
                if (d.y > curr)
                {
                    float add = (d.y - curr) / (float)s.rowSpan;
                    for (int r=0;r<s.rowSpan;++r) m_rowHeights[(size_t)(s.row+r)] += add;
                }
            }
        }
    }
};


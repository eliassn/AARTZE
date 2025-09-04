#pragma once
#include <vector>
#include <memory>
#include <algorithm>
#include "Core.hpp"

struct UiUniformGridSlot : public UiPanelSlot {
    int row{0};
    int column{0};
};

// Uniform grid: each cell is the same size; columns count configurable, rows auto.
class UiUniformGrid : public UiPanel {
public:
    explicit UiUniformGrid(int columns = 1) : m_columns(std::max(1, columns)) {}

    UiUniformGridSlot& Add(const std::shared_ptr<UiWidget>& child)
    {
        m_children.push_back(child);
        m_slots.emplace_back();
        return m_slots.back();
    }

    void SetColumns(int c) { m_columns = std::max(1, c); }
    int  GetColumns() const { return m_columns; }

    glm::vec2 Measure(const UiMeasureCtx& ctx) override
    {
        glm::vec2 cell(0,0);
        for (size_t i=0;i<m_children.size();++i)
        {
            glm::vec2 d = m_children[i]->Measure(ctx);
            d.x += m_slots[i].padding.left + m_slots[i].padding.right;
            d.y += m_slots[i].padding.top + m_slots[i].padding.bottom;
            cell.x = std::max(cell.x, d.x); cell.y = std::max(cell.y, d.y);
        }
        int rows = m_children.empty() ? 1 : (int)((m_children.size() + (size_t)m_columns - 1) / (size_t)m_columns);
        return { cell.x * (float)m_columns, cell.y * (float)rows };
    }

    void Arrange(const UiGeometry& parent, UiArranged& out) override
    {
        glm::vec2 cell(0,0);
        // Determine cell size from desired (uniform)
        for (size_t i=0;i<m_children.size();++i)
        {
            glm::vec2 d = m_children[i]->Measure(UiMeasureCtx{parent.dpiScale*96.0f});
            d.x += m_slots[i].padding.left + m_slots[i].padding.right;
            d.y += m_slots[i].padding.top + m_slots[i].padding.bottom;
            cell.x = std::max(cell.x, d.x); cell.y = std::max(cell.y, d.y);
        }
        if (cell.x <= 0) cell.x = 1.0f; if (cell.y <= 0) cell.y = 1.0f;

        for (size_t i=0;i<m_children.size();++i)
        {
            int col = m_slots[i].column;
            int row = m_slots[i].row;
            // If not explicitly set, derive from index
            if (col < 0 || row < 0) { int idx = (int)i; row = idx / m_columns; col = idx % m_columns; }
            glm::vec2 pos = { parent.rect.pos.x + cell.x * (float)col, parent.rect.pos.y + cell.y * (float)row };
            glm::vec2 size = cell;
            // Apply padding
            pos.x += m_slots[i].padding.left; pos.y += m_slots[i].padding.top;
            size.x -= (m_slots[i].padding.left + m_slots[i].padding.right);
            size.y -= (m_slots[i].padding.top + m_slots[i].padding.bottom);
            UiGeometry g = parent; g.rect = UiRect{pos, size};
            out.Add(m_children[i].get(), g);
        }
    }

    void Paint(UiPaintCtx&, const UiGeometry&) override {}

private:
    int m_columns{1};
    std::vector<UiUniformGridSlot> m_slots;
};


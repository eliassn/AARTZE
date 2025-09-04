#pragma once
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include "Enums.hpp"
#include "Primitives.hpp"
#include "Geometry.hpp"

struct UiMeasureCtx { float dpi{96.0f}; };
struct UiPaintCtx { void* backend{nullptr}; int layer{0}; };

struct UiArrangedItem { class UiWidget* widget; UiGeometry geo; };
struct UiArranged { std::vector<UiArrangedItem> items; void Add(class UiWidget* w, const UiGeometry& g){ items.push_back({w,g}); } };

class UiWidget {
public:
    virtual ~UiWidget() = default;
    virtual glm::vec2 Measure(const UiMeasureCtx&) = 0;
    virtual void Arrange(const UiGeometry& parent, UiArranged& out) = 0;
    virtual void Paint(UiPaintCtx&, const UiGeometry&) {}
    virtual void Tick(float) {}
    virtual bool IsFocusable() const { return false; }
};

struct UiPanelSlot {
    UiMargin padding{};
    UiHAlign hAlign{UiHAlign::Fill};
    UiVAlign vAlign{UiVAlign::Fill};
};

class UiPanel : public UiWidget {
public:
    size_t ChildCount() const { return m_children.size(); }
    std::shared_ptr<UiWidget> Child(size_t i) const { return m_children[i]; }
protected:
    std::vector<std::shared_ptr<UiWidget>> m_children;
};

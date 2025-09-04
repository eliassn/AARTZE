#pragma once
#include "Widget.hpp"
#include "StyleSet.hpp"
#include <string>
#include <vector>
struct GLFWwindow;

namespace ui2 {

class PanelBase : public Widget {
public:
    explicit PanelBase(const std::string& title) : m_title(title) {}
    void layout(const Rect& b) override { Widget::layout(b); }
    void paint(DrawList& dl) override;
    void setWindow(GLFWwindow* w) { m_window = w; }
    virtual void tick() {}
protected:
    std::string m_title;
    float headerH() const { return 28.0f; }
    void paintHeader(DrawList& dl);
    GLFWwindow* m_window{nullptr};
};

class OutlinerPanel : public PanelBase {
public:
    OutlinerPanel() : PanelBase("Outliner") {}
    void paint(DrawList& dl) override;
    void tick() override;
private:
    float m_scroll{0.0f};
    int   m_selected{-1};
};

class DetailsPanel : public PanelBase {\npublic:\n    DetailsPanel() : PanelBase("Details") {}\n    void paint(DrawList& dl) override;\n    void tick() override;\nprivate:\n    bool m_prevPressed{false};\n};

class ContentPanel : public PanelBase {
public:
    ContentPanel() : PanelBase("Content Browser") {}
    void paint(DrawList& dl) override;
    void tick() override;
private:
    float m_scroll{0.0f};
    int   m_selected{-1};
};

} // namespace ui2


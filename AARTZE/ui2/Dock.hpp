#pragma once
#include "Widget.hpp"
#include <memory>
#include <vector>
#include <string>

namespace ui2 {

class Tab : public Widget {
public:
    explicit Tab(std::string title) : m_title(std::move(title)) {}
    const std::string& title() const { return m_title; }
    void paint(DrawList& dl) override { (void)dl; }
private:
    std::string m_title;
};

class TabBar : public Widget {
public:
    void add(std::shared_ptr<Tab> t) { m_tabs.push_back(std::move(t)); }
    void layout(const Rect& b) override { Widget::layout(b); }
    void paint(DrawList& dl) override;
private:
    std::vector<std::shared_ptr<Tab>> m_tabs;
};

class DockSpace : public Widget {
public:
    void setCenter(std::shared_ptr<Widget> w) { m_center = std::move(w); }
    void setLeft(std::shared_ptr<Widget> w, float ratio) { m_left = std::move(w); m_leftRatio = ratio; }
    void setRight(std::shared_ptr<Widget> w, float ratio) { m_right = std::move(w); m_rightRatio = ratio; }
    void setBottom(std::shared_ptr<Widget> w, float ratio) { m_bottom = std::move(w); m_bottomRatio = ratio; }
    void layout(const Rect& b) override;
    void paint(DrawList& dl) override;
private:
    std::shared_ptr<Widget> m_center, m_left, m_right, m_bottom;
    float m_leftRatio{0.23f}, m_rightRatio{0.27f}, m_bottomRatio{0.26f};
};

}
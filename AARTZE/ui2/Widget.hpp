#pragma once
#include <vector>
#include <string>

namespace ui2 {

struct Rect { float x,y,w,h; };
struct Vec2 { float x,y; };

class DrawList {
public:
    struct Cmd { enum Type{RectFilled,Text}; Type type; Rect r; unsigned rgba; std::string text; };
    std::vector<Cmd> cmds;
    void rectFilled(float x,float y,float w,float h, unsigned rgba) { cmds.push_back({Cmd::RectFilled,{x,y,w,h},rgba,{}}); }
    void text(float x,float y,const std::string& t, unsigned rgba) { Cmd c; c.type=Cmd::Text; c.r={x,y,0,0}; c.rgba=rgba; c.text=t; cmds.push_back(std::move(c)); }
    void clear() { cmds.clear(); }
};

class Widget {
public:
    virtual ~Widget() = default;
    virtual void layout(const Rect& bounds) { m_bounds = bounds; }
    virtual void paint(DrawList& dl) = 0;
    const Rect& bounds() const { return m_bounds; }
protected:
    Rect m_bounds{0,0,0,0};
};

}


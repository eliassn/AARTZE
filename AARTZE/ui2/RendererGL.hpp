#pragma once
#include "Widget.hpp"
#include <cstddef>

namespace ui2 {

// Simple GL renderer for DrawList (rectangles now; text later)
class RendererGL {
public:
    void init();
    void shutdown();
    void render(const DrawList& list, int fbWidth, int fbHeight);
private:
    unsigned m_prog{0}, m_vao{0}, m_vbo{0};
    int m_locProj{-1};
    std::size_t m_capacity{0};
};

}

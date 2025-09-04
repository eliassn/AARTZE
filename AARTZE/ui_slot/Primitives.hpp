#pragma once
#include <glm/glm.hpp>

struct UiMargin { float left{0}, top{0}, right{0}, bottom{0}; };

struct UiAnchors { glm::vec2 min{0.0f, 0.0f}; glm::vec2 max{0.0f, 0.0f}; };

struct UiColor { float r{1}, g{1}, b{1}, a{1}; };

struct UiBrush {
    UiColor color{1,1,1,1};
    float cornerRadius{0.0f};
    // Future: image id / nine-slice params
};


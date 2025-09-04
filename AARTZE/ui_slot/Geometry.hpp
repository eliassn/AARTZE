#pragma once
#include <glm/glm.hpp>

struct UiRect { glm::vec2 pos{0,0}; glm::vec2 size{0,0}; };

struct UiTransform {
    glm::vec2 translation{0,0};
    float rotation{0};
    glm::vec2 scale{1,1};
};

struct UiGeometry {
    UiRect rect{};
    float dpiScale{1.0f};
    UiTransform transform{};
};


#pragma once
#include <array>
#include <vector>

struct NavAgentComponent
{
    std::array<float,3> target{0,0,0};
    float speed{2.0f};
    std::vector<std::array<int,2>> path; // grid coordinates
    int currentIndex{-1};
    bool requested{false};
};


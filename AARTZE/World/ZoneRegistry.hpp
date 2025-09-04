#pragma once
#include <array>
#include <string>
#include <utility>
#include <vector>

// Minimal set of named world zones and their representative positions.
// Structured for easy iteration with structured bindings.
inline const std::vector<std::pair<std::string, std::array<float, 3>>> ZonePositions = {
    {"Downtown", {0.0f, 0.0f, 0.0f}},
    {"Harbor", {50.0f, 0.0f, -25.0f}},
    {"Industrial", {-30.0f, 0.0f, 60.0f}},
    {"OldTown", {15.0f, 0.0f, 35.0f}},
};


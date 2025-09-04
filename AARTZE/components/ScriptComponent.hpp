#pragma once
#include <string>
#include <vector>

struct ScriptStep
{
    float triggerTime{0.f};
    std::string actionType;
    std::string actionData;
    bool executed{false};
};

struct ScriptComponent
{
    std::string scriptId;
    std::vector<ScriptStep> steps;
};


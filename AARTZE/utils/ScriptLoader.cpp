#include "ScriptLoader.hpp"

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

ScriptComponent LoadScriptFromJson(const std::string& path)
{
    ScriptComponent script;

    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cerr << "[ScriptLoader] Failed to open " << path << "\n";
        return script;
    }

    nlohmann::json j;
    try
    {
        file >> j;
    }
    catch (const std::exception& e)
    {
        std::cerr << "[ScriptLoader] Failed to parse " << path << ": " << e.what() << "\n";
        return script;
    }

    if (j.contains("scriptId"))
        script.scriptId = j["scriptId"].get<std::string>();

    if (j.contains("steps") && j["steps"].is_array())
    {
        for (const auto& s : j["steps"])
        {
            ScriptStep step;
            step.triggerTime = s.value("time", 0.0f);
            step.actionType = s.value("action", std::string{});
            step.actionData = s.value("data", std::string{});
            step.executed = false;
            script.steps.push_back(step);
        }
    }

    return script;
}

#pragma once
#include <string>
#include "components/ScriptComponent.hpp"

/**
 * @brief Load a ScriptComponent from a JSON file.
 * The JSON must contain a "scriptId" string and a "steps" array where each
 * element has "time", "action" and "data" fields.
 */
ScriptComponent LoadScriptFromJson(const std::string& path);

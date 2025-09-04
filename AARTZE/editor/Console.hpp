#pragma once
#include <string>
#include <vector>

namespace EditorConsole
{
void Clear();
void Log(const std::string& line);
void Exec(const std::string& line);
const std::vector<std::string>& Lines();
}


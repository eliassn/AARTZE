#pragma once
#include <string>

namespace SaveSystem
{
bool SaveWorld(const std::string& path);
bool LoadWorld(const std::string& path);
}


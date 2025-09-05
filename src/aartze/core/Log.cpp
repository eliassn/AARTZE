#include "Log.h"
#include <iostream>

namespace aartze::core {

void LogInfo(const std::string& msg)  { std::cout << "[INFO] "  << msg << std::endl; }
void LogWarn(const std::string& msg)  { std::cout << "[WARN] "  << msg << std::endl; }
void LogError(const std::string& msg) { std::cerr << "[ERROR] " << msg << std::endl; }

}


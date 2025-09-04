#pragma once
#include <string>
#include <vector>

namespace AARTZE {

struct CustomPart {
    std::string name;
    std::string data;
};

std::vector<CustomPart> LoadLearningData();
void SaveCustomPart(const CustomPart& part);

} // namespace AARTZE

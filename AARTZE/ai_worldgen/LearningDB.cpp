#include "LearningDB.hpp"
#include <fstream>
#include <sstream>

namespace AARTZE {
namespace {
const char* kLearningFile = "AARTZE/ai_worldgen/learning_data.txt";
}

std::vector<CustomPart> LoadLearningData() {
    std::vector<CustomPart> parts;
    std::ifstream in(kLearningFile);
    std::string line;
    while (std::getline(in, line)) {
        auto delim = line.find(':');
        if (delim != std::string::npos) {
            CustomPart p{line.substr(0, delim), line.substr(delim + 1)};
            parts.push_back(p);
        }
    }
    return parts;
}

void SaveCustomPart(const CustomPart& part) {
    std::ofstream out(kLearningFile, std::ios::app);
    if (out) {
        out << part.name << ":" << part.data << '\n';
    }
}

} // namespace AARTZE

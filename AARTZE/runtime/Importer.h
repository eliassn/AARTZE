#pragma once
#include "Mesh.h"
#include <string>
#include <vector>

namespace aartze {

struct ImportedModel {
    std::vector<Mesh>     meshes;
    std::vector<Material> materials;
};

class Importer {
public:
    static bool Load(const std::string& path, ImportedModel& out, std::string& err);
};

} // namespace aartze


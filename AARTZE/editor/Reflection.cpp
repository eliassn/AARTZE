#include "Reflection.hpp"

namespace Reflection
{
static std::vector<ComponentInfo> gComponents;

void Register(const std::string& name, DrawFn draw)
{
    gComponents.push_back(ComponentInfo{name, std::move(draw)});
}

const std::vector<ComponentInfo>& List()
{
    return gComponents;
}
}


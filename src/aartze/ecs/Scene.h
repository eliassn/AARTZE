#pragma once
#include <entt/entt.hpp>
namespace aartze::ecs {
struct Scene {
    entt::registry reg;
    entt::entity create() { return reg.create(); }
};
}


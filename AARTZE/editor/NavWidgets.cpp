#include "NavWidgets.hpp"
#include "core/Coordinator.hpp"
#include "components/navigation/NavAgentComponent.hpp"

bool EntityHasNav(int entity)
{
    return gCoordinator.HasComponent<NavAgentComponent>((unsigned)entity);
}

void SetNavTarget(int entity, float x, float z)
{
    auto& nav = gCoordinator.GetComponent<NavAgentComponent>((unsigned)entity);
    nav.target = {x, 0.0f, z};
    nav.requested = true;
}


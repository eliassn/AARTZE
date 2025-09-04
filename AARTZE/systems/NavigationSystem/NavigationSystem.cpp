#include "NavigationSystem.hpp"
#include <cmath>

#include "core/Coordinator.hpp"
#include "components/navigation/NavAgentComponent.hpp"
#include "components/TransformComponent.hpp"

bool NavigationSystem::Initialize()
{
    // Example: mark some blocked cells (optional)
    return true;
}

void NavigationSystem::Update(float dt)
{
    auto agents = gCoordinator.GetEntitiesWithComponents<NavAgentComponent, TransformComponent>();
    for (auto e : agents)
    {
        auto& agent = gCoordinator.GetComponent<NavAgentComponent>(e);
        auto& tr = gCoordinator.GetComponent<TransformComponent>(e);
        auto start = m_grid.ToCell(tr.position[0], tr.position[2]);
        auto goal  = m_grid.ToCell(agent.target[0], agent.target[2]);

        if (agent.requested)
        {
            agent.path = m_grid.FindPath(start, goal);
            agent.currentIndex = agent.path.size() > 0 ? 0 : -1;
            agent.requested = false;
        }
        if (agent.currentIndex >= 0 && agent.currentIndex < (int)agent.path.size())
        {
            auto cell = agent.path[agent.currentIndex];
            auto target = m_grid.CellCenter(cell[0], cell[1]);
            float dx = target[0] - tr.position[0];
            float dz = target[2] - tr.position[2];
            float dist = std::sqrt(dx*dx + dz*dz);
            float step = agent.speed * dt;
            if (dist <= step)
            {
                tr.position[0] = target[0]; tr.position[2] = target[2];
                agent.currentIndex++;
            }
            else
            {
                tr.position[0] += dx / dist * step;
                tr.position[2] += dz / dist * step;
            }
        }
    }
}


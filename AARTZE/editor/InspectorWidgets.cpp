#include "InspectorWidgets.hpp"
#include "core/Coordinator.hpp"
#include "components/TransformComponent.hpp"
#include "components/MaterialComponent.hpp"
#include "components/navigation/NavAgentComponent.hpp"

bool EntityHasTransform(int entity)
{
    return gCoordinator.HasComponent<TransformComponent>((unsigned)entity);
}

bool EntityHasNavAgent(int entity)
{
    return gCoordinator.HasComponent<NavAgentComponent>((unsigned)entity);
}

void AddNavAgent(int entity)
{
    if (!EntityHasNavAgent(entity))
    {
        NavAgentComponent nav; nav.speed = 2.0f; gCoordinator.AddComponent((unsigned)entity, nav);
    }
}

void EditTransform(int entity)
{
    auto& tr = gCoordinator.GetComponent<TransformComponent>((unsigned)entity);
    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::DragFloat3("Position", tr.position.data(), 0.1f);
        ImGui::DragFloat3("Rotation", tr.rotation.data(), 0.5f);
        ImGui::DragFloat3("Scale",    tr.scale.data(),    0.1f, 0.001f, 100.0f);
    }
}

bool EntityHasMaterial(int entity)
{
    return gCoordinator.HasComponent<MaterialComponent>((unsigned)entity);
}

void EditMaterial(int entity)
{
    auto& mat = gCoordinator.GetComponent<MaterialComponent>((unsigned)entity);
    if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::ColorEdit3("BaseColor", mat.baseColor);
        ImGui::SliderFloat("Metallic", &mat.metallic, 0.0f, 1.0f);
        ImGui::SliderFloat("Roughness", &mat.roughness, 0.0f, 1.0f);
    }
}

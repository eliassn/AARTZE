#include "ReflectionBuiltin.hpp"
#include "Reflection.hpp"
#include <imgui.h>
#include "core/Coordinator.hpp"
#include "components/TransformComponent.hpp"
#include "components/MaterialComponent.hpp"
#include "components/physics/RigidBodyComponent.hpp"

static bool passFilter(const char* filter, const char* label)
{
    if (!filter || !*filter) return true;
    return std::string(label).find(filter) != std::string::npos;
}

void RegisterBuiltinInspectors()
{
    using namespace Reflection;
    // Transform
    Register("Transform", [](const DrawCtx& ctx){
        if (!gCoordinator.HasComponent<TransformComponent>((unsigned)ctx.entity)) return;
        auto& tr = gCoordinator.GetComponent<TransformComponent>((unsigned)ctx.entity);
        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (passFilter(ctx.filter, "Position")) ImGui::DragFloat3("Position", tr.position.data(), 0.1f);
            if (passFilter(ctx.filter, "Rotation")) ImGui::DragFloat3("Rotation", tr.rotation.data(), 0.5f);
            if (passFilter(ctx.filter, "Scale"))    ImGui::DragFloat3("Scale",    tr.scale.data(),    0.1f, 0.001f, 100.0f);
            if (ImGui::SmallButton("Reset##Tr")) { tr.position={0,0,0}; tr.rotation={0,0,0}; tr.scale={1,1,1}; }
        }
    });

    // Material
    Register("Material", [](const DrawCtx& ctx){
        if (!gCoordinator.HasComponent<MaterialComponent>((unsigned)ctx.entity)) return;
        auto& mc = gCoordinator.GetComponent<MaterialComponent>((unsigned)ctx.entity);
        if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (passFilter(ctx.filter, "BaseColor")) ImGui::ColorEdit3("BaseColor", mc.baseColor);
            if (passFilter(ctx.filter, "Metallic"))  ImGui::SliderFloat("Metallic", &mc.metallic, 0.0f, 1.0f);
            if (passFilter(ctx.filter, "Roughness")) ImGui::SliderFloat("Roughness", &mc.roughness, 0.0f, 1.0f);
        }
    });

    // RigidBody
    Register("RigidBody", [](const DrawCtx& ctx){
        if (!gCoordinator.HasComponent<RigidBodyComponent>((unsigned)ctx.entity)) return;
        auto& rb = gCoordinator.GetComponent<RigidBodyComponent>((unsigned)ctx.entity);
        if (ImGui::CollapsingHeader("RigidBody", ImGuiTreeNodeFlags_DefaultOpen))
        {
            const char* types[] = {"Static","Dynamic","Kinematic"};
            int t = (int)rb.type;
            if (passFilter(ctx.filter, "Type") && ImGui::Combo("Type", &t, types, IM_ARRAYSIZE(types))) rb.type = (RigidBodyType)t;
            if (passFilter(ctx.filter, "Mass"))        ImGui::DragFloat("Mass", &rb.mass, 0.1f, 0.0f, 1000.0f);
            if (passFilter(ctx.filter, "Friction"))    ImGui::SliderFloat("Friction", &rb.friction, 0.0f, 1.0f);
            if (passFilter(ctx.filter, "Restitution")) ImGui::SliderFloat("Restitution", &rb.restitution, 0.0f, 1.0f);
        }
    });
}


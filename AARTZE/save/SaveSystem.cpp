#include "SaveSystem.hpp"
#include <fstream>
#include <nlohmann/json.hpp>

#include "core/Coordinator.hpp"
#include "components/TransformComponent.hpp"
#include "components/RenderableComponent.hpp"
#include "components/MaterialComponent.hpp"
#include "components/physics/RigidBodyComponent.hpp"
#include "components/physics/BoxColliderComponent.hpp"
#include "components/physics/SphereColliderComponent.hpp"

namespace SaveSystem
{
using json = nlohmann::json;

bool SaveWorld(const std::string& path)
{
    json j;
    j["entities"] = json::array();
    for (unsigned e = 0; e < MAX_ENTITIES; ++e)
    {
        if (!gCoordinator.IsEntityAlive(e)) continue;
        json je; je["id"] = e;
        if (gCoordinator.HasComponent<TransformComponent>(e))
        {
            auto& tr = gCoordinator.GetComponent<TransformComponent>(e);
            je["Transform"] = { {"pos", {tr.position[0],tr.position[1],tr.position[2]}},
                                 {"rot", {tr.rotation[0],tr.rotation[1],tr.rotation[2]}},
                                 {"scl", {tr.scale[0],tr.scale[1],tr.scale[2]}} };
        }
        if (gCoordinator.HasComponent<RenderableComponent>(e))
        {
            auto& rc = gCoordinator.GetComponent<RenderableComponent>(e);
            je["Renderable"] = { {"meshId", rc.meshId}, {"visible", rc.isVisible} };
        }
        if (gCoordinator.HasComponent<MaterialComponent>(e))
        {
            auto& mc = gCoordinator.GetComponent<MaterialComponent>(e);
            je["Material"] = { {"baseColor", {mc.baseColor[0],mc.baseColor[1],mc.baseColor[2]}},
                                {"metallic", mc.metallic}, {"roughness", mc.roughness} };
        }
        if (gCoordinator.HasComponent<RigidBodyComponent>(e))
        {
            auto& rb = gCoordinator.GetComponent<RigidBodyComponent>(e);
            je["RigidBody"] = { {"type", (int)rb.type}, {"mass", rb.mass}, {"friction", rb.friction}, {"restitution", rb.restitution} };
        }
        if (gCoordinator.HasComponent<BoxColliderComponent>(e))
        {
            auto& box = gCoordinator.GetComponent<BoxColliderComponent>(e);
            je["BoxCollider"] = { {"halfExtents", {box.halfExtents[0],box.halfExtents[1],box.halfExtents[2]}} };
        }
        if (gCoordinator.HasComponent<SphereColliderComponent>(e))
        {
            auto& sph = gCoordinator.GetComponent<SphereColliderComponent>(e);
            je["SphereCollider"] = { {"radius", sph.radius} };
        }
        j["entities"].push_back(std::move(je));
    }
    std::ofstream f(path, std::ios::binary); if(!f.is_open()) return false; f << j.dump(2); return true;
}

bool LoadWorld(const std::string& path)
{
    std::ifstream f(path, std::ios::binary); if(!f.is_open()) return false; json j; f >> j;
    // Clear all entities (simple approach)
    for (unsigned e = 0; e < MAX_ENTITIES; ++e) if (gCoordinator.IsEntityAlive(e)) gCoordinator.DestroyEntity(e);
    for (auto& je : j["entities"])
    {
        auto e = gCoordinator.CreateEntity();
        if (je.contains("Transform"))
        {
            TransformComponent tr; auto t = je["Transform"]; auto p = t["pos"]; auto r=t["rot"]; auto s=t["scl"]; tr.position = {p[0],p[1],p[2]}; tr.rotation = {r[0],r[1],r[2]}; tr.scale={s[0],s[1],s[2]}; gCoordinator.AddComponent(e,tr);
        }
        if (je.contains("Renderable"))
        {
            RenderableComponent rc; rc.meshId = je["Renderable"]["meshId"]; rc.isVisible = je["Renderable"]["visible"]; gCoordinator.AddComponent(e,rc);
        }
        if (je.contains("Material"))
        {
            MaterialComponent mc; auto bc = je["Material"]["baseColor"]; mc.baseColor[0]=bc[0]; mc.baseColor[1]=bc[1]; mc.baseColor[2]=bc[2]; mc.metallic=je["Material"]["metallic"]; mc.roughness=je["Material"]["roughness"]; gCoordinator.AddComponent(e,mc);
        }
        if (je.contains("RigidBody"))
        {
            RigidBodyComponent rb; auto r=je["RigidBody"]; rb.type=(RigidBodyType)((int)r["type"]); rb.mass=r["mass"]; rb.friction=r["friction"]; rb.restitution=r["restitution"]; gCoordinator.AddComponent(e,rb);
        }
        if (je.contains("BoxCollider"))
        {
            BoxColliderComponent c; auto he=je["BoxCollider"]["halfExtents"]; c.halfExtents[0]=he[0]; c.halfExtents[1]=he[1]; c.halfExtents[2]=he[2]; gCoordinator.AddComponent(e,c);
        }
        if (je.contains("SphereCollider"))
        {
            SphereColliderComponent c; c.radius=je["SphereCollider"]["radius"]; gCoordinator.AddComponent(e,c);
        }
    }
    return true;
}
}


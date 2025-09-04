#include "EditorActions.hpp"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "World/MeshRegistry.hpp"
#include "components/RenderableComponent.hpp"
#include "components/TransformComponent.hpp"
#include "components/MaterialComponent.hpp"
#include "components/physics/RigidBodyComponent.hpp"
#include "components/physics/BoxColliderComponent.hpp"
#include "systems/RenderingSystem/RenderResources.hpp"
#include "utils/MeshUtils.hpp"
#include "EditorState.hpp"
#include "core/SystemManager.hpp"

static MeshData MakeUnitCube()
{
    // 12 triangles, positions only with normals/uv defaulted by UploadMesh
    const float v[] = {
        // front
        -0.5f,-0.5f, 0.5f,  0.5f,-0.5f, 0.5f,  0.5f, 0.5f, 0.5f,
        -0.5f,-0.5f, 0.5f,  0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
        // back
        -0.5f,-0.5f,-0.5f, -0.5f, 0.5f,-0.5f,  0.5f, 0.5f,-0.5f,
        -0.5f,-0.5f,-0.5f,  0.5f, 0.5f,-0.5f,  0.5f,-0.5f,-0.5f,
        // left
        -0.5f,-0.5f,-0.5f, -0.5f,-0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
        -0.5f,-0.5f,-0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f,-0.5f,
        // right
         0.5f,-0.5f,-0.5f,  0.5f, 0.5f,-0.5f,  0.5f, 0.5f, 0.5f,
         0.5f,-0.5f,-0.5f,  0.5f, 0.5f, 0.5f,  0.5f,-0.5f, 0.5f,
        // top
        -0.5f, 0.5f,-0.5f, -0.5f, 0.5f, 0.5f,  0.5f, 0.5f, 0.5f,
        -0.5f, 0.5f,-0.5f,  0.5f, 0.5f, 0.5f,  0.5f, 0.5f,-0.5f,
        // bottom
        -0.5f,-0.5f,-0.5f,  0.5f,-0.5f,-0.5f,  0.5f,-0.5f, 0.5f,
        -0.5f,-0.5f,-0.5f,  0.5f,-0.5f, 0.5f, -0.5f,-0.5f, 0.5f,
    };
    MeshData md; md.vertices.assign(v, v + sizeof(v)/sizeof(float));
    return md;
}

namespace EditorActions
{
static std::string toLowerExt(const std::string& p){ auto e = std::filesystem::path(p).extension().string(); for(char& c:const_cast<std::string&>(e)) c=(char)tolower((unsigned char)c); return e; }

uint32_t ImportMeshAndUpload(const std::string& path)
{
    MeshData md;
    std::string ext = toLowerExt(path);
    if (ext == ".gltf" || ext == ".glb") md = LoadGltfModel(path, true, 1.0f);
    else md = LoadMeshAny(path, true, 1.0f);
    if (md.vertices.empty()) return 0;
    uint32_t meshId = RegisterMesh(path);
    RenderResources::UploadMesh(meshId, md);
    return meshId;
}
int CreateEntityFromMeshPath(const std::string& path)
{
    uint32_t meshId = ImportMeshAndUpload(path);
    if (meshId == 0) return -1;

    auto e = gCoordinator.CreateEntity();
    TransformComponent tr; tr.position = {0.0f,0.0f,0.0f}; tr.scale = {1.0f,1.0f,1.0f};
    RenderableComponent rc; rc.meshId = meshId; rc.isVisible = true;
    MaterialComponent mc; mc.baseColor[0]=0.8f; mc.baseColor[1]=0.8f; mc.baseColor[2]=0.8f;
    gCoordinator.AddComponent(e, tr);
    gCoordinator.AddComponent(e, rc);
    gCoordinator.AddComponent(e, mc);
    EditorState::SetSelected((int)e);
    return (int)e;
}

int CreateDemoCubeEntity()
{
    MeshData md = MakeUnitCube();
    uint32_t meshId = RegisterMesh("AARTZE:CUBE");
    RenderResources::UploadMesh(meshId, md);
    auto e = gCoordinator.CreateEntity();
    TransformComponent tr; tr.position = {0.0f,0.0f,0.0f};
    RenderableComponent rc; rc.meshId = meshId; rc.isVisible = true;
    MaterialComponent mc; mc.baseColor[0]=0.9f; mc.baseColor[1]=0.4f; mc.baseColor[2]=0.2f; mc.roughness=0.6f;
    gCoordinator.AddComponent(e, tr);
    gCoordinator.AddComponent(e, rc);
    gCoordinator.AddComponent(e, mc);
    return (int)e;
}

void CreatePhysicsDemo()
{
    // Ground
    {
        auto e = gCoordinator.CreateEntity();
        TransformComponent tr; tr.position = {0.0f,-1.0f,0.0f}; tr.scale={20.0f,1.0f,20.0f};
        RenderableComponent rc; rc.meshId = RegisterMesh("AARTZE:GROUND");
        MaterialComponent mc; mc.baseColor[0]=0.2f; mc.baseColor[1]=0.8f; mc.baseColor[2]=0.2f; mc.roughness=1.0f;
        gCoordinator.AddComponent(e,tr); gCoordinator.AddComponent(e,rc); gCoordinator.AddComponent(e,mc);
        // physics
        RigidBodyComponent rb; rb.type=RigidBodyType::Static; rb.mass=0.0f; gCoordinator.AddComponent(e,rb);
        BoxColliderComponent box; box.halfExtents[0]=10.0f; box.halfExtents[1]=0.5f; box.halfExtents[2]=10.0f; gCoordinator.AddComponent(e,box);
        // upload a simple quad cube
        MeshData md = MakeUnitCube();
        RenderResources::UploadMesh(rc.meshId, md);
    }
    // Dynamic cube
    {
        auto e = gCoordinator.CreateEntity();
        TransformComponent tr; tr.position = {0.0f,3.0f,0.0f};
        RenderableComponent rc; rc.meshId = RegisterMesh("AARTZE:CUBE");
        MaterialComponent mc; mc.baseColor[0]=0.8f; mc.baseColor[1]=0.2f; mc.baseColor[2]=0.2f;
        gCoordinator.AddComponent(e,tr); gCoordinator.AddComponent(e,rc); gCoordinator.AddComponent(e,mc);
        RigidBodyComponent rb; rb.type=RigidBodyType::Dynamic; rb.mass=1.0f; gCoordinator.AddComponent(e,rb);
        BoxColliderComponent bc; bc.halfExtents[0]=0.5f; bc.halfExtents[1]=0.5f; bc.halfExtents[2]=0.5f; gCoordinator.AddComponent(e,bc);
        MeshData md = MakeUnitCube();
        RenderResources::UploadMesh(rc.meshId, md);
    }
}
}

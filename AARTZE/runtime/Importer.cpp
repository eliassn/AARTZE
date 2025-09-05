#include "Importer.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>

using namespace aartze;

static glm::vec3 to3(const aiVector3D& v){ return {v.x, v.y, v.z}; }
static glm::vec2 to2(const aiVector3D& v){ return {v.x, v.y}; }

bool Importer::Load(const std::string& path, ImportedModel& out, std::string& err){
    Assimp::Importer imp;
    const unsigned flags = aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_CalcTangentSpace |
                           aiProcess_JoinIdenticalVertices | aiProcess_ImproveCacheLocality |
                           aiProcess_SortByPType | aiProcess_RemoveRedundantMaterials |
                           aiProcess_OptimizeMeshes | aiProcess_FlipUVs;

    const aiScene* sc = imp.ReadFile(path, flags);
    if(!sc || !sc->mRootNode){ err = imp.GetErrorString(); return false; }

    out.materials.clear(); out.materials.reserve(sc->mNumMaterials);
    for(unsigned i=0;i<sc->mNumMaterials;i++){
        Material M; M.name = "Mat" + std::to_string(i);
        aiColor3D col(1,1,1);
        if(sc->mMaterials[i]->Get(AI_MATKEY_COLOR_DIFFUSE, col) == aiReturn_SUCCESS){ M.baseColor = {col.r,col.g,col.b}; }
        float rough=0.7f, metal=0.0f;
        sc->mMaterials[i]->Get(AI_MATKEY_ROUGHNESS_FACTOR, rough);
        sc->mMaterials[i]->Get(AI_MATKEY_METALLIC_FACTOR, metal);
        M.roughness = rough; M.metallic = metal;
        out.materials.push_back(M);
    }

    out.meshes.clear(); out.meshes.reserve(sc->mNumMeshes);
    for(unsigned mi=0; mi<sc->mNumMeshes; ++mi){
        const aiMesh* am = sc->mMeshes[mi];
        Mesh m; m.name = am->mName.C_Str();
        m.positions.resize(am->mNumVertices);
        if(am->HasNormals()) m.normals.resize(am->mNumVertices);
        if(am->HasTextureCoords(0)) m.uvs0.resize(am->mNumVertices);
        if(am->HasTangentsAndBitangents()) m.tangents.resize(am->mNumVertices);
        for(unsigned v=0; v<am->mNumVertices; ++v){
            m.positions[v] = to3(am->mVertices[v]);
            if(am->HasNormals())       m.normals[v]  = to3(am->mNormals[v]);
            if(am->HasTextureCoords(0))m.uvs0[v]     = to2(am->mTextureCoords[0][v]);
            if(am->HasTangentsAndBitangents()){
                glm::vec3 t = to3(am->mTangents[v]);
                glm::vec3 b = to3(am->mBitangents[v]);
                glm::vec3 n = am->HasNormals()? to3(am->mNormals[v]) : glm::vec3(0,0,1);
                float w = (glm::dot(glm::cross(n, t), b) < 0.0f) ? -1.0f : 1.0f;
                m.tangents[v] = { t, w };
            }
        }
        m.indices.reserve(am->mNumFaces*3);
        for(unsigned f=0; f<am->mNumFaces; ++f){ const aiFace& face = am->mFaces[f]; for(unsigned k=0;k<face.mNumIndices;k++) m.indices.push_back(face.mIndices[k]); }
        Submesh sm; sm.firstIndex=0; sm.indexCount=(uint32_t)m.indices.size(); sm.materialIndex=(int)am->mMaterialIndex; m.submeshes.push_back(sm);
        out.meshes.push_back(std::move(m));
    }
    (void)err; return true;
}


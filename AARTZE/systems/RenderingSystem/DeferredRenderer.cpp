#include "DeferredRenderer.hpp"
#include <cstring>
#include "RenderResources.hpp"
#include "core/Coordinator.hpp"
#include "components/RenderableComponent.hpp"
#include "components/TransformComponent.hpp"
#include "components/MaterialComponent.hpp"

static unsigned compile(unsigned type, const char* src){ unsigned s=glCreateShader(type); glShaderSource(s,1,&src,nullptr); glCompileShader(s); return s; }

bool DeferredRenderer::Initialize()
{
    int w=1280,h=720; m_gbuf.Create(w,h); m_fs.Create(); ensurePrograms(); return true;
}
void DeferredRenderer::Shutdown(){ if(m_geomProg) glDeleteProgram(m_geomProg); if(m_lightProg) glDeleteProgram(m_lightProg); m_fs.Destroy(); m_gbuf.Destroy(); }
void DeferredRenderer::Resize(int w,int h){ m_gbuf.Resize(w,h); }

void DeferredRenderer::ensurePrograms()
{
    if (m_geomProg) return;
    const char* vs = R"GLSL(
        #version 330 core
        layout(location=0) in vec3 aPos; layout(location=1) in vec3 aNormal; layout(location=2) in vec3 aColor; layout(location=3) in vec2 aUV;
        uniform mat4 uProj, uView, uModel; out vec3 vNrm; out vec3 vCol; out vec2 vUV; out vec3 vPos;
        void main(){ vec4 wp=uModel*vec4(aPos,1); vPos=wp.xyz; vNrm=mat3(uModel)*aNormal; vCol=aColor; vUV=aUV; gl_Position=uProj*uView*wp; }
    )GLSL";
    const char* fs = R"GLSL(
        #version 330 core
        layout(location=0) out vec4 oAlbedo; layout(location=1) out vec4 oNormal;
        in vec3 vNrm; in vec3 vCol; in vec2 vUV; in vec3 vPos; uniform vec3 uBaseColor; uniform float uMetallic; uniform float uRoughness;
        void main(){ oAlbedo = vec4(uBaseColor, 1.0); oNormal=vec4(normalize(vNrm), uRoughness); }
    )GLSL";
    unsigned v=compile(GL_VERTEX_SHADER,vs), f=compile(GL_FRAGMENT_SHADER,fs); m_geomProg=glCreateProgram(); glAttachShader(m_geomProg,v); glAttachShader(m_geomProg,f); glLinkProgram(m_geomProg); glDeleteShader(v); glDeleteShader(f);

    const char* qvs = R"GLSL(#version 330 core
        layout(location=0) in vec2 aPos; out vec2 uv; void main(){ uv = aPos*0.5+0.5; gl_Position=vec4(aPos,0,1);} )GLSL";
    const char* lfs = R"GLSL(
        #version 330 core
        in vec2 uv; out vec4 FragColor; uniform sampler2D gAlbedo; uniform sampler2D gNormal; uniform vec3 uCamPos;
        const vec3 Ldir = normalize(vec3(-0.3,-1.0,-0.2));
        void main(){ vec3 base = texture(gAlbedo, uv).rgb; vec4 nrmr = texture(gNormal, uv); vec3 N = normalize(nrmr.xyz); float rough = nrmr.w; vec3 V = normalize(uCamPos); float NdotL=max(dot(N,-Ldir),0.0); vec3 color = base*(0.2 + 0.8*NdotL); FragColor = vec4(pow(color, vec3(1.0/2.2)),1.0); }
    )GLSL";
    unsigned vv=compile(GL_VERTEX_SHADER,qvs), lf=compile(GL_FRAGMENT_SHADER,lfs); m_lightProg=glCreateProgram(); glAttachShader(m_lightProg,vv); glAttachShader(m_lightProg,lf); glLinkProgram(m_lightProg); glDeleteShader(vv); glDeleteShader(lf);
}

void DeferredRenderer::GeometryPass(const float* Proj, const float* View)
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_gbuf.fbo);
    glViewport(0,0,m_gbuf.width,m_gbuf.height);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(m_geomProg);
    int locP=glGetUniformLocation(m_geomProg,"uProj"), locV=glGetUniformLocation(m_geomProg,"uView"), locM=glGetUniformLocation(m_geomProg,"uModel");
    int locBC=glGetUniformLocation(m_geomProg,"uBaseColor"), locMet=glGetUniformLocation(m_geomProg,"uMetallic"), locR=glGetUniformLocation(m_geomProg,"uRoughness");
    glUniformMatrix4fv(locP,1,GL_FALSE,Proj); glUniformMatrix4fv(locV,1,GL_FALSE,View);

    auto entities = gCoordinator.GetEntitiesWithComponents<RenderableComponent, TransformComponent>();
    for (auto e : entities)
    {
        auto& tr = gCoordinator.GetComponent<TransformComponent>(e);
        float Tm[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, tr.position[0],tr.position[1],tr.position[2],1};
        glUniformMatrix4fv(locM,1,GL_FALSE,Tm);
        float bc[3]={0.8f,0.8f,0.8f}; float met=0.0f, rough=0.8f;
        if (gCoordinator.HasComponent<MaterialComponent>(e)) { auto& m = gCoordinator.GetComponent<MaterialComponent>(e); bc[0]=m.baseColor[0]; bc[1]=m.baseColor[1]; bc[2]=m.baseColor[2]; met=m.metallic; rough=m.roughness; }
        glUniform3f(locBC,bc[0],bc[1],bc[2]); glUniform1f(locMet,met); glUniform1f(locR,rough);
        auto& rc = gCoordinator.GetComponent<RenderableComponent>(e);
        if (auto gpu = RenderResources::GetMesh(rc.meshId)) { glBindVertexArray(gpu->vao); glDrawArrays(GL_TRIANGLES,0,gpu->vertexCount); }
    }
    glBindFramebuffer(GL_FRAMEBUFFER,0);
}

void DeferredRenderer::LightingPass(const float* Proj, const float* View, const float* CamPos)
{
    (void)Proj; (void)View;
    glDisable(GL_DEPTH_TEST);
    glUseProgram(m_lightProg);
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, m_gbuf.texAlbedoMR); glUniform1i(glGetUniformLocation(m_lightProg,"gAlbedo"),0);
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, m_gbuf.texNormal);   glUniform1i(glGetUniformLocation(m_lightProg,"gNormal"),1);
    glUniform3f(glGetUniformLocation(m_lightProg,"uCamPos"), CamPos[0],CamPos[1],CamPos[2]);
    m_fs.Draw();
}


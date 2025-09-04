#include "RenderingSystem.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <string>
#include <vector>

#include "core/Coordinator.hpp"
#include "components/RenderableComponent.hpp"
#include "components/TransformComponent.hpp"
#include "components/MaterialComponent.hpp"
#include "RenderResources.hpp"
#include "DeferredRenderer.hpp"
#include "../core/Coordinator.hpp"
#include "../components/TransformComponent.hpp"
#include "../editor/EditorState.hpp"
#include <GLFW/glfw3.h>

static unsigned compileShader(unsigned type, const char* src)
{
    unsigned s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    int ok = 0; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if(!ok){ char log[1024]; glGetShaderInfoLog(s, 1024, nullptr, log);} // silent
    return s;
}

bool RenderingSystem::Initialize(GLFWwindow* window)
{
    m_window = window;
    ensureProgram();
    m_deferred = new DeferredRenderer();
    m_deferred->Initialize();
    return true;
}

void RenderingSystem::Shutdown()
{
    if (m_program) glDeleteProgram(m_program);
    RenderResources::Clear();
    if (m_deferred){ m_deferred->Shutdown(); delete m_deferred; m_deferred=nullptr; }
    if (m_lineProg) glDeleteProgram(m_lineProg);
    if (m_gridVbo) glDeleteBuffers(1,&m_gridVbo);
    if (m_gridVao) glDeleteVertexArrays(1,&m_gridVao);
}

void RenderingSystem::ensureProgram()
{
    if (m_program) return;
    // Minimal PBR-ish shader (no env maps)
    const char* vs = R"GLSL(
        #version 330 core
        layout(location=0) in vec3 aPos;
        layout(location=1) in vec3 aNormal;
        layout(location=2) in vec3 aColor;
        layout(location=3) in vec2 aUV;
        uniform mat4 uProj;
        uniform mat4 uView;
        uniform mat4 uModel;
        out vec3 vPos;
        out vec3 vNrm;
        out vec3 vCol;
        void main(){
            vec4 wp = uModel * vec4(aPos,1.0);
            vPos = wp.xyz; vNrm = mat3(uModel) * aNormal; vCol = aColor;
            gl_Position = uProj * uView * wp;
        }
    )GLSL";
    const char* fs = R"GLSL(
        #version 330 core
        in vec3 vPos; in vec3 vNrm; in vec3 vCol;
        out vec4 FragColor;
        uniform vec3 uCamPos;
        uniform vec3 uBaseColor;
        uniform float uMetallic;
        uniform float uRoughness;
        // single directional light
        const vec3 Ldir = normalize(vec3(-0.3, -1.0, -0.2));
        void main(){
            vec3 N = normalize(vNrm);
            vec3 V = normalize(uCamPos - vPos);
            vec3 L = -Ldir;
            vec3 H = normalize(V+L);
            float NdotL = max(dot(N,L),0.0);
            float NdotV = max(dot(N,V),0.0);
            float NdotH = max(dot(N,H),0.0);
            float VdotH = max(dot(V,H),0.0);
            // cheap GGX term
            float a = max(0.04, uRoughness*uRoughness);
            float a2 = a*a;
            float denom = (NdotH*NdotH*(a2-1.0)+1.0);
            float D = a2/(3.14159*denom*denom + 1e-4);
            float k = (uRoughness+1.0);
            k = (k*k)/8.0;
            float Gv = NdotV/(NdotV*(1.0-k)+k);
            float Gl = NdotL/(NdotL*(1.0-k)+k);
            float G = Gv*Gl;
            vec3 F0 = mix(vec3(0.04), uBaseColor, uMetallic);
            vec3 F = F0 + (1.0-F0)*pow(1.0-VdotH,5.0);
            vec3 spec = (D*G*F) / max(4.0*NdotV*NdotL, 1e-4);
            vec3 kd = (1.0 - F)*(1.0 - uMetallic);
            vec3 diffuse = kd * uBaseColor / 3.14159;
            vec3 color = (diffuse + spec) * NdotL;
            color = pow(color, vec3(1.0/2.2));
            FragColor = vec4(color, 1.0);
        }
    )GLSL";
    unsigned v = compileShader(GL_VERTEX_SHADER, vs);
    unsigned f = compileShader(GL_FRAGMENT_SHADER, fs);
    m_program = glCreateProgram();
    glAttachShader(m_program, v); glAttachShader(m_program, f);
    glLinkProgram(m_program);
    glDeleteShader(v); glDeleteShader(f);
}

static void perspective(float out[16], float fovyRad, float aspect, float znear, float zfar)
{
    float f = 1.0f / tanf(fovyRad * 0.5f);
    out[0]=f/aspect; out[1]=0; out[2]=0; out[3]=0;
    out[4]=0; out[5]=f; out[6]=0; out[7]=0;
    out[8]=0; out[9]=0; out[10]=(zfar+znear)/(znear-zfar); out[11]=-1;
    out[12]=0; out[13]=0; out[14]=(2*zfar*znear)/(znear-zfar); out[15]=0;
}

static void identity(float m[16]){ for(int i=0;i<16;++i) m[i] = (i%5==0)?1.0f:0.0f; }

static void mul(float r[16], const float a[16], const float b[16])
{
    float o[16];
    for(int i=0;i<4;++i) for(int j=0;j<4;++j){ o[i*4+j]=0; for(int k=0;k<4;++k) o[i*4+j]+=a[i*4+k]*b[k*4+j]; }
    std::memcpy(r,o,sizeof(o));
}

static void translate(float m[16], float x,float y,float z){ identity(m); m[12]=x; m[13]=y; m[14]=z; }
static void rotateY(float m[16], float degrees){ identity(m); float r=degrees*3.1415926f/180.f; m[0]=cos(r); m[2]=sin(r); m[8]=-sin(r); m[10]=cos(r);} 
static void rotateX(float m[16], float degrees){ identity(m); float r=degrees*3.1415926f/180.f; m[5]=cos(r); m[6]=-sin(r); m[9]=sin(r); m[10]=cos(r);} 
static void rotateZ(float m[16], float degrees){ identity(m); float r=degrees*3.1415926f/180.f; m[0]=cos(r); m[1]=-sin(r); m[4]=sin(r); m[5]=cos(r);} 
static void scaleM(float m[16], float x,float y,float z){ identity(m); m[0]=x; m[5]=y; m[10]=z; }

void RenderingSystem::GetView(float out[16]) const
{
    float cy = cosf(m_camYaw*3.1415926f/180.f), sy = sinf(m_camYaw*3.1415926f/180.f);
    float cp = cosf(m_camPitch*3.1415926f/180.f), sp = sinf(m_camPitch*3.1415926f/180.f);
    float fx = cy*cp, fy = sp, fz = sy*cp;
    float eye[3] = { m_camPos[0], m_camPos[1], m_camPos[2] };
    float center[3] = { eye[0]+fx, eye[1]+fy, eye[2]+fz };
    float F[3] = { center[0]-eye[0], center[1]-eye[1], center[2]-eye[2] };
    float fLen = sqrtf(F[0]*F[0]+F[1]*F[1]+F[2]*F[2]); F[0]/=fLen; F[1]/=fLen; F[2]/=fLen;
    float up[3] = {0,1,0};
    float S[3] = { F[1]*up[2]-F[2]*up[1], F[2]*up[0]-F[0]*up[2], F[0]*up[1]-F[1]*up[0] };
    float sLen = sqrtf(S[0]*S[0]+S[1]*S[1]+S[2]*S[2]); S[0]/=sLen; S[1]/=sLen; S[2]/=sLen;
    float U[3] = { S[1]*F[2]-S[2]*F[1], S[2]*F[0]-S[0]*F[2], S[0]*F[1]-S[1]*F[0] };
    float V[16] = { S[0], U[0], -F[0], 0,
                    S[1], U[1], -F[1], 0,
                    S[2], U[2], -F[2], 0,
                    0,    0,    0,     1 };
    float T[16]; translate(T, -eye[0], -eye[1], -eye[2]); mul(out, V, T);
}

void RenderingSystem::GetProj(float out[16], int w, int h) const
{
    perspective(out, 45.0f*3.1415926f/180.f, (float)w/(float)h, 0.1f, 100.0f);
}

// Helper forward for static function used below
static void projectPoint(const float* M, const float* V, const float* P, const float p[3], int w,int h, float out2[2]);

void RenderingSystem::Render()
{
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Camera controls (WASD + RMB look)
    // Respect ImGui input capture so UI remains clickable.
    if (m_window)
    {
        if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
        {
            static double lastX = -1.0, lastY = -1.0; double x,y; glfwGetCursorPos(m_window,&x,&y);
            if(lastX>=0){ double dx=x-lastX, dy=y-lastY; m_camYaw += (float)dx*0.1f; m_camPitch -= (float)dy*0.1f; if(m_camPitch>89) m_camPitch=89; if(m_camPitch<-89) m_camPitch=-89; }
            lastX=x; lastY=y;
            m_mouseLook = true;
        }
        else { m_mouseLook=false; }
        float yawR = m_camYaw*3.1415926f/180.f; float pitchR=m_camPitch*3.1415926f/180.f;
        float fx = cosf(yawR)*cosf(pitchR), fy = sinf(pitchR), fz = sinf(yawR)*cosf(pitchR);
        float rightX = cosf(yawR-3.1415926f/2), rightZ = sinf(yawR-3.1415926f/2);
        float speed = m_camSpeed/60.0f; // approx per-frame speed
        if (glfwGetKey(m_window, GLFW_KEY_W)==GLFW_PRESS){ m_camPos[0]+=fx*speed; m_camPos[1]+=fy*speed; m_camPos[2]+=fz*speed; }
        if (glfwGetKey(m_window, GLFW_KEY_S)==GLFW_PRESS){ m_camPos[0]-=fx*speed; m_camPos[1]-=fy*speed; m_camPos[2]-=fz*speed; }
        if (glfwGetKey(m_window, GLFW_KEY_A)==GLFW_PRESS){ m_camPos[0]-=rightX*speed; m_camPos[2]-=rightZ*speed; }
        if (glfwGetKey(m_window, GLFW_KEY_D)==GLFW_PRESS){ m_camPos[0]+=rightX*speed; m_camPos[2]+=rightZ*speed; }
    }

    // Build view matrix (look from yaw/pitch)
    float cy = cosf(m_camYaw*3.1415926f/180.f), sy = sinf(m_camYaw*3.1415926f/180.f);
    float cp = cosf(m_camPitch*3.1415926f/180.f), sp = sinf(m_camPitch*3.1415926f/180.f);
    float fx = cy*cp, fy = sp, fz = sy*cp;
    float eye[3] = { m_camPos[0], m_camPos[1], m_camPos[2] };
    float center[3] = { eye[0]+fx, eye[1]+fy, eye[2]+fz };
    // lookAt
    float F[3] = { center[0]-eye[0], center[1]-eye[1], center[2]-eye[2] };
    float fLen = sqrtf(F[0]*F[0]+F[1]*F[1]+F[2]*F[2]); F[0]/=fLen; F[1]/=fLen; F[2]/=fLen;
    float up[3] = {0,1,0};
    float S[3] = { F[1]*up[2]-F[2]*up[1], F[2]*up[0]-F[0]*up[2], F[0]*up[1]-F[1]*up[0] };
    float sLen = sqrtf(S[0]*S[0]+S[1]*S[1]+S[2]*S[2]); S[0]/=sLen; S[1]/=sLen; S[2]/=sLen;
    float U[3] = { S[1]*F[2]-S[2]*F[1], S[2]*F[0]-S[0]*F[2], S[0]*F[1]-S[1]*F[0] };
    float V[16] = { S[0], U[0], -F[0], 0,
                    S[1], U[1], -F[1], 0,
                    S[2], U[2], -F[2], 0,
                    0,    0,    0,     1 };
    float T[16]; translate(T, -eye[0], -eye[1], -eye[2]); float View[16]; mul(View, V, T);

    int w=1280,h=720; if(m_window){ glfwGetFramebufferSize(m_window,&w,&h);} float Proj[16]; perspective(Proj, 45.0f*3.1415926f/180.f, (float)w/(float)h, 0.1f, 100.0f);
    if (m_deferred) m_deferred->Resize(w,h);

    extern bool gUseDeferred; extern bool gEnableSSAO; extern bool gEnableShadows; extern bool gEnableSSR;
    if (gUseDeferred && m_deferred)
    {
        m_deferred->GeometryPass(Proj, View);
        m_deferred->LightingPass(Proj, View, m_camPos);
    }
    else
    {
        if (!m_program) ensureProgram();
        glUseProgram(m_program);
        int locProj = glGetUniformLocation(m_program, "uProj");
        int locView = glGetUniformLocation(m_program, "uView");
        int locModel= glGetUniformLocation(m_program, "uModel");
        int locBC  = glGetUniformLocation(m_program, "uBaseColor");
        int locMet = glGetUniformLocation(m_program, "uMetallic");
        int locRgh = glGetUniformLocation(m_program, "uRoughness");
        int locCam = glGetUniformLocation(m_program, "uCamPos");
        glUniformMatrix4fv(locProj,1,GL_FALSE,Proj); glUniformMatrix4fv(locView,1,GL_FALSE,View);
    glUniform3f(locCam, m_camPos[0], m_camPos[1], m_camPos[2]);

        auto entities = gCoordinator.GetEntitiesWithComponents<RenderableComponent, TransformComponent>();
        for (auto e : entities)
        {
            auto& tr = gCoordinator.GetComponent<TransformComponent>(e);
            float Tm[16], Rx[16], Ry[16], Rz[16], S[16], Rxy[16], Rxyz[16], M[16], TR[16];
            translate(Tm, tr.position[0], tr.position[1], tr.position[2]);
            rotateX(Rx, tr.rotation[0]); rotateY(Ry, tr.rotation[1]); rotateZ(Rz, tr.rotation[2]);
            mul(Rxy, Ry, Rx); mul(Rxyz, Rxy, Rz); scaleM(S, tr.scale[0], tr.scale[1], tr.scale[2]); mul(TR, Tm, Rxyz); mul(M, TR, S);
            glUniformMatrix4fv(locModel,1,GL_FALSE,M);

            auto& rend = gCoordinator.GetComponent<RenderableComponent>(e);
            float baseColor[3] = {0.8f,0.8f,0.8f}; float metallic=0.0f; float rough=0.8f;
            if (gCoordinator.HasComponent<MaterialComponent>(e))
            {
                auto& mat = gCoordinator.GetComponent<MaterialComponent>(e);
                baseColor[0]=mat.baseColor[0]; baseColor[1]=mat.baseColor[1]; baseColor[2]=mat.baseColor[2]; metallic=mat.metallic; rough=mat.roughness;
            }
            glUniform3f(locBC, baseColor[0], baseColor[1], baseColor[2]);
            glUniform1f(locMet, metallic); glUniform1f(locRgh, rough);

            if (const MeshGPU* gpu = RenderResources::GetMesh(rend.meshId))
            {
                glBindVertexArray(gpu->vao);
                glDrawArrays(GL_TRIANGLES, 0, gpu->vertexCount);
            }
        }
    glBindVertexArray(0);
    drawGridAndAxes();
}
}

void RenderingSystem::ensureLineProgram()
{
    if (m_lineProg) return;
    const char* vs = R"GLSL(
        #version 330 core
        layout(location=0) in vec3 aPos;
        uniform mat4 uProj; uniform mat4 uView; uniform mat4 uModel;
        void main(){ gl_Position = uProj * uView * uModel * vec4(aPos,1.0); }
    )GLSL";
    const char* fs = R"GLSL(
        #version 330 core
        uniform vec3 uColor; out vec4 FragColor;
        void main(){ FragColor = vec4(uColor,1.0); }
    )GLSL";
    unsigned v = compileShader(GL_VERTEX_SHADER, vs);
    unsigned f = compileShader(GL_FRAGMENT_SHADER, fs);
    m_lineProg = glCreateProgram(); glAttachShader(m_lineProg, v); glAttachShader(m_lineProg, f); glLinkProgram(m_lineProg);
    glDeleteShader(v); glDeleteShader(f);
    glGenVertexArrays(1,&m_gridVao); glGenBuffers(1,&m_gridVbo);
}

void RenderingSystem::drawGridAndAxes()
{
    ensureLineProgram();
    int w=1280,h=720; if(m_window){ glfwGetFramebufferSize(m_window,&w,&h);} float Proj[16]; GetProj(Proj,w,h); float View[16]; GetView(View);
    glUseProgram(m_lineProg);
    glUniformMatrix4fv(glGetUniformLocation(m_lineProg,"uProj"),1,GL_FALSE,Proj);
    glUniformMatrix4fv(glGetUniformLocation(m_lineProg,"uView"),1,GL_FALSE,View);
    float M[16]; identity(M); glUniformMatrix4fv(glGetUniformLocation(m_lineProg,"uModel"),1,GL_FALSE,M);
    glBindVertexArray(m_gridVao); glBindBuffer(GL_ARRAY_BUFFER,m_gridVbo);

    // Build grid vertices
    std::vector<float> verts; verts.reserve((41*4+6)*3);
    float range=50.0f;
    for (int i=-50;i<=50;++i){
        verts.push_back((float)i); verts.push_back(0.0f); verts.push_back(-range);
        verts.push_back((float)i); verts.push_back(0.0f); verts.push_back( range);
        verts.push_back(-range); verts.push_back(0.0f); verts.push_back((float)i);
        verts.push_back( range); verts.push_back(0.0f); verts.push_back((float)i);
    }
    // Upload and draw grid
    glBufferData(GL_ARRAY_BUFFER, verts.size()*sizeof(float), verts.data(), GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0); glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
    glUniform3f(glGetUniformLocation(m_lineProg,"uColor"), 0.25f,0.25f,0.25f);
    glDrawArrays(GL_LINES,0,(GLsizei)(verts.size()/3));

    // Axes
    float axes[] = { 0,0,0, 1.5f,0,0,  0,0,0, 0,1.5f,0,  0,0,0, 0,0,1.5f };
    glBufferData(GL_ARRAY_BUFFER, sizeof(axes), axes, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);
    glUniform3f(glGetUniformLocation(m_lineProg,"uColor"), 1,0,0); glDrawArrays(GL_LINES,0,2);
    glUniform3f(glGetUniformLocation(m_lineProg,"uColor"), 0,1,0); glDrawArrays(GL_LINES,2,2);
    glUniform3f(glGetUniformLocation(m_lineProg,"uColor"), 0,0,1); glDrawArrays(GL_LINES,4,2);
    glBindVertexArray(0);
}

static void projectPoint(const float* M, const float* V, const float* P, const float p[3], int w,int h, float out2[2])
{
    float mvp[16]; float mv[16]; mul(mv,V,M); mul(mvp,P,mv);
    float v[4] = { p[0],p[1],p[2],1.0f };
    float clip[4] = {0};
    for(int i=0;i<4;++i){ clip[i]=mvp[i*4+0]*v[0]+mvp[i*4+1]*v[1]+mvp[i*4+2]*v[2]+mvp[i*4+3]*v[3]; }
    if (clip[3]==0) clip[3]=1;
    float ndcX = clip[0]/clip[3]; float ndcY = clip[1]/clip[3];
    out2[0] = (ndcX*0.5f+0.5f)*w; out2[1] = (1.0f-(ndcY*0.5f+0.5f))*h;
}

void RenderingSystem::drawGizmo() { /* disabled */ }




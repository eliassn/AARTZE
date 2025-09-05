#include "Renderer.h"

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <string>

using namespace aartze;

static unsigned makeShader(GLenum type, const char* src){
    unsigned s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    return s;
}
static unsigned makeProgram(const char* vs, const char* fs){
    unsigned v = makeShader(GL_VERTEX_SHADER, vs);
    unsigned f = makeShader(GL_FRAGMENT_SHADER, fs);
    unsigned p = glCreateProgram();
    glAttachShader(p, v); glAttachShader(p, f); glLinkProgram(p);
    glDeleteShader(v); glDeleteShader(f);
    return p;
}

void Renderer::initGL(){
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    const char* vs = R"(
        #version 330 core
        layout(location=0) in vec3 aPos;
        uniform mat4 uMVP;
        void main(){ gl_Position = uMVP * vec4(aPos,1.0); }
    )";
    const char* fs = R"(
        #version 330 core
        uniform vec4 uColor;
        out vec4 FragColor;
        void main(){ FragColor = uColor; }
    )";
    m_progColor = makeProgram(vs, fs);
    m_uMVP = glGetUniformLocation(m_progColor, "uMVP");

    // Simple lit program (Lambert-ish)
    const char* vsl = R"(
        #version 330 core
        layout(location=0) in vec3 aPos;
        layout(location=1) in vec3 aNrm;
        layout(location=2) in vec2 aUV;
        uniform mat4 uM; uniform mat4 uV; uniform mat4 uP; uniform mat3 uN;
        out vec3 vN; out vec3 vP;
        void main(){
            vec4 wp = uM * vec4(aPos,1.0);
            vP = wp.xyz; vN = normalize(uN * aNrm);
            gl_Position = uP * uV * wp;
        }
    )";
    const char* fsl = R"(
        #version 330 core
        in vec3 vN; in vec3 vP; out vec4 FragColor;
        uniform vec4 uColor;
        void main(){
            vec3 L = normalize(vec3(0.4,0.8,0.6));
            float ndl = max(dot(normalize(vN), L), 0.0);
            vec3 col = uColor.rgb * (0.2 + 0.8*ndl);
            FragColor = vec4(col, uColor.a);
        }
    )";
    m_progLit = makeProgram(vsl, fsl);
    m_uM = glGetUniformLocation(m_progLit, "uM");
    m_uV = glGetUniformLocation(m_progLit, "uV");
    m_uP = glGetUniformLocation(m_progLit, "uP");
    m_uN = glGetUniformLocation(m_progLit, "uN");
    m_uLitColor = glGetUniformLocation(m_progLit, "uColor");

    // Grid VBO (unit, positions will be scaled in MVP)
    glGenVertexArrays(1, &m_vaoGrid);
    glGenBuffers(1, &m_vboGrid);
    glBindVertexArray(m_vaoGrid);
    glBindBuffer(GL_ARRAY_BUFFER, m_vboGrid);
    glBufferData(GL_ARRAY_BUFFER, 1024, nullptr, GL_DYNAMIC_DRAW); // resized in drawGrid
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // Cube VBO (unit cube)
    static const float cube[] = {
        -0.5f,-0.5f,-0.5f,  0.5f,-0.5f,-0.5f,  0.5f, 0.5f,-0.5f,
        -0.5f,-0.5f,-0.5f,  0.5f, 0.5f,-0.5f, -0.5f, 0.5f,-0.5f,
        -0.5f,-0.5f, 0.5f,  0.5f,-0.5f, 0.5f,  0.5f, 0.5f, 0.5f,
        -0.5f,-0.5f, 0.5f,  0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
        -0.5f,-0.5f,-0.5f, -0.5f,-0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
        -0.5f,-0.5f,-0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f,-0.5f,
         0.5f,-0.5f,-0.5f,  0.5f, 0.5f,-0.5f,  0.5f, 0.5f, 0.5f,
         0.5f,-0.5f,-0.5f,  0.5f, 0.5f, 0.5f,  0.5f,-0.5f, 0.5f,
        -0.5f, 0.5f,-0.5f, -0.5f, 0.5f, 0.5f,  0.5f, 0.5f, 0.5f,
        -0.5f, 0.5f,-0.5f,  0.5f, 0.5f, 0.5f,  0.5f, 0.5f,-0.5f,
        -0.5f,-0.5f,-0.5f,  0.5f,-0.5f,-0.5f,  0.5f,-0.5f, 0.5f,
        -0.5f,-0.5f,-0.5f,  0.5f,-0.5f, 0.5f, -0.5f,-0.5f, 0.5f,
    };
    glGenVertexArrays(1, &m_vaoCube);
    glGenBuffers(1, &m_vboCube);
    glBindVertexArray(m_vaoCube);
    glBindBuffer(GL_ARRAY_BUFFER, m_vboCube);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // Default camera
    setPerspective(50.f, 16.f/9.f, 0.1f, 200.f);
    setViewLookAt({4,3,6}, {0,0,0}, {0,1,0});
}

void Renderer::setViewport(int x, int y, int w, int h){
    glViewport(x,y,w,h);
}
void Renderer::setPerspective(float fovY_deg, float aspect, float znear, float zfar){
    m_proj = glm::perspective(glm::radians(fovY_deg), aspect, znear, zfar);
}
void Renderer::setViewLookAt(const glm::vec3& eye, const glm::vec3& at, const glm::vec3& up){
    m_view = glm::lookAt(eye, at, up);
}
void Renderer::getViewMatrix(float out16[16]) const{ memcpy(out16, glm::value_ptr(m_view), sizeof(float)*16); }
void Renderer::getProjMatrix(float out16[16]) const{ memcpy(out16, glm::value_ptr(m_proj), sizeof(float)*16); }

void Renderer::drawGrid(float extent, float step){
    std::vector<float> v; v.reserve( (size_t)((extent/step)*8) );
    for(float x=-extent; x<=extent; x+=step){ v.push_back(x); v.push_back(0); v.push_back(-extent); v.push_back(x); v.push_back(0); v.push_back(extent); }
    for(float z=-extent; z<=extent; z+=step){ v.push_back(-extent); v.push_back(0); v.push_back(z); v.push_back(extent); v.push_back(0); v.push_back(z); }
    glBindBuffer(GL_ARRAY_BUFFER, m_vboGrid);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(v.size()*sizeof(float)), v.data(), GL_DYNAMIC_DRAW);

    glUseProgram(m_progColor);
    glUniform4f(glGetUniformLocation(m_progColor, "uColor"), 0.6f,0.6f,0.65f,1.0f);
    glm::mat4 mvp = m_proj * m_view;
    glUniformMatrix4fv(m_uMVP, 1, GL_FALSE, glm::value_ptr(mvp));
    glBindVertexArray(m_vaoGrid);
    glDrawArrays(GL_LINES, 0, (GLsizei)(v.size()/3));
    glBindVertexArray(0);
}

void Renderer::drawCube(const float model[16], uint32_t rgba){
    float r = ((rgba>>24)&0xFF)/255.f;
    float g = ((rgba>>16)&0xFF)/255.f;
    float b = ((rgba>>8) &0xFF)/255.f;
    float a = ((rgba)     &0xFF)/255.f;
    glUseProgram(m_progColor);
    glUniform4f(glGetUniformLocation(m_progColor, "uColor"), r,g,b,a);
    glm::mat4 M = glm::make_mat4(model);
    glm::mat4 mvp = m_proj * m_view * M;
    glUniformMatrix4fv(m_uMVP, 1, GL_FALSE, glm::value_ptr(mvp));
    glBindVertexArray(m_vaoCube);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

void Renderer::useLitShader(){ glUseProgram(m_progLit); }
void Renderer::setVP(const glm::mat4& V, const glm::mat4& P){
    glUseProgram(m_progLit);
    glUniformMatrix4fv(m_uV, 1, GL_FALSE, glm::value_ptr(V));
    glUniformMatrix4fv(m_uP, 1, GL_FALSE, glm::value_ptr(P));
}
void Renderer::setModelColor(const glm::mat4& M, const glm::vec4& color){
    glUseProgram(m_progLit);
    glUniformMatrix4fv(m_uM, 1, GL_FALSE, glm::value_ptr(M));
    glm::mat3 N = glm::mat3(glm::transpose(glm::inverse(M)));
    glUniformMatrix3fv(m_uN, 1, GL_FALSE, glm::value_ptr(N));
    glUniform4fv(m_uLitColor, 1, glm::value_ptr(color));
}
void Renderer::draw(GLuint vao, GLsizei indexCount){
    if(!vao || !indexCount) return;
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, (void*)0);
    glBindVertexArray(0);
}

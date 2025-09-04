#include "RendererGL.hpp"
#include <glad/glad.h>
#include <vector>

namespace ui2 {

static unsigned CompileShader(unsigned type, const char* src)
{
    unsigned s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    return s;
}

void RendererGL::init()
{
    if (m_prog) return;
    const char* vs = R"GLSL(
        #version 330 core
        layout(location=0) in vec2 aPos;
        layout(location=1) in vec4 aColor;
        uniform mat4 uProj;
        out vec4 vColor;
        void main(){ vColor=aColor; gl_Position = uProj * vec4(aPos,0,1); }
    )GLSL";
    const char* fs = R"GLSL(
        #version 330 core
        in vec4 vColor; out vec4 FragColor;
        void main(){ FragColor = vColor; }
    )GLSL";
    unsigned v = CompileShader(GL_VERTEX_SHADER, vs);
    unsigned f = CompileShader(GL_FRAGMENT_SHADER, fs);
    m_prog = glCreateProgram(); glAttachShader(m_prog,v); glAttachShader(m_prog,f); glLinkProgram(m_prog);
    glDeleteShader(v); glDeleteShader(f);
    m_locProj = glGetUniformLocation(m_prog, "uProj");
    glGenVertexArrays(1,&m_vao);
    glGenBuffers(1,&m_vbo);
}

void RendererGL::shutdown()
{
    if (m_vbo) glDeleteBuffers(1,&m_vbo); m_vbo=0;
    if (m_vao) glDeleteVertexArrays(1,&m_vao); m_vao=0;
    if (m_prog){ glDeleteProgram(m_prog); m_prog=0; }
}

struct Vtx { float x,y; float r,g,b,a; };

void RendererGL::render(const DrawList& list, int fbWidth, int fbHeight)
{
    if (!m_prog) init();
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    std::vector<Vtx> verts; verts.reserve(list.cmds.size()*6);
    auto pushRect = [&](float x,float y,float w,float h, unsigned rgba){
        float r = ((rgba)     & 0xFF) / 255.0f;
        float g = ((rgba>>8)  & 0xFF) / 255.0f;
        float b = ((rgba>>16) & 0xFF) / 255.0f;
        float a = ((rgba>>24) & 0xFF) / 255.0f;
        Vtx v0{ x,     y,      r,g,b,a}; Vtx v1{ x+w,   y,      r,g,b,a};
        Vtx v2{ x+w,   y+h,    r,g,b,a}; Vtx v3{ x,     y+h,    r,g,b,a};
        verts.push_back(v0); verts.push_back(v1); verts.push_back(v2);
        verts.push_back(v0); verts.push_back(v2); verts.push_back(v3);
    };
    for (auto& c : list.cmds)
        if (c.type == DrawList::Cmd::RectFilled) pushRect(c.r.x, c.r.y, c.r.w, c.r.h, c.rgba);
    if (verts.empty()) return;

    float L=0, R= (float)fbWidth, T=0, B=(float)fbHeight;
    float proj[16] = { 2/(R-L),0,0,0, 0,2/(T-B),0,0, 0,0,-1,0, -(R+L)/(R-L),-(T+B)/(T-B),0,1 };
    glUseProgram(m_prog); glUniformMatrix4fv(m_locProj,1,GL_FALSE,proj);
    glBindVertexArray(m_vao); glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    std::size_t bytes = verts.size()*sizeof(Vtx);
    if (bytes > m_capacity) { glBufferData(GL_ARRAY_BUFFER, bytes, verts.data(), GL_DYNAMIC_DRAW); m_capacity=bytes; }
    else glBufferSubData(GL_ARRAY_BUFFER, 0, bytes, verts.data());
    glEnableVertexAttribArray(0); glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,sizeof(Vtx),(void*)0);
    glEnableVertexAttribArray(1); glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,sizeof(Vtx),(void*)(2*sizeof(float)));
    glDrawArrays(GL_TRIANGLES, 0, (GLint)verts.size());
    glBindVertexArray(0); glUseProgram(0);
}

}
#include "PyBridge.h"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#ifdef _WIN32
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <windows.h>
#  undef min
#  undef max
#endif
#if defined(_WIN32)
#  include <GL/gl.h>
#else
#  include <GL/gl.h>
#endif
#include <unordered_map>
#include <string>
#include <cmath>
#include <array>
#include <tuple>
#include <algorithm>

namespace py = pybind11;

namespace aartze::scripting {

struct Entity {
  int id{}; std::string name; bool visible{true}; bool locked{false};
  float tx{0},ty{0},tz{0}; float rx{0},ry{0},rz{0}; float sx{1},sy{1},sz{1};
};

static std::unordered_map<int, Entity> g_entities;
static std::unordered_map<std::string,int> g_name_to_id;
static int g_next_id = 1;

// --- Simple orbit camera state (Blender-like) ---
static float g_cam_target[3] = {0.f,0.f,0.f};
static float g_cam_dist = 6.f;
static float g_cam_yaw = 45.f;   // degrees
static float g_cam_pitch = 25.f; // degrees
static float g_cam_pan[2] = {0.f,0.f}; // x,z offset
static int   g_vp_w = 1280, g_vp_h = 720;

static int make_entity(const std::string& name){
  int id = g_next_id++;
  Entity e; e.id = id; e.name = name; g_entities[id]=e; g_name_to_id[name]=id; return id;
}

void EngineAPI::init(){
  if(!g_entities.empty()) return;
  make_entity("World");
  int cam = make_entity("Camera");
  int cube = make_entity("Cube");
  make_entity("Light");
  // Default transforms
  if(auto it=g_entities.find(cam); it!=g_entities.end()){
    it->second.tx = -3.0f; it->second.ty = 1.6f; it->second.tz = -2.2f;
  }
  if(auto it=g_entities.find(cube); it!=g_entities.end()){
    it->second.tx = 0.0f; it->second.ty = 0.5f; it->second.tz = 0.0f;
  }
}
void EngineAPI::resize(int w,int h){ g_vp_w = (w>0?w:1); g_vp_h = (h>0?h:1); }

static void draw_grid(){
  glLineWidth(1.0f);
  glColor4f(1.f,1.f,1.f,0.08f);
  glBegin(GL_LINES);
  const int N=20; const float S=1.0f;
  for(int i=-N;i<=N;++i){ glVertex3f(i*S,0,-N*S); glVertex3f(i*S,0,N*S); glVertex3f(-N*S,0,i*S); glVertex3f(N*S,0,i*S);} glEnd();
}
static void draw_cube(){
  glColor3f(0.75f,0.75f,0.78f);
  float s=0.5f;
  glBegin(GL_QUADS);
  // +X
  glNormal3f(1,0,0);  glVertex3f(s,-s,-s); glVertex3f(s,-s,s); glVertex3f(s,s,s); glVertex3f(s,s,-s);
  // -X
  glNormal3f(-1,0,0); glVertex3f(-s,-s,s); glVertex3f(-s,-s,-s); glVertex3f(-s,s,-s); glVertex3f(-s,s,s);
  // +Y
  glNormal3f(0,1,0);  glVertex3f(-s,s,-s); glVertex3f(s,s,-s); glVertex3f(s,s,s); glVertex3f(-s,s,s);
  // -Y
  glNormal3f(0,-1,0); glVertex3f(-s,-s,s); glVertex3f(s,-s,s); glVertex3f(s,-s,-s); glVertex3f(-s,-s,-s);
  // +Z
  glNormal3f(0,0,1);  glVertex3f(-s,-s,s); glVertex3f(-s,s,s); glVertex3f(s,s,s); glVertex3f(s,-s,s);
  // -Z
  glNormal3f(0,0,-1); glVertex3f(s,-s,-s); glVertex3f(s,s,-s); glVertex3f(-s,s,-s); glVertex3f(-s,-s,-s);
  glEnd();
}

static void draw_camera_gizmo(){
  // Simple frustum-like wireframe + small triangle cap, Blender-like orange
  glColor3f(1.0f, 0.64f, 0.0f);
  const float L = 1.2f; // length
  const float n = 0.15f; // near half-size
  const float f = 0.45f; // far half-size
  // Lines
  glLineWidth(2.0f);
  glBegin(GL_LINES);
  // from tip to far plane corners
  glVertex3f(0,0,0); glVertex3f( L,  f,  f);
  glVertex3f(0,0,0); glVertex3f( L,  f, -f);
  glVertex3f(0,0,0); glVertex3f( L, -f,  f);
  glVertex3f(0,0,0); glVertex3f( L, -f, -f);
  // far plane rectangle
  glVertex3f(L, f, f);  glVertex3f(L, f,-f);
  glVertex3f(L, f,-f);  glVertex3f(L,-f,-f);
  glVertex3f(L,-f,-f);  glVertex3f(L,-f, f);
  glVertex3f(L,-f, f);  glVertex3f(L, f, f);
  // near guide
  glVertex3f(0, n, n);  glVertex3f(0, n,-n);
  glVertex3f(0, n,-n);  glVertex3f(0,-n,-n);
  glVertex3f(0,-n,-n);  glVertex3f(0,-n, n);
  glVertex3f(0,-n, n);  glVertex3f(0, n, n);
  glEnd();
  // small filled triangle cap to hint forward direction
  glBegin(GL_TRIANGLES);
  glVertex3f(0.1f, 0, 0); glVertex3f(-0.1f, 0.18f, 0); glVertex3f(-0.1f,-0.18f, 0);
  glEnd();
}

void EngineAPI::render(){
  glViewport(0,0,g_vp_w,g_vp_h);
  glClearColor(0.07f, 0.07f, 0.08f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);

  // Setup projection and view
  auto deg2rad = [](float a){ return a*3.14159265f/180.f; };
  float aspect = (float)g_vp_w / (float)std::max(1, g_vp_h);
  float fov = 45.f, zn=0.1f, zf=100.f;
  float t = std::tan(deg2rad(fov)/2.f);
  float P[16] = { 1.f/(aspect*t),0,0,0,  0,1.f/t,0,0,  0,0,-(zf+zn)/(zf-zn),-1.f,  0,0,-(2*zf*zn)/(zf-zn),0 };

  float cy = deg2rad(g_cam_yaw), cp = deg2rad(g_cam_pitch);
  float cx = g_cam_target[0]-g_cam_pan[0];
  float cz = g_cam_target[2]-g_cam_pan[1];
  float eye[3] = { cx + g_cam_dist*std::cos(cp)*std::cos(cy),
                   g_cam_target[1] + g_cam_dist*std::sin(cp),
                   cz + g_cam_dist*std::cos(cp)*std::sin(cy) };
  auto norm3 = [](float v[3]){ float l=std::sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]); if(l>0){v[0]/=l;v[1]/=l;v[2]/=l;} };
  float f[3] = { g_cam_target[0]-eye[0], g_cam_target[1]-eye[1], g_cam_target[2]-eye[2] }; norm3(f);
  float up[3] = {0,1,0};
  float r[3] = { f[1]*up[2]-f[2]*up[1], f[2]*up[0]-f[0]*up[2], f[0]*up[1]-f[1]*up[0] }; norm3(r);
  float u[3] = { r[1]*f[2]-r[2]*f[1], r[2]*f[0]-r[0]*f[2], r[0]*f[1]-r[1]*f[0] };
  float V[16] = {
    r[0], u[0], -f[0], 0,
    r[1], u[1], -f[1], 0,
    r[2], u[2], -f[2], 0,
    -(r[0]*eye[0]+r[1]*eye[1]+r[2]*eye[2]),
    -(u[0]*eye[0]+u[1]*eye[1]+u[2]*eye[2]),
     (f[0]*eye[0]+f[1]*eye[1]+f[2]*eye[2]),
     1
  };
  glMatrixMode(GL_PROJECTION); glLoadMatrixf(P);
  glMatrixMode(GL_MODELVIEW);  glLoadMatrixf(V);

  // Grid
  glLineWidth(1.0f);
  glColor4f(1.f,1.f,1.f,0.13f);
  draw_grid();

  // World axes (match Blender colors)
  glLineWidth(2.0f);
  glBegin(GL_LINES);
  glColor3f(0.93f, 0.25f, 0.25f); glVertex3f(0,0,0); glVertex3f(2,0,0); // X red
  glColor3f(0.38f, 0.78f, 0.39f); glVertex3f(0,0,0); glVertex3f(0,2,0); // Y green
  glColor3f(0.25f, 0.45f, 0.93f); glVertex3f(0,0,0); glVertex3f(0,0,2); // Z blue
  glEnd();
  // Camera gizmo
  if(auto it = g_name_to_id.find("Camera"); it!=g_name_to_id.end()){
    const auto& e = g_entities[it->second];
    if(e.visible){
      glPushMatrix(); glTranslatef(e.tx, e.ty, e.tz); draw_camera_gizmo(); glPopMatrix();
    }
  }
  // Cube
  if(auto it = g_name_to_id.find("Cube"); it!=g_name_to_id.end()){
    const auto& e = g_entities[it->second];
    if(e.visible){ glPushMatrix(); glTranslatef(e.tx, e.ty-0.5f, e.tz); draw_cube(); glPopMatrix(); }
  }
}

int EngineAPI::get_entity_id(const char* name){ auto it=g_name_to_id.find(name?name:""); return it==g_name_to_id.end()?-1:it->second; }
void EngineAPI::set_visible(int id, bool v){ auto it=g_entities.find(id); if(it!=g_entities.end()) it->second.visible=v; }
bool EngineAPI::is_visible(int id){ auto it=g_entities.find(id); return it!=g_entities.end()?it->second.visible:false; }
void EngineAPI::set_locked(int id, bool v){ auto it=g_entities.find(id); if(it!=g_entities.end()) it->second.locked=v; }
bool EngineAPI::is_locked(int id){ auto it=g_entities.find(id); return it!=g_entities.end()?it->second.locked:false; }

void EngineAPI::set_transform(int id, float px, float py, float pz,
                              float rx, float ry, float rz){
  auto it=g_entities.find(id);
  if(it!=g_entities.end()){
    it->second.tx = px; it->second.ty = py; it->second.tz = pz;
    it->second.rx = rx; it->second.ry = ry; it->second.rz = rz;
  }
}

std::tuple<std::array<float,3>, std::array<float,3>> EngineAPI::get_transform(int id){
  std::array<float,3> p{0,0,0}; std::array<float,3> r{0,0,0};
  auto it=g_entities.find(id);
  if(it!=g_entities.end()){
    p = {it->second.tx, it->second.ty, it->second.tz};
    r = {it->second.rx, it->second.ry, it->second.rz};
  }
  return {p,r};
}

// Camera API
void EngineAPI::camera_set_target(float x, float y, float z){ g_cam_target[0]=x; g_cam_target[1]=y; g_cam_target[2]=z; }
void EngineAPI::camera_set_orbit(float yaw_deg, float pitch_deg, float dist){
  g_cam_yaw = yaw_deg; g_cam_pitch = std::max(-89.f, std::min(89.f, pitch_deg)); g_cam_dist = std::max(0.5f, dist);
}
void EngineAPI::camera_orbit_delta(float dyaw_deg, float dpitch_deg){ camera_set_orbit(g_cam_yaw+dyaw_deg, g_cam_pitch+dpitch_deg, g_cam_dist); }
void EngineAPI::camera_pan_delta(float dx, float dy){ g_cam_pan[0]+=dx; g_cam_pan[1]+=dy; }
void EngineAPI::camera_dolly_factor(float factor){ g_cam_dist = std::max(0.5f, std::min(100.f, g_cam_dist*factor)); }

} // namespace aartze::scripting

using namespace aartze::scripting;

PYBIND11_MODULE(aartze, m){
  m.doc() = "AARTZE engine Python bridge (stub)";
  m.def("init", &EngineAPI::init);
  m.def("resize", &EngineAPI::resize);
  m.def("render", &EngineAPI::render);
  m.def("get_entity_id", &EngineAPI::get_entity_id);
  m.def("set_visible", &EngineAPI::set_visible);
  m.def("is_visible", &EngineAPI::is_visible);
  m.def("set_locked", &EngineAPI::set_locked);
  m.def("is_locked", &EngineAPI::is_locked);
  m.def("set_transform", &EngineAPI::set_transform);
  m.def("get_transform", &EngineAPI::get_transform);
  m.def("camera_set_target", &EngineAPI::camera_set_target);
  m.def("camera_set_orbit", &EngineAPI::camera_set_orbit);
  m.def("camera_orbit_delta", &EngineAPI::camera_orbit_delta);
  m.def("camera_pan_delta", &EngineAPI::camera_pan_delta);
  m.def("camera_dolly_factor", &EngineAPI::camera_dolly_factor);
}

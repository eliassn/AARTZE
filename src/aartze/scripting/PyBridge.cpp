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
// Assimp and cgltf for importers
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <cgltf.h>

#include "aartze/render/scene/GizmoRenderer.h"
#include "aartze/input/InputSystem.h"

namespace py = pybind11;

namespace aartze::scripting {

struct Entity {
  int id{}; std::string name; bool visible{true}; bool locked{false};
  float tx{0},ty{0},tz{0}; float rx{0},ry{0},rz{0}; float sx{1},sy{1},sz{1};
};

static std::unordered_map<int, Entity> g_entities;
static std::unordered_map<std::string,int> g_name_to_id;
static int g_next_id = 1;

// Minimal mesh store for imported assets (triangles only)
struct Mesh { std::vector<float> pos; std::vector<float> nrm; };
static std::unordered_map<int, Mesh> g_meshes; // entity id -> mesh

// --- Simple orbit camera state (Blender-like) ---
static float g_cam_target[3] = {0.f,0.f,0.f};
static float g_cam_dist = 6.f;
static float g_cam_yaw = 45.f;   // degrees
static float g_cam_pitch = 25.f; // degrees
static float g_cam_pan[2] = {0.f,0.f}; // x,z offset
static int   g_vp_w = 1280, g_vp_h = 720;
static float g_P[16] = {0};
static float g_V[16] = {0};
static float g_PV[16] = {0};

static void mul4x4(const float A[16], const float B[16], float R[16]){
  for(int r=0;r<4;++r){ for(int c=0;c<4;++c){ R[r*4+c]=A[r*4+0]*B[0*4+c]+A[r*4+1]*B[1*4+c]+A[r*4+2]*B[2*4+c]+A[r*4+3]*B[3*4+c]; }}
}
static void mul4x4_vec4(const float M[16], const float v[4], float out[4]){
  for(int r=0;r<4;++r){ out[r]=M[r*4+0]*v[0]+M[r*4+1]*v[1]+M[r*4+2]*v[2]+M[r*4+3]*v[3]; }
}
static void world_to_ndc(const float pv[16], const float p[3], float out_ndc[3]){
  float v[4]={p[0],p[1],p[2],1.f}, clip[4];
  mul4x4_vec4(pv, v, clip);
  if(clip[3]!=0.f){ out_ndc[0]=clip[0]/clip[3]; out_ndc[1]=clip[1]/clip[3]; out_ndc[2]=clip[2]/clip[3]; }
  else { out_ndc[0]=out_ndc[1]=out_ndc[2]=0.f; }
}

static int make_entity(const std::string& name){
  int id = g_next_id++;
  Entity e; e.id = id; e.name = name; g_entities[id]=e; g_name_to_id[name]=id; return id;
}

// Selection
static int g_selected_id = -1;

// ---- Gizmo state (used by render + input) ----
static std::string g_gizmo_mode = ""; // move|rotate|scale
static char g_gizmo_axis = 'x';
static float g_drag_start_ndc[2] = {0,0};
static float g_axis_ndc_dir[2] = {1,0};
static float g_axis_ndc_len_per_world = 1.f; // |ndc(p+axis)-ndc(p)| for 1 world unit
static Entity g_start_entity{};
static char g_hover_axis = 0;
static bool g_screen_axis = false;

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
  // Store matrices for gizmo math
  for(int i=0;i<16;++i){ g_P[i]=P[i]; g_V[i]=V[i]; }
  mul4x4(P, V, g_PV);
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
  // Simple one-light setup for basic shading of solid geometry
  glDisable(GL_TEXTURE_2D);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  GLfloat lightDir[4]   = { -0.3f, 0.8f, 0.6f, 0.0f }; // directional
  GLfloat lightDiff[4]  = { 0.9f, 0.9f, 0.9f, 1.0f };
  GLfloat lightAmb [4]  = { 0.12f, 0.12f, 0.14f, 1.0f };
  GLfloat lightSpec[4]  = { 0.2f, 0.2f, 0.2f, 1.0f };
  glLightfv(GL_LIGHT0, GL_POSITION, lightDir);
  glLightfv(GL_LIGHT0, GL_DIFFUSE,  lightDiff);
  glLightfv(GL_LIGHT0, GL_AMBIENT,  lightAmb);
  glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpec);
  glEnable(GL_COLOR_MATERIAL);
  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  glShadeModel(GL_SMOOTH);

  // Cube or imported mesh attached to Cube entity
  if(auto it = g_name_to_id.find("Cube"); it!=g_name_to_id.end()){
    const auto& e = g_entities[it->second];
    if(e.visible){
      auto mit = g_meshes.find(it->second);
      glPushMatrix(); glTranslatef(e.tx, e.ty-0.5f, e.tz);
      if(mit == g_meshes.end() || mit->second.pos.empty()) draw_cube();
      else {
        const auto& m = mit->second;
        size_t triCount = m.pos.size()/9; // 3 verts * 3 comps
        glBegin(GL_TRIANGLES);
        for(size_t t=0;t<triCount;++t){
          size_t i = t*9; size_t in = t*9; // normals per-vertex
          if(m.nrm.size()==m.pos.size()){
            glNormal3f(m.nrm[in+0], m.nrm[in+1], m.nrm[in+2]);
            glVertex3f(m.pos[i+0], m.pos[i+1], m.pos[i+2]);
            glNormal3f(m.nrm[in+3], m.nrm[in+4], m.nrm[in+5]);
            glVertex3f(m.pos[i+3], m.pos[i+4], m.pos[i+5]);
            glNormal3f(m.nrm[in+6], m.nrm[in+7], m.nrm[in+8]);
            glVertex3f(m.pos[i+6], m.pos[i+7], m.pos[i+8]);
          } else {
            glVertex3f(m.pos[i+0], m.pos[i+1], m.pos[i+2]);
            glVertex3f(m.pos[i+3], m.pos[i+4], m.pos[i+5]);
            glVertex3f(m.pos[i+6], m.pos[i+7], m.pos[i+8]);
          }
        }
        glEnd();
      }
      glPopMatrix();
    }
  }

  // Draw other imported mesh entities
  for(const auto& kv : g_meshes){
    int id = kv.first; if(auto itE=g_entities.find(id); itE!=g_entities.end()){
      const auto& e = itE->second; if(!e.visible) continue; if(e.name=="Cube") continue;
      const auto& m = kv.second; if(m.pos.empty()) continue;
      glPushMatrix(); glTranslatef(e.tx, e.ty, e.tz);
      glBegin(GL_TRIANGLES);
      size_t triCount = m.pos.size()/9;
      for(size_t t=0;t<triCount;++t){ size_t i=t*9; size_t in=t*9; if(m.nrm.size()==m.pos.size()) glNormal3f(m.nrm[in+0], m.nrm[in+1], m.nrm[in+2]); glVertex3f(m.pos[i+0],m.pos[i+1],m.pos[i+2]); if(m.nrm.size()==m.pos.size()) glNormal3f(m.nrm[in+3], m.nrm[in+4], m.nrm[in+5]); glVertex3f(m.pos[i+3],m.pos[i+4],m.pos[i+5]); if(m.nrm.size()==m.pos.size()) glNormal3f(m.nrm[in+6], m.nrm[in+7], m.nrm[in+8]); glVertex3f(m.pos[i+6],m.pos[i+7],m.pos[i+8]); }
      glEnd();
      glPopMatrix();
    }
  }

  glDisable(GL_LIGHTING);

  // Draw transform gizmo handles at selected entity pivot
  if(g_selected_id >= 0){
    auto it = g_entities.find(g_selected_id);
    if(it != g_entities.end()){
      float pivot[3] = { it->second.tx, it->second.ty, it->second.tz };
      static aartze::render::scene::GizmoRenderer giz;
      const char axis = g_gizmo_axis;
      const char hover = g_hover_axis;
      if(g_gizmo_mode == "rotate") giz.drawRotate(pivot, axis, hover);
      else if(g_gizmo_mode == "scale") giz.drawScale(pivot, axis, hover);
      else giz.drawTranslate(pivot, axis, hover);
    }
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

// ---- Scene CRUD ----
int EngineAPI::scene_create_entity(const char* name){
  std::string nm = name && *name ? name : "Entity";
  return make_entity(nm);
}
pybind11::list EngineAPI::scene_entities(){
  pybind11::list lst;
  for(const auto& kv : g_entities){
    const auto& e = kv.second;
    pybind11::dict d;
    d["id"] = e.id; d["name"] = e.name; d["visible"] = e.visible; d["locked"] = e.locked;
    lst.append(d);
  }
  return lst;
}
void EngineAPI::scene_select(int id){ g_selected_id = id; }

void EngineAPI::gizmo_set_mode(const char* mode){ g_gizmo_mode = mode?mode:""; }
void EngineAPI::gizmo_set_axis(char axis){ g_gizmo_axis = axis; }
void EngineAPI::gizmo_set_screen_axis(bool enabled){ g_screen_axis = enabled; }
void EngineAPI::gizmo_begin(float /*rox*/, float /*roy*/, float /*roz*/, float /*rdx*/, float /*rdy*/, float /*rdz*/, int /*modifiers*/){
  if(g_selected_id<0) return;
  auto it = g_entities.find(g_selected_id);
  if(it==g_entities.end()) return;
  g_start_entity = it->second;

  // Pivot world pos
  float pivot[3] = { g_start_entity.tx, g_start_entity.ty, g_start_entity.tz };
  float ndc_p[3]; world_to_ndc(g_PV, pivot, ndc_p);
  if(g_screen_axis){
    // Screen-space axis lock: X→screen-X, Y→screen-Y, Z→diagonal fallback
    if(g_gizmo_axis=='x'){ g_axis_ndc_dir[0]=1.f; g_axis_ndc_dir[1]=0.f; }
    else if(g_gizmo_axis=='y'){ g_axis_ndc_dir[0]=0.f; g_axis_ndc_dir[1]=1.f; }
    else { g_axis_ndc_dir[0]=-0.7071f; g_axis_ndc_dir[1]=0.7071f; }
    // Reference scale: use world X axis projection near pivot
    float qx[3]={pivot[0]+1.f,pivot[1],pivot[2]}, ndc_qx[3]; world_to_ndc(g_PV,qx,ndc_qx);
    float axx = ndc_qx[0]-ndc_p[0], axy=ndc_qx[1]-ndc_p[1];
    g_axis_ndc_len_per_world = std::sqrt(axx*axx+axy*axy);
    if(g_axis_ndc_len_per_world<1e-4f) g_axis_ndc_len_per_world=1.f;
  } else {
    // World axis projection
    float axisw[3] = { (g_gizmo_axis=='x')?1.f:0.f, (g_gizmo_axis=='y')?1.f:0.f, (g_gizmo_axis=='z')?1.f:0.f };
    float qworld[3] = { pivot[0]+axisw[0], pivot[1]+axisw[1], pivot[2]+axisw[2] };
    float ndc_q[3]; world_to_ndc(g_PV, qworld, ndc_q);
    float axx = ndc_q[0]-ndc_p[0]; float axy = ndc_q[1]-ndc_p[1];
    float L = std::sqrt(axx*axx + axy*axy);
    if(L<1e-4f){ g_axis_ndc_dir[0]=1; g_axis_ndc_dir[1]=0; g_axis_ndc_len_per_world=1; }
    else { g_axis_ndc_dir[0]=axx/L; g_axis_ndc_dir[1]=axy/L; g_axis_ndc_len_per_world = L; }
  }
}
void EngineAPI::gizmo_drag(float ndc_x, float ndc_y, int modifiers, float /*dt*/){
  if(g_selected_id<0) return; auto it=g_entities.find(g_selected_id); if(it==g_entities.end()) return; auto& e = it->second;
  float delta_ndc_axis = ndc_x*g_axis_ndc_dir[0] + ndc_y*g_axis_ndc_dir[1];
  // Snap if Shift held (assume bit 1<<0 == shift)
  bool snap = (modifiers & 1);
  float snap_step_move = 0.1f; // world units
  float snap_step_rot  = 5.0f; // degrees
  if(g_gizmo_mode=="move"){
    float world_delta = (g_axis_ndc_len_per_world>1e-6f)? (delta_ndc_axis / g_axis_ndc_len_per_world) : 0.f;
    if(snap){ world_delta = std::round(world_delta / snap_step_move) * snap_step_move; }
    if(g_gizmo_axis=='x') e.tx = g_start_entity.tx + world_delta;
    else if(g_gizmo_axis=='y') e.ty = g_start_entity.ty + world_delta;
    else if(g_gizmo_axis=='z') e.tz = g_start_entity.tz + world_delta;
  } else if(g_gizmo_mode=="rotate"){
    float angle = delta_ndc_axis * 180.f; if(snap){ angle = std::round(angle / snap_step_rot) * snap_step_rot; }
    if(g_gizmo_axis=='x') e.rx = g_start_entity.rx + angle;
    else if(g_gizmo_axis=='y') e.ry = g_start_entity.ry + angle;
    else if(g_gizmo_axis=='z') e.rz = g_start_entity.rz + angle;
  } else if(g_gizmo_mode=="scale"){
    float s = 1.f + delta_ndc_axis; if(snap){ s = std::round(s / 0.1f) * 0.1f; }
    s = std::max(0.001f, s);
    if(g_gizmo_axis=='x') e.sx = g_start_entity.sx * s;
    else if(g_gizmo_axis=='y') e.sy = g_start_entity.sy * s;
    else if(g_gizmo_axis=='z') e.sz = g_start_entity.sz * s;
  }
}
void EngineAPI::gizmo_end(){ g_gizmo_mode.clear(); }
void EngineAPI::gizmo_cancel(){
  if(g_selected_id<0) return; auto it=g_entities.find(g_selected_id); if(it==g_entities.end()) return; auto& e=it->second;
  e = g_start_entity; g_gizmo_mode.clear();
}

static float axis_ndc_distance(char axis, const float pivot[3], float px_ndc, float py_ndc){
  float ndc_p[3], ndc_q[3];
  const float handleLen = 1.5f; // world units; must match renderer
  float a[3] = { (axis=='x')?handleLen:0.f, (axis=='y')?handleLen:0.f, (axis=='z')?handleLen:0.f };
  float q[3] = { pivot[0]+a[0], pivot[1]+a[1], pivot[2]+a[2] };
  world_to_ndc(g_PV, pivot, ndc_p); world_to_ndc(g_PV, q, ndc_q);
  float vx = ndc_q[0]-ndc_p[0], vy = ndc_q[1]-ndc_p[1];
  float L = std::sqrt(vx*vx+vy*vy); if(L<1e-6f) return 1e9f;
  vx/=L; vy/=L; // unit axis in screen
  float dx = px_ndc - ndc_p[0]; float dy = py_ndc - ndc_p[1];
  float t = dx*vx + dy*vy; // param along axis in NDC units
  if(t < 0.f || t > L) return 1e9f; // clamp to handle length
  float perp = std::abs(-vy*dx + vx*dy); // distance to line segment
  // Tip cone/cube hit: boost if near tip point
  float tipdx = px_ndc - ndc_q[0]; float tipdy = py_ndc - ndc_q[1];
  float tipd = std::sqrt(tipdx*tipdx + tipdy*tipdy);
  const float tipThresh = 0.035f;
  if(tipd < tipThresh) return 0.0f; // strongest hit
  return perp;
}

char EngineAPI::gizmo_hover(float ndc_x, float ndc_y){
  g_hover_axis = 0;
  if(g_selected_id<0) return 0; auto it = g_entities.find(g_selected_id); if(it==g_entities.end()) return 0;
  float pivot[3] = { it->second.tx, it->second.ty, it->second.tz };
  float best = 1e9f; char best_ax = 0; const float thresh = 0.05f; // ~5% of NDC width
  for(char ax : {'x','y','z'}){
    float d = axis_ndc_distance(ax, pivot, ndc_x, ndc_y);
    if(d < best){ best = d; best_ax = ax; }
  }
  if(best < thresh) g_hover_axis = best_ax; else g_hover_axis = 0;
  return g_hover_axis;
}

// ---- Importers ----
static bool load_with_assimp(const std::string& path, Mesh& out){
  Assimp::Importer imp; const aiScene* sc = imp.ReadFile(path, aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_JoinIdenticalVertices);
  if(!sc || !sc->mRootNode || sc->mNumMeshes==0) return false;
  aiMesh* mesh = sc->mMeshes[0];
  out.pos.clear(); out.nrm.clear();
  out.pos.reserve(mesh->mNumFaces * 9);
  if(mesh->HasNormals()) out.nrm.reserve(mesh->mNumFaces*9);
  for(unsigned f=0; f<mesh->mNumFaces; ++f){
    const aiFace& face = mesh->mFaces[f]; if(face.mNumIndices!=3) continue; for(unsigned i=0;i<3;++i){ unsigned idx = face.mIndices[i]; aiVector3D v = mesh->mVertices[idx]; out.pos.push_back(v.x); out.pos.push_back(v.y); out.pos.push_back(v.z); if(mesh->HasNormals()){ aiVector3D n = mesh->mNormals[idx]; out.nrm.push_back(n.x); out.nrm.push_back(n.y); out.nrm.push_back(n.z);} }
  }
  return !out.pos.empty();
}
static bool load_with_cgltf(const std::string& path, Mesh& out){
  cgltf_options options{}; cgltf_data* data=nullptr; if(cgltf_parse_file(&options, path.c_str(), &data)!=cgltf_result_success) return false; cgltf_load_buffers(&options, data, path.c_str());
  out.pos.clear(); out.nrm.clear();
  bool ok=false;
  for(size_t mi=0; mi<data->meshes_count && !ok; ++mi){ const cgltf_mesh& m = data->meshes[mi]; if(m.primitives_count==0) continue; const cgltf_primitive& p = m.primitives[0]; const cgltf_accessor* pos=nullptr; const cgltf_accessor* nrm=nullptr; for(size_t ai=0; ai<p.attributes_count; ++ai){ const auto& a = p.attributes[ai]; if(a.type==cgltf_attribute_type_position) pos=a.data; else if(a.type==cgltf_attribute_type_normal) nrm=a.data; }
    if(!pos) continue; size_t vcount = pos->count; std::vector<float> tmpPos; tmpPos.reserve(vcount*3); for(size_t i=0;i<vcount;++i){ float v[3]; cgltf_accessor_read_float(pos, i, v, 3); tmpPos.push_back(v[0]); tmpPos.push_back(v[1]); tmpPos.push_back(v[2]); }
    std::vector<unsigned int> idx; if(p.indices){ idx.resize(p.indices->count); for(size_t i=0;i<idx.size();++i){ idx[i]=(unsigned)cgltf_accessor_read_index(p.indices, i); } }
    if(idx.empty()){ // assume sequential triangles
      for(size_t i=0;i+2<vcount; i+=3){ out.pos.insert(out.pos.end(), {tmpPos[i*3+0],tmpPos[i*3+1],tmpPos[i*3+2], tmpPos[(i+1)*3+0],tmpPos[(i+1)*3+1],tmpPos[(i+1)*3+2], tmpPos[(i+2)*3+0],tmpPos[(i+2)*3+1],tmpPos[(i+2)*3+2]}); }
    } else {
      for(size_t i=0;i+2<idx.size(); i+=3){ unsigned a=idx[i], b=idx[i+1], c=idx[i+2]; out.pos.insert(out.pos.end(), {tmpPos[a*3+0],tmpPos[a*3+1],tmpPos[a*3+2], tmpPos[b*3+0],tmpPos[b*3+1],tmpPos[b*3+2], tmpPos[c*3+0],tmpPos[c*3+1],tmpPos[c*3+2]}); }
    }
    if(nrm){ size_t ncount = nrm->count; std::vector<float> tmpN; tmpN.reserve(ncount*3); for(size_t i=0;i<ncount;++i){ float v[3]; cgltf_accessor_read_float(nrm, i, v, 3); tmpN.push_back(v[0]); tmpN.push_back(v[1]); tmpN.push_back(v[2]); }
      if(idx.empty()){ for(size_t i=0;i+2<ncount; i+=3){ out.nrm.insert(out.nrm.end(), {tmpN[i*3+0],tmpN[i*3+1],tmpN[i*3+2], tmpN[(i+1)*3+0],tmpN[(i+1)*3+1],tmpN[(i+1)*3+2], tmpN[(i+2)*3+0],tmpN[(i+2)*3+1],tmpN[(i+2)*3+2]}); } }
      else { for(size_t i=0;i+2<idx.size(); i+=3){ unsigned a=idx[i], b=idx[i+1], c=idx[i+2]; out.nrm.insert(out.nrm.end(), {tmpN[a*3+0],tmpN[a*3+1],tmpN[a*3+2], tmpN[b*3+0],tmpN[b*3+1],tmpN[b*3+2], tmpN[c*3+0],tmpN[c*3+1],tmpN[c*3+2]}); } }
    }
    ok = !out.pos.empty();
  }
  cgltf_free(data);
  return ok;
}

static std::string filenameStem(const std::string& p){ auto s = p; size_t slash = s.find_last_of("/\\"); if(slash!=std::string::npos) s = s.substr(slash+1); size_t dot = s.find_last_of('.'); if(dot!=std::string::npos) s = s.substr(0,dot); return s; }

int import_file_impl(const std::string& path){
  std::string lower = path; std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
  Mesh mesh; bool ok=false;
  if(lower.rfind(".gltf")!=std::string::npos || lower.rfind(".glb")!=std::string::npos) ok = load_with_cgltf(path, mesh);
  if(!ok) ok = load_with_assimp(path, mesh);
  if(!ok) return -1;
  int id = make_entity(filenameStem(path));
  g_meshes[id] = std::move(mesh);
  return id;
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

PYBIND11_MODULE(aartzepy, m){
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
  // Scene
  m.def("create_entity", &EngineAPI::scene_create_entity);
  m.def("entities", &EngineAPI::scene_entities);
  m.def("select", &EngineAPI::scene_select);
  // Gizmo hooks
  m.def("gizmo_begin", &EngineAPI::gizmo_begin);
  m.def("gizmo_drag", &EngineAPI::gizmo_drag);
  m.def("gizmo_end", &EngineAPI::gizmo_end);
  m.def("gizmo_set_mode", &EngineAPI::gizmo_set_mode);
  m.def("gizmo_set_axis", &EngineAPI::gizmo_set_axis);
  m.def("gizmo_cancel", &EngineAPI::gizmo_cancel);
  m.def("gizmo_set_screen_axis", &EngineAPI::gizmo_set_screen_axis);
  m.def("gizmo_hover", &EngineAPI::gizmo_hover);
  m.def("camera_set_target", &EngineAPI::camera_set_target);
  m.def("camera_set_orbit", &EngineAPI::camera_set_orbit);
  m.def("camera_orbit_delta", &EngineAPI::camera_orbit_delta);
  m.def("camera_pan_delta", &EngineAPI::camera_pan_delta);
  m.def("camera_dolly_factor", &EngineAPI::camera_dolly_factor);

  // -------- Input (gainput-backed stub) --------
  m.def("input_create_map", [](const char* name){ aartze::input::Get().ensureMap(name?name:"default"); });
  m.def("input_set_active_map", [](const char* name){ aartze::input::Get().setActiveMap(name?name:"default"); });
  m.def("input_bind_action", [](const char* map, const char* action, const char* device, int code){
    return aartze::input::Get().bindAction(map?map:"default", action?action:"Action", aartze::input::DeviceFromString(device?device:"keyboard"), code);
  });
  m.def("input_bind_axis", [](const char* map, const char* axis, const char* device, int code, float scale){
    return aartze::input::Get().bindAxis(map?map:"default", axis?axis:"Axis", aartze::input::DeviceFromString(device?device:"keyboard"), code, scale);
  }, py::arg("map"), py::arg("axis"), py::arg("device"), py::arg("code"), py::arg("scale") = 1.0f);
  m.def("input_action", [](const char* action){ return aartze::input::Get().action(action?action:"Action"); });
  m.def("input_axis",   [](const char* axis){ return aartze::input::Get().axis(axis?axis:"Axis"); });
  m.def("input_feed_key", [](int key, bool down){ aartze::input::Get().feedKey(key, down); });
  m.def("input_feed_mouse_button", [](int btn, bool down){ aartze::input::Get().feedMouseButton(btn, down); });
  m.def("input_feed_mouse_move", [](float dx, float dy){ aartze::input::Get().feedMouseMove(dx, dy); });
  // -------- Importers --------
  m.def("import_file", [](const char* path){ if(!path) return -1; try{ return import_file_impl(path); } catch(...) { return -1; } });
}

#pragma once
#include <array>
#include <tuple>
namespace aartze::scripting {
struct EngineAPI {
  static void init();
  static void resize(int, int);
  static void render();
  static int  get_entity_id(const char* name);
  static void set_visible(int id, bool v);
  static bool is_visible(int id);
  static void set_locked(int id, bool v);
  static bool is_locked(int id);
  // Transform helpers (position + euler rotation)
  static void set_transform(int id, float px, float py, float pz,
                            float rx, float ry, float rz);
  // Returns ((px,py,pz),(rx,ry,rz)) as a Python tuple
  static std::tuple<std::array<float,3>, std::array<float,3>> get_transform(int id);

  // Camera controls (Blender-like orbit camera)
  static void camera_set_target(float x, float y, float z);
  static void camera_set_orbit(float yaw_deg, float pitch_deg, float dist);
  static void camera_orbit_delta(float dyaw_deg, float dpitch_deg);
  static void camera_pan_delta(float dx, float dy);
  static void camera_dolly_factor(float factor);
};
}

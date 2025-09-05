#pragma once

// Minimal immediate-mode gizmo renderer (OpenGL).
// Draws translate arrows, rotate rings, and scale cubes, with an overlay pass
// to keep them visible (depth-biased rendering).

namespace aartze::render::scene {

struct GizmoRenderer {
  struct Theme {
    float colX[4] = {0.90f, 0.25f, 0.25f, 1.0f};
    float colY[4] = {0.35f, 0.80f, 0.35f, 1.0f};
    float colZ[4] = {0.23f, 0.46f, 0.93f, 1.0f};
    float alphaBase = 0.7f, alphaHover = 0.9f, alphaActive = 1.0f;
    float wBase = 2.0f, wHover = 3.0f, wActive = 4.0f;
    float transLen = 1.5f, transTipLen = 0.25f, transTipRad = 0.06f;
    float rotRadius = 1.2f;
    float scaleLen = 1.5f, scaleBox = 0.12f;
    bool polygonOffset = true;
  };

  GizmoRenderer() = default;
  const Theme& theme() const { return _theme; }
  void setTheme(const Theme& t) { _theme = t; }

  // Draw only; assumes caller set projection/view matrices.
  // active_axis: currently transforming; hover_axis: pre-highlight under cursor.
  void drawTranslate(const float pivot[3], char active_axis, char hover_axis) const;
  void drawRotate(const float pivot[3], char active_axis, char hover_axis) const;
  void drawScale(const float pivot[3], char active_axis, char hover_axis) const;
 private:
  Theme _theme;
};

}

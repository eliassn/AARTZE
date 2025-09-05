#if defined(_WIN32)
#  include <windows.h>
#endif
#if defined(_WIN32)
#  include <GL/gl.h>
#else
#  include <GL/gl.h>
#endif

#include "GizmoRenderer.h"
#include <cmath>

namespace aartze::render::scene {

static void colorAxis(char axis, float alpha=1.0f){
  // Colors tuned to match Blender-like axes (X red, Y green, Z blue)
  switch(axis){
    case 'x': glColor4f(0.90f, 0.25f, 0.25f, alpha); break; // red
    case 'y': glColor4f(0.35f, 0.80f, 0.35f, alpha); break; // green
    default:  glColor4f(0.23f, 0.46f, 0.93f, alpha); break; // blue (z)
  }
}

static void drawCone(float h, float r, int seg=24){
  glBegin(GL_TRIANGLE_FAN);
  glVertex3f(0,0,0);
  for(int i=0;i<=seg;++i){
    float a = (float)i / (float)seg * 6.2831853f;
    glVertex3f(0.0f, r*std::cos(a), r*std::sin(a));
  }
  glEnd();
  glBegin(GL_TRIANGLE_FAN); // cap
  for(int i=0;i<=seg;++i){
    float a = (float)i / (float)seg * 6.2831853f;
    glVertex3f(h, 0.0f, 0.0f);
    glVertex3f(0.0f, r*std::cos(a), r*std::sin(a));
  }
  glEnd();
}

static void drawArrow(char axis){
  const float L=1.5f; const float tip=0.25f; const float rad=0.06f;
  glLineWidth(3.0f);
  glBegin(GL_LINES);
  switch(axis){
    case 'x': glVertex3f(0,0,0); glVertex3f(L,0,0); break;
    case 'y': glVertex3f(0,0,0); glVertex3f(0,L,0); break;
    case 'z': glVertex3f(0,0,0); glVertex3f(0,0,L); break;
  }
  glEnd();
  glPushMatrix();
  if(axis=='x') glTranslatef(L,0,0), glRotatef(90,0,1,0);
  if(axis=='y') glTranslatef(0,L,0), glRotatef(-90,1,0,0);
  if(axis=='z') glTranslatef(0,0,L); // pointing +Z
  drawCone(tip, rad, 20);
  glPopMatrix();
}

static void drawRing(char axis){
  const float R=1.2f; const int seg=64;
  glLineWidth(2.5f);
  glBegin(GL_LINE_LOOP);
  for(int i=0;i<seg;++i){
    float a = (float)i/(float)seg * 6.2831853f;
    float x=0,y=0,z=0;
    switch(axis){
      case 'x': y = R*std::cos(a); z = R*std::sin(a); break;
      case 'y': x = R*std::cos(a); z = R*std::sin(a); break;
      default:  x = R*std::cos(a); y = R*std::sin(a); break;
    }
    glVertex3f(x,y,z);
  }
  glEnd();
}

static void drawScaleBox(char axis){
  const float L=1.5f; const float s=0.12f;
  glLineWidth(3.0f);
  glBegin(GL_LINES);
  switch(axis){
    case 'x': glVertex3f(0,0,0); glVertex3f(L,0,0); break;
    case 'y': glVertex3f(0,0,0); glVertex3f(0,L,0); break;
    case 'z': glVertex3f(0,0,0); glVertex3f(0,0,L); break;
  }
  glEnd();
  glPushMatrix();
  if(axis=='x') glTranslatef(L,0,0);
  if(axis=='y') glTranslatef(0,L,0);
  if(axis=='z') glTranslatef(0,0,L);
  glBegin(GL_LINE_LOOP); glVertex3f(-s,-s,-s); glVertex3f(s,-s,-s); glVertex3f(s,s,-s); glVertex3f(-s,s,-s); glEnd();
  glBegin(GL_LINE_LOOP); glVertex3f(-s,-s,s); glVertex3f(s,-s,s); glVertex3f(s,s,s); glVertex3f(-s,s,s); glEnd();
  glBegin(GL_LINES);
  glVertex3f(-s,-s,-s); glVertex3f(-s,-s,s);
  glVertex3f(s,-s,-s);  glVertex3f(s,-s,s);
  glVertex3f(s,s,-s);   glVertex3f(s,s,s);
  glVertex3f(-s,s,-s);  glVertex3f(-s,s,s);
  glEnd();
  glPopMatrix();
}

static void drawAllAxes(void(*fn)(char), char active, char hover){
  const char axes[3] = {'x','y','z'};
  for(int i=0;i<3;++i){
    char ax = axes[i];
    float alpha = 0.7f; float width = 2.0f;
    if(ax == hover) { alpha = 0.85f; width = 3.0f; }
    if(ax == active){ alpha = 1.0f; width = 4.0f; }
    colorAxis(ax, alpha);
    glLineWidth(width);
    fn(ax);
  }
}

static void overlayPassBegin(){ glDisable(GL_DEPTH_TEST); glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); }
static void overlayPassEnd(){ glDisable(GL_BLEND); glEnable(GL_DEPTH_TEST); }

void GizmoRenderer::drawTranslate(const float pivot[3], char active_axis, char hover_axis) const{
  // Antialiasing and polygon-offset mitigate z-fighting and jaggies
  glEnable(GL_LINE_SMOOTH); glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
  glEnable(GL_POLYGON_OFFSET_LINE); glPolygonOffset(-1.f, -1.f);
  glPushMatrix(); glTranslatef(pivot[0], pivot[1], pivot[2]);
  // Depth-tested pass
  drawAllAxes(drawArrow, active_axis, hover_axis);
  // Overlay to ensure visibility
  overlayPassBegin(); drawAllAxes(drawArrow, active_axis, hover_axis); overlayPassEnd();
  glPopMatrix();
  glDisable(GL_POLYGON_OFFSET_LINE); glDisable(GL_LINE_SMOOTH);
}

void GizmoRenderer::drawRotate(const float pivot[3], char active_axis, char hover_axis) const{
  glEnable(GL_LINE_SMOOTH); glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
  glEnable(GL_POLYGON_OFFSET_LINE); glPolygonOffset(-1.f, -1.f);
  glPushMatrix(); glTranslatef(pivot[0], pivot[1], pivot[2]);
  drawAllAxes(drawRing, active_axis, hover_axis);
  overlayPassBegin(); drawAllAxes(drawRing, active_axis, hover_axis); overlayPassEnd();
  glPopMatrix();
  glDisable(GL_POLYGON_OFFSET_LINE); glDisable(GL_LINE_SMOOTH);
}

void GizmoRenderer::drawScale(const float pivot[3], char active_axis, char hover_axis) const{
  glEnable(GL_LINE_SMOOTH); glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
  glEnable(GL_POLYGON_OFFSET_LINE); glPolygonOffset(-1.f, -1.f);
  glPushMatrix(); glTranslatef(pivot[0], pivot[1], pivot[2]);
  drawAllAxes(drawScaleBox, active_axis, hover_axis);
  overlayPassBegin(); drawAllAxes(drawScaleBox, active_axis, hover_axis); overlayPassEnd();
  glPopMatrix();
  glDisable(GL_POLYGON_OFFSET_LINE); glDisable(GL_LINE_SMOOTH);
}

}

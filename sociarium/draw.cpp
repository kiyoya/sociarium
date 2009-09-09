// s.o.c.i.a.r.i.u.m
// HASHIMOTO, Yasuhiro (E-mail: hy @ sys.t.u-tokyo.ac.jp)
// update: 2009/05/10

/* Copyright (c) 2005-2009, HASHIMOTO, Yasuhiro, All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 *   - Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *   - Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 *   - Neither the name of the University of Tokyo nor the names of its contributors
 *     may be used to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <map>
#include <memory>
#ifdef _MSC_VER
#include <windows.h>
#endif
#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#include <FTGL/ftgl.h>
#include "draw.h"
#include "view.h"
#include "font.h"
#include "../shared/vector3.h"
#include "../shared/math.h"
#include "../shared/GL/gltexture.h"

namespace hashimoto_ut {

  using std::vector;
  using std::multimap;
  using std::make_pair;
  using std::tr1::shared_ptr;

  namespace sociarium_project_draw {

    namespace {
      ///////////////////////////////////////////////////////////////////////////////////////////////////
      // ローカル変数
      float const EDGE_SEGMENT_RESOLUTION = 3.0f;
      int const MIN_NUMBER_OF_EDGE_SEGMENTS = 3; // 必ず3以上
      int const MAX_NUMBER_OF_EDGE_SEGMENTS = 50;

      // スライダー座標オフセット（ビューポート座標）
      Vector2<double> const slider_offset(10.0, 40.0);
      // セレクション結果フレームオフセット（ビューポート座標）
      Vector2<double> const selection_frame_offset(5.0, 5.0);

      // グリッド
      int grid_resolution = 5;

      // レイアウトフレーム
      int layout_frame_size = 50;
      int layout_frame_previous_size = 50;
      int layout_frame_default_size = 50;
      int const LAYOUT_FRAME_SIZE_MIN = 5;
      Vector2<int> layout_frame_position;
      Vector2<int> layout_frame_previous_position;
      bool update_layout_frame = false;

      float default_node_size = 2.0f;
      float edge_width_scale = 0.02f;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // ノードの描画

    void draw_node_with_polygon_circle(Vector2<float> const& pos, float z, float size, float const* rgba) {
      glColor4fv(rgba);
      glBegin(GL_TRIANGLE_FAN);
      float const radius = 0.5f*size;
      glVertex2fv(pos.data);
      for (int i=0; i<36; ++i) glVertex2fv((pos+radius*Vector2<float>(COS360[i*10], SIN360[i*10])).data);
      glVertex2fv((pos+Vector2<float>(radius, 0.0f)).data);
      glEnd();
    }

    void draw_node_with_polygon_curve(Vector2<float> const& center, const vector<Vector2<float> const*>& pos, float z, float const* rgba) {
      glColor4fv(rgba);
      glBegin(GL_TRIANGLE_FAN);
      glVertex2fv(center.data);
      multimap<double, size_t> o;
      for (size_t i=0; i<pos.size(); ++i)
        o.insert(make_pair(atan2(pos[i]->y-center.y, pos[i]->x-center.x), i));
      for (multimap<double, size_t>::iterator i=o.begin(); i!=o.end(); ++i)
        glVertex2fv(pos[i->second]->data);
      glVertex2fv(pos[o.begin()->second]->data);
      glEnd();
    }

    void draw_node_with_texture(Vector2<float> const& pos, float z, float size, float const* rgba, float angle, GLTexture const* texture) {
      glColor4fv(rgba);
      glEnable(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, texture->get());
      glPushMatrix();
      glTranslatef(pos.x, pos.y, 0.0f);
      glRotatef(angle, 0.0f, 0.0f, 1.0f);
      glBegin(GL_TRIANGLE_STRIP);
      float const h = 0.5f*size;
      float const w = float(h*texture->width()/texture->height());
      glTexCoord2f(0.0f, 0.0f); glVertex3f(-w, -h, z);
      glTexCoord2f(0.0f, 1.0f); glVertex3f(-w, +h, z);
      glTexCoord2f(1.0f, 0.0f); glVertex3f(+w, -h, z);
      glTexCoord2f(1.0f, 1.0f); glVertex3f(+w, +h, z);
      glEnd();
      glPopMatrix();
      glDisable(GL_TEXTURE_2D);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // エッジの描画

    void draw_edge(Vector2<float> const& pos0, float size0,
                   Vector2<float> const& pos1, float size1,
                   float width, float const* rgba) {
      glColor4fv(rgba);
      RadiusVector2<float> const rv(pos0, pos1);
      Vector2<float> const p0 = pos0+0.5f*size0*rv.u;
      Vector2<float> const p1 = pos1-0.5f*size1*rv.u;
      glLineWidth(width);
      glBegin(GL_LINES);
      glVertex2fv(p0.data);
      glVertex2fv(p1.data);
      glEnd();
    }

//     void draw_vedge(Vector2<float> const& pos0, float size0,
//                     Vector2<float> const& pos1, float size1,
//                     float width, float z, float const* rgba) {
//       glColor4fv(rgba);
//       RadiusVector2<float> const rv(pos0, pos1);
//       Vector2<float> const p0 = pos0+0.5f*size0*rv.u;
//       Vector2<float> const p1 = pos1-0.5f*size1*rv.u;
//       glBegin(GL_TRIANGLE_STRIP);
//       int segments = int(EDGE_SEGMENT_RESOLUTION*rv.norm)+MIN_NUMBER_OF_EDGE_SEGMENTS;
//       if (segments>MAX_NUMBER_OF_EDGE_SEGMENTS) segments = MAX_NUMBER_OF_EDGE_SEGMENTS;
//       glLineWidth(width);
//       glBegin(GL_LINES);
//       for (int i=0; i<segments; ++i) {
//         float const r = float(i)/segments;
//         Vector2<float> const p = (1.0f-r)*p0+r*p1;
//         glVertex3f(p.x, p.y, z*SIN360[int(180*r)]);
//       }
//       glVertex2fv(p1.data);
//       glEnd();
//     }

    void draw_edge(Vector2<float> const& pos0, float size0,
                   Vector2<float> const& pos1, float size1,
                   float width, float dev, float const* rgba) {
      glColor4fv(rgba);
      RadiusVector2<float> const rv(pos0, pos1);
      Vector2<float> const p0 = pos0+0.5f*size0*rv.u;
      Vector2<float> const p1 = pos1-0.5f*size1*rv.u;
      glLineWidth(width);
      glBegin(GL_LINE_STRIP);
      int segments = int(EDGE_SEGMENT_RESOLUTION*rv.norm)+MIN_NUMBER_OF_EDGE_SEGMENTS;
      if (segments>MAX_NUMBER_OF_EDGE_SEGMENTS) segments = MAX_NUMBER_OF_EDGE_SEGMENTS;
      for (int i=0; i<segments; ++i) {
        float const r = float(i)/(segments-1);
        float const d = dev*SIN360[int(180.0*r)];
        Vector2<float> const p((1.0f-r)*p0.x+r*p1.x-d*rv.u.y, (1.0f-r)*p0.y+r*p1.y+d*rv.u.x);
        glVertex2fv(p.data);
      }
      glEnd();
    }

    void draw_loop_edge(Vector2<float> const& pos, float width, float z, float const* rgba) {
      z *= 10.0f;
      glColor4fv(rgba);
      GLfloat const ctrlpoints[5][3] = {
        { pos.x, pos.y, 0.0f },
        { pos.x+z, pos.y, z },
        { pos.x, pos.y, 2.0f*z },
        { pos.x-z, pos.y, z },
        { pos.x, pos.y, 0.0f }
      };
      glMap1f(GL_MAP1_VERTEX_3, 0.0f, 1.0f, 3, 5, &ctrlpoints[0][0]);
      glEnable(GL_MAP1_VERTEX_3);
      glBegin(GL_LINE_STRIP);
      for (int i=0; i<=20; ++i) glEvalCoord1f(float(i)/20.0f);
      glEnd();
    }

    void draw_edge_with_texture(Vector2<float> const& pos0, float size0,
                                Vector2<float> const& pos1, float size1,
                                float width, float const* rgba, GLTexture const* texture) {
      glColor4fv(rgba);
      RadiusVector2<float> const rv(pos0, pos1);
      Vector2<float> const p0 = pos0+0.5f*size0*rv.u;
      Vector2<float> const p1 = pos1-0.5f*size1*rv.u;
      glEnable(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, texture->get());
      glBegin(GL_TRIANGLE_STRIP);
      int segments = int(EDGE_SEGMENT_RESOLUTION*rv.norm)+MIN_NUMBER_OF_EDGE_SEGMENTS;
      if (segments>MAX_NUMBER_OF_EDGE_SEGMENTS) segments = MAX_NUMBER_OF_EDGE_SEGMENTS;
      Vector2<float> q0_prev;
      Vector2<float> q1_prev;
      float const f = float(segments-1);
      for (int i=0; i<segments; ++i) {
        float const r = i/f;
        Vector2<float> const pos((1.0f-r)*p0+r*p1);
        //float const w = width*(i*(i-f)/(f*f));
        float const w = width;
        Vector2<float> const b(w*rv.u.y, -w*rv.u.x);
        Vector2<float> const q0 = pos+b;
        Vector2<float> const q1 = pos-b;
        if (i>0) {
          glBegin(GL_TRIANGLE_STRIP);
          glTexCoord2f(0.0f, 0.0f); glVertex2fv(q0_prev.data);
          glTexCoord2f(0.0f, 1.0f); glVertex2fv(q0.data);
          glTexCoord2f(1.0f, 0.0f); glVertex2fv(q1_prev.data);
          glTexCoord2f(1.0f, 1.0f); glVertex2fv(q1.data);
          glEnd();
        }
        q0_prev = q0;
        q1_prev = q1;
      }
      glEnd();
      glDisable(GL_TEXTURE_2D);
    }


    void draw_directed_edge_with_texture(Vector2<float> const& pos0, float size0,
                                         Vector2<float> const& pos1, float size1,
                                         float width, float const* rgba, GLTexture const* texture) {
      glColor4fv(rgba);
      Vector2<double> p0 = pos0.dcast();
      Vector2<double> p1 = pos1.dcast();
      RadiusVector2<double> rv(p0, p1);
      Vector2<double> const c = 0.5*(p0+p1)+2.0*rv.norm*rv.n;
      double r = (p0-c).norm();
      if (r<1e-5) r = 1e-5;
      Vector2<double> u = (p0-c)/r;
      double const tip_size = 2.0*width;
      double const radian0 = 2.0*asin(0.25*size0/r);
      double const radian1 = 2.0*(asin(0.5*rv.norm/r)-asin((0.25*size1)/r));
      double const radian2 = 2.0*(asin(0.5*rv.norm/r)-asin((0.25*size1+tip_size)/r));
      double const dr = (radian2-radian0)/10.0;
      Vector2<double> const ut1 = u.rot(-radian1);
      Vector2<double> const ut2 = u.rot(-radian2);
      Vector2<double> const pt0 = c+(r-tip_size)*ut2;
      Vector2<double> const pt1 = c+(r+tip_size)*ut2;
      Vector2<double> const pt2 = c+r*ut1;
      glBegin(GL_TRIANGLE_STRIP);
      glVertex2dv(pt0.data);
      glVertex2dv(pt1.data);
      glVertex2dv(pt2.data);
      glEnd();
      glBindTexture(GL_TEXTURE_2D, texture->get());
      glBegin(GL_TRIANGLE_STRIP);
      u = u.rot(-radian0);
      for (int i=0; i<10; ++i) {
        Vector2<double> const q0 = c+(r-0.5*width)*u;
        Vector2<double> const q1 = c+(r+0.5*width)*u;
        u = u.rot(-dr);
        Vector2<double> const q2 = c+(r-0.5*width)*u;
        Vector2<double> const q3 = c+(r+0.5*width)*u;
        glTexCoord2f(0.0f, 0.0f); glVertex2dv(q0.data);
        glTexCoord2f(1.0f, 0.0f); glVertex2dv(q1.data);
        glTexCoord2f(0.0f, 1.0f); glVertex2dv(q2.data);
        glTexCoord2f(1.0f, 1.0f); glVertex2dv(q3.data);
      }
      glEnd();
      glDisable(GL_TEXTURE_2D);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // ラベルの描画

    void draw_node_label(wchar_t const* label, Vector2<float> const& pos, float z, float size, float const* rgba, float angle) {
      glColor4fv(rgba);
      shared_ptr<FTFont> const& f = sociarium_project_font::get_node_font();
      float llx, lly, llz, urx, ury, urz;
      f->BBox(label, llx, lly, llz, urx, ury, urz);
      float const w = 0.5f*size*(urx-llx);
      float const h =  0.5f*size*(ury-lly);
      glPushMatrix();
      glTranslatef(pos.x, pos.y, z);
      glRotatef(angle, 0.0f, 0.0f, 1.0f);
      glTranslatef(-w, -h, 0.0f);
      glScalef(size, size, 0.0f);
      f->Render(label);
      glPopMatrix();
    }

    void draw_edge_label(wchar_t const* label,
                         Vector2<float> const& pos0, float size0,
                         Vector2<float> const& pos1, float size1,
                         float size, float const* rgba, float angle) {
      glColor4fv(rgba);
      shared_ptr<FTFont> const& f = sociarium_project_font::get_edge_font();
      RadiusVector2<float> const rv(pos0, pos1);
      Vector2<float> const p0 = pos0+0.5f*size0*rv.u;
      Vector2<float> const p1 = pos1-0.5f*size1*rv.u;
      float llx, lly, llz, urx, ury, urz;
      f->BBox(label, llx, lly, llz, urx, ury, urz);
      float const w = 0.5f*size*(urx-llx);
      float const h = 0.5f*size*(ury-lly);
      Vector2<float> const pos = 0.4f*p0+0.6f*p1;
      glPushMatrix();
      glTranslatef(pos.x, pos.y, 0.0f);
      glRotatef(angle, 0.0f, 0.0f, 1.0f);
      glTranslatef(-w, -h, 0.0f);
      glScalef(size, size, 0.0f);
      f->Render(label);
      glPopMatrix();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // その他

    Vector2<double> const& get_slider_offset(void) { return slider_offset; }
    Vector2<double> const& get_selection_frame_offset(void) { return selection_frame_offset; }

    int get_grid_resolution(void) { return grid_resolution; }
    void set_grid_resolution(int resolution) { grid_resolution = resolution; }

    int get_layout_frame_size(void) { return layout_frame_size; }
    void set_layout_frame_size(int size) {
      size = (size/grid_resolution)*grid_resolution;
      layout_frame_size = size<LAYOUT_FRAME_SIZE_MIN?LAYOUT_FRAME_SIZE_MIN:size;
    }

    int get_layout_frame_previous_size(void) { return layout_frame_previous_size; }
    void set_layout_frame_previous_size(int size) {
      size = (size/grid_resolution)*grid_resolution;
      layout_frame_previous_size = size;
    }

    Vector2<int> const& get_layout_frame_position(void) { return layout_frame_position; }
    void set_layout_frame_position(Vector2<int> const& position) {
      layout_frame_position.set((position.x/grid_resolution)*grid_resolution, (position.y/grid_resolution)*grid_resolution);
    }

    Vector2<int> const& get_layout_frame_previous_position(void) { return layout_frame_previous_position; }
    void set_layout_frame_previous_position(Vector2<int> const& position) {
      layout_frame_previous_position.set(position);
    }

    bool get_update_layout_frame(void) { return update_layout_frame; }
    void set_update_layout_frame(bool b) { update_layout_frame = b; }

    int get_layout_frame_default_size(void) { return layout_frame_default_size; }
    void set_layout_frame_default_size(int size) {
      size = (size/grid_resolution)*grid_resolution;
      layout_frame_default_size = size<LAYOUT_FRAME_SIZE_MIN?LAYOUT_FRAME_SIZE_MIN:size;
    }

    float get_default_node_size(void) { return default_node_size; }
    void set_default_node_size(float size) { default_node_size = size; }

    float get_edge_width_scale(void) { return edge_width_scale; }
    void set_edge_width_scale(float scale) { edge_width_scale = scale; }

  } // The end of the namespace "sociarium_project_draw"

} // The end of the namespace "hashimoto_ut"

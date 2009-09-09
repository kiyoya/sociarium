// s.o.c.i.a.r.i.u.m: world_impl_draw_perspective_part.cpp
// HASHIMOTO, Yasuhiro (E-mail: hy @ sys.t.u-tokyo.ac.jp)

/* Copyright (c) 2005-2009, HASHIMOTO, Yasuhiro, All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   - Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   - Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   - Neither the name of the University of Tokyo nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <memory>
#include <windows.h>
#include <GL/gl.h>
#include <FTGL/ftgl.h>
#include "color.h"
#include "font.h"
#include "view.h"
#include "draw.h"
#include "layout.h"
#include "selection.h"
#include "texture.h"
#include "sociarium_graph.h"
#include "../shared/vector3.h"
#include "../shared/math.h"
#include "../shared/predefined_color.h"
#include "../shared/gl/gltexture.h"

namespace hashimoto_ut {

  using std::vector;
  using std::tr1::shared_ptr;
  using std::tr1::array;

  using namespace sociarium_project_color;
  using namespace sociarium_project_draw;
  using namespace sociarium_project_layout;
  using namespace sociarium_project_selection;

  namespace sociarium_project_draw_detail {

    namespace {

      float const EDGE_SEGMENT_RESOLUTION = 0.5f;
      int const MIN_NUMBER_OF_EDGE_SEGMENTS = 3; // must be larger than or equal to 3.
      int const MAX_NUMBER_OF_EDGE_SEGMENTS = 30;

      float const loop_edge_height = 0.2f;

      float const base_alpha = 0.7f;

    } // The end of the anonymous namespace


    ////////////////////////////////////////////////////////////////////////////////
    // Draw the layout frame.
    void draw_layout_frame(void) {

      float const size = get_layout_frame_size();
      Vector2<float> const& pos = get_layout_frame_position();

      float const left   = pos.x-size;
      float const right  = pos.x+size;
      float const top    = pos.y-size;
      float const bottom = pos.y+size;

      if (is_selected(SelectionCategory::LAYOUT_FRAME_BORDER))
        glColor4fv(get_color(ColorCategory::LAYOUT_FRAME_BORDER).data());
      else glColor4f(0.0f, 0.0f, 0.0f, 0.0f);

      glLineWidth(1.0f);
      glLoadName(SelectionCategory::LAYOUT_FRAME_BORDER);
      glPushName(SelectionCategory::FrameBorder::LEFT);
      glBegin(GL_LINES);
      glVertex2f(left, bottom);
      glVertex2f(left, top);
      glEnd();
      glPopName();
      glPushName(SelectionCategory::FrameBorder::RIGHT);
      glBegin(GL_LINES);
      glVertex2f(right, bottom);
      glVertex2f(right, top);
      glEnd();
      glPopName();
      glPushName(SelectionCategory::FrameBorder::BOTTOM);
      glBegin(GL_LINES);
      glVertex2f(right, bottom);
      glVertex2f(left, bottom);
      glEnd();
      glPopName();
      glPushName(SelectionCategory::FrameBorder::TOP);
      glBegin(GL_LINES);
      glVertex2f(right, top);
      glVertex2f(left, top);
      glEnd();
      glPopName();
    }


    ////////////////////////////////////////////////////////////////////////////////
    // Draw a surface of the layout frame.
    void draw_layout_frame_area(Vector2<float> const& pos, float size) {

      float const left   = pos.x-size;
      float const right  = pos.x+size;
      float const top    = pos.y-size;
      float const bottom = pos.y+size;

      glColor4fv(get_color(ColorCategory::LAYOUT_FRAME_AREA).data());
      glLoadName(SelectionCategory::LAYOUT_FRAME);
      glPushName(0);
      glBegin(GL_TRIANGLE_STRIP);
      glVertex2f(left, bottom);
      glVertex2f(left, top);
      glVertex2f(right, bottom);
      glVertex2f(right, top);
      glEnd();
      glPopName();
    }


    ////////////////////////////////////////////////////////////////////////////////
    // Draw the grid.
    void draw_grid(void) {
      glLineWidth(1.0f);
      glColor4fv(get_color(ColorCategory::GRID).data());
      float const grid_interval = get_grid_interval();
      float const range = 500.0f;
      int const num = int(range/grid_interval);

      glBegin(GL_LINE_STRIP);
      glVertex2f(-range, -range);
      glVertex2f(-range, +range);
      glVertex2f(+range, +range);
      glVertex2f(+range, -range);
      glEnd();

      glBegin(GL_LINES);
      for (int i=0; i<=num; ++i) {
        float const ii = i*grid_interval;
        glVertex2f(ii, -range);
        glVertex2f(ii, +range);
        glVertex2f(-ii, -range);
        glVertex2f(-ii, +range);
        glVertex2f(-range, ii);
        glVertex2f(+range, ii);
        glVertex2f(-range, -ii);
        glVertex2f(+range, -ii);
      }
      glEnd();
    }


    ////////////////////////////////////////////////////////////////////////////////
    // Draw the reference point and the center point.
    void draw_center(Vector2<float> const& center) {

      float const size = get_coordinates_size();

      // The reference point.
      glLineWidth(1.0f);
      glBegin(GL_LINES);
      glColor4f(1.0f, 0.0f, 0.0f, 0.9f);
      glVertex3f(-center.x-size, -center.y, 0.0f);
      glVertex3f(-center.x+size, -center.y, 0.0f);
      glColor4f(0.0f, 1.0f, 0.0f, 0.9f);
      glVertex3f(-center.x, -center.y-size, 0.0f);
      glVertex3f(-center.x, -center.y+size, 0.0f);
      glColor4f(0.0f, 0.0f, 1.0f, 0.9f);
      glVertex3f(-center.x, -center.y, -size);
      glVertex3f(-center.x, -center.y, +size);
      glEnd();

      // The center point.
      glLineWidth(1.0f);
      glBegin(GL_LINES);
      glColor4f(0.5f, 0.5f, 0.5f, 0.9f);
      glVertex3f(-size, 0.0f, 0.0f);
      glVertex3f(+size, 0.0f, 0.0f);
      glVertex3f(0.0f, -size, 0.0f);
      glVertex3f(0.0f, +size, 0.0f);
      glVertex3f(0.0f, 0.0f, -size);
      glVertex3f(0.0f, 0.0f, +size);
      glEnd();
    }


    ////////////////////////////////////////////////////////////////////////////////
    void set_color(Vector3<float> const& rgb, int flag, bool is_selected) {
      static unsigned int MARKED = ElementFlag::MARKED|ElementFlag::TEMPORARY_MARKED;
      array<float, 4> rgba = { rgb.x, rgb.y, rgb.z, base_alpha };

      if (flag&ElementFlag::HIGHLIGHT) highlight_effect(rgba);
      else if (flag&ElementFlag::TEMPORARY_UNMARKED);
      else if (flag&MARKED) marking_effect(rgba);

      if (is_selected) selection_effect(rgba);

      glColor4fv(rgba.data());
    }


    ////////////////////////////////////////////////////////////////////////////////
    // Node drawing.

    // --------------------------------------------------------------------------------
    void draw_node_with_polygon_circle(
      DynamicNodeProperty const& dnp, bool is_selected) {

      if (is_hidden(dnp)) return;

      StaticNodeProperty* snp = dnp.get_static_property();
      Vector2<float> const& pos = snp->get_position();
      float const radius = 0.5f*get_node_size()*dnp.get_size();

      glPushMatrix();
      glTranslatef(pos.x, pos.y, 0.0f);
      Node const* n = dnp.get_graph_element();

      set_color(predefined_color[dnp.get_color_id()], dnp.get_flag(), is_selected);

      glBegin(GL_TRIANGLE_FAN);
      glVertex2f(0.0f, 0.0f);
      for (int i=0; i<360; i+=10)
        glVertex2fv((radius*Vector2<float>(COS360[i], SIN360[i])).data);
      glVertex2f(radius, 0.0f);
      glEnd();
      glPopMatrix();
    }

    // --------------------------------------------------------------------------------
    void draw_node_with_polygon_circle2(
      DynamicNodeProperty const& dnp, bool is_selected) {

      if (is_hidden(dnp)) return;

      StaticNodeProperty* snp = dnp.get_static_property();
      Vector2<float> const& pos = snp->get_position();
      float const radius = 0.5f*get_node_size()*dnp.get_size();

      glPushMatrix();
      glTranslatef(pos.x, pos.y, 0.0f);

      Node const* n = dnp.get_graph_element();

      glBegin(GL_TRIANGLE_FAN);

      glColor4fv(get_color(ColorCategory::BACKGROUND).data());
      glVertex2f(0.0f, 0.0f);

      set_color(predefined_color[dnp.get_color_id()], dnp.get_flag(), is_selected);

      for (int i=0; i<360; i+=10)
        glVertex2fv((radius*Vector2<float>(COS360[i], SIN360[i])).data);
      glVertex2f(radius, 0.0f);
      glEnd();
      glPopMatrix();
    }


    // --------------------------------------------------------------------------------
    void draw_node_with_texture(
      DynamicNodeProperty const& dnp, float angleH, float angleV, bool is_selected) {

      if (is_hidden(dnp)) return;

      StaticNodeProperty* snp = dnp.get_static_property();
      Vector2<float> const& pos = snp->get_position();
      GLTexture const* texture = snp->get_texture();
      float const aspect = float(texture->width())/texture->height();
      float const size = 0.5f*get_node_size()*dnp.get_size();

      Vector2<float> const sz(aspect<1.0f?Vector2<float>(aspect*size, size)
                              :Vector2<float>(size, size/aspect));

      Node const* n = dnp.get_graph_element();

      set_color(predefined_color[dnp.get_color_id()], dnp.get_flag(), is_selected);

      glPushMatrix();
      glEnable(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, texture->get());
      glTranslatef(pos.x, pos.y, 0.0f);
      glRotatef(angleH, 0.0f, 0.0f, 1.0f);
      glRotatef(-angleV, 1.0f, 0.0f, 0.0f);
      glBegin(GL_TRIANGLE_STRIP);
      glTexCoord2f(0.0f, 0.0f);
      glVertex3f(-sz.x, -sz.y, 0.0f);
      glTexCoord2f(0.0f, texture->ycoord());
      glVertex3f(-sz.x, +sz.y, 0.0f);
      glTexCoord2f(texture->xcoord(), 0.0f);
      glVertex3f(+sz.x, -sz.y, 0.0f);
      glTexCoord2f(texture->xcoord(), texture->ycoord());
      glVertex3f(+sz.x, +sz.y, 0.0f);
      glEnd();
      glDisable(GL_TEXTURE_2D);
      glPopMatrix();
    }


    ////////////////////////////////////////////////////////////////////////////////
    // Edge drawing.

    // --------------------------------------------------------------------------------
    void draw_loop_edge(
      DynamicEdgeProperty const& dep, SociariumGraph const* g, bool is_selected) {

      Edge const* e = dep.get_graph_element();
      DynamicNodeProperty const& dnp = g->property(e->source());
      Vector2<float> const& pos = dnp.get_static_property()->get_position();

      set_color(predefined_color[dep.get_color_id()], dep.get_flag(), is_selected);

      float const width = is_selected?3.0f:1.0f;

      float const z = loop_edge_height*10.0f;
      GLfloat const ctrlpoints[5][3] = {
        { pos.x, pos.y, 0.0f },
        { pos.x+z, pos.y, z },
        { pos.x, pos.y, 2.0f*z },
        { pos.x-z, pos.y, z },
        { pos.x, pos.y, 0.0f }
      };

      glLineWidth(width);
      glMap1f(GL_MAP1_VERTEX_3, 0.0f, 1.0f, 3, 5, &ctrlpoints[0][0]);
      glEnable(GL_MAP1_VERTEX_3);
      glBegin(GL_LINE_STRIP);
      for (int i=0; i<=20; ++i)
        glEvalCoord1f(float(i)/20.0f);
      glEnd();
    }

    // --------------------------------------------------------------------------------
    void draw_directed_edge(
      DynamicEdgeProperty const& dep, SociariumGraph const* g, bool is_selected) {

      if (is_hidden(dep)) return;

      Edge const* e = dep.get_graph_element();

      if (e->source()==e->target())
        return draw_loop_edge(dep, g, is_selected);

      DynamicNodeProperty const& dnp0 = g->property(e->source());
      DynamicNodeProperty const& dnp1 = g->property(e->target());

      set_color(predefined_color[dep.get_color_id()], dep.get_flag(), is_selected);

      float const width = is_selected?3.0f:1.0f;
      float const size0 = get_node_size()*dnp0.get_size();
      float const size1 = get_node_size()*dnp1.get_size();

      Vector2<float> const& pos0 = dnp0.get_static_property()->get_position();
      Vector2<float> const& pos1 = dnp1.get_static_property()->get_position();
      RadiusVector2<float> const rv(pos0, pos1);
      Vector2<float> const c = 0.5f*(pos0+pos1)+2.0f*float(rv.norm)*rv.n; // Center of the arc.
      float const r = float((pos0-c).norm());                             // Radius of the arc.

      if (r<0.5f*rv.norm || r<0.25f*size0 || r<0.25f*size1) {
        Vector2<float> const p0 = pos0+0.5f*size0*rv.u;
        Vector2<float> const p1 = pos1-0.5f*size1*rv.u;
        glLineWidth(width);
        glBegin(GL_LINES);
        glVertex2fv(p0.data);
        glVertex2fv(p1.data);
        glEnd();
      } else {
        int segments = int(EDGE_SEGMENT_RESOLUTION*rv.norm)+MIN_NUMBER_OF_EDGE_SEGMENTS;
        if (segments>MAX_NUMBER_OF_EDGE_SEGMENTS)
          segments = MAX_NUMBER_OF_EDGE_SEGMENTS;

        float const radian0 = 2.0f*asinf(0.25f*size0/r);
        float const radian1 = 2.0f*(asinf(0.5f*float(rv.norm)/r)-asinf((0.25f*size1)/r));
        float const dr = (radian1-radian0)/segments;
        Vector2<float> u = (pos0-c)/r;
        u = u.rot(-radian0);
        glLineWidth(width);
        glBegin(GL_LINE_STRIP);
        for (int i=0; i<=segments; ++i)
          glVertex2fv((c+r*u.rot(-dr*i).fcast()).data);
        glEnd();
      }
    }

    // --------------------------------------------------------------------------------
    void draw_undirected_edge(
      DynamicEdgeProperty const& dep, SociariumGraph const* g, bool is_selected) {

      if (is_hidden(dep)) return;

      Edge const* e = dep.get_graph_element();

      if (e->source()==e->target())
        return draw_loop_edge(dep, g, is_selected);

      DynamicNodeProperty const& dnp0 = g->property(e->source());
      DynamicNodeProperty const& dnp1 = g->property(e->target());

      set_color(predefined_color[dep.get_color_id()], dep.get_flag(), is_selected);

      float const width = is_selected?3.0f:1.0f;
      float const size0 = get_node_size()*dnp0.get_size();
      float const size1 = get_node_size()*dnp1.get_size();

      Vector2<float> const& pos0 = dnp0.get_static_property()->get_position();
      Vector2<float> const& pos1 = dnp1.get_static_property()->get_position();
      RadiusVector2<float> const rv(pos0, pos1);
      Vector2<float> const p0 = pos0+0.5f*size0*rv.u;
      Vector2<float> const p1 = pos1-0.5f*size1*rv.u;

      glLineWidth(width);
      glBegin(GL_LINES);
      glVertex2fv(p0.data);
      glVertex2fv(p1.data);
      glEnd();
    }

    // --------------------------------------------------------------------------------
    void draw_directed_edge_with_polygon(
      DynamicEdgeProperty const& dep, SociariumGraph const* g, bool is_selected) {

      if (is_hidden(dep)) return;

      Edge const* e = dep.get_graph_element();

      if (e->source()==e->target())
        return draw_loop_edge(dep, g, is_selected);

      DynamicNodeProperty const& dnp0 = g->property(e->source());
      DynamicNodeProperty const& dnp1 = g->property(e->target());

      set_color(predefined_color[dep.get_color_id()], dep.get_flag(), is_selected);

      float const width = (is_selected?1.3f:1.0f)*get_edge_width()*dep.get_width();
      float const size0 = get_node_size()*dnp0.get_size();
      float const size1 = get_node_size()*dnp1.get_size();
      float const tip_size = 4.0f*width;

      Vector2<float> const& pos0 = dnp0.get_static_property()->get_position();
      Vector2<float> const& pos1 = dnp1.get_static_property()->get_position();
      RadiusVector2<float> const rv(pos0, pos1);
      Vector2<float> const c = 0.5f*(pos0+pos1)+2.0f*float(rv.norm)*rv.n; // Center of the arc.
      float const r = float((pos0-c).norm());                             // Radius or the arc.

      int segments = int(EDGE_SEGMENT_RESOLUTION*rv.norm)+MIN_NUMBER_OF_EDGE_SEGMENTS;
      if (segments>MAX_NUMBER_OF_EDGE_SEGMENTS)
        segments = MAX_NUMBER_OF_EDGE_SEGMENTS;

      if (r<0.5f*rv.norm || r<0.25f*size0 || r<0.25f*size1+tip_size) {
        // Draw a straight line without a tip.
        Vector2<float> const p0 = pos0+0.5f*size0*rv.u;
        Vector2<float> const p1 = pos1-0.5f*size1*rv.u;
        Vector2<float> const b(width*rv.u.y, -width*rv.u.x);
        glBegin(GL_TRIANGLE_STRIP);
        Vector2<float> const q0 = p0+b;
        Vector2<float> const q1 = p0-b;
        Vector2<float> const q2 = p1+b;
        Vector2<float> const q3 = p1-b;
        glVertex2fv(q0.data);
        glVertex2fv(q1.data);
        glVertex2fv(q2.data);
        glVertex2fv(q3.data);
        glEnd();
      } else {
        // Draw a curved line with a tip.
        float const rr = 0.5f*float(rv.norm)/r;
        float const radian0 = 2.0f*asinf(0.25f*size0/r);
        float const radian1 = 2.0f*(asinf(rr)-asinf((0.25f*size1)/r));
        float const radian2 = 2.0f*(asinf(rr)-asinf((0.25f*size1+tip_size)/r));
        float const dr = (radian2-radian0)/segments;
        Vector2<float> u = (pos0-c)/r;
        Vector2<float> const ut1 = u.rot(-radian1);
        Vector2<float> const ut2 = u.rot(-radian2);
        glPushMatrix();
        glTranslatef(c.x, c.y, 0.0f);
        // Draw a tip.
        glBegin(GL_TRIANGLE_STRIP);
        glVertex2fv(((r-tip_size)*ut2).data);
        glVertex2fv(((r+tip_size)*ut2).data);
        glVertex2fv((r*ut1).data);
        glEnd();
        // Draw a line.
        u = u.rot(-radian0);
        float const rmw = r-width;
        float const rpw = r+width;
        glBegin(GL_TRIANGLE_STRIP);
        for (int i=0; i<segments; ++i) {
          glVertex2fv((rmw*u).data);
          glVertex2fv((rpw*u).data);
          u = u.rot(-dr);
          glVertex2fv((rmw*u).data);
          glVertex2fv((rpw*u).data);
        }
        glEnd();
        glPopMatrix();
      }
    }

    // --------------------------------------------------------------------------------
    void draw_undirected_edge_with_polygon(
      DynamicEdgeProperty const& dep, SociariumGraph const* g, bool is_selected) {

      if (is_hidden(dep)) return;

      Edge const* e = dep.get_graph_element();

      if (e->source()==e->target())
        return draw_loop_edge(dep, g, is_selected);

      DynamicNodeProperty const& dnp0 = g->property(e->source());
      DynamicNodeProperty const& dnp1 = g->property(e->target());

      set_color(predefined_color[dep.get_color_id()], dep.get_flag(), is_selected);

      float const width = (is_selected?1.3f:1.0f)*get_edge_width()*dep.get_width();
      float const size0 = get_node_size()*dnp0.get_size();
      float const size1 = get_node_size()*dnp1.get_size();

      Vector2<float> const& pos0 = dnp0.get_static_property()->get_position();
      Vector2<float> const& pos1 = dnp1.get_static_property()->get_position();
      RadiusVector2<float> const rv(pos0, pos1);
      Vector2<float> const p0 = pos0+0.5f*size0*rv.u;
      Vector2<float> const p1 = pos1-0.5f*size1*rv.u;

      Vector2<float> const b(width*rv.u.y, -width*rv.u.x);
      glBegin(GL_TRIANGLE_STRIP);
      Vector2<float> const q0 = p0+b;
      Vector2<float> const q1 = p0-b;
      Vector2<float> const q2 = p1+b;
      Vector2<float> const q3 = p1-b;
      glVertex2fv(q0.data);
      glVertex2fv(q1.data);
      glVertex2fv(q2.data);
      glVertex2fv(q3.data);
      glEnd();
    }


    ////////////////////////////////////////////////////////////////////////////////
    // Node name drawing.

    // --------------------------------------------------------------------------------
    void draw_node_name(
      DynamicNodeProperty const& dnp,
      float scale, float angleH, float angleV) {

      using namespace sociarium_project_font;
      using namespace sociarium_project_view;

      if (is_hidden(dnp)) return;

      StaticNodeProperty const* snp = dnp.get_static_property();

      // Set color.
      if (dnp.get_color_id()==get_default_node_color_id()
          || dnp.get_color_id()==PredefinedColor::WHITE) {
        Vector3<float> const& rgb = predefined_color[get_default_node_name_color_id()];
        glColor4f(rgb.x, rgb.y, rgb.z, 0.6f);
      } else {
        Vector3<float> const& rgb = predefined_color[dnp.get_color_id()];
        glColor4f(0.6f*rgb.x, 0.6f*rgb.y, 0.6f*rgb.z, 0.6f);
      }

      // Set size.
      float const size = get_node_name_size_variable()?dnp.get_size()*scale:scale;
      shared_ptr<FTFont> f = get_font(FontCategory::NODE_NAME);
      float llx, lly, llz, urx, ury, urz;
      wchar_t const* name = snp->get_name().c_str();
      f->BBox(name, llx, lly, llz, urx, ury, urz);
      float const w = 0.5f*size*(urx-llx);
      float const h = 0.5f*size*(ury-lly);

      // Set position.
      Vector2<float> const& pos = snp->get_position();

      // Draw.
      glPushMatrix();
      glTranslatef(pos.x, pos.y, 0.0f);
      glRotatef(angleH, 0.0f, 0.0f, 1.0f);
      glRotatef(-angleV, 1.0f, 0.0f, 0.0f);
      glTranslatef(-w, -h, 0.0f);
      glScalef(size, size, 0.0f);
      f->Render(name);
      glPopMatrix();
    }


    ////////////////////////////////////////////////////////////////////////////////
    // Edge name drawing.

    // --------------------------------------------------------------------------------
    void draw_loop_edge_name(
      DynamicEdgeProperty const& dep, SociariumGraph const* g,
      float scale, float angleH, float angleV) {

      using namespace sociarium_project_font;
      using namespace sociarium_project_view;

      Edge const* e = dep.get_graph_element();
      DynamicNodeProperty const& dnp = g->property(e->source());
      StaticEdgeProperty const* sep = dep.get_static_property();

      // Set color.
      if (dep.get_color_id()==get_default_edge_color_id()) {
        Vector3<float> const& rgb = predefined_color[get_default_edge_name_color_id()];
        glColor4f(rgb.x, rgb.y, rgb.z, 0.6f);
      } else {
        Vector3<float> const& rgb = predefined_color[dep.get_color_id()];
        glColor4f(0.6f*rgb.x, 0.6f*rgb.y, 0.6f*rgb.z, 0.6f);
      }

      // Set size.
      wchar_t const* name = sep->get_name().c_str();
      float const size = get_edge_name_size_variable()?dep.get_width()*scale:scale;
      shared_ptr<FTFont> f = get_font(FontCategory::EDGE_NAME);
      float llx, lly, llz, urx, ury, urz;
      f->BBox(name, llx, lly, llz, urx, ury, urz);
      float const w = 0.5f*size*(urx-llx);
      float const h = 0.5f*size*(ury-lly);

      // Set position.
      Vector2<float> const& pos = dnp.get_static_property()->get_position();
      float const z = loop_edge_height*10.0f;

      // Draw.
      glPushMatrix();
      glTranslatef(pos.x, pos.y, z);
      glRotatef(angleH, 0.0f, 0.0f, 1.0f);
      glRotatef(-angleV, 1.0f, 0.0f, 0.0f);
      glTranslatef(-w, -h, 0.0f);
      glScalef(size, size, 0.0f);
      f->Render(name);
      glPopMatrix();
    }

    // --------------------------------------------------------------------------------
    void draw_directed_edge_name(
      DynamicEdgeProperty const& dep, SociariumGraph const* g,
      float scale, float angleH, float angleV) {

      using namespace sociarium_project_font;
      using namespace sociarium_project_view;

      if (is_hidden(dep)) return;

      Edge const* e = dep.get_graph_element();

      if (e->source()==e->target())
        return draw_loop_edge_name(dep, g, scale, angleH, angleV);

      StaticEdgeProperty const* sep = dep.get_static_property();
      DynamicNodeProperty const& dnp0 = g->property(e->source());
      DynamicNodeProperty const& dnp1 = g->property(e->target());

      // Set color.
      if (dep.get_color_id()==get_default_edge_color_id()) {
        Vector3<float> const& rgb = predefined_color[get_default_edge_name_color_id()];
        glColor4f(rgb.x, rgb.y, rgb.z, 0.6f);
      } else {
        Vector3<float> const& rgb = predefined_color[dep.get_color_id()];
        glColor4f(0.6f*rgb.x, 0.6f*rgb.y, 0.6f*rgb.z, 0.6f);
      }

      // Set size.
      wchar_t const* name = sep->get_name().c_str();
      float const size = get_edge_name_size_variable()?dep.get_width()*scale:scale;
      shared_ptr<FTFont> f = get_font(FontCategory::EDGE_NAME);
      float llx, lly, llz, urx, ury, urz;
      f->BBox(name, llx, lly, llz, urx, ury, urz);
      float const w = 0.5f*size*(urx-llx);
      float const h = 0.5f*size*(ury-lly);

      // Set position.
      Vector2<float> const& pos0 = dnp0.get_static_property()->get_position();
      Vector2<float> const& pos1 = dnp1.get_static_property()->get_position();
      RadiusVector2<float> const rv(pos0, pos1);
      float const size0 = dnp0.get_size();
      float const size1 = dnp1.get_size();
      Vector2<float> const p0 = pos0+0.5f*size0*rv.u;
      Vector2<float> const p1 = pos1-0.5f*size1*rv.u;
      Vector2<float> const c = 0.5f*(p0+p1);
      float const dev = (rv.norm==0.0)?0.0f:0.0616f; // sqrt(2*2+0.5*0.5)-2

      // Draw.
      glPushMatrix();
      glTranslatef(c.x-dev*rv.v.y, c.y+dev*rv.v.x, 0.0f);
      glRotatef(angleH, 0.0f, 0.0f, 1.0f);
      glRotatef(-angleV, 1.0f, 0.0f, 0.0f);
      glTranslatef(-w, -h, 0.0f);
      glScalef(size, size, 0.0f);
      f->Render(name);
      glPopMatrix();
    }

    // --------------------------------------------------------------------------------
    void draw_undirected_edge_name(
      DynamicEdgeProperty const& dep, SociariumGraph const* g,
      float scale, float angleH, float angleV) {

      using namespace sociarium_project_font;
      using namespace sociarium_project_view;

      if (is_hidden(dep)) return;

      Edge const* e = dep.get_graph_element();

      if (e->source()==e->target())
        return draw_loop_edge_name(dep, g, scale, angleH, angleV);

      StaticEdgeProperty const* sep = dep.get_static_property();
      DynamicNodeProperty const& dnp0 = g->property(e->source());
      DynamicNodeProperty const& dnp1 = g->property(e->target());

      // Set color.
      if (dep.get_color_id()==get_default_edge_color_id()) {
        Vector3<float> const& rgb = predefined_color[get_default_edge_name_color_id()];
        glColor4f(rgb.x, rgb.y, rgb.z, 0.6f);
      } else {
        Vector3<float> const& rgb = predefined_color[dep.get_color_id()];
        glColor4f(0.6f*rgb.x, 0.6f*rgb.y, 0.6f*rgb.z, 0.6f);
      }

      // Set size.
      wchar_t const* name = sep->get_name().c_str();
      float const size = get_edge_name_size_variable()?dep.get_width()*scale:scale;
      shared_ptr<FTFont> f = get_font(FontCategory::EDGE_NAME);
      float llx, lly, llz, urx, ury, urz;
      f->BBox(name, llx, lly, llz, urx, ury, urz);
      float const w = 0.5f*size*(urx-llx);
      float const h = 0.5f*size*(ury-lly);

      // Set position.
      Vector2<float> const& pos0 = dnp0.get_static_property()->get_position();
      Vector2<float> const& pos1 = dnp1.get_static_property()->get_position();
      RadiusVector2<float> const rv(pos0, pos1);
      float const size0 = dnp0.get_size();
      float const size1 = dnp1.get_size();
      Vector2<float> const p0 = pos0+0.5f*size0*rv.u;
      Vector2<float> const p1 = pos1-0.5f*size1*rv.u;
      Vector2<float> const c = 0.5f*(p0+p1);

      // Draw.
      glPushMatrix();
      glTranslatef(c.x, c.y, 0.0f);
      glRotatef(angleH, 0.0f, 0.0f, 1.0f);
      glRotatef(-angleV, 1.0f, 0.0f, 0.0f);
      glTranslatef(-w, -h, 0.0f);
      glScalef(size, size, 0.0f);
      f->Render(name);
      glPopMatrix();
    }

  } // The end of the namespace "sociarium_project_draw_detail"

} // The end of the namespace "hashimoto_ut"

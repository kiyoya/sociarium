// s.o.c.i.a.r.i.u.m: world_impl_draw.cpp
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

#include <cassert>
#include <windows.h>
#include <GL/gl.h>
#include "world_impl.h"
#include "common.h"
#include "draw.h"
#include "layout.h"
#include "view.h"
#include "color.h"
#include "font.h"
#include "selection.h"
#include "fps_manager.h"
#include "sociarium_graph_time_series.h"
#include "../shared/GL/glview.h"

#pragma comment(lib, "winmm.lib")

namespace hashimoto_ut {

  using std::tr1::shared_ptr;
  using std::tr1::array;

  class CommunityTransitionDiagram;

  typedef SociariumGraph::node_property_iterator node_property_iterator;
  typedef SociariumGraph::edge_property_iterator edge_property_iterator;

  namespace sociarium_project_draw_detail {

    ////////////////////////////////////////////////////////////////////////////////
    // world_impl_draw_orthogonal_part.cpp
    void draw_selection_result(Vector2<float> const& scale);
    void draw_layer_name(Vector2<float> const& scale);
    void draw_time_slider(Vector2<float> const& scale);
    void draw_fps(Vector2<float> const& scale);
    void draw_mouse_drag_region(void);
    void draw_thread_status(Vector2<float> const& scale);
    void draw_community_transition_diagram_frame(Vector2<float> const& scale);
    void draw_community_transition_diagram(Vector2<float> const& scale);

    ////////////////////////////////////////////////////////////////////////////////
    // world_impl_draw_perspective_part.cpp
    void draw_layout_frame(void);
    void draw_layout_frame_area(Vector2<float> const& pos, float size);
    void draw_grid(void);
    void draw_center(Vector2<float> const& center);

    void draw_node_with_polygon_circle(
      DynamicNodeProperty const& dnp, bool is_selected);
    void draw_node_with_polygon_circle2(
      DynamicNodeProperty const& dnp, bool is_selected);
    void draw_node_with_texture(
      DynamicNodeProperty const& dnp, float angleH, float angleV, bool is_selected);

    void draw_directed_edge(
      DynamicEdgeProperty const& dep,
      SociariumGraph const* g, bool is_selected);
    void draw_undirected_edge(
      DynamicEdgeProperty const& dep,
      SociariumGraph const* g, bool is_selected);
    void draw_directed_edge_with_polygon(
      DynamicEdgeProperty const& dep,
      SociariumGraph const* g, bool is_selected);
    void draw_undirected_edge_with_polygon(
      DynamicEdgeProperty const& dep,
      SociariumGraph const* g, bool is_selected);

    void draw_node_name(
      DynamicNodeProperty const& dnp,
      float scale, float angleH, float angleV);
    void draw_directed_edge_name(
      DynamicEdgeProperty const& dep, SociariumGraph const* g,
      float scale, float angleH, float angleV);
    void draw_undirected_edge_name(
      DynamicEdgeProperty const& dep, SociariumGraph const* g,
      float scale, float angleH, float angleV);
    void draw_loop_edge_name(
      DynamicEdgeProperty const& dep, SociariumGraph const* g,
      float scale, float angleH, float angleV);

  } // The end of the namespace "sociarium_project_draw_detail"


  using namespace sociarium_project_draw;
  using namespace sociarium_project_draw_detail;
  using namespace sociarium_project_font;
  using namespace sociarium_project_color;
  using namespace sociarium_project_view;
  using namespace sociarium_project_layout;
  using namespace sociarium_project_common;
  using namespace sociarium_project_selection;


  ////////////////////////////////////////////////////////////////////////////////
  void WorldImpl::draw(void) const {

    // FPS control.
    sociarium_project_fps_manager::count_and_wait();

    // Fill background.
    array<float, 4> const& c = get_color(ColorCategory::BACKGROUND);

    glClearColor(c[0], c[1], c[2], c[3]);
    glClearDepth(1.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    GLint mode;
    glGetIntegerv(GL_RENDER_MODE, &mode);
    assert(mode==GL_RENDER);

    // Draw a perspective part.
    view_->push_matrix();
    draw_perspective_part();
    view_->pop_matrix();

    // Draw an orthogonal part.
    view_->push_matrix();
    draw_orthogonal_part();
    view_->pop_matrix();

    HDC dc = get_device_context();
    SwapBuffers(dc);
  }


  ////////////////////////////////////////////////////////////////////////////////
  void WorldImpl::draw_orthogonal_part(void) const {

    GLint mode;
    glGetIntegerv(GL_RENDER_MODE, &mode);

    if (mode==GL_RENDER) view_->initialize_matrix();
    /*
     * Never initialize the modelview matrix in the GL_SELECT mode.
     */

    view_->set_orthogonal_matrix();

    Vector2<int> const& size = view_->viewport_size();
    Vector2<float> const scale(size.x<1?0.0:1.0f/size.x, size.y<1?0.0:1.0f/size.y);

    // --------------------------------------------------------------------------------
    // Draw objects in selection.
    if (get_show_diagram()) draw_community_transition_diagram(scale);
    if (get_show_slider()) draw_time_slider(scale);
    if (get_show_layer_name()) draw_layer_name(scale);

    // --------------------------------------------------------------------------------
    // Draw objects without selection.
    if (mode==GL_RENDER) {

      if (get_show_fps()) draw_fps(scale);
      if (get_drag_status()) draw_mouse_drag_region();

      draw_thread_status(scale);
      draw_selection_result(scale);
    }
  }


  ////////////////////////////////////////////////////////////////////////////////
  void WorldImpl::draw_perspective_part(void) const {

    GLint mode;
    glGetIntegerv(GL_RENDER_MODE, &mode);

    if (mode==GL_RENDER) view_->initialize_matrix();
    /*
     * Never initialize the modelview matrix in the GL_SELECT mode.
     */

    view_->set_perspective_matrix();

    float const angleH = float(view_->angleH())/DEGREE_RESOLUTION+90.0f;
    float const angleV = float(view_->angleV())/DEGREE_RESOLUTION-90.0f;

    glPushMatrix();
    glTranslatef(center_.x, center_.y, 0.0f);

    if (get_show_layout_frame()) {
      draw_layout_frame_area(get_layout_frame_position(), get_layout_frame_size());
      draw_layout_frame();
    }

    shared_ptr<SociariumGraphTimeSeries> ts = sociarium_project_graph_time_series::get();

    { // Draw graph elements.

      ts->read_lock();
      /*
       * Don't forget to call read_unlock().
       */

      size_t const index_of_current_layer = ts->index_of_current_layer();

      shared_ptr<SociariumGraph const> g0 = ts->get_graph(0, index_of_current_layer);
      shared_ptr<SociariumGraph const> g1 = ts->get_graph(1, index_of_current_layer);

      // --------------------------------------------------------------------------------
      // Draw nodes.
      if (get_show_node()) {
        glLoadName(SelectionCategory::NODE);
        node_property_iterator i   = g0->node_property_begin();
        node_property_iterator end = g0->node_property_end();

        if (get_node_style()&NodeStyle::POLYGON)
          // Polygon circle.
          for (; i!=end; ++i) {
            Node const* n = i->first;
            StaticNodeProperty const* snp = i->second.get_static_property();
            glPushName(GLuint(n->index()));
            draw_node_with_polygon_circle(i->second, is_selected(n)||is_selected(snp));
            glPopName();
          }
        else
          // Texture.
          for (; i!=end; ++i) {
            Node const* n = i->first;
            StaticNodeProperty const* snp = i->second.get_static_property();
            glPushName(GLuint(n->index()));
            draw_node_with_texture(i->second, angleH, angleV, is_selected(n)||is_selected(snp));
            glPopName();
          }
      }

      // --------------------------------------------------------------------------------
      // Draw edges.
      if (get_show_edge()) {
        glLoadName(SelectionCategory::EDGE);
        edge_property_iterator i   = g0->edge_property_begin();
        edge_property_iterator end = g0->edge_property_end();

        if (get_edge_style()&EdgeStyle::LINE) {
          if (g0->is_directed())
            // Arc.
            for (; i!=end; ++i) {
              Edge const* e = i->first;
              glPushName(GLuint(e->index()));
              draw_directed_edge(i->second, g0.get(), is_selected(e));
              glPopName();
            }
          else
            // Straight line.
            for (; i!=end; ++i) {
              Edge const* e = i->first;
              glPushName(GLuint(e->index()));
              draw_undirected_edge(i->second, g0.get(), is_selected(e));
              glPopName();
            }
        } else if (get_edge_style()&EdgeStyle::POLYGON) {
          if (g0->is_directed())
            // Polygon Arrow.
            for (; i!=end; ++i) {
              Edge const* e = i->first;
              glPushName(GLuint(e->index()));
              draw_directed_edge_with_polygon(i->second, g0.get(), is_selected(e));
              glPopName();
            }
          else
            // Polygon Line.
            for (; i!=end; ++i) {
              Edge const* e = i->first;
              glPushName(GLuint(e->index()));
              draw_undirected_edge_with_polygon(i->second, g0.get(),is_selected(e));
              glPopName();
            }
        }
      }

      // --------------------------------------------------------------------------------
      // Draw communities.
      if (get_show_community()) {
        glLoadName(SelectionCategory::COMMUNITY);
        node_property_iterator i   = g1->node_property_begin();
        node_property_iterator end = g1->node_property_end();

        if (get_community_style()&CommunityStyle::POLYGON_CIRCLE)
          // Polygon circle.
          for (; i!=end; ++i) {
            Node const* n = i->first;
            StaticNodeProperty const* snp = i->second.get_static_property();
            glPushName(GLuint(n->index()));
            draw_node_with_polygon_circle(i->second, is_selected(n)||is_selected(snp));
            glPopName();
          }
        else
          // Texture.
          for (; i!=end; ++i) {
            Node const* n = i->first;
            StaticNodeProperty const* snp = i->second.get_static_property();
            glPushName(GLuint(n->index()));
            draw_node_with_texture(i->second, angleH, angleV, is_selected(n)||is_selected(snp));
            glPopName();
          }
      }

      // --------------------------------------------------------------------------------
      // Draw community edges.
      if (get_show_community_edge()) {
        glLoadName(SelectionCategory::COMMUNITY_EDGE);
        edge_property_iterator i   = g1->edge_property_begin();
        edge_property_iterator end = g1->edge_property_end();

        if (get_community_edge_style()&EdgeStyle::LINE) {
          if (g1->is_directed())
            // Arc.
            for (; i!=end; ++i) {
              Edge const* e = i->first;
              glPushName(GLuint(e->index()));
              draw_directed_edge(i->second, g1.get(), is_selected(e));
              glPopName();
            }
          else
            // Straight line.
            for (; i!=end; ++i) {
              Edge const* e = i->first;
              glPushName(GLuint(e->index()));
              draw_undirected_edge(i->second, g1.get(), is_selected(e));
              glPopName();
            }
        } else if (get_community_edge_style()&EdgeStyle::POLYGON) {
          if (g1->is_directed())
            // Polygon Arrow.
            for (; i!=end; ++i) {
              Edge const* e = i->first;
              glPushName(GLuint(e->index()));
              draw_directed_edge_with_polygon(i->second, g1.get(), is_selected(e));
              glPopName();
            }
          else
            // Polygon Line.
            for (; i!=end; ++i) {
              Edge const* e = i->first;
              glPushName(GLuint(e->index()));
              draw_undirected_edge_with_polygon(i->second, g1.get(), is_selected(e));
              glPopName();
            }
        }
      }

      ts->read_unlock();
    }


    if (mode==GL_RENDER) {

      if (get_show_layout_frame() && get_update_layout_frame())
        draw_layout_frame_area(get_layout_frame_previous_position(),
                               get_layout_frame_previous_size());

      if (get_show_grid()) draw_grid();
      if (get_show_center()) draw_center(center_);

      {
        // Draw element names.

        ts->read_lock();
        /*
         * Don't forget to call read_unlock().
         */

        size_t const index_of_current_layer = ts->index_of_current_layer();

        shared_ptr<SociariumGraph const> g0 = ts->get_graph(0, index_of_current_layer);
        shared_ptr<SociariumGraph const> g1 = ts->get_graph(1, index_of_current_layer);

        // --------------------------------------------------------------------------------
        // Draw node names.
        if (get_show_node_name()) {
          float const scale = get_font_scale(FontCategory::NODE_NAME);
          node_property_iterator i   = g0->node_property_begin();
          node_property_iterator end = g0->node_property_end();
          for (; i!=end; ++i)
            draw_node_name(i->second, scale, angleH, angleV);
        }

        // --------------------------------------------------------------------------------
        // Draw edge names.
        if (get_show_edge_name()) {
          float const scale = get_font_scale(FontCategory::EDGE_NAME);
          edge_property_iterator i   = g0->edge_property_begin();
          edge_property_iterator end = g0->edge_property_end();
          if (g0->is_directed())
            for (; i!=end; ++i)
              draw_directed_edge_name(i->second, g0.get(), scale, angleH, angleV);
          else
            for (; i!=end; ++i)
              draw_undirected_edge_name(i->second, g0.get(), scale, angleH, angleV);
        }

        // --------------------------------------------------------------------------------
        // Draw community names.
        if (get_show_community_name()) {
          float const scale = get_font_scale(FontCategory::COMMUNITY_NAME);
          node_property_iterator i   = g1->node_property_begin();
          node_property_iterator end = g1->node_property_end();
          for (; i!=end; ++i)
            draw_node_name(i->second, scale, angleH, angleV);
        }

        // --------------------------------------------------------------------------------
        // Draw community edge names.
        if (get_show_community_edge_name()) {
          float const scale = get_font_scale(FontCategory::COMMUNITY_EDGE_NAME);
          edge_property_iterator i   = g1->edge_property_begin();
          edge_property_iterator end = g1->edge_property_end();
          if (g1->is_directed())
            for (; i!=end; ++i)
              draw_directed_edge_name(i->second, g1.get(), scale, angleH, angleV);
          else
            for (; i!=end; ++i)
              draw_undirected_edge_name(i->second, g1.get(), scale, angleH, angleV);
        }

        ts->read_unlock();
      }
    }

    glPopMatrix();
  }

} // The end of the namespace "hashimoto_ut"

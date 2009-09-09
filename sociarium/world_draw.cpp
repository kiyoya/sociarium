// s.o.c.i.a.r.i.u.m
// HASHIMOTO, Yasuhiro (E-mail: hy @ sys.t.u-tokyo.ac.jp)

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

#include <cassert>
#include <boost/format.hpp>
#ifdef _MSC_VER
#include <windows.h>
#endif
#ifdef __APPLE__
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif
#include <FTGL/ftgl.h>
#include "world.h"
#include "common.h"
#include "message.h"
#include "draw.h"
#include "view.h"
#include "color.h"
#include "font.h"
#include "texture.h"
#include "mouse_and_selection.h"
#include "fps_counter.h"
#include "sociarium_graph_time_series.h"
#include "../shared/GL/glview.h"
#include "../shared/GL/gltexture.h"
#include "../shared/predefined_color.h"

#pragma comment(lib, "winmm.lib")

namespace hashimoto_ut {

  using std::vector;
  using std::map;
  using std::pair;
  using std::make_pair;
  using std::string;
  using std::wstring;
  using std::tr1::shared_ptr;
  typedef SociariumGraph::node_property_iterator::value_type node_property;
  typedef SociariumGraph::edge_property_iterator::value_type edge_property;
  typedef SociariumGraph::node_property_iterator node_property_iterator;
  typedef SociariumGraph::edge_property_iterator edge_property_iterator;

  namespace {
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // ローカル変数

    float const loop_edge_height = 0.2f;

    // 直前のセレクション結果
    unsigned int selection_category_prev = -1;
    unsigned int selection_id_prev = -1;

    // 中心点・参照点の大きさ
    float const size_of_reference = 3.0f;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // ローカル関数

    void set_node_color(float* rgba, shared_ptr<DynamicNodeProperty> const& dnp) {
      if (dnp->get_flag()&ElementFlag::HIGHLIGHT) { rgba[1]*=0.6f; rgba[2]*=0.6f; }
      else if (dnp->get_flag()&ElementFlag::TEMPORARY_UNMARKED);
      else if (dnp->get_flag()&(ElementFlag::MARKED|ElementFlag::TEMPORARY_MARKED)) rgba[3] = 0.9f;
    }

    void set_edge_color(float* rgba, shared_ptr<DynamicEdgeProperty> const& dep) {
      if (dep->get_flag()&ElementFlag::HIGHLIGHT) { rgba[1]*=0.6f; rgba[2]*=0.6f; }
      else if (dep->get_flag()&ElementFlag::TEMPORARY_UNMARKED);
      else if (dep->get_flag()&(ElementFlag::MARKED|ElementFlag::TEMPORARY_MARKED)) rgba[3] = 0.8f;
    }
  }

  using sociarium_project_draw::draw_node_with_polygon_circle;
  using sociarium_project_draw::draw_node_with_polygon_curve;
  using sociarium_project_draw::draw_node_with_texture;
  using sociarium_project_draw::draw_edge;
  using sociarium_project_draw::draw_loop_edge;
  using sociarium_project_draw::draw_edge_with_texture;
  using sociarium_project_draw::draw_directed_edge_with_texture;
  using sociarium_project_draw::draw_node_label;
  using sociarium_project_draw::draw_edge_label;
  using sociarium_project_draw::get_update_layout_frame;
  using sociarium_project_draw::get_layout_frame_size;
  using sociarium_project_draw::get_layout_frame_previous_size;
  using sociarium_project_draw::get_layout_frame_position;
  using sociarium_project_draw::get_layout_frame_previous_position;
  using sociarium_project_draw::get_grid_resolution;
  using sociarium_project_draw::get_selection_frame_offset;
  using sociarium_project_draw::get_slider_offset;
  using sociarium_project_draw::get_edge_width_scale;

  using sociarium_project_view::get_node_style;
  using sociarium_project_view::get_edge_style;
  using sociarium_project_view::get_community_style;
  using sociarium_project_view::get_community_edge_style;

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  void World::draw(void) const {

#ifdef _MSC_VER
    bool must_release_dc = false;
    HWND hwnd = sociarium_project_common::get_window_handle();
    if (hwnd==NULL)
      sociarium_project_common::show_last_error(L"World::initialize/get_window_handle");
    HDC dc = wglGetCurrentDC();
    if (dc==0) {
      dc = GetDC(hwnd);
      if (dc==0)
        sociarium_project_common::show_last_error(L"World::draw/GetDC");
      if (wglMakeCurrent(dc, rc_)==FALSE)
        sociarium_project_common::show_last_error(L"World::draw/wglMakeCurrent(dc)");
      must_release_dc = true;
    }

    assert(dc!=0);
#endif

    sociarium_project_fps_counter::count();

    // 背景初期化
    float const* c = sociarium_project_color::select[sociarium_project_color::BACKGROUND];
    glClearColor(c[0], c[1], c[2], c[3]);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    GLint mode;
    glGetIntegerv(GL_RENDER_MODE, &mode);
    assert(mode==GL_RENDER);

    // 透視射影部分の描画
    view_->push_matrix();
    draw_perspective_part();
    view_->pop_matrix();

    // 直交投影部分の描画
    view_->push_matrix();
    draw_orthogonal_part();
    view_->pop_matrix();

#ifdef _MSC_VER
    SwapBuffers(dc);

    if (must_release_dc) {
      if (wglMakeCurrent(0, 0)==FALSE)
        sociarium_project_common::show_last_error(L"World::draw/wglMakeCurrent(0)");
      if (ReleaseDC(hwnd, dc)==0)
        sociarium_project_common::show_last_error(L"World::draw/ReleaseDC");
    }
#endif
  }


  ////////////////////////////////////////////////////////////////////////////////////////////////////
  void World::draw_perspective_part(void) const {
    GLint mode;
    glGetIntegerv(GL_RENDER_MODE, &mode);
    if (mode==GL_RENDER) view_->initialize_matrix(); // 注意！セレクションモードの場合は視点行列を初期化してはダメ
    view_->set_perspective_matrix();

    glPushMatrix();
    glTranslated(center_.x, center_.y, 0.0);

    if (mode==GL_RENDER) {
      // ----------------------------------------------------------------------------------------------------
      // セレクションを行わないオブジェクトの描画：ここから↓↓↓

      if (sociarium_project_view::get_show_layout_frame()) {
        // レイアウトフレームの描画
        if (sociarium_project_draw::get_update_layout_frame()) {
          // 拡大/縮小/移動前のレイアウトフレームの描画
          int const layout_frame_size = get_layout_frame_previous_size();
          Vector2<int> const& pos = get_layout_frame_previous_position();
          glLineWidth(1.0f);
          glColor4fv(sociarium_project_color::select[sociarium_project_color::LAYOUT_FRAME_AREA]);
          glBegin(GL_TRIANGLE_STRIP);
          glVertex3i(pos.x-layout_frame_size, pos.y-layout_frame_size, 0);
          glVertex3i(pos.x-layout_frame_size, pos.y+layout_frame_size, 0);
          glVertex3i(pos.x+layout_frame_size, pos.y-layout_frame_size, 0);
          glVertex3i(pos.x+layout_frame_size, pos.y+layout_frame_size, 0);
          glEnd();
        }
      }

      if (sociarium_project_view::get_show_grid()) {
        // グリッドの描画
        glLineWidth(1.0f);
        glColor4fv(sociarium_project_color::select[sociarium_project_color::GRID]);
        int const grid_resolution = get_grid_resolution();
        int const xmax = 100*grid_resolution;
        int const xmin = -xmax;
        int const ymax = xmax;
        int const ymin = xmin;
        glBegin(GL_LINES);
        for (int x=xmin; x<=xmax; x+=grid_resolution) {
          glVertex2i(x, ymin);
          glVertex2i(x, ymax);
        }
        for (int y=ymin; y<=ymax; y+=grid_resolution) {
          glVertex2i(xmin, y);
          glVertex2i(xmax, y);
        }
        glEnd();
      }

      if (sociarium_project_view::get_show_center()) {
        // 参照点の描画
        glLineWidth(1.0f);
        glBegin(GL_LINES);
        glColor4f(1.0f, 0.0f, 0.0f, 0.9f);
        glVertex3f(-center_.x-size_of_reference, -center_.y, 0.0f);
        glVertex3f(-center_.x+size_of_reference, -center_.y, 0.0f);
        glColor4f(0.0f, 1.0f, 0.0f, 0.9f);
        glVertex3f(-center_.x, -center_.y-size_of_reference, 0.0f);
        glVertex3f(-center_.x, -center_.y+size_of_reference, 0.0f);
        glColor4f(0.0f, 0.0f, 1.0f, 0.9f);
        glVertex3f(-center_.x, -center_.y, -size_of_reference);
        glVertex3f(-center_.x, -center_.y, +size_of_reference);
        glEnd();
        // 系中心点の描画
        glLineWidth(1.0f);
        glBegin(GL_LINES);
        glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
        glVertex3f(-size_of_reference, 0.0, 0.0f);
        glVertex3f(+size_of_reference, 0.0, 0.0f);
        glVertex3f(0.0, -size_of_reference, 0.0f);
        glVertex3f(0.0, +size_of_reference, 0.0f);
        glVertex3f(0.0, 0.0, -size_of_reference);
        glVertex3f(0.0, 0.0, +size_of_reference);
        glEnd();
      }

      {
        // ラベルの描画
        time_series_->read_lock();
        size_t const index_of_current_layer = time_series_->index_of_current_layer();
        float const angle = float(view_->angleH())/DEGREE_RESOLUTION+90.0f;
        shared_ptr<SociariumGraph> const& g0 = time_series_->get_graph(0, index_of_current_layer);
        shared_ptr<SociariumGraph> const& g1 = time_series_->get_graph(1, index_of_current_layer);

        if (sociarium_project_view::get_show_node_label()) {
          // ノード
          float const scale = sociarium_project_font::get_node_font_scale();
          for (node_property_iterator i=g0->node_property_begin(), end=g0->node_property_end(); i!=end; ++i) {
            shared_ptr<DynamicNodeProperty> const& dnp = i->second;
            shared_ptr<StaticNodeProperty> const& snp = dnp->get_static_property();
            if (is_hidden(dnp)) continue;
            Vector3<float> const& rgb = predefined_color[dnp->get_color_id()];
            float rgba[4] = { rgb.x, rgb.y, rgb.z, 0.8f };
            draw_node_label(snp->get_name().c_str(), snp->get_position(), 0.0f, scale, rgba, angle);
          }
        }

        if (sociarium_project_view::get_show_edge_label()) {
          // エッジ
          float const scale = sociarium_project_font::get_edge_font_scale();
          for (edge_property_iterator i=g0->edge_property_begin(), end=g0->edge_property_end(); i!=end; ++i) {
            Edge const* e = i->first;
            shared_ptr<DynamicEdgeProperty> const& dep = i->second;
            shared_ptr<StaticEdgeProperty> const& sep = dep->get_static_property();
            if (is_hidden(dep)) continue;
            shared_ptr<DynamicNodeProperty> const& dnp0 = g0->property(e->source());
            shared_ptr<DynamicNodeProperty> const& dnp1 = g0->property(e->target());
            Vector2<float> const& pos0 = dnp0->get_static_property()->get_position();
            Vector2<float> const& pos1 = dnp1->get_static_property()->get_position();
            Vector3<float> const& rgb = predefined_color[dep->get_color_id()];
            float rgba[4] = { rgb.x, rgb.y, rgb.z, 0.8f };
            draw_edge_label(sep->get_name().c_str(), pos0, dnp0->get_size(), pos1, dnp1->get_size(), scale, rgba, angle);
          }
        }

        if (sociarium_project_view::get_show_community_label()) {
          // コミュニティ
          float const scale = sociarium_project_font::get_community_font_scale();
          for (node_property_iterator i=g1->node_property_begin(), end=g1->node_property_end(); i!=end; ++i) {
            shared_ptr<DynamicNodeProperty> const& dnp = i->second;
            shared_ptr<StaticNodeProperty> const& snp = dnp->get_static_property();
            if (is_hidden(dnp)) continue;
            Vector3<float> const& rgb = predefined_color[dnp->get_color_id()];
            float rgba[4] = { rgb.x, rgb.y, rgb.z, 0.8f };
            draw_node_label(snp->get_name().c_str(), snp->get_position(), 0.0f, scale, rgba, angle);
          }
        }

        if (sociarium_project_view::get_show_community_edge_label()) {
          // コミュニティエッジ
          float const scale = sociarium_project_font::get_community_edge_font_scale();
          for (edge_property_iterator i=g1->edge_property_begin(), end=g1->edge_property_end(); i!=end; ++i) {
            Edge const* e = i->first;
            shared_ptr<DynamicEdgeProperty> const& dep = i->second;
            shared_ptr<StaticEdgeProperty> const& sep = dep->get_static_property();
            if (is_hidden(dep)) continue;
            shared_ptr<DynamicNodeProperty> const& dnp0 = g1->property(e->source());
            shared_ptr<DynamicNodeProperty> const& dnp1 = g1->property(e->target());
            Vector2<float> const& pos0 = dnp0->get_static_property()->get_position();
            Vector2<float> const& pos1 = dnp1->get_static_property()->get_position();
            Vector3<float> const& rgb = predefined_color[dep->get_color_id()];
            float rgba[4] = { rgb.x, rgb.y, rgb.z, 0.8f };
            draw_edge_label(sep->get_name().c_str(), pos0, dnp0->get_size(), pos1, dnp1->get_size(), scale, rgba, angle);
          }
        }

        time_series_->read_unlock();
      }

      // セレクションを行わないオブジェクトの描画：ここまで↑↑↑
      // ----------------------------------------------------------------------------------------------------
    }

    {
      // ----------------------------------------------------------------------------------------------------
      // セレクションを行うオブジェクトの描画：ここから↓↓↓

      if (sociarium_project_view::get_show_layout_frame()) {
        // レイアウトフレーム面の描画
        int const layout_frame_size = get_layout_frame_size();
        Vector2<int> const& pos = get_layout_frame_position();
        glLoadName(SelectionCategory::LAYOUT_FRAME);
        glColor4fv(sociarium_project_color::select[sociarium_project_color::LAYOUT_FRAME_AREA]);
        glPushName(0);
        glBegin(GL_TRIANGLE_STRIP);
        glVertex3i(pos.x-layout_frame_size, pos.y-layout_frame_size, 0);
        glVertex3i(pos.x-layout_frame_size, pos.y+layout_frame_size, 0);
        glVertex3i(pos.x+layout_frame_size, pos.y-layout_frame_size, 0);
        glVertex3i(pos.x+layout_frame_size, pos.y+layout_frame_size, 0);
        glEnd();
        glPopName();
        // レイアウトフレーム枠の描画
        if (sociarium_project_mouse_and_selection::layout_frame_border_is_selected())
          glColor4fv(sociarium_project_color::select[sociarium_project_color::LAYOUT_FRAME_BORDER]);
        else glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
        glLineWidth(2.0f);
        glLoadName(SelectionCategory::LAYOUT_FRAME_BORDER);
        glPushName(0);
        glBegin(GL_LINES);
        glVertex3i(pos.x-layout_frame_size, pos.y-layout_frame_size, 0);
        glVertex3i(pos.x-layout_frame_size, pos.y+layout_frame_size, 0);
        glEnd();
        glBegin(GL_LINES);
        glVertex3i(pos.x+layout_frame_size, pos.y-layout_frame_size, 0);
        glVertex3i(pos.x+layout_frame_size, pos.y+layout_frame_size, 0);
        glEnd();
        glPopName();
        glPushName(1);
        glBegin(GL_LINES);
        glVertex3i(pos.x+layout_frame_size, pos.y-layout_frame_size, 0);
        glVertex3i(pos.x-layout_frame_size, pos.y-layout_frame_size, 0);
        glEnd();
        glBegin(GL_LINES);
        glVertex3i(pos.x+layout_frame_size, pos.y+layout_frame_size, 0);
        glVertex3i(pos.x-layout_frame_size, pos.y+layout_frame_size, 0);
        glEnd();
        glPopName();
      }

      { // グラフ要素の描画
        time_series_->read_lock();
        size_t const index_of_current_layer = time_series_->index_of_current_layer();
        float const angle = float(view_->angleH())/DEGREE_RESOLUTION+90.0f;
        shared_ptr<SociariumGraph> const& g0 = time_series_->get_graph(0, index_of_current_layer);
        shared_ptr<SociariumGraph> const& g1 = time_series_->get_graph(1, index_of_current_layer);

        if (sociarium_project_view::get_show_node()) {
          // ノード
          glLoadName(SelectionCategory::NODE);
          for (node_property_iterator i=g0->node_property_begin(), end=g0->node_property_end(); i!=end; ++i) {
            Node const* n = i->first;
            shared_ptr<DynamicNodeProperty> const& dnp = i->second;
            shared_ptr<StaticNodeProperty> const& snp = dnp->get_static_property();
            if (is_hidden(dnp)) continue;
            glPushName(GLuint(n->index()));
            // 色を決定
            Vector3<float> const& rgb = predefined_color[dnp->get_color_id()];
            float rgba[4] = { rgb.x, rgb.y, rgb.z, 0.5f };
            set_node_color(rgba, dnp);
            if (sociarium_project_mouse_and_selection::node_is_selected(dnp->get_graph_element())) rgba[3] *= 1.2f;
            // 描画
            if (get_node_style()&sociarium_project_view::NodeView::Style::POLYGON) {
              draw_node_with_polygon_circle(snp->get_position(), 0.0f, dnp->get_size(), rgba);
            } else {
              draw_node_with_texture(snp->get_position(), 0.0f, dnp->get_size(), rgba, angle, snp->get_texture());
            }
            glPopName();
          }
        }

        if (sociarium_project_view::get_show_edge()) {
          // エッジ
          glLoadName(SelectionCategory::EDGE);
          for (edge_property_iterator i=g0->edge_property_begin(), end=g0->edge_property_end(); i!=end; ++i) {
            Edge const* e = i->first;
            shared_ptr<DynamicEdgeProperty> const& dep = i->second;
            shared_ptr<StaticEdgeProperty> const& sep = dep->get_static_property();
            if (is_hidden(dep)) continue;
            glPushName(GLuint(e->index()));
            // 色を決定
            Vector3<float> const& rgb = predefined_color[dep->get_color_id()];
            float rgba[4] = { rgb.x, rgb.y, rgb.z, 0.4f };
            set_edge_color(rgba, dep);
            if (sociarium_project_mouse_and_selection::edge_is_selected(dep->get_graph_element())) rgba[3] *= 1.2f;
            // 描画
            const DynamicNodeProperty* dnp0 = g0->property(e->source()).get();
            const DynamicNodeProperty* dnp1 = g0->property(e->target()).get();
            Vector2<float> const& pos0 = dnp0->get_static_property()->get_position();
            Vector2<float> const& pos1 = dnp1->get_static_property()->get_position();
            if (e->source()==e->target()) {
              float const width = sociarium_project_mouse_and_selection::edge_is_selected(e)?3.0f:1.0f;
              draw_loop_edge(pos0, width, loop_edge_height, rgba);
            } else if (get_edge_style()&sociarium_project_view::EdgeView::Style::TEXTURE) {
              float const width = get_edge_width_scale()*float(log(double(10.0*dep->get_weight())+1.0));
              if (g0->is_directed()) draw_directed_edge_with_texture(pos0, dnp0->get_size(), pos1, dnp1->get_size(), width, rgba, sep->get_texture());
              else draw_edge_with_texture(pos0, dnp0->get_size(), pos1, dnp1->get_size(), width, rgba, sep->get_texture());
            } else if (get_edge_style()&sociarium_project_view::EdgeView::Style::LINE) {
              float const width = sociarium_project_mouse_and_selection::edge_is_selected(e)?3.0f:1.0f;
              if (g0->is_directed()) draw_edge(pos0, dnp0->get_size(), pos1, dnp1->get_size(), width, 0.5f, rgba);
              else draw_edge(pos0, dnp0->get_size(), pos1, dnp1->get_size(), width, rgba);
            }
            glPopName();
          }
        }

        if (sociarium_project_view::get_show_community()) {
          // コミュニティ
          glLoadName(SelectionCategory::COMMUNITY);
          for (node_property_iterator i=g1->node_property_begin(), end=g1->node_property_end(); i!=end; ++i) {
            Node const* n = i->first;
            shared_ptr<DynamicNodeProperty> const& dnp = i->second;
            shared_ptr<StaticNodeProperty> const& snp = dnp->get_static_property();
            if (is_hidden(dnp)) continue;
            glPushName(GLuint(n->index()));
            // 色を決定
            Vector3<float> const& rgb = predefined_color[dnp->get_color_id()];
            float rgba[4] = { rgb.x, rgb.y, rgb.z, 0.4f };
            set_node_color(rgba, dnp);
            if (sociarium_project_mouse_and_selection::community_is_selected(dnp->get_graph_element())) rgba[3] *= 1.2f;
            // 描画
            if (get_community_style()&sociarium_project_view::CommunityView::Style::POLYGON_CIRCLE) {
              draw_node_with_polygon_circle(snp->get_position(), 0.0f, dnp->get_size(), rgba);
            } else if (get_community_style()&sociarium_project_view::CommunityView::Style::POLYGON_CURVE) {
              vector<Vector2<float> const*> pos;
              pos.reserve(dnp->number_of_lower_nodes());
              for (vector<DynamicNodeProperty*>::const_iterator i=dnp->lower_nbegin(); i!=dnp->lower_nend(); ++i)
                pos.push_back(&((*i)->get_static_property()->get_position()));
              draw_node_with_polygon_curve(snp->get_position(), pos, 0.0f, rgba);
            } else {
              draw_node_with_texture(snp->get_position(), 0.0f, dnp->get_size(), rgba, angle, snp->get_texture());
            }
            glPopName();
          }
        }

        if (sociarium_project_view::get_show_community_edge()) {
          // コミュニティエッジ
          glLoadName(SelectionCategory::COMMUNITY_EDGE);
          for (edge_property_iterator i=g1->edge_property_begin(), end=g1->edge_property_end(); i!=end; ++i) {
            Edge const* e = i->first;
            shared_ptr<DynamicEdgeProperty> const& dep = i->second;
            shared_ptr<StaticEdgeProperty> const& sep = dep->get_static_property();
            if (is_hidden(dep)) continue;
            glPushName(GLuint(e->index()));
            // 色を決定
            Vector3<float> const& rgb = predefined_color[dep->get_color_id()];
            float rgba[4] = { rgb.x, rgb.y, rgb.z, 0.3f };
            set_edge_color(rgba, dep);
            if (sociarium_project_mouse_and_selection::community_edge_is_selected(dep->get_graph_element())) rgba[3] *= 1.2f;
            // 描画
            const DynamicNodeProperty* dnp0 = g1->property(e->source()).get();
            const DynamicNodeProperty* dnp1 = g1->property(e->target()).get();
            Vector2<float> const& pos0 = dnp0->get_static_property()->get_position();
            Vector2<float> const& pos1 = dnp1->get_static_property()->get_position();
            if (get_edge_style()&sociarium_project_view::CommunityEdgeView::Style::TEXTURE) {
              float const width = get_edge_width_scale()*float(log(double(10.0*dep->get_weight())+1.0));
              if (g1->is_directed()) draw_directed_edge_with_texture(pos0, dnp0->get_size(), pos1, dnp1->get_size(), width, rgba, sep->get_texture());
              else draw_edge_with_texture(pos0, dnp0->get_size(), pos1, dnp1->get_size(), width, rgba, sep->get_texture());
            } else if (get_edge_style()&sociarium_project_view::CommunityEdgeView::Style::LINE) {
              float const width = sociarium_project_mouse_and_selection::community_edge_is_selected(e)?3.0f:1.0f;
              if (g1->is_directed()) draw_edge(pos0, dnp0->get_size(), pos1, dnp1->get_size(), width, 0.5f, rgba);
              else draw_edge(pos0, dnp0->get_size(), pos1, dnp1->get_size(), width, rgba);
            }
            glPopName();
          }
        }

        time_series_->read_unlock();
      }

      // セレクションを行うオブジェクトの描画：ここまで↑↑↑
      // ----------------------------------------------------------------------------------------------------
    }

    glPopMatrix();
  }


  ////////////////////////////////////////////////////////////////////////////////////////////////////
  void World::draw_orthogonal_part(void) const {
    GLint mode;
    glGetIntegerv(GL_RENDER_MODE, &mode);
    if (mode==GL_RENDER) view_->initialize_matrix(); // 注意！セレクションモードの場合は視点行列を初期化してはダメ
    view_->set_orthogonal_matrix();

    double const scx = 1.0/view_->viewport_size().x;
    double const scy = 1.0/view_->viewport_size().y;

    if (mode==GL_RENDER) {
      // ----------------------------------------------------------------------------------------------------
      // セレクションを行わないオブジェクトの描画：ここから↓↓↓

      if (sociarium_project_view::get_show_fps()) {
        // FPSの描画
        shared_ptr<FTFont> const& f = sociarium_project_font::get_misc_font();
        wstring const text = (boost::wformat(L"FPS=%.1f")%sociarium_project_fps_counter::get_fps()).str();
        float llx, lly, llz, urx, ury, urz;
        f->BBox(text.c_str(), llx, lly, llz, urx, ury, urz);
        double const scx2 = 0.8*scx;
        double const scy2 = 0.8*scy;
        double const w = scx2*(urx-llx);
        double const h = scy2*(ury-lly);
        glPushMatrix();
        glTranslated(1.0-w-scx*get_selection_frame_offset().x,
                     1.0-h-scy*get_selection_frame_offset().y, 0.0); // 右上隅
        glScaled(scx2, scy2, 0.0);
        glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
        f->Render(text.c_str());
        glPopMatrix();
      }

      if (!sociarium_project_message::get()->first.empty()) {
        // 進行状況メッセージの描画
        shared_ptr<FTFont> const& f = sociarium_project_font::get_misc_font();
        wstring const text  = sociarium_project_message::get()->first;
        wstring const text2 = sociarium_project_message::get()->second;
        double const scx2 = 0.8*scx;
        double const scy2 = 0.8*scy;
        float llx, lly, llz, urx, ury, urz;
        f->BBox(text.c_str(), llx, lly, llz, urx, ury, urz);
        double const w = scx*(urx-llx);
        double const h = scy*(ury-lly);
        double w2 = 0.0;
        double h2 = 0.0;
        if (!text2.empty()) {
          f->BBox(text2.c_str(), llx, lly, llz, urx, ury, urz);
          w2 = scx2*(urx-llx);
          h2 = scy2*(ury-lly);
        }
        // 枠の描画
        double const upper_margin = 4;
        double const middle_margin = h2>0.0?8:0;
        double const lower_margin = 8;
        double const frame_height = h+h2+scy*(upper_margin+middle_margin+lower_margin);
        glColor4fv(sociarium_project_color::select[sociarium_project_color::THREAD_MESSAGE_FRAME]);
        glBegin(GL_TRIANGLE_STRIP);
        glVertex2d(-0.1, 0.5*(1.0-frame_height));
        glVertex2d(-0.1, 0.5*(1.0+frame_height));
        glVertex2d(+1.1, 0.5*(1.0-frame_height));
        glVertex2d(+1.1, 0.5*(1.0+frame_height));
        glEnd();
        // メッセージ
        glPushMatrix();
        glTranslated(0.5*(1.0-w), 0.5*(1.0+frame_height)-h-scy*upper_margin, 0.0);
        glScaled(scx, scy, 0.0);
        glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
        f->Render(text.c_str());
        glPopMatrix();
        // サブメッセージ
        glPushMatrix();
        glTranslated(0.5*(1.0-w2), 0.5*(1.0-frame_height)+scy*lower_margin, 0.0);
        glScaled(scx2, scy2, 0.0);
        glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
        f->Render(text2.c_str());
        glPopMatrix();
      }

      {
        if (sociarium_project_mouse_and_selection::get_drag_status()) {
          // マウスドラッグの矩形の描画
          glColor4fv(sociarium_project_color::select[sociarium_project_color::MOUSE_DRAG_REGION]);
          glBegin(GL_TRIANGLE_STRIP);
          glVertex2dv(sociarium_project_mouse_and_selection::get_drag_region(0).data);
          glVertex2dv(sociarium_project_mouse_and_selection::get_drag_region(1).data);
          glVertex2dv(sociarium_project_mouse_and_selection::get_drag_region(2).data);
          glVertex2dv(sociarium_project_mouse_and_selection::get_drag_region(3).data);
          glEnd();
        }
      }

      { // セレクション結果の描画
        time_series_->read_lock();
        size_t const index_of_current_layer = time_series_->index_of_current_layer();
        shared_ptr<SociariumGraph> const& g0 = time_series_->get_graph(0, index_of_current_layer);
        shared_ptr<SociariumGraph> const& g1 = time_series_->get_graph(1, index_of_current_layer);

        if (0) {
          // テキストのみ表示
          shared_ptr<FTFont> const& f = sociarium_project_font::get_misc_font();
          const unsigned int select_id = sociarium_project_mouse_and_selection::get_selection()->get_id();
          wstring const text =
            sociarium_project_mouse_and_selection::node_is_selected()?g0->property(g0->node(select_id))->get_static_property()->get_name()
              :(sociarium_project_mouse_and_selection::edge_is_selected()?g0->property(g0->edge(select_id))->get_static_property()->get_name()
                :(sociarium_project_mouse_and_selection::community_is_selected()?g1->property(g1->node(select_id))->get_static_property()->get_name()
                  :(sociarium_project_mouse_and_selection::community_edge_is_selected()?g1->property(g1->edge(select_id))->get_static_property()->get_name()
                    :L"")));
          float llx, lly, llz, urx, ury, urz;
          f->BBox(text.c_str(), llx, lly, llz, urx, ury, urz);
          double const h = ury-lly;
          glPushMatrix();
          glTranslated(scx*get_selection_frame_offset().x,
                       1.0-scy*(h+get_selection_frame_offset().y), 0.0);
          glScaled(scx, scy, 0.0);
          glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
          f->Render(text.c_str());
          glPopMatrix();
        }

        else {
          // テクスチャ付き表示
          shared_ptr<DynamicNodeProperty> n[2];
          const unsigned int selection_category = sociarium_project_mouse_and_selection::get_selection()->get_category();
          const unsigned int selection_id = sociarium_project_mouse_and_selection::get_selection()->get_id();
          if (selection_category==SelectionCategory::NODE) {
            n[0] = g0->property(g0->node(selection_id));
          } else if (selection_category==SelectionCategory::EDGE) {
            n[0] = g0->property(g0->edge(selection_id)->source());
            n[1] = g0->property(g0->edge(selection_id)->target());
          } else if (selection_category==SelectionCategory::COMMUNITY) {
            n[0] = g1->property(g1->node(selection_id));
          } else if (selection_category==SelectionCategory::COMMUNITY_EDGE) {
            n[0] = g1->property(g1->edge(selection_id)->source());
            n[1] = g1->property(g1->edge(selection_id)->target());
          }
          if (n[0]!=0) {
            // 新しいセレクションに音を鳴らす
            //if (selection_id!=selection_id_prev || selection_category!=selection_category_prev)
            //PlaySound(L"sound\\selection.wav", NULL, SND_ASYNC);
            double const top_margin = get_selection_frame_offset().y;
            double const left_margin = get_selection_frame_offset().x;
            double yy = 0.0;
            glPushMatrix();
            Vector2<double> const frame_position(left_margin*scx, 1.0-top_margin*scy); // 左上
            Vector2<double> const frame_size(0.1, 0.11*view_->viewport_aspect());
            glTranslated(frame_position.x, frame_position.y, 0.0);
            for (int i=0; i<2; ++i) {
              if (n[i]!=0) {
                shared_ptr<DynamicNodeProperty> const& dnp = n[i];
                shared_ptr<StaticNodeProperty> const& snp = dnp->get_static_property();
                wstring const text = snp->get_name();
                glPushMatrix();
                glTranslated(0.0, -yy, 0.0);
                // 枠
                glColor4fv(sociarium_project_color::select[sociarium_project_color::SELECTION_FRAME]);
                glBegin(GL_TRIANGLE_STRIP);
                glVertex2d(0.0, 0.0);
                glVertex2d(0.0, -frame_size.y);
                glVertex2d(frame_size.x, 0.0);
                glVertex2d(frame_size.x, -frame_size.y);
                glEnd();
                // テクスチャ
                double const texture_scale = 0.8;
                Vector2<double> const texture_size(texture_scale*frame_size.x, texture_scale*frame_size.x*view_->viewport_aspect());
                Vector2<double> const texture_position(0.5*(frame_size.x-texture_size.x), 0.5*(frame_size.x-texture_size.y));
                glPushMatrix();
                glTranslated(texture_position.x, texture_position.y, 0.0);
                glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
                glEnable(GL_TEXTURE_2D);
                GLTexture const* texture = snp->get_texture();
                glBindTexture(GL_TEXTURE_2D, texture->get());
                glBegin(GL_TRIANGLE_STRIP);
                glTexCoord2f(0.0f, 1.0f); glVertex2d(0.0, 0.0);
                glTexCoord2f(0.0f, 0.0f); glVertex2d(0.0, -texture_size.y);
                glTexCoord2f(1.0f, 1.0f); glVertex2d(texture_size.x, 0.0);
                glTexCoord2f(1.0f, 0.0f); glVertex2d(texture_size.x, -texture_size.y);
                glEnd();
                glDisable(GL_TEXTURE_2D);
                // 名前
                shared_ptr<FTFont> const& f = sociarium_project_font::get_misc_font();
                float llx, lly, llz, urx, ury, urz;
                f->BBox(text.c_str(), llx, lly, llz, urx, ury, urz);
                Vector2<double> const text_size(urx-llx, ury-lly);
                double const scx2_tmp = texture_size.x/text_size.x;
                double const scx2_max = 0.0005;
                double const scy2_max = scx2_max*view_->viewport_aspect();
                double const scx2 = scx2_tmp>scx2_max?scx2_max:scx2_tmp;
                double const scy2 = scx2*view_->viewport_aspect();
                glPushMatrix();
                double const offset = get_selection_frame_offset().y;
                glTranslated(0.5*(texture_size.x-scx2*text_size.x), -texture_size.y-scy2_max*(text_size.y+offset), 0.0);
                glScaled(scx2, scy2, 0.0);
                glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
                f->Render(text.c_str());
                glPopMatrix();
                glPopMatrix();
                glPopMatrix();
                yy = frame_size.y+top_margin*scy;
              }
            }
            glPopMatrix();
          }
          selection_id_prev = selection_id;
          selection_category_prev = selection_category;
        }

        time_series_->read_unlock();
      }

      // セレクションを行わないオブジェクトの描画：ここまで↑↑↑
      // ----------------------------------------------------------------------------------------------------
    }

    {
      // ----------------------------------------------------------------------------------------------------
      // セレクションを行うオブジェクトの描画：ここから↓↓↓
      time_series_->read_lock();
      size_t const index_of_current_layer = time_series_->index_of_current_layer();
      shared_ptr<SociariumGraph> const& g0 = time_series_->get_graph(0, index_of_current_layer);
      //shared_ptr<SociariumGraph> const& g1 = time_series_->get_graph(1, index_of_current_layer);

      if (sociarium_project_view::get_show_time_label()) {
        // レイヤーのラベルの描画
        shared_ptr<FTFont> const& f = sociarium_project_font::get_misc_font();
        float llx, lly, llz, urx, ury, urz;
        wstring const text = sociarium_project_mouse_and_selection::layer_label_is_selected()?
          (boost::wformat(L"N:%d, E:%d")%g0->nsize()%g0->esize()).str():time_series_->get_layer_label(index_of_current_layer);
        f->BBox(text.c_str(), llx, lly, llz, urx, ury, urz);
        double const w = urx-llx;
        double const h = ury-lly;
        glLoadName(SelectionCategory::LAYER_LABEL);
        glColor4fv(sociarium_project_color::select[sociarium_project_color::LAYER_LABEL_FRAME]);
        glBegin(GL_TRIANGLE_STRIP);
        glVertex2d(0.0, 0.0);
        glVertex2d(0.0, 1.1*scy*(h+get_selection_frame_offset().y));
        glVertex2d(1.0, 0.0);
        glVertex2d(1.0, 1.1*scy*(h+get_selection_frame_offset().y));
        glEnd();
        glPushMatrix();
        glTranslated(0.5*(1.0-scx*w), scy*get_selection_frame_offset().y, 0.0);
        glScaled(scx, scy, 0.1);
        glColor4f(1.0f, 1.0f, 1.0f, 0.6f);
        f->Render(text.c_str());
        glPopMatrix();
      }

      if (sociarium_project_view::get_show_slider()) {
        // 時系列スライダーの描画
        double const xoff = scx*get_slider_offset().x;
        double const yoff = scy*get_slider_offset().y;
        double const x = 1.0-xoff;
        double const w = 0.5*xoff;
        double const h = 1.0-2*yoff;
        glLoadName(SelectionCategory::TIME_SLIDER);
        const pair<int, int> c = (sociarium_project_mouse_and_selection::time_slider_is_selected()?
                                  make_pair(sociarium_project_color::TIME_SLIDER_UPPER_ON,
                                            sociarium_project_color::TIME_SLIDER_LOWER_ON):
                                  make_pair(sociarium_project_color::TIME_SLIDER_UPPER_OFF,
                                            sociarium_project_color::TIME_SLIDER_LOWER_OFF));
        glEnable(GL_TEXTURE_2D);
        GLTexture const* texture = sociarium_project_texture::get_slider_texture();
        glBindTexture(GL_TEXTURE_2D, texture->get());
        if (time_series_->number_of_layers()>100) {
          size_t const num = time_series_->number_of_layers()-1;
          double const y = (h*time_series_->index_of_current_layer())/num+yoff;
          glColor4fv(sociarium_project_color::select[c.second]);
          glBegin(GL_TRIANGLE_STRIP);
          glTexCoord2f(0.0f, 0.0f); glVertex2d(x, yoff);
          glTexCoord2f(0.0f, 1.0f); glVertex2d(x, y);
          glTexCoord2f(1.0f, 0.0f); glVertex2d(x+w, yoff);
          glTexCoord2f(1.0f, 1.0f); glVertex2d(x+w, y);
          glEnd();
          glColor4fv(sociarium_project_color::select[c.first]);
          glBegin(GL_TRIANGLE_STRIP);
          glTexCoord2f(0.0f, 0.0f); glVertex2d(x, y);
          glTexCoord2f(0.0f, 1.0f); glVertex2d(x, yoff+h);
          glTexCoord2f(1.0f, 0.0f); glVertex2d(x+w, y);
          glTexCoord2f(1.0f, 1.0f); glVertex2d(x+w, yoff+h);
          glEnd();
        } else {
          size_t const num = time_series_->number_of_layers();
          size_t const num_lower = time_series_->index_of_current_layer()+1;
          double const interval_scale = 0.1;
          double const dy = h/(interval_scale*(num-1)+num);
          double const my = interval_scale*dy;
          double yb = yoff;
          glColor4fv(sociarium_project_color::select[c.second]);
          for (size_t i=0; i<num_lower; ++i) {
            double yt = yb+dy;
            glBegin(GL_TRIANGLE_STRIP);
            glTexCoord2f(0.0f, 0.0f); glVertex2d(x, yb);
            glTexCoord2f(0.0f, 1.0f); glVertex2d(x, yt);
            glTexCoord2f(1.0f, 0.0f); glVertex2d(x+w, yb);
            glTexCoord2f(1.0f, 1.0f); glVertex2d(x+w, yt);
            glEnd();
            yb = yt+my;
          }
          glColor4fv(sociarium_project_color::select[c.first]);
          for (size_t i=num_lower; i<num; ++i) {
            double yt = yb+dy;
            glBegin(GL_TRIANGLE_STRIP);
            glTexCoord2f(0.0f, 0.0f); glVertex2d(x, yb);
            glTexCoord2f(0.0f, 1.0f); glVertex2d(x, yt);
            glTexCoord2f(1.0f, 0.0f); glVertex2d(x+w, yb);
            glTexCoord2f(1.0f, 1.0f); glVertex2d(x+w, yt);
            glEnd();
            yb = yt+my;
          }
        }
        glDisable(GL_TEXTURE_2D);
      }

      time_series_->read_unlock();
      // セレクションを行うオブジェクトの描画：ここまで↑↑↑
      // ----------------------------------------------------------------------------------------------------
    }
  }

} // The end of the namespace "hashimoto_ut"

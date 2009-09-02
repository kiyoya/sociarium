// s.o.c.i.a.r.i.u.m
// HASHIMOTO, Yasuhiro (E-mail: hy @ sys.t.u-tokyo.ac.jp)
// update: 2009/05/11

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

#include <windows.h>
#include <boost/bind.hpp>
#include "world.h"
#include "common.h"
#include "draw.h"
#include "view.h"
#include "graph_utility.h"
#include "mouse_and_selection.h"
#include "flag_operation.h"
#include "sociarium_graph_time_series.h"
#include "../shared/GL/glview.h"
#include "../graph/graphex.h"
#include "../graph/util/traverser.h"
#include "../graph/util/clustering.h"

namespace hashimoto_ut {

  using std::vector;
  using std::pair;
  using std::make_pair;
  using std::tr1::shared_ptr;

  namespace {

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // ローカル変数

    // マウスポインタの座標
    Vector2<int> mpos_LBUTTONDOWN;
    Vector2<int> mpos_RBUTTONDOWN;
    Vector2<int> mpos_prev;
    Vector2<double> mpos_world;
    Vector2<double> mpos_world_LBUTTONDOWN;
    Vector2<double> mpos_world_RBUTTONDOWN;
    Vector2<double> mpos_world_prev;

    // 視点の角度
    int angleH_prev;
    int angleV_prev;

    struct UpdateMark {
      // マウスドラッグによる一時マーキングを決定
      template <typename T>
      void operator()(T const& p) const {
        typedef typename T::second_type DynamicProperty;
        DynamicProperty const& dp = p.second;
        if (is_visible(dp)) {
          if (is_temporary_marked(dp))
            dp->set_flag((dp->get_flag()|ElementFlag::MARKED)&~ElementFlag::TEMPORARY_MARKED);
          else if (is_temporary_unmarked(dp))
            dp->set_flag(dp->get_flag()&~(ElementFlag::MARKED|ElementFlag::TEMPORARY_UNMARKED));
        }
      }
    };

    struct MoveNode {
      // 指定したフラグのいずれかがアクティブなノードを移動
      void operator()(const SociariumGraph::node_property_iterator::value_type& p, unsigned int flag, Vector2<float> const& diff) const {
        std::tr1::shared_ptr<DynamicNodeProperty> const& dnp = p.second;
        if (is_visible(dnp) && (dnp->get_flag()&flag)) dnp->get_static_property()->set_position(dnp->get_static_property()->get_position()+diff);
      }
    };

  } // The end of the anonymous namespace

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // マウス操作
  void World::do_mouse_action(int action, Vector2<int> const& mpos, WPARAM wp) {

    time_series_->read_lock();

    HWND hwnd = sociarium_project_common::get_window_handle();
    if (hwnd==NULL)
      sociarium_project_common::show_last_error(L"world_do_mouse_action/get_window_handle");
    HDC dc = GetDC(hwnd);
    if (dc==NULL)
      sociarium_project_common::show_last_error(L"world_do_mouse_action/GetDC");
    if (wglMakeCurrent(dc, rc_)==FALSE)
      sociarium_project_common::show_last_error(L"world_do_mouse_action/wglMakeCurrent(dc)");

    view_->scr2world_z0(mpos_world, mpos);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // 左クリック
    if (action==MouseAction::LBUTTON_DOWN) {
      select(mpos);
      mpos_LBUTTONDOWN = mpos;
      mpos_world_LBUTTONDOWN = mpos_world;

      // ----------------------------------------------------------------------------------------------------
      // 時系列スライダーのキャプチャ
      if (sociarium_project_mouse_and_selection::time_slider_is_selected()) {
        double my = double(mpos.y);
        double const soff_y = sociarium_project_draw::get_slider_offset().y;
        if (my<soff_y) my = soff_y;
        else if (my>view_->viewport_size().y-soff_y)
          my = view_->viewport_size().y-soff_y;
        double const y = (my-soff_y)/(view_->viewport_size().y-2*soff_y);
        time_series_->move_layer(int(y*(time_series_->number_of_layers()-1.0)+0.5));
      }
      // ----------------------------------------------------------------------------------------------------
      // ノードのキャプチャ
      else if (sociarium_project_mouse_and_selection::node_is_selected()) {
        shared_ptr<SociariumGraph const> const& g = time_series_->get_graph(0, time_series_->index_of_current_layer());
        Node const* n = g->node(sociarium_project_mouse_and_selection::get_selection()->get_id());
        shared_ptr<DynamicNodeProperty> const& dnp = g->property(n);
        dnp->set_flag(dnp->get_flag()|ElementFlag::CAPTURED);
        sociarium_project_mouse_and_selection::set_captured_object((void*)dnp.get());
      }
      // ----------------------------------------------------------------------------------------------------
      // エッジのキャプチャ
      else if (sociarium_project_mouse_and_selection::edge_is_selected()) {
        shared_ptr<SociariumGraph const> const& g = time_series_->get_graph(0, time_series_->index_of_current_layer());
        Edge const* e = g->edge(sociarium_project_mouse_and_selection::get_selection()->get_id());
        shared_ptr<DynamicEdgeProperty> const& dep = g->property(e);
        shared_ptr<DynamicNodeProperty> const& dnp0 = g->property(e->source());
        shared_ptr<DynamicNodeProperty> const& dnp1 = g->property(e->target());
        dnp0->set_flag(dnp0->get_flag()|ElementFlag::CAPTURED);
        dnp1->set_flag(dnp1->get_flag()|ElementFlag::CAPTURED);
        sociarium_project_mouse_and_selection::set_captured_object((void*)dep.get());
      }
      // ----------------------------------------------------------------------------------------------------
      // コミュニティのキャプチャ
      else if (sociarium_project_mouse_and_selection::community_is_selected()) {
        shared_ptr<SociariumGraph const> const& g = time_series_->get_graph(1, time_series_->index_of_current_layer());
        Node const* n = g->node(sociarium_project_mouse_and_selection::get_selection()->get_id());
        shared_ptr<DynamicNodeProperty> const& dnp = g->property(n);
        dnp->set_flag(dnp->get_flag()|ElementFlag::CAPTURED);
        for_each(dnp->lower_nbegin(), dnp->lower_nend(), boost::bind<void>(ActivateFlag(), _1, ElementFlag::CAPTURED));
        sociarium_project_mouse_and_selection::set_captured_object((void*)dnp.get());
      }
      // ----------------------------------------------------------------------------------------------------
      // コミュニティエッジのキャプチャ
      else if (sociarium_project_mouse_and_selection::community_edge_is_selected()) {
        shared_ptr<SociariumGraph const> const& g = time_series_->get_graph(1, time_series_->index_of_current_layer());
        Edge const* e = g->edge(sociarium_project_mouse_and_selection::get_selection()->get_id());
        shared_ptr<DynamicEdgeProperty> const& dep = g->property(e);
        shared_ptr<DynamicNodeProperty> const& dnp0 = g->property(e->source());
        shared_ptr<DynamicNodeProperty> const& dnp1 = g->property(e->target());
        dnp0->set_flag(dnp0->get_flag()|ElementFlag::CAPTURED);
        dnp1->set_flag(dnp1->get_flag()|ElementFlag::CAPTURED);
        for_each(dnp0->lower_nbegin(), dnp0->lower_nend(), boost::bind<void>(ActivateFlag(), _1, ElementFlag::CAPTURED));
        for_each(dnp1->lower_nbegin(), dnp1->lower_nend(), boost::bind<void>(ActivateFlag(), _1, ElementFlag::CAPTURED));
        sociarium_project_mouse_and_selection::set_captured_object((void*)dep.get());
      }
      else if (!(wp&MK_CONTROL)) {
        // ----------------------------------------------------------------------------------------------------
        // レイアウトフレームの移動
        if (sociarium_project_mouse_and_selection::layout_frame_is_selected()) {
          sociarium_project_draw::set_layout_frame_previous_position(sociarium_project_draw::get_layout_frame_position());
          sociarium_project_draw::set_update_layout_frame(true);
        }
        // ----------------------------------------------------------------------------------------------------
        // レイアウトフレームの拡大縮小
        else if (sociarium_project_mouse_and_selection::layout_frame_border_is_selected()) {
          sociarium_project_draw::set_layout_frame_previous_size(sociarium_project_draw::get_layout_frame_size());
          sociarium_project_draw::set_update_layout_frame(true);
        }
      }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // 右クリック
    else if (action==MouseAction::RBUTTON_DOWN) {
      select(mpos);
      mpos_RBUTTONDOWN = mpos;
      mpos_world_RBUTTONDOWN = mpos_world;
      angleH_prev = view_->angleH();
      angleV_prev = view_->angleV();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // 左リリース
    else if (action==MouseAction::LBUTTON_UP) {
      shared_ptr<SociariumGraph const> const& g0 = time_series_->get_graph(0, time_series_->index_of_current_layer());
      shared_ptr<SociariumGraph const> const& g1 = time_series_->get_graph(1, time_series_->index_of_current_layer());
      if (mpos==mpos_LBUTTONDOWN) {
        // ----------------------------------------------------------------------------------------------------
        // すべてのハイライトをクリア
        if (!(wp&MK_CONTROL)&&!sociarium_project_mouse_and_selection::time_slider_is_selected()) {
          for_each(g0->node_property_begin(), g0->node_property_end(), boost::bind<void>(DeactivateFlag(), _1, ElementFlag::HIGHLIGHT));
          for_each(g0->edge_property_begin(), g0->edge_property_end(), boost::bind<void>(DeactivateFlag(), _1, ElementFlag::HIGHLIGHT));
        }
#define CLICK_WITHOUT_CTRL_KEY_RESET_MARK
#ifdef CLICK_WITHOUT_CTRL_KEY_RESET_MARK
        // ----------------------------------------------------------------------------------------------------
        // すべてのマークをクリア
        if (!(wp&MK_CONTROL)&&!sociarium_project_mouse_and_selection::time_slider_is_selected()) {
          for_each(g0->node_property_begin(), g0->node_property_end(), boost::bind<void>(DeactivateFlag(), _1, ElementFlag::MARKED));
          for_each(g0->edge_property_begin(), g0->edge_property_end(), boost::bind<void>(DeactivateFlag(), _1, ElementFlag::MARKED));
          for_each(g1->node_property_begin(), g1->node_property_end(), boost::bind<void>(DeactivateFlag(), _1, ElementFlag::MARKED));
          for_each(g1->edge_property_begin(), g1->edge_property_end(), boost::bind<void>(DeactivateFlag(), _1, ElementFlag::MARKED));
        }
#endif
        // ----------------------------------------------------------------------------------------------------
        // ノードをマーク
        if (sociarium_project_mouse_and_selection::node_is_selected()) {
          Node const* n = g0->node(sociarium_project_mouse_and_selection::get_selection()->get_id());
          shared_ptr<DynamicNodeProperty> const& dnp = g0->property(n);
          if (is_marked(dnp)) dnp->set_flag(dnp->get_flag()&~ElementFlag::MARKED);
          else dnp->set_flag(dnp->get_flag()|ElementFlag::MARKED);
        }
        // ----------------------------------------------------------------------------------------------------
        // コミュニティをマーク
        else if (sociarium_project_mouse_and_selection::community_is_selected()) {
          Node const* n = g1->node(sociarium_project_mouse_and_selection::get_selection()->get_id());
          shared_ptr<DynamicNodeProperty> const& dnp = g1->property(n);
          if (is_marked(dnp)) dnp->set_flag(dnp->get_flag()&~ElementFlag::MARKED);
          else dnp->set_flag(dnp->get_flag()|ElementFlag::MARKED);
        }
        // ----------------------------------------------------------------------------------------------------
        // エッジをマーク
        else if (sociarium_project_mouse_and_selection::edge_is_selected()) {
          Edge const* e = g0->edge(sociarium_project_mouse_and_selection::get_selection()->get_id());
          shared_ptr<DynamicEdgeProperty> const& dep = g0->property(e);
          if (is_marked(dep)) dep->set_flag(dep->get_flag()&~ElementFlag::MARKED);
          else dep->set_flag(dep->get_flag()|ElementFlag::MARKED);
        }
        // ----------------------------------------------------------------------------------------------------
        // コミュニティエッジをマーク
        else if (sociarium_project_mouse_and_selection::community_edge_is_selected()) {
          Edge const* e = g1->edge(sociarium_project_mouse_and_selection::get_selection()->get_id());
          shared_ptr<DynamicEdgeProperty> const& dep = g1->property(e);
          if (is_marked(dep)) dep->set_flag(dep->get_flag()&~ElementFlag::MARKED);
          else dep->set_flag(dep->get_flag()|ElementFlag::MARKED);
        }
        // ----------------------------------------------------------------------------------------------------
        // 何もしない
        else if (sociarium_project_mouse_and_selection::time_slider_is_selected()) {}
        // ----------------------------------------------------------------------------------------------------
        // レイアウトフレームの移動
        else if (sociarium_project_mouse_and_selection::layout_frame_is_selected()) {
          
        }
        // ----------------------------------------------------------------------------------------------------
        // レイアウトフレームの拡大縮小
        else if (sociarium_project_mouse_and_selection::layout_frame_border_is_selected()) {
          
        }
#ifndef CLICK_WITHOUT_CTRL_KEY_RESET_MARK
        // ----------------------------------------------------------------------------------------------------
        // すべてのマークをクリア
        else if (!(wp&MK_CONTROL)) {
          for_each(g0->node_property_begin(), g0->node_property_end(), boost::bind<void>(DeactivateFlag(), _1, ElementFlag::MARKED));
          for_each(g0->edge_property_begin(), g0->edge_property_end(), boost::bind<void>(DeactivateFlag(), _1, ElementFlag::MARKED));
          for_each(g1->node_property_begin(), g1->node_property_end(), boost::bind<void>(DeactivateFlag(), _1, ElementFlag::MARKED));
          for_each(g1->edge_property_begin(), g1->edge_property_end(), boost::bind<void>(DeactivateFlag(), _1, ElementFlag::MARKED));
        }
#endif
      }
      // ----------------------------------------------------------------------------------------------------
      // キャプチャした要素のリリース
      if (sociarium_project_mouse_and_selection::get_captured_object()) {
        for_each(g0->node_property_begin(), g0->node_property_end(), boost::bind<void>(DeactivateFlag(), _1, ElementFlag::CAPTURED));
        for_each(g1->node_property_begin(), g1->node_property_end(), boost::bind<void>(DeactivateFlag(), _1, ElementFlag::CAPTURED));
        sociarium_project_mouse_and_selection::set_captured_object(0);
      }
      // ----------------------------------------------------------------------------------------------------
      // ドラッグエリアの解除
      if (sociarium_project_mouse_and_selection::get_drag_status()) {
        for_each(g0->node_property_begin(), g0->node_property_end(), boost::bind<void>(UpdateMark(), _1));
        for_each(g0->edge_property_begin(), g0->edge_property_end(), boost::bind<void>(UpdateMark(), _1));
        for_each(g1->node_property_begin(), g1->node_property_end(), boost::bind<void>(UpdateMark(), _1));
        for_each(g1->edge_property_begin(), g1->edge_property_end(), boost::bind<void>(UpdateMark(), _1));
        sociarium_project_mouse_and_selection::set_drag_status(false);
      }

      sociarium_project_draw::set_layout_frame_previous_position(sociarium_project_draw::get_layout_frame_position());
      sociarium_project_draw::set_layout_frame_previous_size(sociarium_project_draw::get_layout_frame_size());
      sociarium_project_draw::set_update_layout_frame(false);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // 右リリース
    else if (action==MouseAction::RBUTTON_UP) {
      shared_ptr<SociariumGraph const> const& g0 = time_series_->get_graph(0, time_series_->index_of_current_layer());
      shared_ptr<SociariumGraph const> const& g1 = time_series_->get_graph(1, time_series_->index_of_current_layer());
      if (mpos==mpos_RBUTTONDOWN) {
        // ----------------------------------------------------------------------------------------------------
        // すべてのハイライトをクリア
        if (!(wp&MK_CONTROL)) {
          for_each(g0->node_property_begin(), g0->node_property_end(), boost::bind<void>(DeactivateFlag(), _1, ElementFlag::HIGHLIGHT));
          for_each(g0->edge_property_begin(), g0->edge_property_end(), boost::bind<void>(DeactivateFlag(), _1, ElementFlag::HIGHLIGHT));
        }
        // ----------------------------------------------------------------------------------------------------
        // 隣接ノードをハイライト
        if (sociarium_project_mouse_and_selection::node_is_selected()) {
          Node const* n = g0->node(sociarium_project_mouse_and_selection::get_selection()->get_id());
          shared_ptr<DynamicNodeProperty> const& dnp = g0->property(n);
          dnp->set_flag(dnp->get_flag()|ElementFlag::HIGHLIGHT);
          for_each(n->obegin(), n->oend(), boost::bind<void>(ActivateFlag(), _1, ElementFlag::HIGHLIGHT, g0, +1));
          for_each(n->ibegin(), n->iend(), boost::bind<void>(ActivateFlag(), _1, ElementFlag::HIGHLIGHT, g0, -1));
        }
        // ----------------------------------------------------------------------------------------------------
        // 隣接エッジをハイライト
        else if (sociarium_project_mouse_and_selection::edge_is_selected()) {
          Edge const* e = g0->edge(sociarium_project_mouse_and_selection::get_selection()->get_id());
          shared_ptr<DynamicEdgeProperty> const& dep = g0->property(e);
          shared_ptr<DynamicNodeProperty> const& dnp0 = g0->property(e->source());
          shared_ptr<DynamicNodeProperty> const& dnp1 = g0->property(e->target());
          dep->set_flag(dep->get_flag()|ElementFlag::HIGHLIGHT);
          dnp0->set_flag(dnp0->get_flag()|ElementFlag::HIGHLIGHT);
          dnp1->set_flag(dnp1->get_flag()|ElementFlag::HIGHLIGHT);
        }
      }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // 移動
    else if (action==MouseAction::MOVE) {
      shared_ptr<SociariumGraph const> const& g0 = time_series_->get_graph(0, time_series_->index_of_current_layer());
      shared_ptr<SociariumGraph const> const& g1 = time_series_->get_graph(1, time_series_->index_of_current_layer());
      if (wp&MK_LBUTTON) {
        // ----------------------------------------------------------------------------------------------------
        // 時系列スライダーの移動
        if (sociarium_project_mouse_and_selection::time_slider_is_selected()) {
          double my = double(mpos.y);
          double const soff_y = sociarium_project_draw::get_slider_offset().y;
          if (my<soff_y) my = soff_y;
          else if (my>view_->viewport_size().y-soff_y)
            my = view_->viewport_size().y-soff_y;
          double const y = (my-soff_y)/(view_->viewport_size().y-2*soff_y);
          time_series_->move_layer(int(y*(time_series_->number_of_layers()-1.0)+0.5));
        }
        // ----------------------------------------------------------------------------------------------------
        // ノードの移動
        else if (sociarium_project_mouse_and_selection::get_captured_object()) {
          Vector2<float> const diff = mpos_world-mpos_world_prev;
          for_each(g0->node_property_begin(), g0->node_property_end(),
                   boost::bind<void>(MoveNode(), _1, ElementFlag::CAPTURED+ElementFlag::MARKED+ElementFlag::HIGHLIGHT, diff));
          for (SociariumGraph::node_property_iterator i=g1->node_property_begin(); i!=g1->node_property_end(); ++i)
            i->second->move_to_center_of_lower_nodes_position();
        }
        // --------------------------------------------------------------------------------
        // オブジェクト選択エリアの変更
        else if (wp&MK_CONTROL) {
          sociarium_project_mouse_and_selection::set_drag_status(true);
          {
            Vector2<double> const p0(double(mpos.x)/view_->viewport_size().x,
                                     double(mpos.y)/view_->viewport_size().y);
            Vector2<double> const p1(double(mpos_LBUTTONDOWN.x)/view_->viewport_size().x,
                                     double(mpos_LBUTTONDOWN.y)/view_->viewport_size().y);
            sociarium_project_mouse_and_selection::set_drag_region(0, p0);
            sociarium_project_mouse_and_selection::set_drag_region(1, Vector2<double>(p0.x, p1.y));
            sociarium_project_mouse_and_selection::set_drag_region(2, Vector2<double>(p1.x, p0.y));
            sociarium_project_mouse_and_selection::set_drag_region(3, p1);
          }

          Vector2<int> const p0(mpos_LBUTTONDOWN.x, mpos.y);
          Vector2<int> const p1(mpos.x, mpos_LBUTTONDOWN.y);
          Vector2<double> wp0;
          Vector2<double> wp1;
          view_->scr2world_z0(wp0, p0);
          view_->scr2world_z0(wp1, p1);
          Vector2<double> drag_region[4] = { mpos_world, wp0, wp1, mpos_world_LBUTTONDOWN };
          // 矩形内のノードを一時マーキング
          if (sociarium_project_view::get_show_node()) {
            for (SociariumGraph::node_property_iterator i=g0->node_property_begin(); i!=g0->node_property_end(); ++i) {
              shared_ptr<DynamicNodeProperty> const& dnp = i->second;
              if (point_is_in_trapezoid<double>(dnp->get_static_property()->get_position()+center_, drag_region)) {
                if (is_marked(dnp)) dnp->set_flag(dnp->get_flag()|ElementFlag::TEMPORARY_UNMARKED);
                else dnp->set_flag(dnp->get_flag()|ElementFlag::TEMPORARY_MARKED);
              } else dnp->set_flag(dnp->get_flag()&~(ElementFlag::TEMPORARY_MARKED|ElementFlag::TEMPORARY_UNMARKED));
            }
          }
          // 矩形内のエッジを一時マーキング
          if (sociarium_project_view::get_show_edge()) {
            for (SociariumGraph::edge_property_iterator i=g0->edge_property_begin(); i!=g0->edge_property_end(); ++i) {
              shared_ptr<DynamicEdgeProperty> const& dep = i->second;
              shared_ptr<DynamicNodeProperty> const& dnp0 = g0->property(i->first->source());
              shared_ptr<DynamicNodeProperty> const& dnp1 = g0->property(i->first->target());
              if (dnp0->get_flag()&(ElementFlag::TEMPORARY_MARKED|ElementFlag::TEMPORARY_UNMARKED) &&
                  dnp1->get_flag()&(ElementFlag::TEMPORARY_MARKED|ElementFlag::TEMPORARY_UNMARKED)) {
                if (is_marked(dep)) dep->set_flag(dep->get_flag()|ElementFlag::TEMPORARY_UNMARKED);
                else dep->set_flag(dep->get_flag()|ElementFlag::TEMPORARY_MARKED);
              } else dep->set_flag(dep->get_flag()&~(ElementFlag::TEMPORARY_MARKED|ElementFlag::TEMPORARY_UNMARKED));
            }
          }
          // 矩形内のコミュニティを一時マーキング
          if (sociarium_project_view::get_show_community()) {
            for (SociariumGraph::node_property_iterator i=g1->node_property_begin(); i!=g1->node_property_end(); ++i) {
              shared_ptr<DynamicNodeProperty> const& dnp = i->second;
              if (point_is_in_trapezoid<double>(dnp->get_static_property()->get_position()+center_, drag_region)) {
                if (is_marked(dnp)) dnp->set_flag(dnp->get_flag()|ElementFlag::TEMPORARY_UNMARKED);
                else dnp->set_flag(dnp->get_flag()|ElementFlag::TEMPORARY_MARKED);
              } else dnp->set_flag(dnp->get_flag()&~(ElementFlag::TEMPORARY_MARKED|ElementFlag::TEMPORARY_UNMARKED));
            }
          }
          // 矩形内のコミュニティエッジを一時マーキング
          if (sociarium_project_view::get_show_community_edge()) {
            for (SociariumGraph::edge_property_iterator i=g1->edge_property_begin(); i!=g1->edge_property_end(); ++i) {
              shared_ptr<DynamicEdgeProperty> const& dep = i->second;
              shared_ptr<DynamicNodeProperty> const& dnp0 = g1->property(i->first->source());
              shared_ptr<DynamicNodeProperty> const& dnp1 = g1->property(i->first->target());
              if (dnp0->get_flag()&(ElementFlag::TEMPORARY_MARKED|ElementFlag::TEMPORARY_UNMARKED) &&
                  dnp1->get_flag()&(ElementFlag::TEMPORARY_MARKED|ElementFlag::TEMPORARY_UNMARKED)) {
                if (is_marked(dep)) dep->set_flag(dep->get_flag()|ElementFlag::TEMPORARY_UNMARKED);
                else dep->set_flag(dep->get_flag()|ElementFlag::TEMPORARY_MARKED);
              } else dep->set_flag(dep->get_flag()&~(ElementFlag::TEMPORARY_MARKED|ElementFlag::TEMPORARY_UNMARKED));
            }
          }
        }
        // ----------------------------------------------------------------------------------------------------
        // レイアウトフレームの移動
        else if (sociarium_project_mouse_and_selection::layout_frame_is_selected()) {
          Vector2<int> diff(mpos_world-mpos_world_LBUTTONDOWN);
          sociarium_project_draw::set_layout_frame_position(sociarium_project_draw::get_layout_frame_previous_position()+diff);
        }
        // ----------------------------------------------------------------------------------------------------
        // レイアウトフレームの拡大縮小
        else if (sociarium_project_mouse_and_selection::layout_frame_border_is_selected()) {
          int const dx = int(abs(mpos_world.x-center_.x-double(sociarium_project_draw::get_layout_frame_position().x)));
          int const dy = int(abs(mpos_world.y-center_.y-double(sociarium_project_draw::get_layout_frame_position().y)));
          sociarium_project_draw::set_layout_frame_size(dx>dy?dx:dy);
        }
        // ----------------------------------------------------------------------------------------------------
        // 描画レイヤーの平行移動
        else {
          center_ += mpos_world-mpos_world_prev;
        }
      }
      // ----------------------------------------------------------------------------------------------------
      // 描画レイヤーの回転
      else if (wp&MK_RBUTTON) {
        int const dx = 5*(mpos.x-mpos_RBUTTONDOWN.x);
        int const dy = 5*(mpos.y-mpos_RBUTTONDOWN.y);
        view_->set_angle(angleH_prev+dx, angleV_prev-dy);
      }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // 左ダブルクリック
    else if (action==MouseAction::LBUTTON_DBL) {
      Node const* n = 0;
      Node const* c = 0;
      shared_ptr<SociariumGraph const> const& g0 = time_series_->get_graph(0, time_series_->index_of_current_layer());
      shared_ptr<SociariumGraph const> const& g1 = time_series_->get_graph(1, time_series_->index_of_current_layer());
      // ----------------------------------------------------------------------------------------------------
      // スライダーをダブルクリック
      if (sociarium_project_mouse_and_selection::time_slider_is_selected()); // 何もしない
      // ----------------------------------------------------------------------------------------------------
      // ノードをダブルクリック
      else if (sociarium_project_mouse_and_selection::node_is_selected()) n = g0->node(sociarium_project_mouse_and_selection::get_selection()->get_id());
      // ----------------------------------------------------------------------------------------------------
      // エッジをダブルクリック
      else if (sociarium_project_mouse_and_selection::edge_is_selected()) n = g0->edge(sociarium_project_mouse_and_selection::get_selection()->get_id())->source();
      // ----------------------------------------------------------------------------------------------------
      // コミュニティをダブルクリック
      else if (sociarium_project_mouse_and_selection::community_is_selected()) c = g1->node(sociarium_project_mouse_and_selection::get_selection()->get_id());
      // ----------------------------------------------------------------------------------------------------
      // コミュニティエッジをダブルクリック
      else if (sociarium_project_mouse_and_selection::community_edge_is_selected());
      // ----------------------------------------------------------------------------------------------------
      // 上記以外はマウス座標を画面中心に移動してズーム
      else {
        static int const step = 20;
        sociarium_project_mouse_and_selection::initialize_selection();
        Vector2<double> to;
        HWND hwnd = sociarium_project_common::get_window_handle();
        if (hwnd==0)
          sociarium_project_common::show_last_error(L"World::do_mouse_action/get_window_handle");
        if (view_->scr2world_z0(to, mpos)) {
          for (int i=0; i<step; ++i) {
            center_ -= to/double(step);
            zoom(1.0/1.05);
            draw();
          }
        }
      }
      // ----------------------------------------------------------------------------------------------------
      // ノードかエッジがダブルクリックされた場合は連結成分をマーク
      if (n!=0) {
        shared_ptr<AvoidHiddenElements> cond(new AvoidHiddenElements(g0));
        shared_ptr<BFSTraverser> t = BFSTraverser::create<bidirectional_tag>(g0);
        t->set_condition(cond);
        vector<Node*> cn = sociarium_project_graph_utility::connected_component(0, 0, t, n).second;
        vector<Edge*> ce = induced_edges(cn.begin(), cn.end());
        for_each(cn.begin(), cn.end(), boost::bind<void>(ActivateFlag(), _1, ElementFlag::MARKED, g0));
        for_each(ce.begin(), ce.end(), boost::bind<void>(ActivateFlag(), _1, ElementFlag::MARKED, g0));
      }
      // ----------------------------------------------------------------------------------------------------
      // コミュニティがダブルクリックされた場合はメンバーをマーク
      else if (c!=0) {
        shared_ptr<DynamicNodeProperty> const& dnp = g1->property(c);;
        for_each(dnp->lower_nbegin(), dnp->lower_nend(), boost::bind<void>(ActivateFlag(), _1, ElementFlag::MARKED));
        for_each(dnp->lower_ebegin(), dnp->lower_eend(), boost::bind<void>(ActivateFlag(), _1, ElementFlag::MARKED));
      }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // ホイールクリック
    else if (action==MouseAction::MBUTTON_DOWN) {
      // 視点の初期化
      initialize_view();
      // レイアウトフレームサイズの初期化
      if (wp&MK_CONTROL) sociarium_project_draw::set_layout_frame_size(sociarium_project_draw::get_layout_frame_default_size());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // ホイール回転
    else if (action==MouseAction::WHEEL) {
      // ズーム
      int const delta = short(HIWORD(wp))/120; // ±120 per 1ノッチ
      double const mag = 1.0+(delta>0.0?delta:-delta)/16.0;
      if (delta>0) zoom(mag);
      else zoom(1.0/mag);
    }

    mpos_prev = mpos;
    mpos_world_prev = mpos_world;

    if (wglMakeCurrent(0, 0)==FALSE)
      sociarium_project_common::show_last_error(L"World::do_mouse_action/wglMakeCurrent(0)");
    if (ReleaseDC(hwnd, dc)==0)
      sociarium_project_common::show_last_error(L"World::do_mouse_action/ReleaseDC");

    time_series_->read_unlock();
  }

} // The end of the namespace "hashimoto_ut"

// s.o.c.i.a.r.i.u.m: world_impl_mouse_action.cpp
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

#include <windows.h>
#include <boost/bind.hpp>
#include "world_impl.h"
#include "common.h"
#include "draw.h"
#include "layout.h"
#include "view.h"
#include "graph_utility.h"
#include "selection.h"
#include "flag_operation.h"
#include "algorithm_selector.h"
#include "sociarium_graph_time_series.h"
#include "thread/force_direction.h"
#include "../shared/GL/glview.h"
#include "../graph/graphex.h"
#include "../graph/util/traverser.h"

namespace hashimoto_ut {

  using std::vector;
  using std::pair;
  using std::make_pair;
  using std::tr1::shared_ptr;

  typedef SociariumGraph::node_property_iterator node_property_iterator;
  typedef SociariumGraph::edge_property_iterator edge_property_iterator;

  using namespace sociarium_project_draw;
  using namespace sociarium_project_view;

  namespace {

    ////////////////////////////////////////////////////////////////////////////////
    // Coordinates of the mouse pointer.
    Vector2<int> mpos_LBUTTONDOWN;
    Vector2<int> mpos_RBUTTONDOWN;
    Vector2<int> mpos_prev;
    Vector2<double> mpos_world;
    Vector2<double> mpos_world_LBUTTONDOWN;
    Vector2<double> mpos_world_RBUTTONDOWN;
    Vector2<double> mpos_world_prev;

    ////////////////////////////////////////////////////////////////////////////////
    int const ALL_MARKED =
      ElementFlag::TEMPORARY_MARKED|ElementFlag::TEMPORARY_UNMARKED;

    int const HIGHLIGHT_AND_CAPTURED =
      ElementFlag::CAPTURED|ElementFlag::HIGHLIGHT;

    int const HIGHLIGHT_AND_MARKED =
      ElementFlag::MARKED|ElementFlag::HIGHLIGHT;

    ////////////////////////////////////////////////////////////////////////////////
    // Angles of viewpoint.
    int angleH_prev;
    int angleV_prev;

    ////////////////////////////////////////////////////////////////////////////////
    // Marking by mouse dragging.
    struct UpdateMark {
      template <typename T>
      void operator()(T& pp) const {
        typedef typename T::second_type DynamicProperty;
        DynamicProperty& p = pp.second;
        if (is_visible(p)) {
          if (is_temporary_marked(p))
            p.set_flag((p.get_flag()|ElementFlag::MARKED)&~ElementFlag::TEMPORARY_MARKED);
          else if (is_temporary_unmarked(p))
            p.set_flag(p.get_flag()&~(ElementFlag::MARKED|ElementFlag::TEMPORARY_UNMARKED));
        }
      }
    };

    ////////////////////////////////////////////////////////////////////////////////
    // Move elements by mouse dragging.
    struct MoveNode {
      void operator()(node_property_iterator::value_type& pp,
                      unsigned int flag, Vector2<double> const& diff) const {
        DynamicNodeProperty& dnp = pp.second;
        if (is_visible(dnp) && (dnp.get_flag()&flag))
          dnp.get_static_property()->set_position(
            dnp.get_static_property()->get_position()+diff);
      }
    };

    ////////////////////////////////////////////////////////////////////////////////
    struct ActivateFlagIfMarked {
      void operator()(node_property_iterator::value_type& pp,
                      unsigned int flag) const {
        DynamicNodeProperty& dnp = pp.second;
        if (is_visible(dnp) && dnp.get_flag()&HIGHLIGHT_AND_MARKED)
          dnp.set_flag(dnp.get_flag()|flag);
      }
    };

  } // The end of the anonymous namespace


  void WorldImpl::do_mouse_action(int action, Vector2<int> const& mpos, WPARAM wp) {

    using namespace sociarium_project_selection;

    shared_ptr<SociariumGraphTimeSeries> ts
      = sociarium_project_graph_time_series::get();

    ts->read_lock();
    /*
     * Don't forget to call read_unlock().
     */

    shared_ptr<SociariumGraph const> g0 =
      ts->get_graph(0, ts->index_of_current_layer());

    shared_ptr<SociariumGraph const> g1 =
      ts->get_graph(1, ts->index_of_current_layer());

    view_->scr2world_z0(mpos_world, mpos);

    ////////////////////////////////////////////////////////////////////////////////
    // Left click.
    if (action==MouseAction::LBUTTON_DOWN) {
      select(mpos);
      mpos_LBUTTONDOWN = mpos;
      mpos_world_LBUTTONDOWN = mpos_world;

      // --------------------------------------------------------------------------------
      // Capture the time slider.
      if (is_selected(SelectionCategory::TIME_SLIDER)) {
        double my = mpos.y;
        double const soff_y = get_slider_offset().y;

        if (my<soff_y) my = soff_y;
        else if (my>view_->viewport_size().y-soff_y)
          my = view_->viewport_size().y-soff_y;

        double const h = view_->viewport_size().y-2*soff_y;
        double const y = (h<=0.0)?0.0:(my-soff_y)/h;
        ts->move_layer(int(y*(ts->number_of_layers()-1.0)+0.5));
      }

      // --------------------------------------------------------------------------------
      // Capture a node.
      else if (is_selected(SelectionCategory::NODE)) {
        Node const* n = static_cast<Node*>(get_selected_object());
        assert(n!=0);
        DynamicNodeProperty* dnp
          = static_cast<DynamicNodeProperty*>(get_selected_dynamic_object());
        assert(dnp!=0);

        dnp->set_flag(dnp->get_flag()|ElementFlag::CAPTURED);

        for_each(g0->node_property_begin(), g0->node_property_end(),
                 boost::bind<void>(ActivateFlagIfMarked(), _1, ElementFlag::CAPTURED));

        set_captured_object((void*)dnp);
      }

      // --------------------------------------------------------------------------------
      // Capture an edge.
      else if (is_selected(SelectionCategory::EDGE)) {
        Edge const* e = static_cast<Edge*>(get_selected_object());
        assert(e!=0);
        DynamicEdgeProperty* dep
          = static_cast<DynamicEdgeProperty*>(get_selected_dynamic_object());
        assert(dep!=0);

        DynamicNodeProperty& dnp0 = g0->property(e->source());
        DynamicNodeProperty& dnp1 = g0->property(e->target());

        dnp0.set_flag(dnp0.get_flag()|ElementFlag::CAPTURED);
        dnp1.set_flag(dnp1.get_flag()|ElementFlag::CAPTURED);

        for_each(g0->node_property_begin(), g0->node_property_end(),
                 boost::bind<void>(ActivateFlagIfMarked(), _1, ElementFlag::CAPTURED));

        set_captured_object((void*)dep);
      }

      // --------------------------------------------------------------------------------
      // Capture a community.
      else if (is_selected(SelectionCategory::COMMUNITY)) {
        Node const* n = static_cast<Node*>(get_selected_object());
        assert(n!=0);
        DynamicNodeProperty* dnp
          = static_cast<DynamicNodeProperty*>(get_selected_dynamic_object());
        assert(dnp!=0);

        dnp->set_flag(dnp->get_flag()|ElementFlag::CAPTURED);

        for_each(dnp->lower_nbegin(), dnp->lower_nend(),
                 boost::bind<void>(ActivateFlag(), _1, ElementFlag::CAPTURED));

        for_each(g0->node_property_begin(), g0->node_property_end(),
                 boost::bind<void>(ActivateFlagIfMarked(), _1, ElementFlag::CAPTURED));

        set_captured_object((void*)dnp);
      }

      // --------------------------------------------------------------------------------
      // Capture a community edge.
      else if (is_selected(SelectionCategory::COMMUNITY_EDGE)) {
        Edge const* e = static_cast<Edge*>(get_selected_object());
        assert(e!=0);
        DynamicEdgeProperty* dep
          = static_cast<DynamicEdgeProperty*>(get_selected_dynamic_object());
        assert(dep!=0);

        DynamicNodeProperty& dnp0 = g1->property(e->source());
        DynamicNodeProperty& dnp1 = g1->property(e->target());

        dnp0.set_flag(dnp0.get_flag()|ElementFlag::CAPTURED);
        dnp1.set_flag(dnp1.get_flag()|ElementFlag::CAPTURED);

        for_each(dnp0.lower_nbegin(), dnp0.lower_nend(),
                 boost::bind<void>(ActivateFlag(), _1, ElementFlag::CAPTURED));

        for_each(dnp1.lower_nbegin(), dnp1.lower_nend(),
                 boost::bind<void>(ActivateFlag(), _1, ElementFlag::CAPTURED));

        for_each(g0->node_property_begin(), g0->node_property_end(),
                 boost::bind<void>(ActivateFlagIfMarked(), _1, ElementFlag::CAPTURED));

        set_captured_object((void*)dep);
      }

      else if (!(wp&MK_CONTROL)) {
        using namespace sociarium_project_layout;
        // --------------------------------------------------------------------------------
        // Move the layout frame.
        if (is_selected(SelectionCategory::LAYOUT_FRAME)) {
          set_layout_frame_previous_position(get_layout_frame_position());
          set_update_layout_frame(true);
        }

        // --------------------------------------------------------------------------------
        // Resize the layout frame.
        else if (is_selected(SelectionCategory::LAYOUT_FRAME_BORDER)) {
          set_layout_frame_previous_size(get_layout_frame_size());
          set_update_layout_frame(true);
        }
      }
    }

    ////////////////////////////////////////////////////////////////////////////////
    // Right click.
    else if (action==MouseAction::RBUTTON_DOWN) {
      select(mpos);
      mpos_RBUTTONDOWN = mpos;
      mpos_world_RBUTTONDOWN = mpos_world;
      angleH_prev = view_->angleH();
      angleV_prev = view_->angleV();
    }

    ////////////////////////////////////////////////////////////////////////////////
    // Left release.
    else if (action==MouseAction::LBUTTON_UP) {
      if (mpos==mpos_LBUTTONDOWN) {
        // --------------------------------------------------------------------------------
        // Clear all highlight.
        if (!(wp&MK_CONTROL)&&!is_selected(SelectionCategory::TIME_SLIDER)) {
          for_each(g0->node_property_begin(), g0->node_property_end(),
                   boost::bind<void>(DeactivateFlag(), _1, ElementFlag::HIGHLIGHT));
          for_each(g0->edge_property_begin(), g0->edge_property_end(),
                   boost::bind<void>(DeactivateFlag(), _1, ElementFlag::HIGHLIGHT));
        }

#define CLICK_WITHOUT_CTRL_KEY_CLEAR_MARKING
#ifdef CLICK_WITHOUT_CTRL_KEY_CLEAR_MARKING
        // --------------------------------------------------------------------------------
        // Clear all marking.
        if (!(wp&MK_CONTROL)&&!is_selected(SelectionCategory::TIME_SLIDER)) {
          for_each(g0->node_property_begin(), g0->node_property_end(),
                   boost::bind<void>(DeactivateFlag(), _1, ElementFlag::MARKED));

          for_each(g0->edge_property_begin(), g0->edge_property_end(),
                   boost::bind<void>(DeactivateFlag(), _1, ElementFlag::MARKED));

          for_each(g1->node_property_begin(), g1->node_property_end(),
                   boost::bind<void>(DeactivateFlag(), _1, ElementFlag::MARKED));

          for_each(g1->edge_property_begin(), g1->edge_property_end(),
                   boost::bind<void>(DeactivateFlag(), _1, ElementFlag::MARKED));
        }
#endif

        // --------------------------------------------------------------------------------
        // Mark a node/community.
        if (is_selected(SelectionCategory::NODE)
            || is_selected(SelectionCategory::COMMUNITY)) {
          Node const* n = static_cast<Node*>(get_selected_object());
          assert(n!=0);
          DynamicNodeProperty* dnp
            = static_cast<DynamicNodeProperty*>(get_selected_dynamic_object());
          assert(dnp!=0);

          if (is_marked(*dnp)) dnp->set_flag(dnp->get_flag()&~ElementFlag::MARKED);
          else dnp->set_flag(dnp->get_flag()|ElementFlag::MARKED);
        }

        // --------------------------------------------------------------------------------
        // Mark an edge/community_edge.
        else if (is_selected(SelectionCategory::EDGE)
                 || is_selected(SelectionCategory::COMMUNITY_EDGE)) {
          Edge const* e = static_cast<Edge*>(get_selected_object());
          assert(e!=0);
          DynamicEdgeProperty* dep
            = static_cast<DynamicEdgeProperty*>(get_selected_dynamic_object());
          assert(dep!=0);

          if (is_marked(*dep)) dep->set_flag(dep->get_flag()&~ElementFlag::MARKED);
          else dep->set_flag(dep->get_flag()|ElementFlag::MARKED);
        }

        // --------------------------------------------------------------------------------
        else if (is_selected(SelectionCategory::TIME_SLIDER)) {
        }

        // --------------------------------------------------------------------------------
        else if (is_selected(SelectionCategory::LAYOUT_FRAME)) {
        }

        // --------------------------------------------------------------------------------
        else if (is_selected(SelectionCategory::LAYOUT_FRAME_BORDER)) {
        }

#ifndef CLICK_WITHOUT_CTRL_KEY_CLEAR_MARKING
        // --------------------------------------------------------------------------------
        // Clear all marking.
        else if (!(wp&MK_CONTROL)) {
          for_each(g0->node_property_begin(), g0->node_property_end(),
                   boost::bind<void>(DeactivateFlag(), _1, ElementFlag::MARKED));

          for_each(g0->edge_property_begin(), g0->edge_property_end(),
                   boost::bind<void>(DeactivateFlag(), _1, ElementFlag::MARKED));

          for_each(g1->node_property_begin(), g1->node_property_end(),
                   boost::bind<void>(DeactivateFlag(), _1, ElementFlag::MARKED));

          for_each(g1->edge_property_begin(), g1->edge_property_end(),
                   boost::bind<void>(DeactivateFlag(), _1, ElementFlag::MARKED));
        }
#endif
      }

      // --------------------------------------------------------------------------------
      // Release captured object.
      if (get_captured_object()) {
        for_each(g0->node_property_begin(), g0->node_property_end(),
                 boost::bind<void>(DeactivateFlag(), _1, ElementFlag::CAPTURED));

        for_each(g1->node_property_begin(), g1->node_property_end(),
                 boost::bind<void>(DeactivateFlag(), _1, ElementFlag::CAPTURED));

        set_captured_object(0);
      }

      // --------------------------------------------------------------------------------
      // Cancel the marking region.
      if (get_drag_status()) {
        for_each(g0->node_property_begin(), g0->node_property_end(),
                 boost::bind<void>(UpdateMark(), _1));

        for_each(g0->edge_property_begin(), g0->edge_property_end(),
                 boost::bind<void>(UpdateMark(), _1));

        for_each(g1->node_property_begin(), g1->node_property_end(),
                 boost::bind<void>(UpdateMark(), _1));

        for_each(g1->edge_property_begin(), g1->edge_property_end(),
                 boost::bind<void>(UpdateMark(), _1));

        set_drag_status(false);
      }

      using namespace sociarium_project_layout;
      set_layout_frame_previous_position(get_layout_frame_position());
      set_layout_frame_previous_size(get_layout_frame_size());
      set_update_layout_frame(false);
    }

    ////////////////////////////////////////////////////////////////////////////////
    // Right release.
    else if (action==MouseAction::RBUTTON_UP) {
      if (mpos==mpos_RBUTTONDOWN) {
        // --------------------------------------------------------------------------------
        // Clear all highlight.
        if (!(wp&MK_CONTROL)) {
          for_each(g0->node_property_begin(), g0->node_property_end(),
                   boost::bind<void>(DeactivateFlag(), _1, ElementFlag::HIGHLIGHT));

          for_each(g0->edge_property_begin(), g0->edge_property_end(),
                   boost::bind<void>(DeactivateFlag(), _1, ElementFlag::HIGHLIGHT));
        }

        // --------------------------------------------------------------------------------
        // Highlight adjacent nodes.
        if (is_selected(SelectionCategory::NODE)) {
          Node const* n = static_cast<Node*>(get_selected_object());
          assert(n!=0);
          DynamicNodeProperty* dnp
            = static_cast<DynamicNodeProperty*>(get_selected_dynamic_object());
          assert(dnp!=0);

          dnp->set_flag(dnp->get_flag()|HIGHLIGHT_AND_MARKED);

          for_each(n->obegin(), n->oend(),
                   boost::bind<void>(ActivateDynamicFlag(), _1, HIGHLIGHT_AND_MARKED, g0, +1));

          for_each(n->ibegin(), n->iend(),
                   boost::bind<void>(ActivateDynamicFlag(), _1, HIGHLIGHT_AND_MARKED, g0, -1));
        }

        // --------------------------------------------------------------------------------
        // Highlight connecting edges.
        else if (is_selected(SelectionCategory::EDGE)) {
          Edge const* e = static_cast<Edge*>(get_selected_object());
          assert(e!=0);
          DynamicEdgeProperty* dep
            = static_cast<DynamicEdgeProperty*>(get_selected_dynamic_object());
          assert(dep!=0);

          DynamicNodeProperty& dnp0 = g0->property(e->source());
          DynamicNodeProperty& dnp1 = g0->property(e->target());

          dep->set_flag(dep->get_flag()|HIGHLIGHT_AND_MARKED);
          dnp0.set_flag(dnp0.get_flag()|HIGHLIGHT_AND_MARKED);
          dnp1.set_flag(dnp1.get_flag()|HIGHLIGHT_AND_MARKED);
        }
      }
    }

    ////////////////////////////////////////////////////////////////////////////////
    // Mouse move.
    else if (action==MouseAction::MOVE) {
      if (wp&MK_LBUTTON) {
        // --------------------------------------------------------------------------------
        // Scroll the time slider.
        if (is_selected(SelectionCategory::TIME_SLIDER)) {
          double my = mpos.y;
          double const soff_y = get_slider_offset().y;

          if (my<soff_y) my = soff_y;
          else if (my>view_->viewport_size().y-soff_y)
            my = view_->viewport_size().y-soff_y;

          double const h = view_->viewport_size().y-2*soff_y;
          double const y = (h<=0.0)?0.0:(my-soff_y)/h;
          ts->move_layer(int(y*(ts->number_of_layers()-1.0)+0.5));
        }

        // --------------------------------------------------------------------------------
        // Move all marked elements.
        else if (get_captured_object()) {

          using namespace sociarium_project_algorithm_selector;

          Vector2<double> const diff = mpos_world-mpos_world_prev;
          for_each(g0->node_property_begin(), g0->node_property_end(),
                   boost::bind<void>(MoveNode(), _1, HIGHLIGHT_AND_CAPTURED, diff));

          if (get_force_direction_algorithm()!=RealTimeForceDirectionAlgorithm::COMMUNITY_ORIENTED)
            for_each(g1->node_property_begin(), g1->node_property_end(),
                     boost::bind<void>(
                       sociarium_project_layout::reset_position,
                       boost::bind<DynamicNodeProperty&>(
                         &SociariumGraph::NodePropertyMap::value_type::second, _1)));
          else
            for_each(g1->node_property_begin(), g1->node_property_end(),
                     boost::bind<void>(MoveNode(), _1, HIGHLIGHT_AND_CAPTURED, diff));
        }

        // --------------------------------------------------------------------------------
        // Resize the marking region.
        else if (wp&MK_CONTROL) {
          set_drag_status(true);

          { // Store for drawing.
            Vector2<double> const p0(
              view_->viewport_size().x==0
              ?0:double(mpos.x)/view_->viewport_size().x,
              view_->viewport_size().y==0
              ?0:double(mpos.y)/view_->viewport_size().y);

            Vector2<double> const p1(
              view_->viewport_size().x==0
              ?0:double(mpos_LBUTTONDOWN.x)/view_->viewport_size().x,
              view_->viewport_size().y==0
              ?0:double(mpos_LBUTTONDOWN.y)/view_->viewport_size().y);

            set_drag_region(0, p0);
            set_drag_region(1, Vector2<double>(p0.x, p1.y));
            set_drag_region(2, Vector2<double>(p1.x, p0.y));
            set_drag_region(3, p1);
          }

          // Get world coordinates of the mouse position.
          Vector2<int> const p0(mpos_LBUTTONDOWN.x, mpos.y);
          Vector2<int> const p1(mpos.x, mpos_LBUTTONDOWN.y);
          Vector2<double> wp0;
          Vector2<double> wp1;
          view_->scr2world_z0(wp0, p0);
          view_->scr2world_z0(wp1, p1);
          Vector2<double> drag_region[4] = { mpos_world, wp0, wp1, mpos_world_LBUTTONDOWN };

          // Temporary mark nodes in the dragged region.
          if (get_show_node()) {

            node_property_iterator i   = g0->node_property_begin();
            node_property_iterator end = g0->node_property_end();

            for (; i!=end; ++i) {
              DynamicNodeProperty& dnp = i->second;
              StaticNodeProperty* snp = dnp.get_static_property();

              if (point_is_in_trapezoid<double>(snp->get_position()+center_, drag_region)) {
                if (is_marked(dnp))
                  dnp.set_flag(dnp.get_flag()|ElementFlag::TEMPORARY_UNMARKED);
                else
                  dnp.set_flag(dnp.get_flag()|ElementFlag::TEMPORARY_MARKED);
              } else
                dnp.set_flag(dnp.get_flag()&~ALL_MARKED);
            }
          }

          // Temporary mark edges in the dragged region.
          if (get_show_edge()) {

            edge_property_iterator i   = g0->edge_property_begin();
            edge_property_iterator end = g0->edge_property_end();

            for (; i!=end; ++i) {
              DynamicEdgeProperty& dep = i->second;
              DynamicNodeProperty& dnp0 = g0->property(i->first->source());
              DynamicNodeProperty& dnp1 = g0->property(i->first->target());

              if (dnp0.get_flag()&ALL_MARKED && dnp1.get_flag()&ALL_MARKED) {
                if (is_marked(dep))
                  dep.set_flag(dep.get_flag()|ElementFlag::TEMPORARY_UNMARKED);
                else
                  dep.set_flag(dep.get_flag()|ElementFlag::TEMPORARY_MARKED);
              } else
                dep.set_flag(dep.get_flag()&~ALL_MARKED);
            }
          }

          // Temporary mark communities in the dragged region.
          if (get_show_community()) {

            node_property_iterator i   = g1->node_property_begin();
            node_property_iterator end = g1->node_property_end();

            for (; i!=end; ++i) {
              DynamicNodeProperty& dnp = i->second;
              StaticNodeProperty* snp = dnp.get_static_property();

              if (point_is_in_trapezoid<double>(snp->get_position()+center_, drag_region)) {
                if (is_marked(dnp))
                  dnp.set_flag(dnp.get_flag()|ElementFlag::TEMPORARY_UNMARKED);
                else
                  dnp.set_flag(dnp.get_flag()|ElementFlag::TEMPORARY_MARKED);
              } else
                dnp.set_flag(dnp.get_flag()&~ALL_MARKED);
            }
          }

          // Temporary mark community edges in the dragged region.
          if (get_show_community_edge()) {

            edge_property_iterator i   = g1->edge_property_begin();
            edge_property_iterator end = g1->edge_property_end();

            for (; i!=end; ++i) {
              DynamicEdgeProperty& dep = i->second;
              DynamicNodeProperty& dnp0 = g1->property(i->first->source());
              DynamicNodeProperty& dnp1 = g1->property(i->first->target());

              if (dnp0.get_flag()&ALL_MARKED && dnp1.get_flag()&ALL_MARKED) {
                if (is_marked(dep))
                  dep.set_flag(dep.get_flag()|ElementFlag::TEMPORARY_UNMARKED);
                else
                  dep.set_flag(dep.get_flag()|ElementFlag::TEMPORARY_MARKED);
              } else
                dep.set_flag(dep.get_flag()&~ALL_MARKED);
            }
          }
        }

        // --------------------------------------------------------------------------------
        // Move the layout frame.
        else if (is_selected(SelectionCategory::LAYOUT_FRAME)) {

          using namespace sociarium_project_layout;
          using namespace sociarium_project_algorithm_selector;

          Vector2<float> diff((mpos_world-mpos_world_LBUTTONDOWN).fcast());
          set_layout_frame_position(get_layout_frame_previous_position()+diff);

          if (get_force_direction_algorithm()
              ==RealTimeForceDirectionAlgorithm::LATTICE_GAS_METHOD)
            sociarium_project_force_direction::should_be_updated();
        }

        // --------------------------------------------------------------------------------
        // Resize the layout frame.
        else if (is_selected(SelectionCategory::LAYOUT_FRAME_BORDER)) {

          using namespace sociarium_project_layout;
          using namespace sociarium_project_algorithm_selector;

          float const dx = fabs(float(mpos_world.x)-center_.x-get_layout_frame_position().x);
          float const dy = fabs(float(mpos_world.y)-center_.y-get_layout_frame_position().y);
          set_layout_frame_size(dx>dy?dx:dy);

          if (get_force_direction_algorithm()
              ==RealTimeForceDirectionAlgorithm::LATTICE_GAS_METHOD)
            sociarium_project_force_direction::should_be_updated();
        }

        // --------------------------------------------------------------------------------
        else if (is_selected(SelectionCategory::DIAGRAM_FRAME)) {
        }

        // --------------------------------------------------------------------------------
        else if (is_selected(SelectionCategory::DIAGRAM_FRAME_BORDER)) {
        }

        // --------------------------------------------------------------------------------
        // Translate a layer.
        else {
          center_ += mpos_world-mpos_world_prev;
        }
      }

      // --------------------------------------------------------------------------------
      // Rotate a layer.
      else if (wp&MK_RBUTTON) {
        int const dx = 5*(mpos.x-mpos_RBUTTONDOWN.x);
        int const dy = 5*(mpos.y-mpos_RBUTTONDOWN.y);
        view_->set_angle(angleH_prev+dx, angleV_prev-dy);
      }
    }

    ////////////////////////////////////////////////////////////////////////////////
    // Left double click.
    else if (action==MouseAction::LBUTTON_DBL) {
      Node const* n = 0;
      Node const* c = 0;
      DynamicNodeProperty* dnp = 0;

      // --------------------------------------------------------------------------------
      if (is_selected(SelectionCategory::TIME_SLIDER)); // do nothing.

      // --------------------------------------------------------------------------------
      else if (is_selected(SelectionCategory::NODE)) {
        n = static_cast<Node*>(get_selected_object());
        assert(n!=0);
      }

      // --------------------------------------------------------------------------------
      else if (is_selected(SelectionCategory::EDGE)) {
        Edge const* e = static_cast<Edge*>(get_selected_object());
        assert(e!=0);
        n = e->source();
      }

      // --------------------------------------------------------------------------------
      else if (is_selected(SelectionCategory::COMMUNITY)) {
        c = static_cast<Node*>(get_selected_object());
        assert(c!=0);
        dnp = static_cast<DynamicNodeProperty*>(get_selected_dynamic_object());
        assert(dnp!=0);
      }

      // --------------------------------------------------------------------------------
      else if (is_selected(SelectionCategory::COMMUNITY_EDGE));

      // --------------------------------------------------------------------------------
      // Focusing zoom to the mouse position.
      else {
        static int const step = 20;
        sociarium_project_selection::initialize();
        Vector2<double> to;
        if (view_->scr2world_z0(to, mpos)) {
          for (int i=0; i<step; ++i) {
            center_ -= to/double(step);
            zoom(1.0/1.05);
            draw();
          }
        }
      }

      // --------------------------------------------------------------------------------
      // If a node or an edge is double clicked, the connected component is marked.
      if (n!=0) {
        shared_ptr<CircumventHiddenElements> cond(new CircumventHiddenElements(g0));
        shared_ptr<BFSTraverser> t = BFSTraverser::create<bidirectional_tag>(g0);
        t->set_condition(cond);
        vector<Node*> cn
          = sociarium_project_graph_utility::connected_component(0, 0, 0, t, n).second;
        vector<Edge*> ce = induced_edges(cn.begin(), cn.end());
        // Mark nodes in the connected component.
        for_each(cn.begin(), cn.end(),
                 boost::bind<void>(ActivateDynamicFlag(), _1, ElementFlag::MARKED, g0));
        // Mark edges in the connected component.
        for_each(ce.begin(), ce.end(),
                 boost::bind<void>(ActivateDynamicFlag(), _1, ElementFlag::MARKED, g0));
      }

      // --------------------------------------------------------------------------------
      // If a community is double clicked, community members are marked.
      else if (c!=0) {
        // Mark member nodes.
        for_each(dnp->lower_nbegin(), dnp->lower_nend(),
                 boost::bind<void>(ActivateFlag(), _1, ElementFlag::MARKED));
        // Mark member edges.
        for_each(dnp->lower_ebegin(), dnp->lower_eend(),
                 boost::bind<void>(ActivateFlag(), _1, ElementFlag::MARKED));
      }
    }

    ///////////////////////////////////////////////////////////////////////////////
    // Wheel click.
    else if (action==MouseAction::MBUTTON_DOWN) {
      using namespace sociarium_project_layout;
      initialize_view();
      if (wp&MK_CONTROL) set_layout_frame_size(get_layout_frame_default_size());
    }

    ////////////////////////////////////////////////////////////////////////////////
    // Wheel rotation.
    else if (action==MouseAction::WHEEL) {
      // zoom in or out.
      int const delta = short(HIWORD(wp))/120; // ±120 per 1 notch.
      double const mag = 1.0f+(delta>0?delta:-delta)/16.0f;
      if (delta>0) zoom(mag);
      else zoom(1.0/mag);
    }

    mpos_prev = mpos;
    mpos_world_prev = mpos_world;

    ts->read_unlock();
  }

} // The end of the namespace "hashimoto_ut"

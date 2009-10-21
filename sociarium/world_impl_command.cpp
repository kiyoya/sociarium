// s.o.c.i.a.r.i.u.m: world_impl_command.cpp
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
#include "algorithm_selector.h"
#include "common.h"
#include "community_transition_diagram.h"
#include "font.h"
#include "layout.h"
#include "menu_and_message.h"
#include "resource.h"
#include "thread.h"
#include "timeline.h"
#include "view.h"
#include "world_impl.h"
#include "thread/force_direction.h"
#include "../shared/msgbox.h"
#include "../shared/thread.h"

namespace hashimoto_ut {

  using std::pair;
  using std::tr1::shared_ptr;

  using namespace sociarium_project_algorithm_selector;
  using namespace sociarium_project_common;
  using namespace sociarium_project_font;
  using namespace sociarium_project_layout;
  using namespace sociarium_project_menu_and_message;
  using namespace sociarium_project_timeline;
  using namespace sociarium_project_view;

  void WorldImpl::do_command(int action) {
    switch (action) {
#if 0
    case IDM_KEY_W: {
      sociarium_project_designtide::switch_cvframe();
      break;
    }
#endif
    case IDM_KEY_Q:
    case IDM_KEY_CTRL_Q: {
#if 0
      if (sociarium_project_cvframe::joinable()) {
        sociarium_project_cvframe::terminate();
        sociarium_project_tamabi_library::set_cell_texture(false);
      } else {
        sociarium_project_tamabi_library::set_cell_texture(true);
        shared_ptr<CVFrame> cvframe = sociarium_project_cvframe::create();
        if (GetKeyState(VK_CONTROL)<0) cvframe->set_camera();
        else cvframe->set_movie(L"movie.avi");
        sociarium_project_cvframe::invoke(cvframe);
      }
#endif
#if 0
      static Texture const* texture_prev = 0;

      if (sociarium_project_cvframe::joinable()) {
        sociarium_project_cvframe::terminate();

        shared_ptr<SociariumGraphTimeSeries> ts
          = sociarium_project_graph_time_series::get();

        ts->read_lock();

        SociariumGraphTimeSeries::StaticNodePropertySet::iterator i
          = ts->find_static_node(0, 0);

        if (i==ts->static_node_property_end(0)) {
          ts->read_unlock();
          break;
        }

        i->set_texture(texture_prev);
        ts->read_unlock();

      } else {

        shared_ptr<SociariumGraphTimeSeries> ts
          = sociarium_project_graph_time_series::get();

        ts->read_lock();

        SociariumGraphTimeSeries::StaticNodePropertySet::iterator i
          = ts->find_static_node(0, 0);

        if (i==ts->static_node_property_end(0)) {
          ts->read_unlock();
          break;
        }

        texture_prev = i->get_texture();
        ts->read_unlock();

        shared_ptr<CVFrame> cvframe = sociarium_project_cvframe::create();
        if (GetKeyState(VK_CONTROL)<0) cvframe->set_camera();
        else cvframe->set_movie(L"movie.avi");
        cvframe->set_masking_image(L"balloon_mask.png");
        sociarium_project_cvframe::invoke(cvframe);
      }
#endif
#if 0
      sociarium_project_designtide::update();
#endif
      break;
    }

    case IDM_KEY_ESCAPE: {
      using namespace sociarium_project_thread;
      shared_ptr<Thread> tf = get_thread_function(GRAPH_CREATION);

      if (joinable(GRAPH_CREATION)) {
        tf->suspend();
        if (message_box(hwnd_, mb_ok_cancel, APPLICATION_TITLE,
                        get_message(Message::CANCEL_RUNNING_THREAD))) {
          tf->cancel();
          join(GRAPH_CREATION);
        } else
          tf->resume();
      } else
        SendMessage(hwnd_, WM_CLOSE, 0, 0);

      break;
    }

    case IDM_FILE_QUIT:
      SendMessage(hwnd_, WM_CLOSE, 0, 0);
      break;

    case IDM_FILE_CANCEL: {
      using namespace sociarium_project_thread;
      shared_ptr<Thread> tf = get_thread_function(GRAPH_CREATION);

      if (joinable(GRAPH_CREATION)) {
        tf->suspend();
        if (message_box(hwnd_, mb_ok_cancel, APPLICATION_TITLE,
                        get_message(Message::CANCEL_RUNNING_THREAD))) {
          tf->cancel();
          join(GRAPH_CREATION);
        } else
          tf->resume();
      }

      break;
    }

    case IDM_LAYOUT_CANCEL: {
      using namespace sociarium_project_thread;
      shared_ptr<Thread> tf = get_thread_function(LAYOUT);

      if (joinable(LAYOUT)) {
        tf->suspend();
        if (message_box(hwnd_, mb_ok_cancel, APPLICATION_TITLE,
                        get_message(Message::CANCEL_RUNNING_THREAD))) {
          tf->cancel();
          join(LAYOUT);
        } else
          tf->resume();
      }

      break;
    }

    case IDM_COMMUNITY_DETECTION_CANCEL: {
      using namespace sociarium_project_thread;
      shared_ptr<Thread> tf = get_thread_function(COMMUNITY_DETECTION);

      if (joinable(COMMUNITY_DETECTION)) {
        tf->suspend();
        if (message_box(hwnd_, mb_ok_cancel, APPLICATION_TITLE,
                        get_message(Message::CANCEL_RUNNING_THREAD))) {
          tf->cancel();
          join(COMMUNITY_DETECTION);
        } else
          tf->resume();
      }

      break;
    }

    case IDM_VIEW_NODE_SIZE_CANCEL: {
      using namespace sociarium_project_thread;
      shared_ptr<Thread> tf = get_thread_function(NODE_SIZE_UPDATE);

      if (joinable(NODE_SIZE_UPDATE)) {
        tf->suspend();
        if (message_box(hwnd_, mb_ok_cancel, APPLICATION_TITLE,
                        get_message(Message::CANCEL_RUNNING_THREAD))) {
          tf->cancel();
          join(NODE_SIZE_UPDATE);
        } else
          tf->resume();
      }

      break;
    }

    case IDM_VIEW_EDGE_WIDTH_CANCEL: {
      using namespace sociarium_project_thread;
      shared_ptr<Thread> tf = get_thread_function(EDGE_WIDTH_UPDATE);

      if (joinable(EDGE_WIDTH_UPDATE)) {
        tf->suspend();
        if (message_box(hwnd_, mb_ok_cancel, APPLICATION_TITLE,
                        get_message(Message::CANCEL_RUNNING_THREAD))) {
          tf->cancel();
          join(EDGE_WIDTH_UPDATE);
        } else
          tf->resume();
      }

      break;
    }

    case IDM_LAYOUT_UPDATE:
    case IDM_KEY_SPACE:
      layout();
      break;

    case IDM_COMMUNITY_DETECTION_UPDATE:
    case IDM_KEY_ENTER:
      detect_community();
      break;

    case IDM_LAYOUT_INITIALIZE_EYEPOINT:
    case IDM_KEY_HOME:
      initialize_view();
      break;

    case IDM_LAYOUT_INITIALIZE_LAYOUT_FRAME:
    case IDM_KEY_CTRL_HOME: {

      set_layout_frame_size(get_layout_frame_default_size());
      set_layout_frame_previous_size(get_layout_frame_size());
      set_layout_frame_position(Vector2<int>(0, 0));
      set_layout_frame_previous_position(get_layout_frame_position());

      if (get_force_direction_algorithm()
          ==RealTimeForceDirectionAlgorithm::LATTICE_GAS_METHOD)
        sociarium_project_force_direction::should_be_updated();

      break;
    }

      {
        // --------------------------------------------------------------------------------
        // COMMUNITY TRANSITION DIAGRAM

        using namespace sociarium_project_community_transition_diagram;

      case IDM_KEY_CTRL_RIGHT: {
        pair<int, int> const& scope = get_scope();
        set_scope(scope.first-1, scope.second+1);
        break;
      }

      case IDM_KEY_CTRL_LEFT: {
        pair<int, int> const& scope = get_scope();
        set_scope(scope.first+1, scope.second-1);
        break;
      }

      case IDM_COMMUNITY_TRANSITION_DIAGRAM_SCOPE_WIDER: {
        int const size = int(2*get_resolution());
        for (int i=0; i<size; ++i) {
          pair<int, int> const& scope = get_scope();
          set_scope(scope.first-1, scope.second+1);
          draw();
        }
        break;
      }

      case IDM_COMMUNITY_TRANSITION_DIAGRAM_SCOPE_NARROWER: {
        int const size = int(2*get_resolution());
        for (int i=0; i<size; ++i) {
          pair<int, int> const& scope = get_scope();
          set_scope(scope.first+1, scope.second-1);
          draw();
        }
        break;
      }

      }


      // --------------------------------------------------------------------------------
      // TIMELINE

    case IDM_TIMELINE_STOP:
      set_auto_run_id(AutoRun::STOP);
      break;

    case IDM_TIMELINE_FORWARD_1:
      set_auto_run_id(AutoRun::FORWARD_1);
      break;

    case IDM_TIMELINE_FORWARD_2:
      set_auto_run_id(AutoRun::FORWARD_2);
      break;

    case IDM_TIMELINE_FORWARD_3:
      set_auto_run_id(AutoRun::FORWARD_3);
      break;

    case IDM_TIMELINE_FORWARD_4:
      set_auto_run_id(AutoRun::FORWARD_4);
      break;

    case IDM_TIMELINE_BACKWARD_1:
      set_auto_run_id(AutoRun::BACKWARD_1);
      break;

    case IDM_TIMELINE_BACKWARD_2:
      set_auto_run_id(AutoRun::BACKWARD_2);
      break;

    case IDM_TIMELINE_BACKWARD_3:
      set_auto_run_id(AutoRun::BACKWARD_3);
      break;

    case IDM_TIMELINE_BACKWARD_4:
      set_auto_run_id(AutoRun::BACKWARD_4);
      break;


      // --------------------------------------------------------------------------------
      // EDIT

      // MARK
    case IDM_EDIT_MARK_ALL_NODES_ON_CURRENT_LAYER:
    case IDM_EDIT_MARK_ALL_EDGES_ON_CURRENT_LAYER:
    case IDM_EDIT_MARK_NODES_INSIDE_COMMUNITY_ON_CURRENT_LAYER:
    case IDM_EDIT_MARK_EDGES_INSIDE_COMMUNITY_ON_CURRENT_LAYER:
    case IDM_EDIT_MARK_ELEMENTS_INSIDE_COMMUNITY_ON_CURRENT_LAYER:
    case IDM_EDIT_MARK_NODES_OUTSIDE_COMMUNITY_ON_CURRENT_LAYER:
    case IDM_EDIT_MARK_EDGES_OUTSIDE_COMMUNITY_ON_CURRENT_LAYER:
    case IDM_EDIT_MARK_ELEMENTS_OUTSIDE_COMMUNITY_ON_CURRENT_LAYER:
    case IDM_EDIT_MARK_NODES_IN_SELECTED_COMMUNITY_CONTINUUMS_ON_CURRENT_LAYER:
    case IDM_EDIT_MARK_EDGES_IN_SELECTED_COMMUNITY_CONTINUUMS_ON_CURRENT_LAYER:
    case IDM_EDIT_MARK_ELEMENTS_IN_SELECTED_COMMUNITY_CONTINUUMS_ON_CURRENT_LAYER:
      mark_elements(action);
      break;

    case IDM_EDIT_MARK_ALL_NODES_ON_EACH_LAYER:
    case IDM_EDIT_MARK_ALL_EDGES_ON_EACH_LAYER:
    case IDM_EDIT_MARK_NODES_INSIDE_COMMUNITY_ON_EACH_LAYER:
    case IDM_EDIT_MARK_EDGES_INSIDE_COMMUNITY_ON_EACH_LAYER:
    case IDM_EDIT_MARK_ELEMENTS_INSIDE_COMMUNITY_ON_EACH_LAYER:
    case IDM_EDIT_MARK_NODES_OUTSIDE_COMMUNITY_ON_EACH_LAYER:
    case IDM_EDIT_MARK_EDGES_OUTSIDE_COMMUNITY_ON_EACH_LAYER:
    case IDM_EDIT_MARK_ELEMENTS_OUTSIDE_COMMUNITY_ON_EACH_LAYER:
    case IDM_EDIT_MARK_NODES_IN_SELECTED_COMMUNITY_CONTINUUMS_ON_EACH_LAYER:
    case IDM_EDIT_MARK_EDGES_IN_SELECTED_COMMUNITY_CONTINUUMS_ON_EACH_LAYER:
    case IDM_EDIT_MARK_ELEMENTS_IN_SELECTED_COMMUNITY_CONTINUUMS_ON_EACH_LAYER:
      mark_elements(action);
      break;

    case IDM_KEY_CTRL_A:
    case IDM_EDIT_MARK_ALL_ELEMENTS_ON_CURRENT_LAYER:
      mark_elements(IDM_EDIT_MARK_ALL_ELEMENTS_ON_CURRENT_LAYER);
      break;

    case IDM_KEY_SHIFT_CTRL_A:
    case IDM_EDIT_MARK_ALL_ELEMENTS_ON_EACH_LAYER:
      mark_elements(IDM_EDIT_MARK_ALL_ELEMENTS_ON_EACH_LAYER);
      break;

    case IDM_KEY_CTRL_I:
    case IDM_EDIT_INVERT_MARK_ON_CURRENT_LAYER:
      inverse_mark(IDM_EDIT_INVERT_MARK_ON_CURRENT_LAYER);
      break;

    case IDM_KEY_SHIFT_CTRL_I:
    case IDM_EDIT_INVERT_MARK_ON_EACH_LAYER:
      inverse_mark(IDM_EDIT_INVERT_MARK_ON_EACH_LAYER);
      break;

      // HIDE
    case IDM_EDIT_HIDE_MARKED_NODES_ON_CURRENT_LAYER:
    case IDM_EDIT_HIDE_MARKED_EDGES_ON_CURRENT_LAYER:
      hide_marked_element(action);
      break;

    case IDM_KEY_CTRL_H:
    case IDM_EDIT_HIDE_MARKED_ELEMENTS_ON_CURRENT_LAYER:
      hide_marked_element(IDM_EDIT_HIDE_MARKED_ELEMENTS_ON_CURRENT_LAYER);
      break;

    case IDM_EDIT_HIDE_MARKED_NODES_ON_EACH_LAYER:
    case IDM_EDIT_HIDE_MARKED_EDGES_ON_EACH_LAYER:
      hide_marked_element(action);
      break;

    case IDM_KEY_SHIFT_CTRL_H:
    case IDM_EDIT_HIDE_MARKED_ELEMENTS_ON_EACH_LAYER:
      hide_marked_element(IDM_EDIT_HIDE_MARKED_ELEMENTS_ON_EACH_LAYER);
      break;

    case IDM_EDIT_HIDE_MARKED_NODES_ON_CURRENT_LAYER_ALL:
    case IDM_EDIT_HIDE_MARKED_EDGES_ON_CURRENT_LAYER_ALL:
    case IDM_EDIT_HIDE_MARKED_ELEMENTS_ON_CURRENT_LAYER_ALL:
      hide_marked_element(action);
      break;

    case IDM_EDIT_HIDE_MARKED_NODES_ON_EACH_LAYER_ALL:
    case IDM_EDIT_HIDE_MARKED_EDGES_ON_EACH_LAYER_ALL:
    case IDM_EDIT_HIDE_MARKED_ELEMENTS_ON_EACH_LAYER_ALL:
      hide_marked_element(action);
      break;

      // REMOVE
    case IDM_EDIT_REMOVE_MARKED_NODES_ON_CURRENT_LAYER:
    case IDM_EDIT_REMOVE_MARKED_EDGES_ON_CURRENT_LAYER:
      remove_marked_elements(action);
      break;

    case IDM_KEY_DELETE:
    case IDM_EDIT_REMOVE_MARKED_ELEMENTS_ON_CURRENT_LAYER:
      remove_marked_elements(IDM_EDIT_REMOVE_MARKED_ELEMENTS_ON_CURRENT_LAYER);
      break;

    case IDM_EDIT_REMOVE_MARKED_NODES_ON_EACH_LAYER:
    case IDM_EDIT_REMOVE_MARKED_EDGES_ON_EACH_LAYER:
      remove_marked_elements(action);
      break;

    case IDM_KEY_CTRL_DELETE:
    case IDM_EDIT_REMOVE_MARKED_ELEMENTS_ON_EACH_LAYER:
      remove_marked_elements(IDM_EDIT_REMOVE_MARKED_ELEMENTS_ON_EACH_LAYER);
      break;

    case IDM_EDIT_REMOVE_MARKED_NODES_ON_CURRENT_LAYER_ALL:
    case IDM_EDIT_REMOVE_MARKED_EDGES_ON_CURRENT_LAYER_ALL:
    case IDM_EDIT_REMOVE_MARKED_ELEMENTS_ON_CURRENT_LAYER_ALL:
      remove_marked_elements(action);
      break;

    case IDM_EDIT_REMOVE_MARKED_NODES_ON_EACH_LAYER_ALL:
    case IDM_EDIT_REMOVE_MARKED_EDGES_ON_EACH_LAYER_ALL:
    case IDM_EDIT_REMOVE_MARKED_ELEMENTS_ON_EACH_LAYER_ALL:
      remove_marked_elements(action);
      break;

      // SHOW
    case IDM_EDIT_SHOW_HIDDEN_NODES_ON_CURRENT_LAYER:
    case IDM_EDIT_SHOW_HIDDEN_EDGES_ON_CURRENT_LAYER:
      show_elements(action);
      break;

    case IDM_KEY_CTRL_U:
    case IDM_EDIT_SHOW_HIDDEN_ELEMENTS_ON_CURRENT_LAYER:
      show_elements(IDM_EDIT_SHOW_HIDDEN_ELEMENTS_ON_CURRENT_LAYER);
      break;

    case IDM_EDIT_SHOW_HIDDEN_EDGES_ON_EACH_LAYER:
    case IDM_EDIT_SHOW_HIDDEN_NODES_ON_EACH_LAYER:
      show_elements(action);
      break;

    case IDM_KEY_SHIFT_CTRL_U:
    case IDM_EDIT_SHOW_HIDDEN_ELEMENTS_ON_EACH_LAYER:
      show_elements(IDM_EDIT_SHOW_HIDDEN_ELEMENTS_ON_EACH_LAYER);
      break;


      // --------------------------------------------------------------------------------
      // VIEW

    case IDM_KEY_1:
    case IDM_VIEW_SHOW_NODE:
      set_show_node(!get_show_node());
      break;

    case IDM_KEY_2:
    case IDM_VIEW_SHOW_EDGE:
      set_show_edge(!get_show_edge());
      break;

    case IDM_KEY_3:
    case IDM_VIEW_SHOW_COMMUNITY:
      set_show_community(!get_show_community());
      break;

    case IDM_KEY_4:
    case IDM_VIEW_SHOW_COMMUNITY_EDGE:
      set_show_community_edge(!get_show_community_edge());
      break;

    case IDM_KEY_CTRL_1:
    case IDM_STRING_SHOW_NODE_NAME:
      set_show_node_name(!get_show_node_name());
      break;

    case IDM_KEY_CTRL_2:
    case IDM_STRING_SHOW_EDGE_NAME:
      set_show_edge_name(!get_show_edge_name());
      break;

    case IDM_KEY_CTRL_3:
    case IDM_STRING_SHOW_COMMUNITY_NAME:
      set_show_community_name(!get_show_community_name());
      break;

    case IDM_KEY_CTRL_4:
    case IDM_STRING_SHOW_COMMUNITY_EDGE_NAME:
      set_show_community_edge_name(!get_show_community_edge_name());
      break;

    case IDM_STRING_NODE_NAME_SIZE_VARIABLE:
      set_node_name_size_variable(!get_node_name_size_variable());
      break;

    case IDM_STRING_EDGE_NAME_SIZE_VARIABLE:
      set_edge_name_size_variable(!get_edge_name_size_variable());
      break;

    case IDM_STRING_COMMUNITY_NAME_SIZE_VARIABLE:
      set_community_name_size_variable(!get_community_name_size_variable());
      break;

    case IDM_STRING_COMMUNITY_EDGE_NAME_SIZE_VARIABLE:
      set_community_edge_name_size_variable(!get_community_edge_name_size_variable());
      break;

    case IDM_KEY_SHIFT_CTRL_1:
      shift_node_style();
      break;

    case IDM_KEY_SHIFT_CTRL_2:
      shift_edge_style();
      break;

    case IDM_KEY_SHIFT_CTRL_3:
      shift_community_style();
      break;

    case IDM_KEY_SHIFT_CTRL_4:
      shift_community_edge_style();
      break;

    case IDM_VIEW_NODE_STYLE_POLYGON:
      set_node_style(NodeStyle::POLYGON);
      break;

    case IDM_VIEW_NODE_STYLE_TEXTURE:
      set_node_style(NodeStyle::TEXTURE);
      break;

    case IDM_VIEW_EDGE_STYLE_LINE:
      set_edge_style(EdgeStyle::LINE);
      break;

    case IDM_VIEW_EDGE_STYLE_POLYGON:
      set_edge_style(EdgeStyle::POLYGON);
      break;

    case IDM_VIEW_COMMUNITY_STYLE_POLYGON_CIRCLE:
      set_community_style(CommunityStyle::POLYGON_CIRCLE);
      break;

    case IDM_VIEW_COMMUNITY_STYLE_TEXTURE:
      set_community_style(CommunityStyle::TEXTURE);
      break;

    case IDM_VIEW_COMMUNITY_EDGE_STYLE_LINE:
      set_community_edge_style(CommunityEdgeStyle::LINE);
      break;

    case IDM_VIEW_COMMUNITY_EDGE_STYLE_POLYGON:
      set_community_edge_style(CommunityEdgeStyle::POLYGON);
      break;

    case IDM_KEY_S:
    case IDM_TIMELINE_SHOW_SLIDER:
      set_show_slider(!get_show_slider());
      break;

    case IDM_KEY_L:
    case IDM_STRING_SHOW_LAYER_NAME:
      set_show_layer_name(!get_show_layer_name());
      break;

    case IDM_KEY_F:
    case IDM_LAYOUT_SHOW_LAYOUT_FRAME:
      set_show_layout_frame(!get_show_layout_frame());
      break;

    case IDM_KEY_G:
    case IDM_LAYOUT_SHOW_GRID:
      set_show_grid(!get_show_grid());
      break;

    case IDM_KEY_CTRL_F:
    case IDM_STRING_SHOW_FPS:
      set_show_fps(!get_show_fps());
      break;

    case IDM_LAYOUT_SHOW_CENTER:
      set_show_center(!get_show_center());
      break;


      // --------------------------------------------------------------------------------
      // NODE SIZE

    case IDM_VIEW_NODE_SIZE_UPDATE:
      update_node_size();
      break;

    case IDM_VIEW_NODE_SIZE_UNIFORM:
      set_node_size_update_algorithm(NodeSizeUpdateAlgorithm::UNIFORM);
      update_node_size();
      break;

    case IDM_VIEW_NODE_SIZE_WEIGHT:
      set_node_size_update_algorithm(NodeSizeUpdateAlgorithm::WEIGHT);
      update_node_size();
      break;

    case IDM_VIEW_NODE_SIZE_DEGREE_CENTRALITY:
      set_node_size_update_algorithm(NodeSizeUpdateAlgorithm::DEGREE_CENTRALITY);
      update_node_size();
      break;

    case IDM_VIEW_NODE_SIZE_CLOSENESS_CENTRALITY:
      set_node_size_update_algorithm(NodeSizeUpdateAlgorithm::CLOSENESS_CENTRALITY);
      update_node_size();
      break;

    case IDM_VIEW_NODE_SIZE_BETWEENNESS_CENTRALITY:
      set_node_size_update_algorithm(NodeSizeUpdateAlgorithm::BETWEENNESS_CENTRALITY);
      update_node_size();
      break;

    case IDM_VIEW_NODE_SIZE_PAGERANK:
      set_node_size_update_algorithm(NodeSizeUpdateAlgorithm::PAGERANK);
      update_node_size();
      break;

    case IDM_VIEW_NODE_SIZE_POINT:
      set_node_size_update_algorithm(NodeSizeUpdateAlgorithm::POINT);
      update_node_size();
      break;


      // --------------------------------------------------------------------------------
      // EDGE WIDTH

    case IDM_VIEW_EDGE_WIDTH_UPDATE:
      update_edge_width();
      break;

    case IDM_VIEW_EDGE_WIDTH_UNIFORM:
      set_edge_width_update_algorithm(EdgeWidthUpdateAlgorithm::UNIFORM);
      update_edge_width();
      break;

    case IDM_VIEW_EDGE_WIDTH_WEIGHT:
      set_edge_width_update_algorithm(EdgeWidthUpdateAlgorithm::WEIGHT);
      update_edge_width();
      break;

    case IDM_VIEW_EDGE_WIDTH_BETWEENNESS_CENTRALITY:
      set_edge_width_update_algorithm(EdgeWidthUpdateAlgorithm::BETWEENNESS_CENTRALITY);
      update_edge_width();
      break;


      // --------------------------------------------------------------------------------
      // FONT

    case IDM_STRING_NODE_NAME_SIZE_0: set_font_scale(FontCategory::NODE_NAME, 0); break;
    case IDM_STRING_NODE_NAME_SIZE_1: set_font_scale(FontCategory::NODE_NAME, 1); break;
    case IDM_STRING_NODE_NAME_SIZE_2: set_font_scale(FontCategory::NODE_NAME, 2); break;
    case IDM_STRING_NODE_NAME_SIZE_3: set_font_scale(FontCategory::NODE_NAME, 3); break;
    case IDM_STRING_NODE_NAME_SIZE_4: set_font_scale(FontCategory::NODE_NAME, 4); break;

    case IDM_STRING_EDGE_NAME_SIZE_0: set_font_scale(FontCategory::EDGE_NAME, 0); break;
    case IDM_STRING_EDGE_NAME_SIZE_1: set_font_scale(FontCategory::EDGE_NAME, 1); break;
    case IDM_STRING_EDGE_NAME_SIZE_2: set_font_scale(FontCategory::EDGE_NAME, 2); break;
    case IDM_STRING_EDGE_NAME_SIZE_3: set_font_scale(FontCategory::EDGE_NAME, 3); break;
    case IDM_STRING_EDGE_NAME_SIZE_4: set_font_scale(FontCategory::EDGE_NAME, 4); break;

    case IDM_STRING_COMMUNITY_NAME_SIZE_0: set_font_scale(FontCategory::COMMUNITY_NAME, 0); break;
    case IDM_STRING_COMMUNITY_NAME_SIZE_1: set_font_scale(FontCategory::COMMUNITY_NAME, 1); break;
    case IDM_STRING_COMMUNITY_NAME_SIZE_2: set_font_scale(FontCategory::COMMUNITY_NAME, 2); break;
    case IDM_STRING_COMMUNITY_NAME_SIZE_3: set_font_scale(FontCategory::COMMUNITY_NAME, 3); break;
    case IDM_STRING_COMMUNITY_NAME_SIZE_4: set_font_scale(FontCategory::COMMUNITY_NAME, 4); break;

    case IDM_STRING_COMMUNITY_EDGE_NAME_SIZE_0: set_font_scale(FontCategory::COMMUNITY_EDGE_NAME, 0); break;
    case IDM_STRING_COMMUNITY_EDGE_NAME_SIZE_1: set_font_scale(FontCategory::COMMUNITY_EDGE_NAME, 1); break;
    case IDM_STRING_COMMUNITY_EDGE_NAME_SIZE_2: set_font_scale(FontCategory::COMMUNITY_EDGE_NAME, 2); break;
    case IDM_STRING_COMMUNITY_EDGE_NAME_SIZE_3: set_font_scale(FontCategory::COMMUNITY_EDGE_NAME, 3); break;
    case IDM_STRING_COMMUNITY_EDGE_NAME_SIZE_4: set_font_scale(FontCategory::COMMUNITY_EDGE_NAME, 4); break;

    case IDM_STRING_FONT_TYPE_POLYGON: set_font_type(FontType::POLYGON_FONT); break;
    case IDM_STRING_FONT_TYPE_TEXTURE: set_font_type(FontType::TEXTURE_FONT); break;


      // --------------------------------------------------------------------------------
      // LAYOUT

    case IDM_LAYOUT_KAMADA_KAWAI_METHOD:
      set_layout_algorithm(LayoutAlgorithm::KAMADA_KAWAI_METHOD);
      layout();
      break;

    case IDM_LAYOUT_HIGH_DIMENSIONAL_EMBEDDING_1_2:
      set_layout_algorithm(LayoutAlgorithm::HIGH_DIMENSIONAL_EMBEDDING_1_2);
      layout();
      break;

    case IDM_LAYOUT_HIGH_DIMENSIONAL_EMBEDDING_1_3:
      set_layout_algorithm(LayoutAlgorithm::HIGH_DIMENSIONAL_EMBEDDING_1_3);
      layout();
      break;

    case IDM_LAYOUT_HIGH_DIMENSIONAL_EMBEDDING_2_3:
      set_layout_algorithm(LayoutAlgorithm::HIGH_DIMENSIONAL_EMBEDDING_2_3);
      layout();
      break;

    case IDM_LAYOUT_CIRCLE:
      set_layout_algorithm(LayoutAlgorithm::CIRCLE);
      layout();
      break;

    case IDM_LAYOUT_CIRCLE_IN_SIZE_ORDER:
      set_layout_algorithm(LayoutAlgorithm::CIRCLE_IN_SIZE_ORDER);
      layout();
      break;

    case IDM_LAYOUT_LATTICE:
      set_layout_algorithm(LayoutAlgorithm::LATTICE);
      layout();
      break;

    case IDM_LAYOUT_RANDOM:
      set_layout_algorithm(LayoutAlgorithm::RANDOM);
      layout();
      break;

    case IDM_LAYOUT_CARTOGRAMS:
      set_layout_algorithm(LayoutAlgorithm::CARTOGRAMS);
      layout();
      break;

    case IDM_KEY_CTRL_SPACE:
    case IDM_LAYOUT_FORCE_DIRECTION_RUN: {
      using namespace sociarium_project_thread;
      sociarium_project_force_direction::toggle_execution();
      break;
    }

    case IDM_LAYOUT_FORCE_DIRECTION_KAMADA_KAWAI_METHOD:
      set_force_direction_algorithm(
        RealTimeForceDirectionAlgorithm::KAMADA_KAWAI_METHOD);
      break;

    case IDM_LAYOUT_FORCE_DIRECTION_KAMADA_KAWAI_METHOD_WITH_COMMUNITY_SEPARATION:
      set_force_direction_algorithm(
        RealTimeForceDirectionAlgorithm::KAMADA_KAWAI_METHOD_WITH_COMMUNITY_SEPARATION);
      break;

    case IDM_LAYOUT_FORCE_DIRECTION_COMMUNITY_ORIENTED:
      set_force_direction_algorithm(
        RealTimeForceDirectionAlgorithm::COMMUNITY_ORIENTED);
      break;

    case IDM_LAYOUT_FORCE_DIRECTION_SPRING_AND_REPULSIVE_FORCE:
      set_force_direction_algorithm(
        RealTimeForceDirectionAlgorithm::SPRING_AND_REPULSIVE_FORCE);
      break;

    case IDM_LAYOUT_FORCE_DIRECTION_LATTICE_GAS_METHOD:
      set_force_direction_algorithm(
        RealTimeForceDirectionAlgorithm::LATTICE_GAS_METHOD);
      break;

    case IDM_LAYOUT_FORCE_DIRECTION_DESIGNTIDE:
      set_force_direction_algorithm(
        RealTimeForceDirectionAlgorithm::DESIGNTIDE);
      break;


      // --------------------------------------------------------------------------------
      // COMMUNITY_DETECTION

    case IDM_COMMUNITY_DETECTION_WEAKLY_CONNECTED_COMPONENTS:
      set_community_detection_algorithm(
        CommunityDetectionAlgorithm::CONNECTED_COMPONENTS);
      detect_community();
      break;

    case IDM_COMMUNITY_DETECTION_STRONGLY_CONNECTED_COMPONENTS:
      set_community_detection_algorithm(
        CommunityDetectionAlgorithm::STRONGLY_CONNECTED_COMPONENTS);
      detect_community();
      break;

    case IDM_COMMUNITY_DETECTION_MODULARITY_MAXIMIZATION_USING_GREEDY_METHOD:
      set_community_detection_algorithm(
        CommunityDetectionAlgorithm::MODULARITY_MAXIMIZATION_USING_GREEDY_METHOD);
      detect_community();
      break;

    case IDM_COMMUNITY_DETECTION_MODULARITY_MAXIMIZATION_USING_TEO_METHOD:
      set_community_detection_algorithm(
        CommunityDetectionAlgorithm::MODULARITY_MAXIMIZATION_USING_TEO_METHOD);
      detect_community();
      break;

    case IDM_COMMUNITY_DETECTION_USE_WEIGHTED_MODULARITY:
      use_weighted_modularity(!use_weighted_modularity());
      break;

    case IDM_COMMUNITY_DETECTION_CLIQUE_PERCOLATION_3:
      set_community_detection_algorithm(
        CommunityDetectionAlgorithm::CLIQUE_PERCOLATION_3);
      detect_community();
      break;

    case IDM_COMMUNITY_DETECTION_CLIQUE_PERCOLATION_4:
      set_community_detection_algorithm(
        CommunityDetectionAlgorithm::CLIQUE_PERCOLATION_4);
      detect_community();
      break;

    case IDM_COMMUNITY_DETECTION_CLIQUE_PERCOLATION_5:
      set_community_detection_algorithm(
        CommunityDetectionAlgorithm::CLIQUE_PERCOLATION_5);
      detect_community();
      break;

    case IDM_COMMUNITY_DETECTION_BETWEENNESS_CENTRALITY_SEPARATION:
      set_community_detection_algorithm(
        CommunityDetectionAlgorithm::BETWEENNESS_CENTRALITY_SEPARATION);
      detect_community();
      break;

    case IDM_COMMUNITY_DETECTION_INFORMATION_FLOW_MAPPING:
      set_community_detection_algorithm(
        CommunityDetectionAlgorithm::INFORMATION_FLOW_MAPPING);
      detect_community();
      break;


    case IDM_COMMUNITY_DETECTION_CLEAR:
      clear_community();
      break;

    case IDM_KEY_D:
    case IDM_COMMUNITY_TRANSITION_DIAGRAM:
      sociarium_project_view::set_show_diagram(!sociarium_project_view::get_show_diagram());
      break;

    case IDM_LAYOUT_ZOOM_OUT:
      for (int i=0; i<10; ++i) {
        zoom(1.05);
        draw();
      }
      break;

    case IDM_LAYOUT_ZOOM_IN:
      for (int i=0; i<10; ++i) {
        zoom(1.0/1.05);
        draw();
      }
      break;

    default: break;
    }
  }

} // The end of the namespace "hashimoto_ut"

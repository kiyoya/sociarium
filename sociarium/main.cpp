// s.o.c.i.a.r.i.u.m: main.cpp
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

#include <vector>
#include <unordered_map>
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <zmouse.h>
#include <GL/gl.h>
#include "algorithm_selector.h"
#include "common.h"
#include "community_transition_diagram.h"
#include "draw.h"
#include "font.h"
#include "layout.h"
#include "main.h"
#include "menu_and_message.h"
#include "resource.h"
#include "selection.h"
#include "sociarium_graph_time_series.h"
#include "thread.h"
#include "timeline.h"
#include "view.h"
#include "world.h"
#include "thread/force_direction.h"
#include "../shared/msgbox.h"
#include "../shared/thread.h"
#include "../shared/timer.h"
#include "../shared/util.h"
#include "../shared/win32api.h"

//#include "cvframe.h"
//#include "designtide.h"
//#include "tamabi_library.h"

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "ftgl.lib")

namespace hashimoto_ut {

  using std::vector;
  using std::string;
  using std::wstring;
  using std::pair;
  using std::make_pair;
  using std::tr1::shared_ptr;
  using std::tr1::unordered_map;

  using namespace sociarium_project_common;
  using namespace sociarium_project_menu_and_message;
  using namespace sociarium_project_timeline;
  using namespace sociarium_project_layout;
  using namespace sociarium_project_algorithm_selector;
  using namespace sociarium_project_font;
  using namespace sociarium_project_view;
  using namespace sociarium_project_timeline;


  ////////////////////////////////////////////////////////////////////////////////
  void MainWindow::wmCreate(HWND hwnd) {

    { // Set module path.
      wchar_t filename[_MAX_PATH];
      GetModuleFileName(GetModuleHandle(NULL), filename, _MAX_PATH);
      PathSplitter path(filename);
      set_module_path(path.drive()+path.dir());
      SetCurrentDirectory(get_module_path().c_str());
    }

    unordered_map<wstring, wstring> data_ini;

    { // Read *.ini file.
      wstring const filename = get_module_path()+L"sociarium.ini";
      wchar_t buf[_MAX_PATH];
      GetPrivateProfileSection(L"system", buf, _MAX_PATH, filename.c_str());

      vector<wstring> tok1 = tokenize(buf, L'\0');

      for (size_t i=0; i<tok1.size(); ++i) {
        vector<wstring> tok2 = tokenize(tok1[i], L'=');
        if (tok2.size()>1) data_ini.insert(make_pair(tok2[0], tok2[1]));
      }
    }

    // Set message.
    unordered_map<wstring, wstring>::const_iterator m
      = data_ini.find(L"language");

    // Default language is English.
    wchar_t const* ini_filename
      = (m!=data_ini.end())?m->second.c_str():L"language_en.dll";

    try {
      sociarium_project_menu_and_message::set_message(ini_filename);
    } catch (wchar_t const* errmsg) {
      show_last_error(hwnd, errmsg);
      exit(1);
    }

    // Create the OpenGL world.
    world_ = World::create(hwnd);

    // Set menu.
    try {
      sociarium_project_menu_and_message::set_menu(hwnd, ini_filename);
    } catch (wchar_t const* errmsg) {
      show_last_error(hwnd, errmsg);
      exit(1);
    }
    /* 'SetMenu()' will cause the WM_SIZE command, and window sizing requires
     * the world instance. So, 'SetMenu()' should be called after 'World::create()'.
     * In 'World::create()', some messages may be shown depending on the PC environment.
     * So, the message object should be created before 'World::create()'.
     */
    
    // Create and start the timer.
    timer_.reset(new Timer(hwnd));
    timer_->add(ID_TIMER_DRAW, 1);
    timer_->add(ID_TIMER_ZOOM, 10);
    timer_->start(ID_TIMER_DRAW);

    timer_->add(ID_TIMER_AUTO_RUN, 1000);
    if (get_auto_run_id()!=AutoRun::STOP) {
      timer_->reset(ID_TIMER_AUTO_RUN, get_latency());
      timer_->start(ID_TIMER_AUTO_RUN);
    }
  }


  ////////////////////////////////////////////////////////////////////////////////
  void MainWindow::wmDestroy(HWND hwnd) {
    world_.reset();
    PostQuitMessage(0);
  }


  ////////////////////////////////////////////////////////////////////////////////
  void MainWindow::wmCommand(HWND hwnd, WPARAM wp, LPARAM lp) {
    switch (LOWORD(wp)) {
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
        if (message_box(hwnd, mb_ok_cancel, APPLICATION_TITLE,
                        get_message(Message::CANCEL_RUNNING_THREAD))) {
          tf->cancel();
          join(GRAPH_CREATION);
        } else
          tf->resume();
      } else
        SendMessage(hwnd, WM_CLOSE, 0, 0);

      break;
    }

    case IDM_FILE_QUIT:
      SendMessage(hwnd, WM_CLOSE, 0, 0);
      break;

    case IDM_FILE_CANCEL: {
      using namespace sociarium_project_thread;
      shared_ptr<Thread> tf = get_thread_function(GRAPH_CREATION);

      if (joinable(GRAPH_CREATION)) {
        tf->suspend();
        if (message_box(hwnd, mb_ok_cancel, APPLICATION_TITLE,
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
        if (message_box(hwnd, mb_ok_cancel, APPLICATION_TITLE,
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
        if (message_box(hwnd, mb_ok_cancel, APPLICATION_TITLE,
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
        if (message_box(hwnd, mb_ok_cancel, APPLICATION_TITLE,
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
        if (message_box(hwnd, mb_ok_cancel, APPLICATION_TITLE,
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
      world_->layout();
      break;

    case IDM_COMMUNITY_DETECTION_UPDATE:
    case IDM_KEY_ENTER:
      world_->detect_community();
      break;

    case IDM_LAYOUT_INITIALIZE_EYEPOINT:
    case IDM_KEY_HOME:
      world_->initialize_view();
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

    case IDM_KEY_NEXT:
      if (!timer_->is_active(ID_TIMER_ZOOM)) {
        zoom_ = 0;
        timer_->start(ID_TIMER_ZOOM);
      }
      break;

    case IDM_KEY_PRIOR:
      if (!timer_->is_active(ID_TIMER_ZOOM)) {
        zoom_ = 1;
        timer_->start(ID_TIMER_ZOOM);
      }
      break;

    case IDM_TIMELINE_NEXT:
    case IDM_KEY_RIGHT:
    case IDM_KEY_UP:
      world_->forward_layer(window2viewport(get_mouse_position(hwnd)));
      break;

    case IDM_TIMELINE_PREV:
    case IDM_KEY_LEFT:
    case IDM_KEY_DOWN:
      world_->backward_layer(window2viewport(get_mouse_position(hwnd)));
      break;

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
        timer_->stop(ID_TIMER_DRAW);
        int const size = int(2*get_resolution());
        for (int i=0; i<size; ++i) {
          pair<int, int> const& scope = get_scope();
          set_scope(scope.first-1, scope.second+1);
          world_->draw();
        }
        timer_->start(ID_TIMER_DRAW);
        break;
      }

      case IDM_COMMUNITY_TRANSITION_DIAGRAM_SCOPE_NARROWER: {
        timer_->stop(ID_TIMER_DRAW);
        int const size = int(2*get_resolution());
        for (int i=0; i<size; ++i) {
          pair<int, int> const& scope = get_scope();
          set_scope(scope.first+1, scope.second-1);
          world_->draw();
        }
        timer_->start(ID_TIMER_DRAW);
        break;
      }

      }


      // --------------------------------------------------------------------------------
      // TIMELINE

    case IDM_TIMELINE_STOP:
      set_auto_run_id(AutoRun::STOP);
      timer_->stop(ID_TIMER_AUTO_RUN);
      break;

    case IDM_TIMELINE_FORWARD_1:
      set_auto_run_id(AutoRun::FORWARD_1);
      timer_->reset(ID_TIMER_AUTO_RUN, get_latency());
      timer_->start(ID_TIMER_AUTO_RUN);
      break;

    case IDM_TIMELINE_FORWARD_2:
      set_auto_run_id(AutoRun::FORWARD_2);
      timer_->reset(ID_TIMER_AUTO_RUN, get_latency());
      timer_->start(ID_TIMER_AUTO_RUN);
      break;

    case IDM_TIMELINE_FORWARD_3:
      set_auto_run_id(AutoRun::FORWARD_3);
      timer_->reset(ID_TIMER_AUTO_RUN, get_latency());
      timer_->start(ID_TIMER_AUTO_RUN);
      break;

    case IDM_TIMELINE_FORWARD_4:
      set_auto_run_id(AutoRun::FORWARD_4);
      timer_->reset(ID_TIMER_AUTO_RUN, get_latency());
      timer_->start(ID_TIMER_AUTO_RUN);
      break;

    case IDM_TIMELINE_BACKWARD_1:
      set_auto_run_id(AutoRun::BACKWARD_1);
      timer_->reset(ID_TIMER_AUTO_RUN, get_latency());
      timer_->start(ID_TIMER_AUTO_RUN);
      break;

    case IDM_TIMELINE_BACKWARD_2:
      set_auto_run_id(AutoRun::BACKWARD_2);
      timer_->reset(ID_TIMER_AUTO_RUN, get_latency());
      timer_->start(ID_TIMER_AUTO_RUN);
      break;

    case IDM_TIMELINE_BACKWARD_3:
      set_auto_run_id(AutoRun::BACKWARD_3);
      timer_->reset(ID_TIMER_AUTO_RUN, get_latency());
      timer_->start(ID_TIMER_AUTO_RUN);
      break;

    case IDM_TIMELINE_BACKWARD_4:
      set_auto_run_id(AutoRun::BACKWARD_4);
      timer_->reset(ID_TIMER_AUTO_RUN, get_latency());
      timer_->start(ID_TIMER_AUTO_RUN);
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
      world_->mark_elements(LOWORD(wp));
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
      world_->mark_elements(LOWORD(wp));
      break;

    case IDM_KEY_CTRL_A:
    case IDM_EDIT_MARK_ALL_ELEMENTS_ON_CURRENT_LAYER:
      world_->mark_elements(IDM_EDIT_MARK_ALL_ELEMENTS_ON_CURRENT_LAYER);
      break;

    case IDM_KEY_SHIFT_CTRL_A:
    case IDM_EDIT_MARK_ALL_ELEMENTS_ON_EACH_LAYER:
      world_->mark_elements(IDM_EDIT_MARK_ALL_ELEMENTS_ON_EACH_LAYER);
      break;

    case IDM_KEY_CTRL_I:
    case IDM_EDIT_INVERT_MARK_ON_CURRENT_LAYER:
      world_->inverse_mark(IDM_EDIT_INVERT_MARK_ON_CURRENT_LAYER);
      break;

    case IDM_KEY_SHIFT_CTRL_I:
    case IDM_EDIT_INVERT_MARK_ON_EACH_LAYER:
      world_->inverse_mark(IDM_EDIT_INVERT_MARK_ON_EACH_LAYER);
      break;

      // HIDE
    case IDM_EDIT_HIDE_MARKED_NODES_ON_CURRENT_LAYER:
    case IDM_EDIT_HIDE_MARKED_EDGES_ON_CURRENT_LAYER:
      world_->hide_marked_element(LOWORD(wp));
      break;

    case IDM_KEY_CTRL_H:
    case IDM_EDIT_HIDE_MARKED_ELEMENTS_ON_CURRENT_LAYER:
      world_->hide_marked_element(IDM_EDIT_HIDE_MARKED_ELEMENTS_ON_CURRENT_LAYER);
      break;

    case IDM_EDIT_HIDE_MARKED_NODES_ON_EACH_LAYER:
    case IDM_EDIT_HIDE_MARKED_EDGES_ON_EACH_LAYER:
      world_->hide_marked_element(LOWORD(wp));
      break;

    case IDM_KEY_SHIFT_CTRL_H:
    case IDM_EDIT_HIDE_MARKED_ELEMENTS_ON_EACH_LAYER:
      world_->hide_marked_element(IDM_EDIT_HIDE_MARKED_ELEMENTS_ON_EACH_LAYER);
      break;

    case IDM_EDIT_HIDE_MARKED_NODES_ON_CURRENT_LAYER_ALL:
    case IDM_EDIT_HIDE_MARKED_EDGES_ON_CURRENT_LAYER_ALL:
    case IDM_EDIT_HIDE_MARKED_ELEMENTS_ON_CURRENT_LAYER_ALL:
      world_->hide_marked_element(LOWORD(wp));
      break;

    case IDM_EDIT_HIDE_MARKED_NODES_ON_EACH_LAYER_ALL:
    case IDM_EDIT_HIDE_MARKED_EDGES_ON_EACH_LAYER_ALL:
    case IDM_EDIT_HIDE_MARKED_ELEMENTS_ON_EACH_LAYER_ALL:
      world_->hide_marked_element(LOWORD(wp));
      break;

      // REMOVE
    case IDM_EDIT_REMOVE_MARKED_NODES_ON_CURRENT_LAYER:
    case IDM_EDIT_REMOVE_MARKED_EDGES_ON_CURRENT_LAYER:
      world_->remove_marked_elements(LOWORD(wp));
      break;

    case IDM_KEY_DELETE:
    case IDM_EDIT_REMOVE_MARKED_ELEMENTS_ON_CURRENT_LAYER:
      world_->remove_marked_elements(IDM_EDIT_REMOVE_MARKED_ELEMENTS_ON_CURRENT_LAYER);
      break;

    case IDM_EDIT_REMOVE_MARKED_NODES_ON_EACH_LAYER:
    case IDM_EDIT_REMOVE_MARKED_EDGES_ON_EACH_LAYER:
      world_->remove_marked_elements(LOWORD(wp));
      break;

    case IDM_KEY_CTRL_DELETE:
    case IDM_EDIT_REMOVE_MARKED_ELEMENTS_ON_EACH_LAYER:
      world_->remove_marked_elements(IDM_EDIT_REMOVE_MARKED_ELEMENTS_ON_EACH_LAYER);
      break;

    case IDM_EDIT_REMOVE_MARKED_NODES_ON_CURRENT_LAYER_ALL:
    case IDM_EDIT_REMOVE_MARKED_EDGES_ON_CURRENT_LAYER_ALL:
    case IDM_EDIT_REMOVE_MARKED_ELEMENTS_ON_CURRENT_LAYER_ALL:
      world_->remove_marked_elements(LOWORD(wp));
      break;

    case IDM_EDIT_REMOVE_MARKED_NODES_ON_EACH_LAYER_ALL:
    case IDM_EDIT_REMOVE_MARKED_EDGES_ON_EACH_LAYER_ALL:
    case IDM_EDIT_REMOVE_MARKED_ELEMENTS_ON_EACH_LAYER_ALL:
      world_->remove_marked_elements(LOWORD(wp));
      break;

      // SHOW
    case IDM_EDIT_SHOW_HIDDEN_NODES_ON_CURRENT_LAYER:
    case IDM_EDIT_SHOW_HIDDEN_EDGES_ON_CURRENT_LAYER:
      world_->show_elements(LOWORD(wp));
      break;

    case IDM_KEY_CTRL_U:
    case IDM_EDIT_SHOW_HIDDEN_ELEMENTS_ON_CURRENT_LAYER:
      world_->show_elements(IDM_EDIT_SHOW_HIDDEN_ELEMENTS_ON_CURRENT_LAYER);
      break;

    case IDM_EDIT_SHOW_HIDDEN_EDGES_ON_EACH_LAYER:
    case IDM_EDIT_SHOW_HIDDEN_NODES_ON_EACH_LAYER:
      world_->show_elements(LOWORD(wp));
      break;

    case IDM_KEY_SHIFT_CTRL_U:
    case IDM_EDIT_SHOW_HIDDEN_ELEMENTS_ON_EACH_LAYER:
      world_->show_elements(IDM_EDIT_SHOW_HIDDEN_ELEMENTS_ON_EACH_LAYER);
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
      world_->update_node_size();
      break;

    case IDM_VIEW_NODE_SIZE_UNIFORM:
      set_node_size_update_algorithm(NodeSizeUpdateAlgorithm::UNIFORM);
      world_->update_node_size();
      break;

    case IDM_VIEW_NODE_SIZE_WEIGHT:
      set_node_size_update_algorithm(NodeSizeUpdateAlgorithm::WEIGHT);
      world_->update_node_size();
      break;

    case IDM_VIEW_NODE_SIZE_DEGREE_CENTRALITY:
      set_node_size_update_algorithm(NodeSizeUpdateAlgorithm::DEGREE_CENTRALITY);
      world_->update_node_size();
      break;

    case IDM_VIEW_NODE_SIZE_CLOSENESS_CENTRALITY:
      set_node_size_update_algorithm(NodeSizeUpdateAlgorithm::CLOSENESS_CENTRALITY);
      world_->update_node_size();
      break;

    case IDM_VIEW_NODE_SIZE_BETWEENNESS_CENTRALITY:
      set_node_size_update_algorithm(NodeSizeUpdateAlgorithm::BETWEENNESS_CENTRALITY);
      world_->update_node_size();
      break;

    case IDM_VIEW_NODE_SIZE_PAGERANK:
      set_node_size_update_algorithm(NodeSizeUpdateAlgorithm::PAGERANK);
      world_->update_node_size();
      break;

    case IDM_VIEW_NODE_SIZE_POINT:
      set_node_size_update_algorithm(NodeSizeUpdateAlgorithm::POINT);
      world_->update_node_size();
      break;


      // --------------------------------------------------------------------------------
      // EDGE WIDTH

    case IDM_VIEW_EDGE_WIDTH_UPDATE:
      world_->update_edge_width();
      break;

    case IDM_VIEW_EDGE_WIDTH_UNIFORM:
      set_edge_width_update_algorithm(EdgeWidthUpdateAlgorithm::UNIFORM);
      world_->update_edge_width();
      break;

    case IDM_VIEW_EDGE_WIDTH_WEIGHT:
      set_edge_width_update_algorithm(EdgeWidthUpdateAlgorithm::WEIGHT);
      world_->update_edge_width();
      break;

    case IDM_VIEW_EDGE_WIDTH_BETWEENNESS_CENTRALITY:
      set_edge_width_update_algorithm(EdgeWidthUpdateAlgorithm::BETWEENNESS_CENTRALITY);
      world_->update_edge_width();
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
      world_->layout();
      break;

    case IDM_LAYOUT_HIGH_DIMENSIONAL_EMBEDDING_1_2:
      set_layout_algorithm(LayoutAlgorithm::HIGH_DIMENSIONAL_EMBEDDING_1_2);
      world_->layout();
      break;

    case IDM_LAYOUT_HIGH_DIMENSIONAL_EMBEDDING_1_3:
      set_layout_algorithm(LayoutAlgorithm::HIGH_DIMENSIONAL_EMBEDDING_1_3);
      world_->layout();
      break;

    case IDM_LAYOUT_HIGH_DIMENSIONAL_EMBEDDING_2_3:
      set_layout_algorithm(LayoutAlgorithm::HIGH_DIMENSIONAL_EMBEDDING_2_3);
      world_->layout();
      break;

    case IDM_LAYOUT_CIRCLE:
      set_layout_algorithm(LayoutAlgorithm::CIRCLE);
      world_->layout();
      break;

    case IDM_LAYOUT_CIRCLE_IN_SIZE_ORDER:
      set_layout_algorithm(LayoutAlgorithm::CIRCLE_IN_SIZE_ORDER);
      world_->layout();
      break;

    case IDM_LAYOUT_LATTICE:
      set_layout_algorithm(LayoutAlgorithm::LATTICE);
      world_->layout();
      break;

    case IDM_LAYOUT_RANDOM:
      set_layout_algorithm(LayoutAlgorithm::RANDOM);
      world_->layout();
      break;

    case IDM_LAYOUT_CARTOGRAMS:
      set_layout_algorithm(LayoutAlgorithm::CARTOGRAMS);
      world_->layout();
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
      world_->detect_community();
      break;

    case IDM_COMMUNITY_DETECTION_STRONGLY_CONNECTED_COMPONENTS:
      set_community_detection_algorithm(
        CommunityDetectionAlgorithm::STRONGLY_CONNECTED_COMPONENTS);
      world_->detect_community();
      break;

    case IDM_COMMUNITY_DETECTION_MODULARITY_MAXIMIZATION_USING_GREEDY_METHOD:
      set_community_detection_algorithm(
        CommunityDetectionAlgorithm::MODULARITY_MAXIMIZATION_USING_GREEDY_METHOD);
      world_->detect_community();
      break;

    case IDM_COMMUNITY_DETECTION_MODULARITY_MAXIMIZATION_USING_TEO_METHOD:
      set_community_detection_algorithm(
        CommunityDetectionAlgorithm::MODULARITY_MAXIMIZATION_USING_TEO_METHOD);
      world_->detect_community();
      break;

    case IDM_COMMUNITY_DETECTION_USE_WEIGHTED_MODULARITY:
      use_weighted_modularity(!use_weighted_modularity());
      break;

    case IDM_COMMUNITY_DETECTION_CLIQUE_PERCOLATION_3:
      set_community_detection_algorithm(
        CommunityDetectionAlgorithm::CLIQUE_PERCOLATION_3);
      world_->detect_community();
      break;

    case IDM_COMMUNITY_DETECTION_CLIQUE_PERCOLATION_4:
      set_community_detection_algorithm(
        CommunityDetectionAlgorithm::CLIQUE_PERCOLATION_4);
      world_->detect_community();
      break;

    case IDM_COMMUNITY_DETECTION_CLIQUE_PERCOLATION_5:
      set_community_detection_algorithm(
        CommunityDetectionAlgorithm::CLIQUE_PERCOLATION_5);
      world_->detect_community();
      break;

    case IDM_COMMUNITY_DETECTION_BETWEENNESS_CENTRALITY_SEPARATION:
      set_community_detection_algorithm(
        CommunityDetectionAlgorithm::BETWEENNESS_CENTRALITY_SEPARATION);
      world_->detect_community();
      break;

    case IDM_COMMUNITY_DETECTION_INFORMATION_FLOW_MAPPING:
      set_community_detection_algorithm(
        CommunityDetectionAlgorithm::INFORMATION_FLOW_MAPPING);
      world_->detect_community();
      break;


    case IDM_COMMUNITY_DETECTION_CLEAR:
      world_->clear_community();
      break;

    case IDM_KEY_D:
    case IDM_COMMUNITY_TRANSITION_DIAGRAM:
      sociarium_project_view::set_show_diagram(!sociarium_project_view::get_show_diagram());
      break;

    case IDM_LAYOUT_ZOOM_OUT:
      timer_->stop(ID_TIMER_DRAW);
      for (int i=0; i<10; ++i) {
        world_->zoom(1.05);
        world_->draw();
      }
      timer_->start(ID_TIMER_DRAW);
      break;

    case IDM_LAYOUT_ZOOM_IN:
      timer_->stop(ID_TIMER_DRAW);
      for (int i=0; i<10; ++i) {
        world_->zoom(1.0/1.05);
        world_->draw();
      }
      timer_->start(ID_TIMER_DRAW);
      break;

    default: break;
    }
  }


  ////////////////////////////////////////////////////////////////////////////////
  void MainWindow::wmSize(HWND hwnd) {
    RECT rect;
    GetClientRect(hwnd, &rect);
    wsize_.set(rect.right-rect.left, rect.bottom-rect.top);
    world_->resize_window(wsize_);
    InvalidateRect(hwnd, 0, FALSE);
  }


  ////////////////////////////////////////////////////////////////////////////////
  void MainWindow::wmPaint(HWND hwnd) {
    world_->draw();
    ValidateRect(hwnd, NULL);
  }


  ////////////////////////////////////////////////////////////////////////////////
  void MainWindow::wmTimer(HWND hwnd, WPARAM wp, LPARAM lp) {

    switch (wp) {

    case ID_TIMER_DRAW: {
      RECT rect;
      GetClientRect(hwnd, &rect);
      InvalidateRect(hwnd, &rect, FALSE);
      break;
    }

    case ID_TIMER_ZOOM:
      world_->zoom(zoom_?1.05:1.0/1.05);
      break;

    case ID_TIMER_AUTO_RUN: {
      using namespace sociarium_project_timeline;
      if (AutoRun::FORWARD_1<=sociarium_project_timeline::get_auto_run_id()
          && sociarium_project_timeline::get_auto_run_id()<=AutoRun::FORWARD_4)
        world_->forward_layer(window2viewport(get_mouse_position(hwnd)));
      else if (AutoRun::BACKWARD_1<=sociarium_project_timeline::get_auto_run_id()
               && sociarium_project_timeline::get_auto_run_id()<=AutoRun::BACKWARD_4)
        world_->backward_layer(window2viewport(get_mouse_position(hwnd)));
      break;
    }

    default: {
      break;
    }
    }
  }


  ////////////////////////////////////////////////////////////////////////////////
  void MainWindow::wmActivate(HWND hwnd, WPARAM wp, LPARAM lp) {
  }


  ////////////////////////////////////////////////////////////////////////////////
  void MainWindow::wmInitMenu(HWND hwnd, WPARAM wp, LPARAM lp) {
    HMENU hmenu = HMENU(wp);

    using namespace sociarium_project_view;
    using namespace sociarium_project_font;
    using namespace sociarium_project_algorithm_selector;

    // --------------------------------------------------------------------------------
    // FILE

    {
      using namespace sociarium_project_thread;

      if (joinable(GRAPH_CREATION)
          || joinable(GRAPH_RETOUCH))
        EnableMenuItem(GetMenu(hwnd), IDM_FILE_CANCEL, MF_BYCOMMAND|MF_ENABLED);
      else
        EnableMenuItem(GetMenu(hwnd), IDM_FILE_CANCEL, MF_BYCOMMAND|MF_GRAYED);
    }

    // --------------------------------------------------------------------------------
    // SHOW/HIDE

    {
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_show_node()?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_VIEW_SHOW_NODE, FALSE, &mii);
    }{
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_show_edge()?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_VIEW_SHOW_EDGE, FALSE, &mii);
    }{
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_show_community()?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_VIEW_SHOW_COMMUNITY, FALSE, &mii);
    }{
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_show_community_edge()?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_VIEW_SHOW_COMMUNITY_EDGE, FALSE, &mii);
    }

    {
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_show_node_name()?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_STRING_SHOW_NODE_NAME, FALSE, &mii);
    }{
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_show_edge_name()?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_STRING_SHOW_EDGE_NAME, FALSE, &mii);
    }{
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_show_community_name()?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_STRING_SHOW_COMMUNITY_NAME, FALSE, &mii);
    }{
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_show_community_edge_name()?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_STRING_SHOW_COMMUNITY_EDGE_NAME, FALSE, &mii);
    }

    {
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_show_slider()?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_TIMELINE_SHOW_SLIDER, FALSE, &mii);
    }{
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_show_layer_name()?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_STRING_SHOW_LAYER_NAME, FALSE, &mii);
    }{
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_show_grid()?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_LAYOUT_SHOW_GRID, FALSE, &mii);
    }{
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_show_fps()?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_STRING_SHOW_FPS, FALSE, &mii);
    }{
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_show_center()?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_LAYOUT_SHOW_CENTER, FALSE, &mii);
    }{
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_show_layout_frame()?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_LAYOUT_SHOW_LAYOUT_FRAME, FALSE, &mii);
    }

    // --------------------------------------------------------------------------------
    // DRAWING STYLE

    {
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_node_style()&NodeStyle::POLYGON
        ?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_VIEW_NODE_STYLE_POLYGON, FALSE, &mii);
    }{
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_node_style()&NodeStyle::TEXTURE
        ?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_VIEW_NODE_STYLE_TEXTURE, FALSE, &mii);
    }

    {
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_edge_style()&EdgeStyle::LINE
        ?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_VIEW_EDGE_STYLE_LINE, FALSE, &mii);
    }{
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_edge_style()&EdgeStyle::POLYGON
        ?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_VIEW_EDGE_STYLE_POLYGON, FALSE, &mii);
    }

    {
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_community_style()&CommunityStyle::POLYGON_CIRCLE
        ?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_VIEW_COMMUNITY_STYLE_POLYGON_CIRCLE, FALSE, &mii);
    }{
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_community_style()&CommunityStyle::TEXTURE
        ?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_VIEW_COMMUNITY_STYLE_TEXTURE, FALSE, &mii);
    }

    {
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_community_edge_style()&CommunityEdgeStyle::LINE
        ?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_VIEW_COMMUNITY_EDGE_STYLE_LINE, FALSE, &mii);
    }{
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_community_edge_style()&CommunityEdgeStyle::POLYGON
        ?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_VIEW_COMMUNITY_EDGE_STYLE_POLYGON, FALSE, &mii);
    }

    // --------------------------------------------------------------------------------
    // NODE SIZE

    {
      using namespace sociarium_project_thread;

      if (joinable(NODE_SIZE_UPDATE)) {
        EnableMenuItem(GetMenu(hwnd), IDM_VIEW_NODE_SIZE_UPDATE, MF_BYCOMMAND|MF_GRAYED);
        EnableMenuItem(GetMenu(hwnd), IDM_VIEW_NODE_SIZE_CANCEL, MF_BYCOMMAND|MF_ENABLED);
      } else {
        EnableMenuItem(GetMenu(hwnd), IDM_VIEW_NODE_SIZE_UPDATE, MF_BYCOMMAND|MF_ENABLED);
        EnableMenuItem(GetMenu(hwnd), IDM_VIEW_NODE_SIZE_CANCEL, MF_BYCOMMAND|MF_GRAYED);
      }
    }

    {
      using namespace NodeSizeUpdateAlgorithm;

      {
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_node_size_update_algorithm()==POINT
          ?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_VIEW_NODE_SIZE_POINT, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_node_size_update_algorithm()==UNIFORM
          ?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_VIEW_NODE_SIZE_UNIFORM, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_node_size_update_algorithm()==WEIGHT
          ?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_VIEW_NODE_SIZE_WEIGHT, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_node_size_update_algorithm()==DEGREE_CENTRALITY
          ?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_VIEW_NODE_SIZE_DEGREE_CENTRALITY, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_node_size_update_algorithm()==CLOSENESS_CENTRALITY
          ?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_VIEW_NODE_SIZE_CLOSENESS_CENTRALITY, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_node_size_update_algorithm()==BETWEENNESS_CENTRALITY
          ?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_VIEW_NODE_SIZE_BETWEENNESS_CENTRALITY, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_node_size_update_algorithm()==PAGERANK
          ?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_VIEW_NODE_SIZE_PAGERANK, FALSE, &mii);
      }
    }

    // --------------------------------------------------------------------------------
    // EDGE WIDTH

    {
      using namespace sociarium_project_thread;

      if (joinable(EDGE_WIDTH_UPDATE)) {
        EnableMenuItem(GetMenu(hwnd), IDM_VIEW_EDGE_WIDTH_UPDATE, MF_BYCOMMAND|MF_GRAYED);
        EnableMenuItem(GetMenu(hwnd), IDM_VIEW_EDGE_WIDTH_CANCEL, MF_BYCOMMAND|MF_ENABLED);
      } else {
        EnableMenuItem(GetMenu(hwnd), IDM_VIEW_EDGE_WIDTH_UPDATE, MF_BYCOMMAND|MF_ENABLED);
        EnableMenuItem(GetMenu(hwnd), IDM_VIEW_EDGE_WIDTH_CANCEL, MF_BYCOMMAND|MF_GRAYED);
      }
    }

    {
      using namespace EdgeWidthUpdateAlgorithm;

      {
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_edge_width_update_algorithm()==UNIFORM
          ?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_VIEW_EDGE_WIDTH_UNIFORM, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_edge_width_update_algorithm()==WEIGHT
          ?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_VIEW_EDGE_WIDTH_WEIGHT, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_edge_width_update_algorithm()==BETWEENNESS_CENTRALITY
          ?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_VIEW_EDGE_WIDTH_BETWEENNESS_CENTRALITY, FALSE, &mii);
      }
    }

    // --------------------------------------------------------------------------------
    // FONT

    {
      using namespace sociarium_project_font;

      {
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_font_scale(FontCategory::NODE_NAME)==get_default_font_scale(0)
          ?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_STRING_NODE_NAME_SIZE_0, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_font_scale(FontCategory::NODE_NAME)==get_default_font_scale(1)
          ?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_STRING_NODE_NAME_SIZE_1, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_font_scale(FontCategory::NODE_NAME)==get_default_font_scale(2)
          ?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_STRING_NODE_NAME_SIZE_2, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_font_scale(FontCategory::NODE_NAME)==get_default_font_scale(3)
          ?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_STRING_NODE_NAME_SIZE_3, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_font_scale(FontCategory::NODE_NAME)==get_default_font_scale(4)
          ?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_STRING_NODE_NAME_SIZE_4, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_node_name_size_variable()?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_STRING_NODE_NAME_SIZE_VARIABLE, FALSE, &mii);
      }

      {
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_font_scale(FontCategory::EDGE_NAME)==get_default_font_scale(0)
          ?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_STRING_EDGE_NAME_SIZE_0, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_font_scale(FontCategory::EDGE_NAME)==get_default_font_scale(1)
          ?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_STRING_EDGE_NAME_SIZE_1, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_font_scale(FontCategory::EDGE_NAME)==get_default_font_scale(2)
          ?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_STRING_EDGE_NAME_SIZE_2, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_font_scale(FontCategory::EDGE_NAME)==get_default_font_scale(3)
          ?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_STRING_EDGE_NAME_SIZE_3, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_font_scale(FontCategory::EDGE_NAME)==get_default_font_scale(4)
          ?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_STRING_EDGE_NAME_SIZE_4, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_edge_name_size_variable()?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_STRING_EDGE_NAME_SIZE_VARIABLE, FALSE, &mii);
      }

      {
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_font_scale(FontCategory::COMMUNITY_NAME)==get_default_font_scale(0)
          ?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_STRING_COMMUNITY_NAME_SIZE_0, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_font_scale(FontCategory::COMMUNITY_NAME)==get_default_font_scale(1)
          ?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_STRING_COMMUNITY_NAME_SIZE_1, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_font_scale(FontCategory::COMMUNITY_NAME)==get_default_font_scale(2)
          ?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_STRING_COMMUNITY_NAME_SIZE_2, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_font_scale(FontCategory::COMMUNITY_NAME)==get_default_font_scale(3)
          ?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_STRING_COMMUNITY_NAME_SIZE_3, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_font_scale(FontCategory::COMMUNITY_NAME)==get_default_font_scale(4)
          ?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_STRING_COMMUNITY_NAME_SIZE_4, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_community_name_size_variable()?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_STRING_COMMUNITY_NAME_SIZE_VARIABLE, FALSE, &mii);
      }

      {
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_font_scale(FontCategory::COMMUNITY_EDGE_NAME)==get_default_font_scale(0)
          ?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_STRING_COMMUNITY_EDGE_NAME_SIZE_0, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_font_scale(FontCategory::COMMUNITY_EDGE_NAME)==get_default_font_scale(1)
          ?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_STRING_COMMUNITY_EDGE_NAME_SIZE_1, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_font_scale(FontCategory::COMMUNITY_EDGE_NAME)==get_default_font_scale(2)
          ?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_STRING_COMMUNITY_EDGE_NAME_SIZE_2, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_font_scale(FontCategory::COMMUNITY_EDGE_NAME)==get_default_font_scale(3)
          ?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_STRING_COMMUNITY_EDGE_NAME_SIZE_3, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_font_scale(FontCategory::COMMUNITY_EDGE_NAME)==get_default_font_scale(4)
          ?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_STRING_COMMUNITY_EDGE_NAME_SIZE_4, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_community_edge_name_size_variable()?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_STRING_COMMUNITY_EDGE_NAME_SIZE_VARIABLE, FALSE, &mii);
      }

      {
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_font_type()==FontType::POLYGON_FONT?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_STRING_FONT_TYPE_POLYGON, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_font_type()==FontType::TEXTURE_FONT?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_STRING_FONT_TYPE_TEXTURE, FALSE, &mii);
      }
    }

    // --------------------------------------------------------------------------------
    // LAYOUT

    {
      using namespace sociarium_project_thread;

      if (joinable(LAYOUT)) {
        EnableMenuItem(GetMenu(hwnd), IDM_LAYOUT_UPDATE, MF_BYCOMMAND|MF_GRAYED);
        EnableMenuItem(GetMenu(hwnd), IDM_LAYOUT_CANCEL, MF_BYCOMMAND|MF_ENABLED);
      } else {
        EnableMenuItem(GetMenu(hwnd), IDM_LAYOUT_UPDATE, MF_BYCOMMAND|MF_ENABLED);
        EnableMenuItem(GetMenu(hwnd), IDM_LAYOUT_CANCEL, MF_BYCOMMAND|MF_GRAYED);
      }
    }

    {
      using namespace LayoutAlgorithm;

      {
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_layout_algorithm()==KAMADA_KAWAI_METHOD
          ?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_LAYOUT_KAMADA_KAWAI_METHOD, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_layout_algorithm()==HIGH_DIMENSIONAL_EMBEDDING_1_2
          ?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_LAYOUT_HIGH_DIMENSIONAL_EMBEDDING_1_2, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_layout_algorithm()==HIGH_DIMENSIONAL_EMBEDDING_1_3
          ?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_LAYOUT_HIGH_DIMENSIONAL_EMBEDDING_1_3, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_layout_algorithm()==HIGH_DIMENSIONAL_EMBEDDING_2_3
          ?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_LAYOUT_HIGH_DIMENSIONAL_EMBEDDING_2_3, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_layout_algorithm()==CIRCLE
          ?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_LAYOUT_CIRCLE, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_layout_algorithm()==CIRCLE_IN_SIZE_ORDER
          ?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_LAYOUT_CIRCLE_IN_SIZE_ORDER, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_layout_algorithm()==LATTICE
          ?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_LAYOUT_LATTICE, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_layout_algorithm()==RANDOM
          ?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_LAYOUT_RANDOM, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_layout_algorithm()==CARTOGRAMS
          ?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_LAYOUT_CARTOGRAMS, FALSE, &mii);
      }
    }

    {
      using namespace RealTimeForceDirectionAlgorithm;

      {
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = sociarium_project_force_direction::is_active()
          ?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_LAYOUT_FORCE_DIRECTION_RUN, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_force_direction_algorithm()==KAMADA_KAWAI_METHOD
          ?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_LAYOUT_FORCE_DIRECTION_KAMADA_KAWAI_METHOD, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_force_direction_algorithm()==KAMADA_KAWAI_METHOD_WITH_COMMUNITY_SEPARATION
          ?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_LAYOUT_FORCE_DIRECTION_KAMADA_KAWAI_METHOD_WITH_COMMUNITY_SEPARATION, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_force_direction_algorithm()==COMMUNITY_ORIENTED
          ?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_LAYOUT_FORCE_DIRECTION_COMMUNITY_ORIENTED, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_force_direction_algorithm()==SPRING_AND_REPULSIVE_FORCE
          ?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_LAYOUT_FORCE_DIRECTION_SPRING_AND_REPULSIVE_FORCE, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_force_direction_algorithm()==LATTICE_GAS_METHOD
          ?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_LAYOUT_FORCE_DIRECTION_LATTICE_GAS_METHOD, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_force_direction_algorithm()==DESIGNTIDE
          ?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_LAYOUT_FORCE_DIRECTION_DESIGNTIDE, FALSE, &mii);
      }
    }

    // --------------------------------------------------------------------------------
    // COMMUNITY_DETECTION

    {
      using namespace sociarium_project_thread;

      if (joinable(COMMUNITY_DETECTION)) {
        EnableMenuItem(GetMenu(hwnd), IDM_COMMUNITY_DETECTION_UPDATE, MF_BYCOMMAND|MF_GRAYED);
        EnableMenuItem(GetMenu(hwnd), IDM_COMMUNITY_DETECTION_CANCEL, MF_BYCOMMAND|MF_ENABLED);
      } else {
        EnableMenuItem(GetMenu(hwnd), IDM_COMMUNITY_DETECTION_UPDATE, MF_BYCOMMAND|MF_ENABLED);
        EnableMenuItem(GetMenu(hwnd), IDM_COMMUNITY_DETECTION_CANCEL, MF_BYCOMMAND|MF_GRAYED);
      }
    }

    {
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = sociarium_project_view::get_show_diagram()?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_COMMUNITY_TRANSITION_DIAGRAM, FALSE, &mii);
    }

    {
      using namespace CommunityDetectionAlgorithm;

      {
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_community_detection_algorithm()==CONNECTED_COMPONENTS
          ?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_COMMUNITY_DETECTION_WEAKLY_CONNECTED_COMPONENTS, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_community_detection_algorithm()==STRONGLY_CONNECTED_COMPONENTS
          ?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_COMMUNITY_DETECTION_STRONGLY_CONNECTED_COMPONENTS, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_community_detection_algorithm()==CLIQUE_PERCOLATION_3
          ?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_COMMUNITY_DETECTION_CLIQUE_PERCOLATION_3, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_community_detection_algorithm()==CLIQUE_PERCOLATION_4
          ?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_COMMUNITY_DETECTION_CLIQUE_PERCOLATION_4, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_community_detection_algorithm()==CLIQUE_PERCOLATION_5
          ?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_COMMUNITY_DETECTION_CLIQUE_PERCOLATION_5, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_community_detection_algorithm()==MODULARITY_MAXIMIZATION_USING_GREEDY_METHOD
          ?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_COMMUNITY_DETECTION_MODULARITY_MAXIMIZATION_USING_GREEDY_METHOD, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_community_detection_algorithm()==MODULARITY_MAXIMIZATION_USING_TEO_METHOD
          ?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_COMMUNITY_DETECTION_MODULARITY_MAXIMIZATION_USING_TEO_METHOD, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = use_weighted_modularity()
          ?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_COMMUNITY_DETECTION_USE_WEIGHTED_MODULARITY, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_community_detection_algorithm()==BETWEENNESS_CENTRALITY_SEPARATION
          ?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_COMMUNITY_DETECTION_BETWEENNESS_CENTRALITY_SEPARATION, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_community_detection_algorithm()==INFORMATION_FLOW_MAPPING
          ?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_COMMUNITY_DETECTION_INFORMATION_FLOW_MAPPING, FALSE, &mii);
      }
    }

    // --------------------------------------------------------------------------------
    // TIMELINE

    {
      using namespace sociarium_project_timeline;

      {
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_auto_run_id()==AutoRun::STOP?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_TIMELINE_STOP, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_auto_run_id()==AutoRun::FORWARD_1?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_TIMELINE_FORWARD_1, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_auto_run_id()==AutoRun::FORWARD_2?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_TIMELINE_FORWARD_2, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_auto_run_id()==AutoRun::FORWARD_3?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_TIMELINE_FORWARD_3, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_auto_run_id()==AutoRun::FORWARD_4?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_TIMELINE_FORWARD_4, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_auto_run_id()==AutoRun::BACKWARD_1?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_TIMELINE_BACKWARD_1, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_auto_run_id()==AutoRun::BACKWARD_2?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_TIMELINE_BACKWARD_2, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_auto_run_id()==AutoRun::BACKWARD_3?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_TIMELINE_BACKWARD_3, FALSE, &mii);
      }{
        MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
        mii.fState = get_auto_run_id()==AutoRun::BACKWARD_4?MFS_CHECKED:MFS_UNCHECKED;
        SetMenuItemInfo(hmenu, IDM_TIMELINE_BACKWARD_4, FALSE, &mii);
      }
    }
  }


  ////////////////////////////////////////////////////////////////////////////////
  void MainWindow::wmKeyUp(HWND hwnd, WPARAM wp, LPARAM lp) {
    switch (wp) {
    case VK_NEXT:
    case VK_PRIOR:
      timer_->stop(ID_TIMER_ZOOM);
      break;

    default:
      break;
    }
  }


  ////////////////////////////////////////////////////////////////////////////////
  void MainWindow::wmLButtonDown(HWND hwnd, WPARAM wp, LPARAM lp) {
    using namespace sociarium_project_selection;
    SetFocus(hwnd);
    SetCapture(hwnd);
    world_->do_mouse_action(MouseAction::LBUTTON_DOWN, window2viewport(lp), mousemodifier(wp));
  }


  ////////////////////////////////////////////////////////////////////////////////
  void MainWindow::wmRButtonDown(HWND hwnd, WPARAM wp, LPARAM lp) {
    using namespace sociarium_project_selection;
    SetFocus(hwnd);
    SetCapture(hwnd);
    world_->do_mouse_action(MouseAction::RBUTTON_DOWN, window2viewport(lp), mousemodifier(wp));
  }


  ////////////////////////////////////////////////////////////////////////////////
  void MainWindow::wmMButtonDown(HWND hwnd, WPARAM wp, LPARAM lp) {
    using namespace sociarium_project_selection;
    SetFocus(hwnd);
    SetCapture(hwnd);
    world_->do_mouse_action(MouseAction::MBUTTON_DOWN, window2viewport(lp), mousemodifier(wp));
  }


  ////////////////////////////////////////////////////////////////////////////////
  void MainWindow::wmLButtonUp(HWND hwnd, WPARAM wp, LPARAM lp) {
    using namespace sociarium_project_selection;
    if (GetCapture()==hwnd) {
      ReleaseCapture();
      world_->do_mouse_action(MouseAction::LBUTTON_UP, window2viewport(lp), mousemodifier(wp));
    }
  }


  ////////////////////////////////////////////////////////////////////////////////
  void MainWindow::wmRButtonUp(HWND hwnd, WPARAM wp, LPARAM lp) {
    using namespace sociarium_project_selection;
    if (GetCapture()==hwnd) {
      ReleaseCapture();
      world_->do_mouse_action(MouseAction::RBUTTON_UP, window2viewport(lp), mousemodifier(wp));
      //timer_->start(ID_TIMER_SELECT);
    }
  }


  ////////////////////////////////////////////////////////////////////////////////
  void MainWindow::wmMButtonUp(HWND hwnd, WPARAM wp, LPARAM lp) {
    using namespace sociarium_project_selection;
    if (GetCapture()==hwnd) {
      ReleaseCapture();
      world_->do_mouse_action(MouseAction::MBUTTON_UP, window2viewport(lp), mousemodifier(wp));
    }
  }


  ////////////////////////////////////////////////////////////////////////////////
  void MainWindow::wmLButtonDBL(HWND hwnd, WPARAM wp, LPARAM lp) {
    using namespace sociarium_project_selection;
    timer_->stop(ID_TIMER_DRAW);
    world_->do_mouse_action(MouseAction::LBUTTON_DBL, window2viewport(lp), mousemodifier(wp));
    timer_->start(ID_TIMER_DRAW);
  }


  ////////////////////////////////////////////////////////////////////////////////
  void MainWindow::wmRButtonDBL(HWND hwnd, WPARAM wp, LPARAM lp) {
    wmRButtonDown(hwnd, wp, lp);
  }


  ////////////////////////////////////////////////////////////////////////////////
  void MainWindow::wmMouseMove(HWND hwnd, WPARAM wp, LPARAM lp) {
    using namespace sociarium_project_selection;
    if (GetCapture()==hwnd && wp&MK_LBUTTON)
      world_->do_mouse_action(MouseAction::LBUTTON_DRAG, window2viewport(lp), mousemodifier(wp));
    else if (GetCapture()==hwnd && wp&MK_RBUTTON)
      world_->do_mouse_action(MouseAction::RBUTTON_DRAG, window2viewport(lp), mousemodifier(wp));
    else
      world_->select(window2viewport(get_mouse_position(hwnd)));
  }


  ////////////////////////////////////////////////////////////////////////////////
  void MainWindow::wmMouseWheel(HWND hwnd, WPARAM wp, LPARAM lp) {
    using namespace sociarium_project_selection;
    world_->do_mouse_action(MouseAction::WHEEL, window2viewport(lp), short(HIWORD(wp))/120); // ±120 per 1 notch.
  }


  ////////////////////////////////////////////////////////////////////////////////
  void MainWindow::wmDropFiles(HWND hwnd, WPARAM wp, LPARAM lp) {
    static wchar_t filename[_MAX_PATH];
    HDROP hdrop = (HDROP)wp;
    DragQueryFile(hdrop, 0, filename, _MAX_PATH);
    world_->create_graph(filename);
  }


  ////////////////////////////////////////////////////////////////////////////////
  void MainWindow::wmClose(HWND hwnd) {
    if (message_box(hwnd, mb_ok_cancel, APPLICATION_TITLE, get_message(Message::QUIT)))
      DestroyWindow(hwnd);
  }

} // The end of the namespace "hashimoto_ut"


////////////////////////////////////////////////////////////////////////////////
// Entrypoint of the application.
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPSTR lpCmdLine, int nCmdShow) {

  // --------------------------------------------------------------------------------
  // Set locale.
  std::locale::global(std::locale("japanese", std::locale::ctype));

  // --------------------------------------------------------------------------------
  // Register window class.
  hashimoto_ut::RegisterWindowClass
    wcmain(L"WCMAIN", CS_DBLCLKS, 0, 0, GetModuleHandle(0),
           LoadIcon(NULL, IDI_APPLICATION), LoadCursor(NULL, IDC_ARROW),
           HBRUSH(GetStockObject(HOLLOW_BRUSH)), L"MENU", NULL);

  hashimoto_ut::sociarium_project_common::set_instance_handle(hInstance);

  // --------------------------------------------------------------------------------
  // Create a GL window.
  const DWORD style = WS_OVERLAPPEDWINDOW | CS_DBLCLKS;
  const DWORD exstyle = WS_EX_CLIENTEDGE;
  hashimoto_ut::MainWindow wnd;
  wnd.create(L"WCMAIN", hashimoto_ut::sociarium_project_common::APPLICATION_TITLE,
             style, exstyle, CW_USEDEFAULT, CW_USEDEFAULT, 809, 500, 0, 0);

  // --------------------------------------------------------------------------------
  // Other initializations.
  InitCommonControls();
  HACCEL hAccelTbl(LoadAccelerators(hInstance, L"ACCEL"));
  DragAcceptFiles(wnd.handle(), TRUE);

  ShowWindow(wnd.handle(), nCmdShow);
  UpdateWindow(wnd.handle());

  // --------------------------------------------------------------------------------
  // Start a message loop.
  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0)) {
    if (!TranslateAccelerator(msg.hwnd, hAccelTbl, &msg)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }

  return int(msg.wParam);
}

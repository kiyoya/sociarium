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

#include <vector>
#include <memory>
#include <fstream>
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <zmouse.h>
#include "resource.h"
#include "common.h"
#include "message.h"
#include "view.h"
#include "draw.h"
#include "layout.h"
#include "font.h"
#include "thread.h"
#include "algorithm_selector.h"
#include "mouse_and_selection.h"
#include "world.h"
#include "../shared/general.h"
#include "../shared/wndbase.h"
#include "../shared/win32api.h"
#include "../shared/thread.h"
#include "../shared/vector2.h"
#include "../shared/timer.h"
#include "../shared/msgbox.h"

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "ftgl.lib")

namespace hashimoto_ut {

  using std::vector;
  using std::string;
  using std::wstring;
  using std::tr1::shared_ptr;

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  class MainWindow : public WindowBase {
  private:
    void wmCreate(HWND hwnd);
    void wmDestroy(HWND hwnd);
    void wmCommand(HWND hwnd, WPARAM wp, LPARAM lp);
    void wmSize(HWND hwnd);
    void wmPaint(HWND hwnd);
    void wmTimer(HWND hwnd, WPARAM wp, LPARAM lp);
    void wmActivate(HWND hwnd, WPARAM wp, LPARAM lp);
    void wmInitMenu(HWND hwnd, WPARAM wp, LPARAM lp);
    void wmKeyUp(HWND hwnd, WPARAM wp, LPARAM lp);
    void wmLButtonDown(HWND hwnd, WPARAM wp, LPARAM lp);
    void wmRButtonDown(HWND hwnd, WPARAM wp, LPARAM lp);
    void wmLButtonUp(HWND hwnd, WPARAM wp, LPARAM lp);
    void wmRButtonUp(HWND hwnd, WPARAM wp, LPARAM lp);
    void wmLButtonDBL(HWND hwnd, WPARAM wp, LPARAM lp);
    void wmRButtonDBL(HWND hwnd, WPARAM wp, LPARAM lp);
    void wmMButtonDown(HWND hwnd, WPARAM wp, LPARAM lp);
    void wmMButtonUp(HWND hwnd, WPARAM wp, LPARAM lp);
    void wmMouseMove(HWND hwnd, WPARAM wp, LPARAM lp);
    void wmMouseWheel(HWND hwnd, WPARAM wp, LPARAM lp);
    void wmDropFiles(HWND hwnd, WPARAM wp, LPARAM lp);
    void wmClose(HWND hwnd);

    LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
      switch (msg) {
      case WM_CREATE:        wmCreate(hwnd); break;
      case WM_DESTROY:       wmDestroy(hwnd); return 0;
      case WM_COMMAND:       wmCommand(hwnd, wp, lp); break;
      case WM_SIZE:          wmSize(hwnd); break;
      case WM_PAINT:         wmPaint(hwnd); break;
      case WM_TIMER:         wmTimer(hwnd, wp, lp); break;
      case WM_ACTIVATE:      wmActivate(hwnd, wp, lp); break;
      case WM_INITMENU:      wmInitMenu(hwnd, wp, lp); break;
      case WM_KEYUP:         wmKeyUp(hwnd, wp, lp); break;
      case WM_LBUTTONDOWN:   wmLButtonDown(hwnd, wp, lp); break;
      case WM_RBUTTONDOWN:   wmRButtonDown(hwnd, wp, lp); break;
      case WM_LBUTTONUP:     wmLButtonUp(hwnd, wp, lp); break;
      case WM_RBUTTONUP:     wmRButtonUp(hwnd, wp, lp); break;
      case WM_LBUTTONDBLCLK: wmLButtonDBL(hwnd, wp, lp); break;
      case WM_RBUTTONDBLCLK: wmRButtonDBL(hwnd, wp, lp); break;
      case WM_MBUTTONDOWN:   wmMButtonDown(hwnd, wp, lp); break;
      case WM_MBUTTONUP:     wmMButtonUp(hwnd, wp, lp); break;
      case WM_MOUSEMOVE:     wmMouseMove(hwnd, wp, lp); break;
      case WM_MOUSEWHEEL:    wmMouseWheel(hwnd, wp, lp); break;
      case WM_DROPFILES:     wmDropFiles(hwnd, wp, lp); break;
      case WM_CLOSE:         wmClose(hwnd); return 0;
      default: break;
      }

      return DefWindowProc(hwnd, msg, wp, lp);
    }

    // Convert window coordinates (the origin is upper-left) to viewport coordinates (the origin is lower-left)
    Vector2<int> window2viewport(LPARAM lp) const { return Vector2<int>(GET_X_LPARAM(lp), wsize_.y-GET_Y_LPARAM(lp)); }
    Vector2<int> window2viewport(const POINT& pt) const { return Vector2<int>(pt.x, wsize_.y-pt.y); }
    
    int mousemodifier(WPARAM wp) const { return (wp&MK_CONTROL) ? MouseModifier::CONTROL : MouseModifier::NONE; }

    // Get a position of the mouse pointer (in window coordinates)
    POINT get_mouse_position(HWND hwnd) const {
      POINT pt;
      GetCursorPos(&pt);
      ScreenToClient(hwnd, &pt);
      return pt;
    }

  public:
    MainWindow(void) : force_direction_(0) {}
    ~MainWindow() {}

  private:
    Vector2<int> wsize_; // window size
    shared_ptr<Timer> timer_; // event timer
    shared_ptr<World> world_;
    int zoom_; // flag for the timer (0:zoom-in, 1:zoom-out)
    int force_direction_; // flag for the timer (0:stop, 1:running)
  };


  ////////////////////////////////////////////////////////////////////////////////////////////////////
  void MainWindow::wmCreate(HWND hwnd) {
    // module path
    wchar_t filename[65536];
    GetModuleFileName(GetModuleHandle(NULL), filename, 65536);
    PathSplitter path(filename);
    sociarium_project_common::set_module_path(path.drive()+path.dir());
    SetCurrentDirectory(sociarium_project_common::get_module_path().c_str());
    // window handle
    sociarium_project_common::set_window_handle(hwnd);
    // Create the OpenGL window
    world_.reset(new World);
    // Create and start the timer
    timer_.reset(new Timer(hwnd));
    timer_->add(ID_TIMER_DRAW, 10);
    timer_->add(ID_TIMER_SELECT, 20);
    timer_->add(ID_TIMER_ZOOM, 10);
    timer_->add(ID_TIMER_FORCE_DIRECTION, 20);
    timer_->start(ID_TIMER_DRAW);
  }


  ////////////////////////////////////////////////////////////////////////////////////////////////////
  void MainWindow::wmDestroy(HWND hwnd) {
    world_.reset();
    PostQuitMessage(0);
  }


  ////////////////////////////////////////////////////////////////////////////////////////////////////
  void MainWindow::wmCommand(HWND hwnd, WPARAM wp, LPARAM lp) {
    switch (LOWORD(wp)) {

    case IDM_KEY_ESCAPE: {
      // Cancel the running thread
      shared_ptr<Thread> const& graph_creation_thread = sociarium_project_thread::get_current_graph_creation_thread();
      shared_ptr<Thread> const& graph_layout_thread = sociarium_project_thread::get_current_graph_layout_thread();
      shared_ptr<Thread> const& community_detection_thread = sociarium_project_thread::get_current_community_detection_thread();
      if (graph_creation_thread!=0) {
        graph_creation_thread->suspend();
        if (MessageBox(hwnd, sociarium_project_message::DO_YOU_CANCEL_THE_RUNNING_THREAD,
                       sociarium_project_common::APPLICATION_TITLE,
                       MB_OKCANCEL|MB_ICONQUESTION|MB_DEFBUTTON1)==IDOK) graph_creation_thread->cancel();
        else graph_creation_thread->resume();
      } else if (graph_layout_thread!=0) {
        graph_layout_thread->suspend();
        if (MessageBox(hwnd, sociarium_project_message::DO_YOU_CANCEL_THE_RUNNING_THREAD,
                       sociarium_project_common::APPLICATION_TITLE,
                       MB_OKCANCEL|MB_ICONQUESTION|MB_DEFBUTTON1)==IDOK) graph_layout_thread->cancel();
        else graph_layout_thread->resume();
      } else if (community_detection_thread!=0) {
        if (MessageBox(hwnd, sociarium_project_message::DO_YOU_CANCEL_THE_RUNNING_THREAD,
                       sociarium_project_common::APPLICATION_TITLE,
                       MB_OKCANCEL|MB_ICONQUESTION|MB_DEFBUTTON1)==IDOK) community_detection_thread->cancel();
        else community_detection_thread->resume();
      }

      else {
        SendMessage(hwnd, WM_CLOSE, 0, 0);
      }
      break;
    }

    case IDM_LAYOUT_EXECUTE:
    case IDM_KEY_SPACE: {
      world_->layout_graph();
      break;
    }

    case IDM_COMMUNITY_DETECTION_EXECUTE:
    case IDM_KEY_ENTER: {
      world_->detect_community();
      break;
    }

    case IDM_KEY_HOME: {
      world_->initialize_view();
      break;
    }

    case IDM_KEY_CTRL_HOME: {
      sociarium_project_draw::set_layout_frame_size(sociarium_project_draw::get_layout_frame_default_size());
      sociarium_project_draw::set_layout_frame_previous_size(sociarium_project_draw::get_layout_frame_size());
      sociarium_project_draw::set_layout_frame_position(Vector2<int>(0, 0));
      sociarium_project_draw::set_layout_frame_previous_position(sociarium_project_draw::get_layout_frame_position());
      break;
    }

    case IDM_KEY_NEXT: {
      if (!timer_->is_active(ID_TIMER_ZOOM)) {
        zoom_ = 0;
        timer_->start(ID_TIMER_ZOOM);
      }
      break;
    }

    case IDM_KEY_PRIOR: {
      if (!timer_->is_active(ID_TIMER_ZOOM)) {
        zoom_ = 1;
        timer_->start(ID_TIMER_ZOOM);
      }
      break;
    }

    case IDM_KEY_RIGHT: {
      world_->move_layer(world_->index_of_current_layer()+1);
      break;
    }

    case IDM_KEY_LEFT: {
      world_->move_layer(world_->index_of_current_layer()-1);
      break;
    }

    case IDM_KEY_UP: {
      world_->move_layer(world_->index_of_current_layer()+1);
      break;
    }

    case IDM_KEY_DOWN: {
      world_->move_layer(world_->index_of_current_layer()-1);
      break;
    }

      // ----------------------------------------------------------------------------------------------------
      // EDIT

      // MARK
    case IDM_EDIT_FOR_CURRENT_SELECT_NODES:
    case IDM_EDIT_FOR_CURRENT_SELECT_EDGES:
    case IDM_EDIT_FOR_CURRENT_SELECT_COMMUNITIES:
    case IDM_EDIT_FOR_CURRENT_SELECT_COMMUNITY_EDGES:
    case IDM_EDIT_FOR_CURRENT_SELECT_INNER_COMMUNITY_NODES:
    case IDM_EDIT_FOR_CURRENT_SELECT_INNER_COMMUNITY_EDGES:
    case IDM_EDIT_FOR_CURRENT_SELECT_INNER_COMMUNITY_ELEMENTS:
    case IDM_EDIT_FOR_CURRENT_SELECT_OUTER_COMMUNITY_NODES:
    case IDM_EDIT_FOR_CURRENT_SELECT_OUTER_COMMUNITY_EDGES:
    case IDM_EDIT_FOR_CURRENT_SELECT_OUTER_COMMUNITY_ELEMENTS:
      world_->mark_elements(LOWORD(wp));
      break;

    case IDM_EDIT_FOR_ALL_SELECT_NODES:
    case IDM_EDIT_FOR_ALL_SELECT_EDGES:
    case IDM_EDIT_FOR_ALL_SELECT_COMMUNITIES:
    case IDM_EDIT_FOR_ALL_SELECT_COMMUNITY_EDGES:
    case IDM_EDIT_FOR_ALL_SELECT_INNER_COMMUNITY_NODES:
    case IDM_EDIT_FOR_ALL_SELECT_INNER_COMMUNITY_EDGES:
    case IDM_EDIT_FOR_ALL_SELECT_INNER_COMMUNITY_ELEMENTS:
    case IDM_EDIT_FOR_ALL_SELECT_OUTER_COMMUNITY_NODES:
    case IDM_EDIT_FOR_ALL_SELECT_OUTER_COMMUNITY_EDGES:
    case IDM_EDIT_FOR_ALL_SELECT_OUTER_COMMUNITY_ELEMENTS:
      world_->mark_elements(LOWORD(wp));
      break;

    case IDM_KEY_CTRL_A:
    case IDM_EDIT_FOR_CURRENT_SELECT_ALL:
      world_->mark_elements(IDM_EDIT_FOR_CURRENT_SELECT_ALL);
      break;

    case IDM_KEY_SHIFT_CTRL_A:
    case IDM_EDIT_FOR_ALL_SELECT_ALL:
      world_->mark_elements(IDM_EDIT_FOR_ALL_SELECT_ALL);
      break;

    case IDM_KEY_CTRL_I:
    case IDM_EDIT_FOR_CURRENT_SELECT_INVERT:
      world_->inverse_mark(IDM_EDIT_FOR_CURRENT_SELECT_INVERT);
      break;

    case IDM_KEY_SHIFT_CTRL_I:
    case IDM_EDIT_FOR_ALL_SELECT_INVERT:
      world_->inverse_mark(IDM_EDIT_FOR_ALL_SELECT_INVERT);
      break;

      // HIDE
    case IDM_EDIT_FOR_CURRENT_HIDE_MARKED_NODES:
    case IDM_EDIT_FOR_CURRENT_HIDE_MARKED_EDGES:
      world_->hide_marked_element(LOWORD(wp));
      break;

    case IDM_KEY_CTRL_H:
    case IDM_EDIT_FOR_CURRENT_HIDE_MARKED_ELEMENTS:
      world_->hide_marked_element(IDM_EDIT_FOR_CURRENT_HIDE_MARKED_ELEMENTS);
      break;

    case IDM_EDIT_FOR_ALL_HIDE_MARKED_NODES:
    case IDM_EDIT_FOR_ALL_HIDE_MARKED_EDGES:
      world_->hide_marked_element(LOWORD(wp));
      break;

    case IDM_KEY_SHIFT_CTRL_H:
    case IDM_EDIT_FOR_ALL_HIDE_MARKED_ELEMENTS:
      world_->hide_marked_element(IDM_EDIT_FOR_ALL_HIDE_MARKED_ELEMENTS);
      break;

    case IDM_EDIT_FOR_CURRENT_HIDE_MARKED_NODES_ALL:
    case IDM_EDIT_FOR_CURRENT_HIDE_MARKED_EDGES_ALL:
    case IDM_EDIT_FOR_CURRENT_HIDE_MARKED_ELEMENTS_ALL:
      world_->hide_marked_element(LOWORD(wp));
      break;

    case IDM_EDIT_FOR_ALL_HIDE_MARKED_NODES_ALL:
    case IDM_EDIT_FOR_ALL_HIDE_MARKED_EDGES_ALL:
    case IDM_EDIT_FOR_ALL_HIDE_MARKED_ELEMENTS_ALL:
      world_->hide_marked_element(LOWORD(wp));
      break;

      // REMOVE
    case IDM_EDIT_FOR_CURRENT_REMOVE_MARKED_NODES:
    case IDM_EDIT_FOR_CURRENT_REMOVE_MARKED_EDGES:
      world_->remove_marked_elements(LOWORD(wp));
      break;

    case IDM_KEY_DELETE:
    case IDM_EDIT_FOR_CURRENT_REMOVE_MARKED_ELEMENTS:
      world_->remove_marked_elements(IDM_EDIT_FOR_CURRENT_REMOVE_MARKED_ELEMENTS);
      break;

    case IDM_EDIT_FOR_ALL_REMOVE_MARKED_NODES:
    case IDM_EDIT_FOR_ALL_REMOVE_MARKED_EDGES:
      world_->remove_marked_elements(LOWORD(wp));
      break;

    case IDM_KEY_CTRL_DELETE:
    case IDM_EDIT_FOR_ALL_REMOVE_MARKED_ELEMENTS:
      world_->remove_marked_elements(IDM_EDIT_FOR_ALL_REMOVE_MARKED_ELEMENTS);
      break;

    case IDM_EDIT_FOR_CURRENT_REMOVE_MARKED_NODES_ALL:
    case IDM_EDIT_FOR_CURRENT_REMOVE_MARKED_EDGES_ALL:
    case IDM_EDIT_FOR_CURRENT_REMOVE_MARKED_ELEMENTS_ALL:
      world_->remove_marked_elements(LOWORD(wp));
      break;

    case IDM_EDIT_FOR_ALL_REMOVE_MARKED_NODES_ALL:
    case IDM_EDIT_FOR_ALL_REMOVE_MARKED_EDGES_ALL:
    case IDM_EDIT_FOR_ALL_REMOVE_MARKED_ELEMENTS_ALL:
      world_->remove_marked_elements(LOWORD(wp));
      break;

      // SHOW
    case IDM_EDIT_FOR_CURRENT_SHOW_NODES:
    case IDM_EDIT_FOR_CURRENT_SHOW_EDGES:
      world_->show_elements(LOWORD(wp));
      break;

    case IDM_KEY_CTRL_U:
    case IDM_EDIT_FOR_CURRENT_SHOW_ELEMENTS:
      world_->show_elements(IDM_EDIT_FOR_CURRENT_SHOW_ELEMENTS);
      break;

    case IDM_EDIT_FOR_ALL_SHOW_EDGES:
    case IDM_EDIT_FOR_ALL_SHOW_NODES:
      world_->show_elements(LOWORD(wp));
      break;

    case IDM_KEY_SHIFT_CTRL_U:
    case IDM_EDIT_FOR_ALL_SHOW_ELEMENTS:
      world_->show_elements(IDM_EDIT_FOR_ALL_SHOW_ELEMENTS);
      break;

      // ----------------------------------------------------------------------------------------------------
      // VIEW

    case IDM_KEY_1:
    case IDM_SHOW_NODE: {
      sociarium_project_view::set_show_node(!sociarium_project_view::get_show_node());
      break;
    }

    case IDM_KEY_2:
    case IDM_SHOW_EDGE: {
      sociarium_project_view::set_show_edge(!sociarium_project_view::get_show_edge());
      break;
    }

    case IDM_KEY_3:
    case IDM_SHOW_COMMUNITY: {
      sociarium_project_view::set_show_community(!sociarium_project_view::get_show_community());
      break;
    }

    case IDM_KEY_4:
    case IDM_SHOW_COMMUNITY_EDGE: {
      sociarium_project_view::set_show_community_edge(!sociarium_project_view::get_show_community_edge());
      break;
    }

    case IDM_KEY_SHIFT_CTRL_1:
    case IDM_SHOW_NODE_LABEL: {
      sociarium_project_view::set_show_node_label(!sociarium_project_view::get_show_node_label());
      break;
    }

    case IDM_KEY_SHIFT_CTRL_2:
    case IDM_SHOW_EDGE_LABEL: {
      sociarium_project_view::set_show_edge_label(!sociarium_project_view::get_show_edge_label());
      break;
    }

    case IDM_KEY_SHIFT_CTRL_3:
    case IDM_SHOW_COMMUNITY_LABEL: {
      sociarium_project_view::set_show_community_label(!sociarium_project_view::get_show_community_label());
      break;
    }

    case IDM_KEY_SHIFT_CTRL_4:
    case IDM_SHOW_COMMUNITY_EDGE_LABEL: {
      sociarium_project_view::set_show_community_edge_label(!sociarium_project_view::get_show_community_edge_label());
      break;
    }

    case IDM_KEY_CTRL_1: {
      sociarium_project_view::shift_node_style();
      break;
    }

    case IDM_KEY_CTRL_2: {
      sociarium_project_view::shift_edge_style();
      break;
    }

    case IDM_KEY_CTRL_3: {
      sociarium_project_view::shift_community_style();
      break;
    }

    case IDM_KEY_CTRL_4: {
      sociarium_project_view::shift_community_edge_style();
      break;
    }

    case IDM_NODE_STYLE_POLYGON: {
      sociarium_project_view::set_node_style(sociarium_project_view::NodeView::Style::POLYGON);
      break;
    }

    case IDM_NODE_STYLE_TEXTURE: {
      sociarium_project_view::set_node_style(sociarium_project_view::NodeView::Style::TEXTURE);
      break;
    }

      //     case IDM_EDGE_STYLE_STRAIGHT: {
      //       sociarium_project_view::set_edge_style(sociarium_project_view::EdgeView::Style::STRAIGHT);
      //       break;
      //     }
      //
      //     case IDM_EDGE_STYLE_HCURVE: {
      //       sociarium_project_view::set_edge_style(sociarium_project_view::EdgeView::Style::HCURVE);
      //       break;
      //     }
      //
      //     case IDM_EDGE_STYLE_VCURVE: {
      //       sociarium_project_view::set_edge_style(sociarium_project_view::EdgeView::Style::VCURVE);
      //       break;
      //     }

    case IDM_EDGE_STYLE_LINE: {
      sociarium_project_view::set_edge_style(sociarium_project_view::EdgeView::Style::LINE);
      break;
    }

    case IDM_EDGE_STYLE_TEXTURE: {
      sociarium_project_view::set_edge_style(sociarium_project_view::EdgeView::Style::TEXTURE);
      break;
    }

    case IDM_COMMUNITY_STYLE_POLYGON_CIRCLE: {
      sociarium_project_view::set_community_style(sociarium_project_view::CommunityView::Style::POLYGON_CIRCLE);
      break;
    }

    case IDM_COMMUNITY_STYLE_POLYGON_CURVE: {
      sociarium_project_view::set_community_style(sociarium_project_view::CommunityView::Style::POLYGON_CURVE);
      break;
    }

    case IDM_COMMUNITY_STYLE_TEXTURE: {
      sociarium_project_view::set_community_style(sociarium_project_view::CommunityView::Style::TEXTURE);
      break;
    }

    case IDM_COMMUNITY_EDGE_STYLE_LINE: {
      sociarium_project_view::set_community_edge_style(sociarium_project_view::CommunityEdgeView::Style::LINE);
      break;
    }

    case IDM_COMMUNITY_EDGE_STYLE_TEXTURE: {
      sociarium_project_view::set_community_edge_style(sociarium_project_view::CommunityEdgeView::Style::TEXTURE);
      break;
    }

    case IDM_NODE_SIZE_UNIFORM: {
      world_->update_node_size(sociarium_project_view::NodeView::SizeFactor::UNIFORM);
      break;
    }

    case IDM_NODE_SIZE_REPRESENTS_DEGREE_CENTRALITY: {
      world_->update_node_size(sociarium_project_view::NodeView::SizeFactor::DEGREE_CENTRALITY);
      break;
    }

    case IDM_NODE_SIZE_REPRESENTS_CLOSENESS_CENTRALITY: {
      world_->update_node_size(sociarium_project_view::NodeView::SizeFactor::CLOSENESS_CENTRALITY);
      break;
    }

    case IDM_NODE_SIZE_REPRESENTS_BETWEENNESS_CENTRALITY: {
      world_->update_node_size(sociarium_project_view::NodeView::SizeFactor::BETWEENNESS_CENTRALITY);
      break;
    }

    case IDM_NODE_SIZE_REPRESENTS_PAGERANK: {
      world_->update_node_size(sociarium_project_view::NodeView::SizeFactor::PAGERANK);
      break;
    }

    case IDM_NODE_SIZE_NONE: {
      world_->update_node_size(sociarium_project_view::NodeView::SizeFactor::NONE);
      break;
    }

    case IDM_FONT_SIZE_NODE_0: { sociarium_project_font::set_node_font_scale(0); break; }
    case IDM_FONT_SIZE_NODE_1: { sociarium_project_font::set_node_font_scale(1); break; }
    case IDM_FONT_SIZE_NODE_2: { sociarium_project_font::set_node_font_scale(2); break; }
    case IDM_FONT_SIZE_NODE_3: { sociarium_project_font::set_node_font_scale(3); break; }

    case IDM_FONT_SIZE_EDGE_0: { sociarium_project_font::set_edge_font_scale(0); break; }
    case IDM_FONT_SIZE_EDGE_1: { sociarium_project_font::set_edge_font_scale(1); break; }
    case IDM_FONT_SIZE_EDGE_2: { sociarium_project_font::set_edge_font_scale(2); break; }
    case IDM_FONT_SIZE_EDGE_3: { sociarium_project_font::set_edge_font_scale(3); break; }

    case IDM_FONT_SIZE_COMMUNITY_0: { sociarium_project_font::set_community_font_scale(0); break; }
    case IDM_FONT_SIZE_COMMUNITY_1: { sociarium_project_font::set_community_font_scale(1); break; }
    case IDM_FONT_SIZE_COMMUNITY_2: { sociarium_project_font::set_community_font_scale(2); break; }
    case IDM_FONT_SIZE_COMMUNITY_3: { sociarium_project_font::set_community_font_scale(3); break; }

    case IDM_FONT_SIZE_COMMUNITY_EDGE_0: { sociarium_project_font::set_community_edge_font_scale(0); break; }
    case IDM_FONT_SIZE_COMMUNITY_EDGE_1: { sociarium_project_font::set_community_edge_font_scale(1); break; }
    case IDM_FONT_SIZE_COMMUNITY_EDGE_2: { sociarium_project_font::set_community_edge_font_scale(2); break; }
    case IDM_FONT_SIZE_COMMUNITY_EDGE_3: { sociarium_project_font::set_community_edge_font_scale(3); break; }

    case IDM_KEY_SHIFT_CTRL_Z: {
      timer_->stop(ID_TIMER_DRAW);
      for (int i=0; i<10; ++i) {
        world_->zoom(1.05);
        world_->draw();
      }
      timer_->start(ID_TIMER_DRAW);
      break;
    }

    case IDM_KEY_SHIFT_CTRL_X: {
      timer_->stop(ID_TIMER_DRAW);
      for (int i=0; i<10; ++i) {
        world_->zoom(1.0/1.05);
        world_->draw();
      }
      timer_->start(ID_TIMER_DRAW);
      break;
    }

    case IDM_KEY_S:
    case IDM_SHOW_SLIDER: {
      sociarium_project_view::set_show_slider(!sociarium_project_view::get_show_slider());
      break;
    }

    case IDM_KEY_T:
    case IDM_SHOW_TIME_LABEL: {
      sociarium_project_view::set_show_time_label(!sociarium_project_view::get_show_time_label());
      break;
    }

    case IDM_KEY_F:
    case IDM_SHOW_LAYOUT_FRAME: {
      sociarium_project_view::set_show_layout_frame(!sociarium_project_view::get_show_layout_frame());
      break;
    }

    case IDM_KEY_G:
    case IDM_SHOW_GRID: {
      sociarium_project_view::set_show_grid(!sociarium_project_view::get_show_grid());
      break;
    }

    case IDM_KEY_CTRL_F:
    case IDM_SHOW_FPS: {
      sociarium_project_view::set_show_fps(!sociarium_project_view::get_show_fps());
      break;
    }

    case IDM_SHOW_CENTER: {
      sociarium_project_view::set_show_center(!sociarium_project_view::get_show_center());
      break;
    }

      // ----------------------------------------------------------------------------------------------------
      // LAYOUT

    case IDM_LAYOUT_KAMADA_KAWAI_METHOD: {
      sociarium_project_algorithm_selector::set_graph_layout_algorithm(sociarium_project_algorithm_selector::LayoutAlgorithm::KAMADA_KAWAI_METHOD);
      break;
    }

    case IDM_LAYOUT_HIGH_DIMENSIONAL_EMBEDDING: {
      sociarium_project_algorithm_selector::set_graph_layout_algorithm(sociarium_project_algorithm_selector::LayoutAlgorithm::HDE);
      break;
    }

    case IDM_LAYOUT_CIRCLE: {
      sociarium_project_algorithm_selector::set_graph_layout_algorithm(sociarium_project_algorithm_selector::LayoutAlgorithm::CIRCLE);
      break;
    }

    case IDM_LAYOUT_CIRCLE_IN_SIZE_ORDER: {
      sociarium_project_algorithm_selector::set_graph_layout_algorithm(sociarium_project_algorithm_selector::LayoutAlgorithm::CIRCLE_IN_SIZE_ORDER);
      break;
    }

    case IDM_LAYOUT_ARRAY: {
      sociarium_project_algorithm_selector::set_graph_layout_algorithm(sociarium_project_algorithm_selector::LayoutAlgorithm::ARRAY);
      break;
    }

    case IDM_LAYOUT_RANDOM: {
      sociarium_project_algorithm_selector::set_graph_layout_algorithm(sociarium_project_algorithm_selector::LayoutAlgorithm::RANDOM);
      break;
    }

    case IDM_KEY_CTRL_SPACE:
    case IDM_LAYOUT_FORCE_DIRECTION: {
      if (force_direction_==0) {
        force_direction_ = 1;
        timer_->start(ID_TIMER_FORCE_DIRECTION);
      } else {
        force_direction_ = 0;
        timer_->stop(ID_TIMER_FORCE_DIRECTION);
      }
      break;
    }

    case IDM_LAYOUT_AUTO: {
      sociarium_project_layout::set_auto_layout(!sociarium_project_layout::get_auto_layout());
      break;
    }

      // ----------------------------------------------------------------------------------------------------
      // COMMUNITY_DETECTION

    case IDM_CONNECTED_COMPONENTS: {
      sociarium_project_algorithm_selector::set_community_detection_algorithm(sociarium_project_algorithm_selector::CommunityDetectionAlgorithm::CONNECTED_COMPONENTS);
      break;
    }

    case IDM_STRONGLY_CONNECTED_COMPONENTS: {
      sociarium_project_algorithm_selector::set_community_detection_algorithm(sociarium_project_algorithm_selector::CommunityDetectionAlgorithm::STRONGLY_CONNECTED_COMPONENTS);
      break;
    }

    case IDM_CLIQUE_PERCOLATION_3: {
      sociarium_project_algorithm_selector::set_community_detection_algorithm(sociarium_project_algorithm_selector::CommunityDetectionAlgorithm::CLIQUE_PERCOLATION_3);
      break;
    }

    case IDM_CLIQUE_PERCOLATION_4: {
      sociarium_project_algorithm_selector::set_community_detection_algorithm(sociarium_project_algorithm_selector::CommunityDetectionAlgorithm::CLIQUE_PERCOLATION_4);
      break;
    }

    case IDM_CLIQUE_PERCOLATION_5: {
      sociarium_project_algorithm_selector::set_community_detection_algorithm(sociarium_project_algorithm_selector::CommunityDetectionAlgorithm::CLIQUE_PERCOLATION_5);
      break;
    }

    case IDM_MODULARITY_MAXIMIZATION_USING_GREEDY_METHOD: {
      sociarium_project_algorithm_selector::set_community_detection_algorithm(sociarium_project_algorithm_selector::CommunityDetectionAlgorithm::MODULARITY_MAXIMIZATION_USING_GREEDY_METHOD);
      break;
    }

    case IDM_MODULARITY_MAXIMIZATION_USING_TEO_METHOD:
      sociarium_project_algorithm_selector::set_community_detection_algorithm(sociarium_project_algorithm_selector::CommunityDetectionAlgorithm::MODULARITY_MAXIMIZATION_USING_TEO_METHOD);
      break;

    case IDM_BETWEENNESS_CENTRALITY_CLUSTERING:
      sociarium_project_algorithm_selector::set_community_detection_algorithm(sociarium_project_algorithm_selector::CommunityDetectionAlgorithm::BETWEENNESS_CENTRALITY_CLUSTERING);
      break;

    case IDM_USE_WEIGHTED_MODULARITY:
      sociarium_project_algorithm_selector::use_weighted_modularity(!sociarium_project_algorithm_selector::use_weighted_modularity());
      break;

    case IDM_COMMUNITY_DETECTION_CLEAR: {
      world_->clear_community();
      break;
    }

    default: {
      break;
    }
    }
  }


  ////////////////////////////////////////////////////////////////////////////////////////////////////
  void MainWindow::wmSize(HWND hwnd) {
    RECT rect;
    GetClientRect(hwnd, &rect);
    wsize_.set(rect.right-rect.left, rect.bottom-rect.top);
    world_->resize_window(wsize_);
    InvalidateRect(hwnd, 0, FALSE);
  }


  ////////////////////////////////////////////////////////////////////////////////////////////////////
  void MainWindow::wmPaint(HWND hwnd) {
    world_->draw();
    ValidateRect(hwnd, NULL);
  }


  ////////////////////////////////////////////////////////////////////////////////////////////////////
  void MainWindow::wmTimer(HWND hwnd, WPARAM wp, LPARAM lp) {

    switch (wp) {

    case ID_TIMER_DRAW: {
      RECT rect;
      GetClientRect(hwnd, &rect);
      InvalidateRect(hwnd, &rect, FALSE);
      break;
    }

    case ID_TIMER_ZOOM: {
      world_->zoom(zoom_?1.05:1.0/1.05);
      break;
    }

    case ID_TIMER_FORCE_DIRECTION: {
      world_->do_force_direction();
      break;
    }

    default: {
      break;
    }
    }
  }


  ////////////////////////////////////////////////////////////////////////////////////////////////////
  void MainWindow::wmActivate(HWND hwnd, WPARAM wp, LPARAM lp) {
    //if (LOWORD(wp)==WA_INACTIVE) timer_->stop(ID_TIMER_SELECT);
    //else timer_->start(ID_TIMER_SELECT);
  }


  ////////////////////////////////////////////////////////////////////////////////////////////////////
  void MainWindow::wmInitMenu(HWND hwnd, WPARAM wp, LPARAM lp) {
    HMENU hmenu = HMENU(wp);

    using namespace sociarium_project_view;
    using namespace sociarium_project_font;
    using namespace sociarium_project_algorithm_selector;

    // ----------------------------------------------------------------------------------------------------
    // SHOW/HIDE

    { // Show/Hide nodes
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_show_node()?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_SHOW_NODE, FALSE, &mii);
    }{ // Show/Hide edges
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_show_edge()?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_SHOW_EDGE, FALSE, &mii);
    }{ // Show/Hide communities
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_show_community()?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_SHOW_COMMUNITY, FALSE, &mii);
    }{ // Show/Hide community edges
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_show_community_edge()?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_SHOW_COMMUNITY_EDGE, FALSE, &mii);
    }

    { // Show/Hide node label
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_show_node_label()?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_SHOW_NODE_LABEL, FALSE, &mii);
    }{ // Show/Hide edge label
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_show_edge_label()?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_SHOW_EDGE_LABEL, FALSE, &mii);
    }{ // Show/Hide community label
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_show_community_label()?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_SHOW_COMMUNITY_LABEL, FALSE, &mii);
    }{ // Show/Hide community edge label
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_show_community_edge_label()?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_SHOW_COMMUNITY_EDGE_LABEL, FALSE, &mii);
    }

    { // Show/Hide slider
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_show_slider()?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_SHOW_SLIDER, FALSE, &mii);
    }{ // Show/Hide layer label
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_show_time_label()?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_SHOW_TIME_LABEL, FALSE, &mii);
    }{ // Show/Hide GRID
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_show_grid()?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_SHOW_GRID, FALSE, &mii);
    }{ // Show/Hide FPS
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_show_fps()?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_SHOW_FPS, FALSE, &mii);
    }{ // Show/Hide reference point
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_show_center()?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_SHOW_CENTER, FALSE, &mii);
    }{ // Show/Hide layout frame
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_show_layout_frame()?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_SHOW_LAYOUT_FRAME, FALSE, &mii);
    }

    // ----------------------------------------------------------------------------------------------------
    // DRAWING STYLE

    { // Node style: polygon
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_node_style()&NodeView::Style::POLYGON?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_NODE_STYLE_POLYGON, FALSE, &mii);
    }{ // Node style: texture
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_node_style()&NodeView::Style::TEXTURE?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_NODE_STYLE_TEXTURE, FALSE, &mii);
    }

    { // Edge style
      //       MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      //       mii.fState = get_edge_style()&EdgeView::Style::STRAIGHT?MFS_CHECKED:MFS_UNCHECKED;
      //       SetMenuItemInfo(hmenu, IDM_EDGE_STYLE_STRAIGHT, FALSE, &mii);
      //     }{
      //       MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      //       mii.fState = get_edge_style()&EdgeView::Style::HCURVE?MFS_CHECKED:MFS_UNCHECKED;
      //       SetMenuItemInfo(hmenu, IDM_EDGE_STYLE_HCURVE, FALSE, &mii);
      //     }{
      //       MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      //       mii.fState = get_edge_style()&EdgeView::Style::VCURVE?MFS_CHECKED:MFS_UNCHECKED;
      //       SetMenuItemInfo(hmenu, IDM_EDGE_STYLE_VCURVE, FALSE, &mii);
      //     }{
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_edge_style()&EdgeView::Style::LINE?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_EDGE_STYLE_LINE, FALSE, &mii);
    }{
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_edge_style()&EdgeView::Style::TEXTURE?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_EDGE_STYLE_TEXTURE, FALSE, &mii);
    }

    { // Community style: polygon circle
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_community_style()&CommunityView::Style::POLYGON_CIRCLE?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_COMMUNITY_STYLE_POLYGON_CIRCLE, FALSE, &mii);
    }{ // Community style: polygon curve
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_community_style()&CommunityView::Style::POLYGON_CURVE?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_COMMUNITY_STYLE_POLYGON_CURVE, FALSE, &mii);
    }{ // Node style: texture
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_community_style()&CommunityView::Style::TEXTURE?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_COMMUNITY_STYLE_TEXTURE, FALSE, &mii);
    }

    { // CommunityEdge style
      //       MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      //       mii.fState = get_community_edge_style()&CommunityEdgeView::Style::STRAIGHT?MFS_CHECKED:MFS_UNCHECKED;
      //       SetMenuItemInfo(hmenu, IDM_COMMUNITY_EDGE_STYLE_STRAIGHT, FALSE, &mii);
      //     }{
      //       MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      //       mii.fState = get_community_edge_style()&CommunityEdgeView::Style::HCURVE?MFS_CHECKED:MFS_UNCHECKED;
      //       SetMenuItemInfo(hmenu, IDM_COMMUNITY_EDGE_STYLE_HCURVE, FALSE, &mii);
      //     }{
      //       MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      //       mii.fState = get_community_edge_style()&CommunityEdgeView::Style::VCURVE?MFS_CHECKED:MFS_UNCHECKED;
      //       SetMenuItemInfo(hmenu, IDM_COMMUNITY_EDGE_STYLE_VCURVE, FALSE, &mii);
      //     }{
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_community_edge_style()&CommunityEdgeView::Style::LINE?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_COMMUNITY_EDGE_STYLE_LINE, FALSE, &mii);
    }{
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_community_edge_style()&CommunityEdgeView::Style::TEXTURE?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_COMMUNITY_EDGE_STYLE_TEXTURE, FALSE, &mii);
    }

    { // Node size: uniform
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_node_size_factor()==NodeView::SizeFactor::UNIFORM?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_NODE_SIZE_UNIFORM, FALSE, &mii);
    }{ // Node size: degree centrality
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_node_size_factor()==NodeView::SizeFactor::DEGREE_CENTRALITY?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_NODE_SIZE_REPRESENTS_DEGREE_CENTRALITY, FALSE, &mii);
    }{ // Node size: closeness centrality
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_node_size_factor()==NodeView::SizeFactor::CLOSENESS_CENTRALITY?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_NODE_SIZE_REPRESENTS_CLOSENESS_CENTRALITY, FALSE, &mii);
    }{ // Node size: betweenness centrality
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_node_size_factor()==NodeView::SizeFactor::BETWEENNESS_CENTRALITY?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_NODE_SIZE_REPRESENTS_BETWEENNESS_CENTRALITY, FALSE, &mii);
    }{ // Node size: pagerank
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_node_size_factor()==NodeView::SizeFactor::PAGERANK?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_NODE_SIZE_REPRESENTS_PAGERANK, FALSE, &mii);
    }{ // Node size: point
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_node_size_factor()==NodeView::SizeFactor::NONE?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_NODE_SIZE_NONE, FALSE, &mii);
    }

    // ----------------------------------------------------------------------------------------------------
    // FONT

    { // Label size of nodes
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_node_font_scale()==get_font_scale(0)?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_FONT_SIZE_NODE_0, FALSE, &mii);
    }{
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_node_font_scale()==get_font_scale(1)?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_FONT_SIZE_NODE_1, FALSE, &mii);
    }{
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_node_font_scale()==get_font_scale(2)?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_FONT_SIZE_NODE_2, FALSE, &mii);
    }{
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_node_font_scale()==get_font_scale(3)?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_FONT_SIZE_NODE_3, FALSE, &mii);
    }

    { // Label size of edges
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_edge_font_scale()==get_font_scale(0)?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_FONT_SIZE_EDGE_0, FALSE, &mii);
    }{
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_edge_font_scale()==get_font_scale(1)?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_FONT_SIZE_EDGE_1, FALSE, &mii);
    }{
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_edge_font_scale()==get_font_scale(2)?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_FONT_SIZE_EDGE_2, FALSE, &mii);
    }{
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_edge_font_scale()==get_font_scale(3)?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_FONT_SIZE_EDGE_3, FALSE, &mii);
    }

    { // Label size of communities
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_community_font_scale()==get_font_scale(0)?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_FONT_SIZE_COMMUNITY_0, FALSE, &mii);
    }{
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_community_font_scale()==get_font_scale(1)?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_FONT_SIZE_COMMUNITY_1, FALSE, &mii);
    }{
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_community_font_scale()==get_font_scale(2)?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_FONT_SIZE_COMMUNITY_2, FALSE, &mii);
    }{
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_community_font_scale()==get_font_scale(3)?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_FONT_SIZE_COMMUNITY_3, FALSE, &mii);
    }

    { // Label size of community edges
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_community_edge_font_scale()==get_font_scale(0)?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_FONT_SIZE_COMMUNITY_EDGE_0, FALSE, &mii);
    }{
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_community_edge_font_scale()==get_font_scale(1)?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_FONT_SIZE_COMMUNITY_EDGE_1, FALSE, &mii);
    }{
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_community_edge_font_scale()==get_font_scale(2)?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_FONT_SIZE_COMMUNITY_EDGE_2, FALSE, &mii);
    }{
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_community_edge_font_scale()==get_font_scale(3)?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_FONT_SIZE_COMMUNITY_EDGE_3, FALSE, &mii);
    }

    // ----------------------------------------------------------------------------------------------------
    // LAYOUT

    { // Kamada-Kawai Method
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_graph_layout_algorithm()==LayoutAlgorithm::KAMADA_KAWAI_METHOD?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_LAYOUT_KAMADA_KAWAI_METHOD, FALSE, &mii);
    }{ // High Dimensional Embedding
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_graph_layout_algorithm()==LayoutAlgorithm::HDE?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_LAYOUT_HIGH_DIMENSIONAL_EMBEDDING, FALSE, &mii);
    }{ // Ring (initial state)
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_graph_layout_algorithm()==LayoutAlgorithm::CIRCLE?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_LAYOUT_CIRCLE, FALSE, &mii);
    }{ // Ring (descending order of node size)
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_graph_layout_algorithm()==LayoutAlgorithm::CIRCLE_IN_SIZE_ORDER?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_LAYOUT_CIRCLE_IN_SIZE_ORDER, FALSE, &mii);
    }{ // Array
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_graph_layout_algorithm()==LayoutAlgorithm::ARRAY?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_LAYOUT_ARRAY, FALSE, &mii);
    }{ // Random
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_graph_layout_algorithm()==LayoutAlgorithm::RANDOM?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_LAYOUT_RANDOM, FALSE, &mii);
    }{ // Force Direction
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = force_direction_==1?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_LAYOUT_FORCE_DIRECTION, FALSE, &mii);
    }{ // Auto Layout
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = sociarium_project_layout::get_auto_layout()?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_LAYOUT_AUTO, FALSE, &mii);
    }

    // ----------------------------------------------------------------------------------------------------
    // COMMUNITY_DETECTION

    { // Connected Connected
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_community_detection_algorithm()==CommunityDetectionAlgorithm::CONNECTED_COMPONENTS?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_CONNECTED_COMPONENTS, FALSE, &mii);
    }{ // Strongly Connected Connected
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_community_detection_algorithm()==CommunityDetectionAlgorithm::STRONGLY_CONNECTED_COMPONENTS?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_STRONGLY_CONNECTED_COMPONENTS, FALSE, &mii);
    }{ // 3-Clique Percolation
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_community_detection_algorithm()==CommunityDetectionAlgorithm::CLIQUE_PERCOLATION_3?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_CLIQUE_PERCOLATION_3, FALSE, &mii);
    }{ // 4-Clique Percolation
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_community_detection_algorithm()==CommunityDetectionAlgorithm::CLIQUE_PERCOLATION_4?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_CLIQUE_PERCOLATION_4, FALSE, &mii);
    }{ // 5-Clique Percolation
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_community_detection_algorithm()==CommunityDetectionAlgorithm::CLIQUE_PERCOLATION_5?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_CLIQUE_PERCOLATION_5, FALSE, &mii);
    }{ // Modularity Maximization [Greedy Method]
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_community_detection_algorithm()==CommunityDetectionAlgorithm::MODULARITY_MAXIMIZATION_USING_GREEDY_METHOD?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_MODULARITY_MAXIMIZATION_USING_GREEDY_METHOD, FALSE, &mii);
    }{ // Modularity Maximization [t-EO Method]
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_community_detection_algorithm()==CommunityDetectionAlgorithm::MODULARITY_MAXIMIZATION_USING_TEO_METHOD?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_MODULARITY_MAXIMIZATION_USING_TEO_METHOD, FALSE, &mii);
    }{ // Betweenness Centrality Clustering
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = get_community_detection_algorithm()==CommunityDetectionAlgorithm::BETWEENNESS_CENTRALITY_CLUSTERING?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_BETWEENNESS_CENTRALITY_CLUSTERING, FALSE, &mii);
    }{
      MENUITEMINFO mii = { sizeof(MENUITEMINFO),MIIM_STATE,0,0,0,0,0,0,0,0,0 };
      mii.fState = use_weighted_modularity()?MFS_CHECKED:MFS_UNCHECKED;
      SetMenuItemInfo(hmenu, IDM_USE_WEIGHTED_MODULARITY, FALSE, &mii);
    }
  }


  ////////////////////////////////////////////////////////////////////////////////////////////////////
  void MainWindow::wmKeyUp(HWND hwnd, WPARAM wp, LPARAM lp) {
    switch (wp) {
    case VK_NEXT:
    case VK_PRIOR: {
      timer_->stop(ID_TIMER_ZOOM);
      break;
    }

    default: {
      break;
    }
    }
  }


  ////////////////////////////////////////////////////////////////////////////////////////////////////
  void MainWindow::wmLButtonDown(HWND hwnd, WPARAM wp, LPARAM lp) {
    SetFocus(hwnd);
    SetCapture(hwnd);
    //timer_->stop(ID_TIMER_SELECT);
    world_->do_mouse_action(MouseAction::LBUTTON_DOWN, window2viewport(lp), mousemodifier(wp));
  }


  ////////////////////////////////////////////////////////////////////////////////////////////////////
  void MainWindow::wmRButtonDown(HWND hwnd, WPARAM wp, LPARAM lp) {
    SetFocus(hwnd);
    SetCapture(hwnd);
    //timer_->stop(ID_TIMER_SELECT);
    world_->do_mouse_action(MouseAction::RBUTTON_DOWN, window2viewport(lp), mousemodifier(wp));
  }


  ////////////////////////////////////////////////////////////////////////////////////////////////////
  void MainWindow::wmMButtonDown(HWND hwnd, WPARAM wp, LPARAM lp) {
    SetFocus(hwnd);
    SetCapture(hwnd);
    //timer_->stop(ID_TIMER_SELECT);
    world_->do_mouse_action(MouseAction::MBUTTON_DOWN, window2viewport(lp), mousemodifier(wp));
  }


  ////////////////////////////////////////////////////////////////////////////////////////////////////
  void MainWindow::wmLButtonUp(HWND hwnd, WPARAM wp, LPARAM lp) {
    if (GetCapture()==hwnd) {
      ReleaseCapture();
      world_->do_mouse_action(MouseAction::LBUTTON_UP, window2viewport(lp), mousemodifier(wp));
      //timer_->start(ID_TIMER_SELECT);
    }
  }


  ////////////////////////////////////////////////////////////////////////////////////////////////////
  void MainWindow::wmRButtonUp(HWND hwnd, WPARAM wp, LPARAM lp) {
    if (GetCapture()==hwnd) {
      ReleaseCapture();
      world_->do_mouse_action(MouseAction::RBUTTON_UP, window2viewport(lp), mousemodifier(wp));
      //timer_->start(ID_TIMER_SELECT);
    }
  }


  ////////////////////////////////////////////////////////////////////////////////////////////////////
  void MainWindow::wmMButtonUp(HWND hwnd, WPARAM wp, LPARAM lp) {
    if (GetCapture()==hwnd) {
      ReleaseCapture();
      world_->do_mouse_action(MouseAction::MBUTTON_UP, window2viewport(lp), mousemodifier(wp));
      //timer_->start(ID_TIMER_SELECT);
    }
  }


  ////////////////////////////////////////////////////////////////////////////////////////////////////
  void MainWindow::wmLButtonDBL(HWND hwnd, WPARAM wp, LPARAM lp) {
    timer_->stop(ID_TIMER_DRAW);
    //timer_->stop(ID_TIMER_SELECT);
    world_->do_mouse_action(MouseAction::LBUTTON_DBL, window2viewport(lp), mousemodifier(wp));
    timer_->start(ID_TIMER_DRAW);
    //timer_->start(ID_TIMER_SELECT);
  }


  ////////////////////////////////////////////////////////////////////////////////////////////////////
  void MainWindow::wmRButtonDBL(HWND hwnd, WPARAM wp, LPARAM lp) {
    wmRButtonDown(hwnd, wp, lp);
  }


  ////////////////////////////////////////////////////////////////////////////////////////////////////
  void MainWindow::wmMouseMove(HWND hwnd, WPARAM wp, LPARAM lp) {
    if (GetCapture()==hwnd && wp&(MK_LBUTTON|MK_RBUTTON))
      world_->do_mouse_action((wp&MK_LBUTTON) ? MouseAction::LBUTTON_DRAG : MouseAction::RBUTTON_DRAG, window2viewport(lp), mousemodifier(wp));
    else
      world_->select(window2viewport(get_mouse_position(hwnd)));
  }


  ////////////////////////////////////////////////////////////////////////////////////////////////////
  void MainWindow::wmMouseWheel(HWND hwnd, WPARAM wp, LPARAM lp) {
    world_->do_mouse_action(MouseAction::WHEEL, window2viewport(lp), short(HIWORD(wp))/120); // ±120 per 1ノッチ
  }


  ////////////////////////////////////////////////////////////////////////////////////////////////////
  void MainWindow::wmDropFiles(HWND hwnd, WPARAM wp, LPARAM lp) {
    static wchar_t filename[_MAX_PATH];
    HDROP hdrop = (HDROP)wp;
    DragQueryFile(hdrop, 0, filename, _MAX_PATH);
    string const& filename_s = wcs2mbcs(filename, wcslen(filename));
    if (is_text(filename_s.c_str())) world_->read_file(filename);
  }


  ////////////////////////////////////////////////////////////////////////////////////////////////////
  void MainWindow::wmClose(HWND hwnd) {
    if (MessageBox(hwnd, sociarium_project_message::DO_YOU_QUIT,
                   sociarium_project_common::APPLICATION_TITLE,
                   MB_OKCANCEL|MB_ICONQUESTION|MB_DEFBUTTON1)==IDOK)
      DestroyWindow(hwnd);
  }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// Entrypoint of the application
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPSTR lpCmdLine, int nCmdShow) {

  // --------------------------------------------------------------------------------
  // Set locale (日本語の混じったパスへの対応)
  std::locale::global(std::locale("japanese", std::locale::ctype));

  // --------------------------------------------------------------------------------
  // Register window class
  hashimoto_ut::RegisterWindowClass
    wcmain(L"WCMAIN", CS_DBLCLKS, 0, 0, GetModuleHandle(0),
           LoadIcon(NULL, IDI_APPLICATION), LoadCursor(NULL, IDC_ARROW),
           HBRUSH(GetStockObject(HOLLOW_BRUSH)), L"MENU", NULL);

  // --------------------------------------------------------------------------------
  // Create window
  const DWORD style = WS_OVERLAPPEDWINDOW | CS_DBLCLKS;
  const DWORD exstyle = WS_EX_CLIENTEDGE;
  hashimoto_ut::MainWindow wnd;
  wnd.create(L"WCMAIN", hashimoto_ut::sociarium_project_common::APPLICATION_TITLE,
             style, exstyle, CW_USEDEFAULT, CW_USEDEFAULT, 809, 500, 0, 0);

  // --------------------------------------------------------------------------------
  // Some initializations
  InitCommonControls();
  HACCEL hAccelTbl(LoadAccelerators(hInstance, L"ACCEL"));
  DragAcceptFiles(wnd.handle(), TRUE);

  ShowWindow(wnd.handle(), nCmdShow);
  UpdateWindow(wnd.handle());

  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0)) {
    if (!TranslateAccelerator(msg.hwnd, hAccelTbl, &msg)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }

  return int(msg.wParam);
}

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
#include "draw.h"
#include "font.h"
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
  using std::tr1::unordered_map;

  using namespace sociarium_project_common;
  using namespace sociarium_project_menu_and_message;
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
    int action = LOWORD(wp);

    switch (action) {
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

    default:
      switch (action) {
      case IDM_COMMUNITY_TRANSITION_DIAGRAM_SCOPE_WIDER:
      case IDM_COMMUNITY_TRANSITION_DIAGRAM_SCOPE_NARROWER:
        timer_->stop(ID_TIMER_DRAW);
        break;
      case IDM_LAYOUT_ZOOM_OUT:
      case IDM_LAYOUT_ZOOM_IN:
        timer_->stop(ID_TIMER_DRAW);
        break;
      }

      world_->do_command(action);

      switch (action) {
      case IDM_COMMUNITY_TRANSITION_DIAGRAM_SCOPE_WIDER:
      case IDM_COMMUNITY_TRANSITION_DIAGRAM_SCOPE_NARROWER:
        timer_->start(ID_TIMER_DRAW);
        break;
      case IDM_TIMELINE_STOP:
        timer_->stop(ID_TIMER_AUTO_RUN);
        break;
      case IDM_TIMELINE_FORWARD_1:
      case IDM_TIMELINE_FORWARD_2:
      case IDM_TIMELINE_FORWARD_3:
      case IDM_TIMELINE_FORWARD_4:
      case IDM_TIMELINE_BACKWARD_1:
      case IDM_TIMELINE_BACKWARD_2:
      case IDM_TIMELINE_BACKWARD_3:
      case IDM_TIMELINE_BACKWARD_4:
        timer_->reset(ID_TIMER_AUTO_RUN, get_latency());
        timer_->start(ID_TIMER_AUTO_RUN);
        break;
      case IDM_LAYOUT_ZOOM_OUT:
      case IDM_LAYOUT_ZOOM_IN:
        timer_->start(ID_TIMER_DRAW);
        break;
      }
      break;
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

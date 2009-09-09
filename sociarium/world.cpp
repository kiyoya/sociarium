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

#include "mouse_and_selection.h"
#include "view.h"
#include "world.h"
#include "common.h"
#include "sociarium_graph_time_series.h"
#include "thread/layout.h"
#include "../shared/GL/glview.h"

namespace hashimoto_ut {

  using std::vector;
  using std::string;
  using std::wstring;
  using std::tr1::shared_ptr;

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // 視点の制御
  void World::resize_window(Vector2<int> const& size) {

#ifdef _MSC_VER
    HWND hwnd = sociarium_project_common::get_window_handle();
    HDC dc = GetDC(hwnd);
    if (dc==NULL)
      sociarium_project_common::show_last_error(L"World::resize_window/GetDC");
    if (wglMakeCurrent(dc, rc_)==FALSE)
      sociarium_project_common::show_last_error(L"World::resize_window/wglMakeCurrent(dc)");
#endif

    view_->set_viewport(Vector2<int>(0, 0), size);
    view_->initialize_matrix();

#ifdef _MSC_VER
    if (wglMakeCurrent(0, 0)==FALSE)
      sociarium_project_common::show_last_error(L"World::resize_window/wglMakeCurrent(0)");
    if (ReleaseDC(hwnd, dc)==0)
      sociarium_project_common::show_last_error(L"World::resize_window/ReleaseDC");
#endif
  }

  void World::initialize_view(void) {

#ifdef _MSC_VER
    HWND hwnd = sociarium_project_common::get_window_handle();
    HDC dc = GetDC(hwnd);
    if (dc==NULL) sociarium_project_common::show_last_error(L"World::initialize_view/GetDC");
    if (wglMakeCurrent(dc, rc_)==FALSE)
      sociarium_project_common::show_last_error(L"World::initialize_view/wglMakeCurrent(dc)");
#endif

    center_.set(0.0, 0.0);
    view_->set_angle(-DEGREEV_MAX, DEGREEV_MAX);
    view_->set_distance(sociarium_project_view::VIEW_DISTANCE);
    view_->initialize_matrix();

#ifdef _MSC_VER
    if (wglMakeCurrent(0, 0)==FALSE)
      sociarium_project_common::show_last_error(L"World::initialize_view/wglMakeCurrent(0)");
    if (ReleaseDC(hwnd, dc)==0)
      sociarium_project_common::show_last_error(L"World::initialize_view/ReleaseDC");
#endif
  }


  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // 拡大／縮小
  void World::zoom(double mag) {

#ifdef _MSC_VER
    HWND hwnd = 0;
    HDC dc = wglGetCurrentDC();
    if (dc==0) {
      hwnd = sociarium_project_common::get_window_handle();
      if (hwnd==0)
        sociarium_project_common::show_last_error(L"World::zoom/get_window_handle");
      dc = GetDC(hwnd);
      if (dc==0)
        sociarium_project_common::show_last_error(L"World::zoom/GetDC");
      if (wglMakeCurrent(dc, rc_)==FALSE)
        sociarium_project_common::show_last_error(L"world_zoom/wglMakeCurrent(dc)");
    }

    assert(dc!=0);
#endif

    double distance = view_->distance()*mag;
    distance = distance<sociarium_project_view::VIEW_DISTANCE_MIN?sociarium_project_view::VIEW_DISTANCE_MIN:distance;
    distance = distance>sociarium_project_view::VIEW_DISTANCE_MAX?sociarium_project_view::VIEW_DISTANCE_MAX:distance;
    view_->set_distance(distance);
    view_->initialize_matrix();

#ifdef _MSC_VER
    if (hwnd!=0) {
      if (wglMakeCurrent(0, 0)==FALSE)
        sociarium_project_common::show_last_error(L"World::zoom/wglMakeCurrent(0)");
      if (ReleaseDC(hwnd, dc)==0)
        sociarium_project_common::show_last_error(L"World::zoom/ReleaseDC");
    }
#endif
  }


  ////////////////////////////////////////////////////////////////////////////////////////////////////
  void World::clear_community(void) const {
    time_series_->clear_community();
  }
  

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // レイヤーの移動
  void World::move_layer(int index) {
//     Thread* th = sociarium_project_thread::get_current_graph_layout_thread();
//     if (th!=0) th->cancel();

    time_series_->move_layer(index);
    sociarium_project_mouse_and_selection::initialize_selection();

//     if (index!=time_series_->index_of_current_layer()) return; // 時刻両端の場合
//     if (sociarium_project_thread::get_current_graph_layout_thread()==0) {
//       graph_layout_thread.reset(new LayoutThread(time_series_, false));
//       sociarium_project_thread::set_current_graph_layout_thread(graph_layout_thread.get());
//       boost::thread(boost::ref(*graph_layout_thread));
//     }
  }

  int World::index_of_current_layer(void) const {
    return int(time_series_->index_of_current_layer());
  }

} // The end of the namespace "hashimoto_ut"

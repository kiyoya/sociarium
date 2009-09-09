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

#include <fstream>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include "common.h"
#include "message.h"
#include "world.h"
#include "thread.h"
#include "view.h"
#include "draw.h"
#include "sociarium_graph_time_series.h"
#include "thread/creation.h"
#include "thread/layout.h"
#include "thread/community_detection.h"
#include "thread/node_size.h"
#include "../shared/msgbox.h"
#include "../shared/win32api.h"

namespace hashimoto_ut {

  using std::vector;
  using std::string;
  using std::wstring;
  using std::ifstream;
  using std::getline;
  using std::tr1::shared_ptr;

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // グラフ時系列の作成
  void World::read_file(wchar_t const* filename) const {

    {
      string const filename_mb = wcs2mbcs(filename, wcslen(filename));
      ifstream ifs(filename_mb.c_str());
      string line;
      getline(ifs, line);
      if (!line.empty() && (line.find("#parameter")==0)) {
        while (getline(ifs, line)) {
          if (line.find("#edge_width=")==0) {
            string const s = line.substr(string("#edge_width=").size());
            try {
              float const v = boost::lexical_cast<float>(s);
              if (v>0.0f) sociarium_project_draw::set_edge_width_scale(v);
            } catch (...) {}
          } else if (line.find("#node_size=")==0) {
            string const s = line.substr(string("#node_size=").size());
            try {
              float const v = boost::lexical_cast<float>(s);
              if (v>0.0f) sociarium_project_draw::set_default_node_size(v);
            } catch (...) {}
          }
        }
        return;
      }
    }

    if (sociarium_project_thread::get_current_graph_creation_thread()) {
      MsgBox(sociarium_project_common::get_window_handle(), sociarium_project_common::APPLICATION_TITLE,
             L"%s", sociarium_project_message::GRAPH_TIME_SERIES_IS_LOCKED);
      return;
    } else if (sociarium_project_thread::get_current_graph_layout_thread() ||
               sociarium_project_thread::get_current_graph_statistics_thread() ||
               sociarium_project_thread::get_current_community_detection_thread() ||
               sociarium_project_thread::get_current_node_size_thread()) {
      MsgBox(sociarium_project_common::get_window_handle(), sociarium_project_common::APPLICATION_TITLE,
             L"%s", sociarium_project_message::ANOTHER_THREAD_IS_RUNNING);
      return;
    }

    // CreationThreadでwglShareListsを呼ぶまでのタイミング調整
    boost::condition c;
    boost::mutex m;
    bool state = true;

    shared_ptr<Thread> th(new CreationThread(m, c, state, rc_, time_series_, filename));
    sociarium_project_thread::set_current_graph_creation_thread(th);
    boost::thread(boost::ref(*th));

    boost::mutex::scoped_lock lock(m);
    if (state==true) c.wait(lock);
  }


  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // グラフのレイアウト
  void World::layout_graph(void) const {

    if (sociarium_project_thread::get_current_graph_creation_thread()) {
      MsgBox(sociarium_project_common::get_window_handle(), sociarium_project_common::APPLICATION_TITLE,
             L"%s", sociarium_project_message::GRAPH_TIME_SERIES_IS_LOCKED);
      return;
    } else if (sociarium_project_thread::get_current_graph_layout_thread()) {
      MsgBox(sociarium_project_common::get_window_handle(), sociarium_project_common::APPLICATION_TITLE,
             L"%s", sociarium_project_message::ANOTHER_THREAD_IS_RUNNING);
      return;
    }

    shared_ptr<Thread> th(new LayoutThread(time_series_));
    sociarium_project_thread::set_current_graph_layout_thread(th);
    boost::thread(boost::ref(*th));
  }


  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // コミュニティ検出
  void World::detect_community(void) const {

    if (sociarium_project_thread::get_current_graph_creation_thread()) {
      MsgBox(sociarium_project_common::get_window_handle(), sociarium_project_common::APPLICATION_TITLE,
             L"%s", sociarium_project_message::GRAPH_TIME_SERIES_IS_LOCKED);
      return;
    } else if (sociarium_project_thread::get_current_community_detection_thread()) {
      MsgBox(sociarium_project_common::get_window_handle(), sociarium_project_common::APPLICATION_TITLE,
             L"%s", sociarium_project_message::ANOTHER_THREAD_IS_RUNNING);
      return;
    }

    shared_ptr<Thread> th = CommunityDetectionThread::create(time_series_);
    sociarium_project_thread::set_current_community_detection_thread(th);
    boost::thread(boost::ref(*th));
  }


  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // ノードサイズの変更
  void World::update_node_size(int size_factor) const {

    if (size_factor==sociarium_project_view::NodeView::SizeFactor::UNIFORM ||
        size_factor==sociarium_project_view::NodeView::SizeFactor::NONE ||
        size_factor==sociarium_project_view::NodeView::SizeFactor::DEGREE_CENTRALITY) {
      sociarium_project_view::set_node_size_factor(size_factor);
      time_series_->read_lock();
      time_series_->update_node_size(size_factor);
      time_series_->read_unlock();
      return;
    }

    if (sociarium_project_thread::get_current_graph_creation_thread()) {
      MsgBox(sociarium_project_common::get_window_handle(), sociarium_project_common::APPLICATION_TITLE,
             L"%s", sociarium_project_message::GRAPH_TIME_SERIES_IS_LOCKED);
      return;
    } else if (sociarium_project_thread::get_current_node_size_thread()) {
      MsgBox(sociarium_project_common::get_window_handle(), sociarium_project_common::APPLICATION_TITLE,
             L"%s", sociarium_project_message::ANOTHER_THREAD_IS_RUNNING);
      return;
    }

    sociarium_project_view::set_node_size_factor(size_factor);
    shared_ptr<Thread> th(new NodeSizeThread(time_series_));
    sociarium_project_thread::set_current_node_size_thread(th);
    boost::thread(boost::ref(*th));
  }

  //
  //   ////////////////////////////////////////////////////////////////////////////////////////////////////
  //   // グラフの統計量
  //   void World::calc_graph_statistics(int statistics) {
  //     if (sociarium_project_thread::get_current_graph_creation_thread()) {
  //       MsgBox(sociarium_project_common::get_window_handle(), sociarium_project_common::APPLICATION_TITLE, L"%s",
  //              sociarium_project_message::GRAPH_TIME_SERIES_IS_LOCKED);
  //       return;
  //     }
  //
  //     graph_statistics_thread.reset(new GraphStatisticsThread(time_series_, statistics));
  //     sociarium_project_thread::set_current_graph_statistics_thread(graph_statistics_thread.get());
  //     boost::thread(boost::ref(*graph_statistics_thread_thread));
  //   }


} // The end of the namespace "hashimoto_ut"

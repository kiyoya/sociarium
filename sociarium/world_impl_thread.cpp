// s.o.c.i.a.r.i.u.m: world_impl_thread.cpp
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

#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#endif
#include "world_impl.h"
#include "common.h"
#include "language.h"
#include "thread.h"
#include "sociarium_graph_time_series.h"
#include "thread/graph_creation.h"
#include "thread/graph_retouch.h"
#include "thread/layout.h"
#include "thread/community_detection.h"
#include "thread/node_size_update.h"
#include "thread/edge_width_update.h"
#include "../shared/msgbox.h"

namespace hashimoto_ut {

  using std::pair;
  using std::tr1::shared_ptr;

  using namespace sociarium_project_thread;
  using namespace sociarium_project_common;
  using namespace sociarium_project_language;

  ////////////////////////////////////////////////////////////////////////////////
  void WorldImpl::create_graph(wchar_t const* filename) const {

    if (joinable(GRAPH_CREATION)
        || joinable(GRAPH_RETOUCH)) {
      /* During the graph creation thread is running,
       * other threads can't be invoked.
       */
      message_box(
#ifdef _MSC_VER
        get_window_handle(),
        MB_OK|MB_ICONEXCLAMATION|MB_SYSTEMMODAL,
#endif
        APPLICATION_TITLE,
        L"%s",
        get_message(Message::GRAPH_IS_LOCKED));
      return;
    }

    else if (joinable(LAYOUT)
             || joinable(COMMUNITY_DETECTION)
             || joinable(NODE_SIZE_UPDATE)
             || joinable(EDGE_WIDTH_UPDATE)) {
      /* During other threads are running,
       * the graph creation thread can't be invoked.
       */
      message_box(
#ifdef _MSC_VER
        get_window_handle(),
        MB_OK|MB_ICONEXCLAMATION|MB_SYSTEMMODAL,
#endif
        APPLICATION_TITLE,
        L"%s",
        get_message(Message::ANOTHER_THREAD_IS_RUNNING));
      return;
    }

    invoke(GRAPH_CREATION, GraphCreationThread::create(filename));
  }


  ////////////////////////////////////////////////////////////////////////////////
  void WorldImpl::retouch_graph(wchar_t const* filename) const {

    if (joinable(GRAPH_CREATION)
        || joinable(GRAPH_RETOUCH)) {
      /* During the graph creation thread is running,
       * other threads can't be invoked.
       */
      message_box(
#ifdef _MSC_VER
        get_window_handle(),
        MB_OK|MB_ICONEXCLAMATION|MB_SYSTEMMODAL,
#endif
        APPLICATION_TITLE,
        L"%s",
        get_message(Message::GRAPH_IS_LOCKED));
      return;
    }

    else if (joinable(LAYOUT)
             || joinable(COMMUNITY_DETECTION)
             || joinable(NODE_SIZE_UPDATE)
             || joinable(EDGE_WIDTH_UPDATE)) {
      /* During other threads are running,
       * the graph creation thread can't be invoked.
       */
      message_box(
#ifdef _MSC_VER
        get_window_handle(),
        MB_OK|MB_ICONEXCLAMATION|MB_SYSTEMMODAL,
#endif
        APPLICATION_TITLE,
        L"%s",
        get_message(Message::ANOTHER_THREAD_IS_RUNNING));
      return;
    }

    invoke(GRAPH_RETOUCH, GraphRetouchThread::create(filename));
  }


  ////////////////////////////////////////////////////////////////////////////////
  void WorldImpl::layout(void) const {

    if (joinable(GRAPH_CREATION)
        || joinable(GRAPH_RETOUCH)) {
      /* During the graph creation thread is running,
       * other threads can't be invoked.
       */
      message_box(
#ifdef _MSC_VER
        get_window_handle(),
        MB_OK|MB_ICONEXCLAMATION|MB_SYSTEMMODAL,
#endif
        APPLICATION_TITLE,
        L"%s",
        get_message(Message::GRAPH_IS_LOCKED));
      return;
    } else if (joinable(LAYOUT)) {
      /* Multiple execution of the same kind of thread is prohibited.
       */
      message_box(
#ifdef _MSC_VER
        get_window_handle(),
        MB_OK|MB_ICONEXCLAMATION|MB_SYSTEMMODAL,
#endif
        APPLICATION_TITLE,
        L"%s",
        get_message(Message::ANOTHER_THREAD_IS_RUNNING));
      return;
    }

    invoke(LAYOUT, LayoutThread::create());
  }


  ////////////////////////////////////////////////////////////////////////////////
  void WorldImpl::detect_community(void) const {

    if (joinable(GRAPH_CREATION)
        || joinable(GRAPH_RETOUCH)) {
      /* During the graph creation thread is running,
       * other threads can't be invoked.
       */
      message_box(
#ifdef _MSC_VER
        get_window_handle(),
        MB_OK|MB_ICONEXCLAMATION|MB_SYSTEMMODAL,
#endif
        APPLICATION_TITLE,
        L"%s",
        get_message(Message::GRAPH_IS_LOCKED));
      return;
    } else if (joinable(COMMUNITY_DETECTION)) {
      /* Multiple execution of the same kind of thread is prohibited.
       */
      message_box(
#ifdef _MSC_VER
        get_window_handle(),
        MB_OK|MB_ICONEXCLAMATION|MB_SYSTEMMODAL,
#endif
        APPLICATION_TITLE,
        L"%s",
        get_message(Message::ANOTHER_THREAD_IS_RUNNING));
      return;
    }

    invoke(COMMUNITY_DETECTION, CommunityDetectionThread::create());
  }


  ////////////////////////////////////////////////////////////////////////////////
  void WorldImpl::update_node_size(void) const {

    if (joinable(GRAPH_CREATION)
        || joinable(GRAPH_RETOUCH)) {
      /* During the graph creation thread is running,
       * other threads can't be invoked.
       */
      message_box(
#ifdef _MSC_VER
        get_window_handle(),
        MB_OK|MB_ICONEXCLAMATION|MB_SYSTEMMODAL,
#endif
        APPLICATION_TITLE,
        L"%s",
        get_message(Message::GRAPH_IS_LOCKED));
      return;
    } else if (joinable(NODE_SIZE_UPDATE)) {
      /* Multiple execution of the same kind of thread is prohibited.
       */
      message_box(
#ifdef _MSC_VER
        get_window_handle(),
        MB_OK|MB_ICONEXCLAMATION|MB_SYSTEMMODAL,
#endif
        APPLICATION_TITLE,
        L"%s",
        get_message(Message::ANOTHER_THREAD_IS_RUNNING));
      return;
    }

    invoke(NODE_SIZE_UPDATE, NodeSizeUpdateThread::create());
  }


  ////////////////////////////////////////////////////////////////////////////////
  void WorldImpl::update_edge_width(void) const {

    if (joinable(GRAPH_CREATION)
        || joinable(GRAPH_RETOUCH)) {
      /* During the graph creation thread is running,
       * other threads can't be invoked.
       */
      message_box(
#ifdef _MSC_VER
        get_window_handle(),
        MB_OK|MB_ICONEXCLAMATION|MB_SYSTEMMODAL,
#endif
        APPLICATION_TITLE,
        L"%s",
        get_message(Message::GRAPH_IS_LOCKED));
      return;
    } else if (joinable(EDGE_WIDTH_UPDATE)) {
      /* Multiple execution of the same kind of thread is prohibited.
       */
      message_box(
#ifdef _MSC_VER
        get_window_handle(),
        MB_OK|MB_ICONEXCLAMATION|MB_SYSTEMMODAL,
#endif
        APPLICATION_TITLE,
        L"%s",
        get_message(Message::ANOTHER_THREAD_IS_RUNNING));
      return;
    }

    invoke(EDGE_WIDTH_UPDATE, EdgeWidthUpdateThread::create());
  }

} // The end of the namespace "hashimoto_ut"

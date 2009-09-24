// s.o.c.i.a.r.i.u.m: world_impl_others.cpp
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
#include "world_impl.h"
#include "common.h"
#include "language.h"
#include "selection.h"
#include "thread.h"
#include "sociarium_graph_time_series.h"
#include "../shared/msgbox.h"

namespace hashimoto_ut {

  using std::tr1::shared_ptr;

  using namespace sociarium_project_common;
  using namespace sociarium_project_thread;
  using namespace sociarium_project_language;

  ////////////////////////////////////////////////////////////////////////////////
  void WorldImpl::forward_layer(Vector2<int> const& mpos) {
    shared_ptr<SociariumGraphTimeSeries> ts
      = sociarium_project_graph_time_series::get();

    int const layer_prev = ts->index_of_current_layer();
    ts->move_layer(layer_prev+1);

    if (ts->index_of_current_layer()!=layer_prev)
      select(mpos);
  }

  void WorldImpl::backward_layer(Vector2<int> const& mpos) {
    shared_ptr<SociariumGraphTimeSeries> ts
      = sociarium_project_graph_time_series::get();

    int const layer_prev = ts->index_of_current_layer();
    ts->move_layer(layer_prev-1);

    if (ts->index_of_current_layer()!=layer_prev)
      select(mpos);
  }


  ////////////////////////////////////////////////////////////////////////////////
  void WorldImpl::clear_community(void) const {

    bool another_thread_is_running = false;

    for (int i=0; i<NUMBER_OF_THREAD_CATEGORIES; ++i) {
      if (i!=FORCE_DIRECTION && joinable(i)) {
        another_thread_is_running = true;
        break;
      }
    }

    if (another_thread_is_running) {
      message_box(get_window_handle(), mb_notice, APPLICATION_TITLE,
                  L"%s", get_message(Message::ANOTHER_THREAD_IS_RUNNING));
      return;
    }

    if (message_box(get_window_handle(), mb_ok_cancel, APPLICATION_TITLE,
                    L"%s", get_message(Message::CLEAR_COMMUNITY))==IDCANCEL)
      return;

    shared_ptr<SociariumGraphTimeSeries> ts
      = sociarium_project_graph_time_series::get();

    ts->clear_community();
  }

} // The end of the namespace "hashimoto_ut"

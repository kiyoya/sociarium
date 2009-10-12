// s.o.c.i.a.r.i.u.m: world_impl_view.cpp
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

#include "view.h"
#include "world_impl.h"
#include "../shared/GL/glview.h"

namespace hashimoto_ut {

  ////////////////////////////////////////////////////////////////////////////////
  void WorldImpl::resize_window(Vector2<int> const& size) {
    view_->set_viewport(Vector2<int>(0, 0), size);
    view_->initialize_matrix();
  }


  ////////////////////////////////////////////////////////////////////////////////
  void WorldImpl::initialize_view(void) {
    center_.set(0.0, 0.0);
    view_->set_angle(-DEGREEV_MAX, DEGREEV_MAX);
    view_->set_distance(sociarium_project_view::VIEW_DISTANCE);
    view_->initialize_matrix();
  }


  ////////////////////////////////////////////////////////////////////////////////
  void WorldImpl::zoom(double mag) {
    double distance = view_->distance()*mag;

    if (distance<sociarium_project_view::VIEW_DISTANCE_MIN)
      distance = sociarium_project_view::VIEW_DISTANCE_MIN;
    else if (distance>sociarium_project_view::VIEW_DISTANCE_MAX)
      distance = sociarium_project_view::VIEW_DISTANCE_MAX;

    view_->set_distance(distance);
    view_->initialize_matrix();
  }

} // The end of the namespace "hashimoto_ut"

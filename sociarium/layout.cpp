// s.o.c.i.a.r.i.u.m: layout.cpp
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
#include <memory>
#include "layout.h"
#include "sociarium_graph.h"

namespace hashimoto_ut {

  using std::vector;
  using std::tr1::shared_ptr;

  namespace sociarium_project_layout {

    namespace {

      ////////////////////////////////////////////////////////////////////////////////
      float const GRID_INTERVAL_MIN = 5.0f;
      float grid_interval = GRID_INTERVAL_MIN;

      float const LAYOUT_FRAME_SIZE_MIN = GRID_INTERVAL_MIN;
      float layout_frame_default_size = 10*LAYOUT_FRAME_SIZE_MIN;
      float layout_frame_size = layout_frame_default_size;
      float layout_frame_previous_size = layout_frame_default_size;

      Vector2<float> layout_frame_position(0.0f, 0.0f);
      Vector2<float> layout_frame_previous_position(layout_frame_position);

      // True if the layout frame is moved or resized.
      bool update_layout_frame = false; 

    } // The end of the anonymous namespace


    ////////////////////////////////////////////////////////////////////////////////
    float get_layout_frame_default_size(void) {
      return layout_frame_default_size;
    }

    void set_layout_frame_default_size(float size) {
      int const num = int(size/grid_interval);
      size = num*grid_interval;
      layout_frame_default_size = size<grid_interval?grid_interval:size;
    }


    ////////////////////////////////////////////////////////////////////////////////
    float get_layout_frame_size(void) {
      return layout_frame_size;
    }

    void set_layout_frame_size(float size) {
      int const num = int(size/grid_interval);
      size = num*grid_interval;
      layout_frame_size = size<grid_interval?grid_interval:size;
    }


    ////////////////////////////////////////////////////////////////////////////////
    float get_layout_frame_previous_size(void) {
      return layout_frame_previous_size;
    }

    void set_layout_frame_previous_size(float size) {
      int const num = int(size/grid_interval);
      size = num*grid_interval;
      layout_frame_previous_size = size;
    }


    ////////////////////////////////////////////////////////////////////////////////
    Vector2<float> const& get_layout_frame_position(void) {
      return layout_frame_position;
    }

    void set_layout_frame_position(Vector2<float> const& position) {
      Vector2<int> const num = position/grid_interval;
      layout_frame_position.set(grid_interval*num.fcast());
    }


    ////////////////////////////////////////////////////////////////////////////////
    Vector2<float> const& get_layout_frame_previous_position(void) {
      return layout_frame_previous_position;
    }

    void set_layout_frame_previous_position(Vector2<float> const& position) {
      layout_frame_previous_position.set(position);
    }


    ////////////////////////////////////////////////////////////////////////////////
    bool get_update_layout_frame(void) {
      return update_layout_frame;
    }

    void set_update_layout_frame(bool b) {
      update_layout_frame = b;
    }


    ////////////////////////////////////////////////////////////////////////////////
    float get_grid_interval(void) {
      return grid_interval;
    }

    void set_grid_interval(float interval) {
      if (grid_interval<GRID_INTERVAL_MIN)
        grid_interval = GRID_INTERVAL_MIN;
      else
        grid_interval = interval;
    }


    ////////////////////////////////////////////////////////////////////////////////
    void reset_position(DynamicNodeProperty& dnp) {
      Vector2<float> pos;
      int number_of_visible_members = 0;

      vector<DynamicNodeProperty*>::const_iterator i   = dnp.lower_nbegin();
      vector<DynamicNodeProperty*>::const_iterator end = dnp.lower_nend();

      for (; i!=end; ++i) {
        if (is_visible(**i)) {
          pos += (*i)->get_static_property()->get_position();
          ++number_of_visible_members;
        }
      }

      if (number_of_visible_members>0) {
        pos /= float(number_of_visible_members);
        dnp.get_static_property()->set_position(pos);
      }
    }

  } // The end of the namespace "sociarium_project_layout"

} // The end of the namespace "hashimoto_ut"

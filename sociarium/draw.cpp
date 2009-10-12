// s.o.c.i.a.r.i.u.m: draw.cpp
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
#include "draw.h"

namespace hashimoto_ut {

  namespace sociarium_project_draw {

    namespace {

      Vector2<float> const slider_offset(10.0f, 40.0f); // pixel (viewport coordinates)
      Vector2<float> const selection_frame_offset(10.0f, 10.0f); // pixel (viewport coordinates)

      float default_node_size = 2.0f;
      float default_edge_width = 0.05f;
      float node_size = default_node_size;
      float edge_width = default_edge_width;
      float coordinates_size = 3.0f;

    } // The end of the anonymous namespace


    ////////////////////////////////////////////////////////////////////////////////
    Vector2<float> const& get_slider_offset(void) {
      return slider_offset;
    }

    Vector2<float> const& get_frame_offset(void) {
      return selection_frame_offset;
    }


    ////////////////////////////////////////////////////////////////////////////////
    float get_default_node_size(void) {
      return default_node_size;
    }

    void set_default_node_size(float size) {
      default_node_size = size;
    }

    float get_default_edge_width(void) {
      return default_edge_width;
    }

    void set_default_edge_width(float width) {
      default_edge_width = width;
    }


    ////////////////////////////////////////////////////////////////////////////////
    float get_node_size(void) {
      return node_size;
    }

    void set_node_size(float size) {
      node_size = size;
    }

    float get_edge_width(void) {
      return edge_width;
    }

    void set_edge_width(float width) {
      edge_width = width;
    }


    ////////////////////////////////////////////////////////////////////////////////
    float get_coordinates_size(void) {
      return coordinates_size;
    }

    void set_coordinates_size(float size) {
      coordinates_size = size;
    }

  } // The end of the namespace "sociarium_project_draw"

} // The end of the namespace "hashimoto_ut"

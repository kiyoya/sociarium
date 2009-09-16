// s.o.c.i.a.r.i.u.m: color.h
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

#ifndef INCLUDE_GUARD_SOCIARIUM_PROJECT_COLOR_H
#define INCLUDE_GUARD_SOCIARIUM_PROJECT_COLOR_H

#ifdef _MSC_VER
#include <array>
#else
#include <tr1/array>
#endif

namespace hashimoto_ut {

  namespace sociarium_project_color {

    ////////////////////////////////////////////////////////////////////////////////
    namespace ColorCategory {
      enum _ {
        BACKGROUND = 0,
        BACKGROUND2,
        LAYOUT_FRAME_BORDER,
        LAYOUT_FRAME_AREA,
        GRID,
        TIME_SLIDER_UPPER_ON,
        TIME_SLIDER_UPPER_OFF,
        TIME_SLIDER_LOWER_ON,
        TIME_SLIDER_LOWER_OFF,
        MOUSE_DRAG_REGION,
        SELECTION_FRAME,
        SELECTION_NAME,
        FPS,
        THREAD_MESSAGE,
        THREAD_MESSAGE_FRAME,
        LAYER_NAME,
        LAYER_NAME_FRAME,
        THREAD,
        NUMBER_OF_CATEGORIES
      };
    }

    ////////////////////////////////////////////////////////////////////////////////
    std::tr1::array<float, 4> const& get_color(int color_id);

    ////////////////////////////////////////////////////////////////////////////////
    int get_default_node_color_id(void);
    int get_default_edge_color_id(void);
    int get_default_community_color_id(void);
    int get_default_community_edge_color_id(void);

    ////////////////////////////////////////////////////////////////////////////////
    int get_default_node_name_color_id(void);
    int get_default_edge_name_color_id(void);
    int get_default_community_name_color_id(void);
    int get_default_community_edge_name_color_id(void);

    ////////////////////////////////////////////////////////////////////////////////
    int get_independent_node_color_id(void);
    int get_independent_edge_color_id(void);

    ////////////////////////////////////////////////////////////////////////////////
    void highlight_effect(std::tr1::array<float, 4>& rgba);
    void selection_effect(std::tr1::array<float, 4>& rgba);
    void marking_effect(std::tr1::array<float, 4>& rgba);

  } // The end of the namespace "sociarium_project_color"

} // The end of the namespace "hashimoto_ut"

#endif // INCLUDE_GUARD_SOCIARIUM_PROJECT_COLOR_H

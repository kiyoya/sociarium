// s.o.c.i.a.r.i.u.m
// HASHIMOTO, Yasuhiro (E-mail: hy @ sys.t.u-tokyo.ac.jp)
// update: 2009/04/23

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

#ifndef INCLUDE_GUARD_SOCIARIUM_PROJECT_DRAW_H
#define INCLUDE_GUARD_SOCIARIUM_PROJECT_DRAW_H

#include <vector>
#include "../shared/vector2.h"

namespace hashimoto_ut {

  class GLTexture;

  namespace sociarium_project_draw {

    void draw_node_with_polygon_circle(Vector2<float> const& pos, float size, float z, float const* rgba);
    void draw_node_with_polygon_curve(Vector2<float> const& center, const std::vector<Vector2<float> const*>& pos, float z, float const* rgba);
    void draw_node_with_texture(Vector2<float> const& pos, float size, float z, float const* rgba, float angle, GLTexture const* texture);
    void draw_edge(Vector2<float> const& pos0, float size0, Vector2<float> const& pos1, float size1, float width, float const* rgba);
    void draw_edge(Vector2<float> const& pos0, float size0, Vector2<float> const& pos1, float size1, float width, float dev, float const* rgba);
    void draw_loop_edge(Vector2<float> const& pos, float width, float z, float const* rgba);
    void draw_edge_with_texture(Vector2<float> const& pos0, float size0, Vector2<float> const& pos1, float size1, float width, float const* rgba, GLTexture const* texture);
    void draw_directed_edge_with_texture(Vector2<float> const& pos0, float size0, Vector2<float> const& pos1, float size1, float width, float const* rgba, GLTexture const* texture);
    void draw_node_label(wchar_t const* label, Vector2<float> const& pos, float size, float z, float const* rgba, float angle);
    void draw_edge_label(wchar_t const* label, Vector2<float> const& pos0, float size0, Vector2<float> const& pos1, float size1, float size, float const* rgba, float angle);

    // その他の描画パラメータ

    Vector2<double> const& get_slider_offset(void);
    Vector2<double> const& get_selection_frame_offset(void);

    int get_grid_resolution(void);
    void set_grid_resolution(int resolution);

    int get_layout_frame_size(void);
    void set_layout_frame_size(int size);

    int get_layout_frame_previous_size(void);
    void set_layout_frame_previous_size(int size);

    Vector2<int> const& get_layout_frame_position(void);
    void set_layout_frame_position(Vector2<int> const& position);

    Vector2<int> const& get_layout_frame_previous_position(void);
    void set_layout_frame_previous_position(Vector2<int> const& position);

    bool get_update_layout_frame(void);
    void set_update_layout_frame(bool b);

    int get_layout_frame_default_size(void);
    void set_layout_frame_default_size(int size);

    float get_default_node_size(void);
    void set_default_node_size(float size);

    float get_edge_width_scale(void);
    void set_edge_width_scale(float scale);

  } // The end of the namespace "sociarium_project_draw"

} // The end of the namespace "hashimoto_ut"

#endif // INCLUDE_GUARD_SOCIARIUM_PROJECT_DRAW_H

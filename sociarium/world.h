// s.o.c.i.a.r.i.u.m
// HASHIMOTO, Yasuhiro (E-mail: hy @ sys.t.u-tokyo.ac.jp)
// update: 2009/03/31

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

#ifndef INCLUDE_GUARD_SOCIARIUM_PROJECT_WORLD_H
#define INCLUDE_GUARD_SOCIARIUM_PROJECT_WORLD_H

#include <vector>
#include <map>
#include <string>
#ifdef _MSC_VER
#include <memory>
#include <windows.h>
#else
#include <tr1/memory>
#endif
#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#include "../shared/vector2.h"

namespace hashimoto_ut {

  class GLView;
  class CreationThread;
  class LayoutThread;
  class CommunityDetectionThread;
  class SociariumGraphTimeSeries;

  ///////////////////////////////////////////////////////////////////////////////////////////////////
  class World {
  public:
    // world_initialize.cpp
#ifdef __APPLE__
    World(void * context);
#else
    World(void);
#endif
    ~World();

    // world_draw.cpp
    void draw(void) const;
    void draw_perspective_part(void) const;
    void draw_orthogonal_part(void) const;

    // world_select.cpp
    void select(Vector2<int> const& mpos) const;

    // world_do_mouse_action.cpp
    void do_mouse_action(int action, Vector2<int> const& mpos, int modifier);

    // world.cpp
    void resize_window(Vector2<int> const& size);
    void initialize_view(void);
    void zoom(double mag);
    void clear_community(void) const;

    void move_layer(int index);
    int index_of_current_layer(void) const;

    // world_run_thread.cpp
    void read_file(wchar_t const* filename) const;
    void layout_graph(void) const;
    void detect_community(void) const;
    void update_node_size(int size_factor) const;

    // world_do_force_direction.cpp
    void do_force_direction(void) const;

    // world_edit_graph.cpp
    void mark_elements(int menu) const;
    void inverse_mark(int menu) const;
    void hide_marked_element(int menu) const;
    void show_elements(int menu) const;
    void remove_marked_elements(int menu) const;

  private:
#ifdef _MSC_VER
    HGLRC rc_;
#else
    void * rc_;
#endif
    
    // 視点
    Vector2<float> center_;
    std::tr1::shared_ptr<GLView> view_;

    // グラフ時系列
    std::tr1::shared_ptr<SociariumGraphTimeSeries> time_series_;
  };

} // The end of the namespace "hashimoto_ut"

#endif // INCLUDE_GUARD_SOCIARIUM_PROJECT_WORLD_H

// s.o.c.i.a.r.i.u.m - world_impl.h
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

#ifndef INCLUDE_GUARD_SOCIARIUM_PROJECT_WORLD_IMPL_H
#define INCLUDE_GUARD_SOCIARIUM_PROJECT_WORLD_IMPL_H

#include <vector>
#include "world.h"

namespace hashimoto_ut {

  class GLView;

  class WorldImpl : public World {
  public:
    ////////////////////////////////////////////////////////////////////////////////
#ifdef __APPLE__
    WorldImpl(void* window, CGLContextObj context);
#elif _MSC_VER
    WorldImpl(HWND hwnd);
#else
#error Not implemented
#endif
    ~WorldImpl();

    ////////////////////////////////////////////////////////////////////////////////
#ifdef __APPLE__
    void* get_window_handle(void) const;
    CGLContextObj get_rendering_context(int thread_id) const;
#elif _MSC_VER
    HWND get_window_handle(void) const;
    HDC get_device_context(void) const;
    HGLRC get_rendering_context(int thread_id) const;
#else
#error Not implemented
#endif

    ////////////////////////////////////////////////////////////////////////////////
    void draw(void) const;
    void draw_perspective_part(void) const;
    void draw_orthogonal_part(void) const;

    ////////////////////////////////////////////////////////////////////////////////
    void select(Vector2<int> const& mpos) const;

    ////////////////////////////////////////////////////////////////////////////////
    void do_mouse_action(int action, Vector2<int> const& mpos, int modifier);

    ////////////////////////////////////////////////////////////////////////////////
    void resize_window(Vector2<int> const& size);
    void initialize_view(void);
    void zoom(double mag);

    ////////////////////////////////////////////////////////////////////////////////
    void clear_community(void) const;
    void forward_layer(Vector2<int> const& mpos);
    void backward_layer(Vector2<int> const& mpos);

    ////////////////////////////////////////////////////////////////////////////////
    void create_graph(wchar_t const* filename) const;
    void retouch_graph(wchar_t const* filename) const;
    void layout(void) const;
    void detect_community(void) const;
    void update_node_size(void) const;
    void update_edge_width(void) const;

    ////////////////////////////////////////////////////////////////////////////////
    void mark_elements(int menu) const;
    void inverse_mark(int menu) const;
    void hide_marked_element(int menu) const;
    void show_elements(int menu) const;
    void remove_marked_elements(int menu) const;

  private:
#ifdef __APPLE__
    void* hwnd_;
    std::vector<CGLContextObj> rc_;
#elif _MSC_VER
    HWND hwnd_;
    HDC dc_;
    std::vector<HGLRC> rc_;
#else
#error Not implemented
#endif
    Vector2<float> center_;
    std::tr1::shared_ptr<GLView> view_;
  };

} // The end of the namespace "hashimoto_ut"

#endif // INCLUDE_GUARD_SOCIARIUM_PROJECT_WORLD_IMPL_H

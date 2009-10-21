// s.o.c.i.a.r.i.u.m: world.h
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

#ifndef INCLUDE_GUARD_SOCIARIUM_PROJECT_WORLD_H
#define INCLUDE_GUARD_SOCIARIUM_PROJECT_WORLD_H

#ifdef _MSC_VER
#include <memory>
#include <windows.h>
#else
#include <tr1/memory>
#endif
#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#else
#include <gl/gl.h>
#endif
#include "../shared/vector2.h"

namespace hashimoto_ut {

  class ThreadManager;

  ////////////////////////////////////////////////////////////////////////////////
  class World {
  public:
    virtual ~World() {}

#ifdef __APPLE__
    virtual void * get_window_handle(void) const = 0;
    virtual CGLContextObj get_rendering_context(int thread_id) const = 0;
#elif _MSC_VER
    virtual HWND get_window_handle(void) const = 0;
    virtual HDC get_device_context(void) const = 0;
    virtual HGLRC get_rendering_context(int thread_id) const = 0;
#else
#error Not implemented
#endif

    virtual void draw(void) const = 0;
    virtual void select(Vector2<int> const& mpos) const = 0;

    virtual void do_command(int action) = 0;
    virtual void do_mouse_action(int action, Vector2<int> const& mpos, int modifier) = 0;

    virtual void resize_window(Vector2<int> const& size) = 0;
    virtual void initialize_view(void) = 0;
    virtual void zoom(double mag) = 0;

    virtual void clear_community(void) const = 0;
    virtual void forward_layer(Vector2<int> const& mpos) = 0;
    virtual void backward_layer(Vector2<int> const& mpos) = 0;

    virtual void create_graph(wchar_t const* filename) const = 0;
    virtual void layout(void) const = 0;
    virtual void detect_community(void) const = 0;
    virtual void update_node_size(void) const = 0;
    virtual void update_edge_width(void) const = 0;

    virtual void mark_elements(int menu) const = 0;
    virtual void inverse_mark(int menu) const = 0;
    virtual void hide_marked_element(int menu) const = 0;
    virtual void show_elements(int menu) const = 0;
    virtual void remove_marked_elements(int menu) const = 0;

#ifdef __APPLE__
    static World* create(void* window, CGLContextObj context);
    static void destroy(World* world);
#elif _MSC_VER
    static std::tr1::shared_ptr<World> create(HWND hwnd);
#else
#error Not implemented
#endif
  };

} // The end of the namespace "hashimoto_ut"

#endif // INCLUDE_GUARD_SOCIARIUM_PROJECT_WORLD_H

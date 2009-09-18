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

#include <memory>
#include <windows.h>
#include "../shared/vector2.h"

namespace hashimoto_ut {

  class ThreadManager;

	namespace RenderingContext {
		enum {
			DRAW = 0,
			LOAD_TEXTURES,
			NUMBER_OF_CATEGORIES
		};
	}

  ////////////////////////////////////////////////////////////////////////////////
  class World {
  public:
    virtual ~World() {}

    virtual void draw(void) const = 0;
    virtual void select(Vector2<int> const& mpos) const = 0;

    virtual void do_mouse_action(int action, Vector2<int> const& mpos, WPARAM wp) = 0;

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

    ////////////////////////////////////////////////////////////////////////////////
    // Absolute path of the module.
    std::wstring const& get_module_path(void);
    void set_module_path(std::wstring const& path);

    ////////////////////////////////////////////////////////////////////////////////
    // Application instance.
    HINSTANCE get_instance_handle(void);
    void set_instance_handle(HINSTANCE hinst);

    ////////////////////////////////////////////////////////////////////////////////
    // Handle of the main window.
    HWND get_window_handle(void);
    void set_window_handle(HWND hwnd);

    ////////////////////////////////////////////////////////////////////////////////
    // Handle of the device context.
    HDC get_device_context(void);
    void set_device_context(HDC dc);

    ////////////////////////////////////////////////////////////////////////////////
    // Handle of the rendering context.
    HGLRC get_rendering_context(int thread_id);
    void set_rendering_context(int thread_id, HGLRC rc);

    ////////////////////////////////////////////////////////////////////////////////
    void show_last_error(wchar_t const* text=L"");

    ////////////////////////////////////////////////////////////////////////////////
#ifdef DEBUG
    void dump_error_log(wchar_t const* fmt, ...);
#endif

    static std::tr1::shared_ptr<World> create(void);

  protected:
    World(void) {}
  };

} // The end of the namespace "hashimoto_ut"

#endif // INCLUDE_GUARD_SOCIARIUM_PROJECT_WORLD_H

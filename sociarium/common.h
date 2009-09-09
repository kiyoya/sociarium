// s.o.c.i.a.r.i.u.m: common.h
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

#ifndef INCLUDE_GUARD_SOCIARIUM_PROJECT_COMMON_H
#define INCLUDE_GUARD_SOCIARIUM_PROJECT_COMMON_H

#include <string>
#ifdef _MSC_VER
#include <windows.h>
#endif
#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <gl/gl.h>
#endif

namespace hashimoto_ut {

  namespace sociarium_project_common {

    namespace {
      wchar_t const* APPLICATION_TITLE = L"s.o.c.i.a.r.i.u.m";
    }

    namespace RenderingContext {
      enum {
        DRAW = 0,
        LOAD_TEXTURES,
        NUMBER_OF_CATEGORIES
      };
    }

    ////////////////////////////////////////////////////////////////////////////////
    // Absolute path of the module.
    std::wstring const& get_module_path(void);
    void set_module_path(std::wstring const& path);

    ////////////////////////////////////////////////////////////////////////////////
    // Application instance.
#ifdef _MSC_VER
    HINSTANCE get_instance_handle(void);
    void set_instance_handle(HINSTANCE hinst);
#else
    void * get_instance_handle(void);
    void set_instance_handle(void *);
#endif

    ////////////////////////////////////////////////////////////////////////////////
    // Handle of the main window.
#ifdef _MSC_VER
    HWND get_window_handle(void);
    void set_window_handle(HWND hwnd);
#else
    void * get_window_handle(void);
    void set_window_handle(void *);
#endif

    ////////////////////////////////////////////////////////////////////////////////
    // Handle of the device context.
#ifdef _MSC_VER
    HDC get_device_context(void);
    void set_device_context(HDC dc);
#else
    void * get_device_context(void);
    void set_device_context(void *);
#endif

    ////////////////////////////////////////////////////////////////////////////////
    // Handle of the rendering context.
#ifdef _MSC_VER
    HGLRC get_rendering_context(int thread_id);
    void set_rendering_context(int thread_id, HGLRC rc);
#else
    void * get_rendering_context(int thread_id);
    void set_rendering_context(int thread_id, void * rc);
#endif

    ////////////////////////////////////////////////////////////////////////////////
    void show_last_error(wchar_t const* text=L"");

    ////////////////////////////////////////////////////////////////////////////////
#ifdef DEBUG
    void dump_error_log(wchar_t const* fmt, ...);
#endif

  } // The end of the namespace "sociarium_project_common"

} // The end of the namespace "hashimoto_ut"

#endif // INCLUDE_GUARD_SOCIARIUM_PROJECT_COMMON_H

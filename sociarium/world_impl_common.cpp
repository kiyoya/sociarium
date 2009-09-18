// s.o.c.i.a.r.i.u.m: world_impl_common.cpp
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

#include <cassert>
#include <vector>
#include <windows.h>
#include "common.h"
#include "../shared/msgbox.h"

namespace hashimoto_ut {

  using std::vector;
  using std::wstring;
  using std::endl;

  namespace {
    wstring module_path;
    HWND window_handle = NULL;
    HINSTANCE instance_handle = NULL;
    HDC device_context = NULL;
    vector<HGLRC> rendering_context(RenderingContext::NUMBER_OF_CATEGORIES, NULL);
  }


  ////////////////////////////////////////////////////////////////////////////////
  wstring const& WorldImpl::get_module_path(void) {
    return module_path;
  }

  void WorldImpl::set_module_path(wstring const& path) {
    module_path = path;
  }


  ////////////////////////////////////////////////////////////////////////////////
  HINSTANCE WorldImpl::get_instance_handle(void) {
    return instance_handle;
  }

  void WorldImpl::set_instance_handle(HINSTANCE hinst) {
    instance_handle = hinst;
  }


  ////////////////////////////////////////////////////////////////////////////////
  HWND WorldImpl::get_window_handle(void) {
    return window_handle;
  }

  void WorldImpl::set_window_handle(HWND hwnd) {
    window_handle = hwnd;
  }


  ////////////////////////////////////////////////////////////////////////////////
  HDC WorldImpl::get_device_context(void) {
    return device_context;
  }

  void WorldImpl::set_device_context(HDC dc) {
    device_context = dc;
  }


  ////////////////////////////////////////////////////////////////////////////////
  HGLRC WorldImpl::get_rendering_context(int thread_id) {
    assert(0<=thread_id && thread_id<RenderingContext::NUMBER_OF_CATEGORIES);
    return rendering_context[thread_id];
  }

  void WorldImpl::set_rendering_context(int thread_id, HGLRC rc) {
    assert(0<=thread_id && thread_id<RenderingContext::NUMBER_OF_CATEGORIES);
    rendering_context[thread_id] = rc;
  }


  ////////////////////////////////////////////////////////////////////////////////
  void WorldImpl::show_last_error(wchar_t const* text) {
    LPVOID buf;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER
                  | FORMAT_MESSAGE_FROM_SYSTEM
                  | FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL, GetLastError(),
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPTSTR)&buf, 0, NULL);
    message_box(get_window_handle(),
                MessageType::CRITICAL,
                APPLICATION_TITLE,
                L"%s%s", (wchar_t*)buf, text);
    LocalFree(buf);
  }

} // The end of the namespace "hashimoto_ut"


#ifdef DEBUG
#include <cstdio>
#include <fstream>
#include <boost/shared_array.hpp>

namespace hashimoto_ut {

  using std::wstring;
  using std::wofstream;
  using std::endl;

  namespace {
    wofstream error_log("error.log", std::ios::app);
  }

  void WorldImpl::dump_error_log(wchar_t const* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int const len = _vscwprintf(fmt, ap)+1;
    boost::shared_array<wchar_t> buf(new wchar_t [len]);
    vswprintf_s(buf.get(), len, fmt, ap);
    va_end(ap);
    error_log << buf << endl;
  }

} // The end of the namespace "hashimoto_ut"
#endif

﻿// HASHIMOTO, Yasuhiro (E-mail: hy @ sys.t.u-tokyo.ac.jp)
// update: 2009/02/02

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

#include <cstdio> // vsprintf_s
#include <boost/shared_array.hpp>
#ifdef _MSC_VER
#include <windows.h>
#endif
#include "msgbox.h"

namespace hashimoto_ut {

#ifdef __APPLE__
  // ANSI版
  MsgBox::MsgBox(void *, char const* title, char const* fmt, ...) {
  }

  // UNICODE版
  MsgBox::MsgBox(void *, wchar_t const* title, wchar_t const* fmt, ...) {
  }
#elif _MSC_VER
  // ANSI版
  MsgBox::MsgBox(HWND hwnd, char const* title, char const* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int const len = _vscprintf(fmt, ap)+1;
    boost::shared_array<char> buf(new char [len]);
    vsprintf_s(buf.get(), len, fmt, ap);
    va_end(ap);
    MessageBoxA(hwnd, buf.get(), title, MB_OK|MB_ICONEXCLAMATION|MB_SYSTEMMODAL);
  }

  // UNICODE版
  MsgBox::MsgBox(HWND hwnd, wchar_t const* title, wchar_t const* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int const len = _vscwprintf(fmt, ap)+1;
    boost::shared_array<wchar_t> buf(new wchar_t [len]);
    vswprintf_s(buf.get(), len, fmt, ap);
    va_end(ap);
    MessageBoxW(hwnd, buf.get(), title, MB_OK|MB_ICONEXCLAMATION|MB_SYSTEMMODAL);
  }
#endif

} // The end of the namespace "hashimoto_ut"

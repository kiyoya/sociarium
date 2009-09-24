// s.o.c.i.a.r.i.u.m: glew.cpp
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
#include <gl/glew.h>
#include <gl/wglew.h>
#include "common.h"
#include "glew.h"

namespace hashimoto_ut {

  using namespace sociarium_project_common;

  namespace sociarium_project_glew {

    bool initialize(void) {

      ////////////////////////////////////////////////////////////////////////////////
      // Make a dummy window, then initialize GLEW environment.

      WNDCLASS wc;
      ZeroMemory(&wc, sizeof(WNDCLASS));
      wc.hInstance = GetModuleHandle(NULL);
      wc.lpfnWndProc = DefWindowProc;
      wc.lpszClassName = L"GLEW";

      if (RegisterClass(&wc)==0) {
        show_last_error(L"sociarium_project_glew::initialize/RegisterClass");
        return false;
      }

      // --------------------------------------------------------------------------------
      // 1. Make a dummy window.
      HWND hwnd
        = CreateWindow(L"GLEW", L"GLEW", 0,
                       CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                       NULL, NULL, GetModuleHandle(NULL), NULL);;

      if (hwnd==NULL) {
        show_last_error(L"sociarium_project_glew::initialize/CreateWindow");
        return false;
      }

      ShowWindow(hwnd, SW_HIDE); // Don't show.

      // --------------------------------------------------------------------------------
      // 2. Get a device context.
      HDC dc = GetDC(hwnd);

      if (dc==NULL) {
        show_last_error(L"sociarium_project_glew::initialize/GetDC");
        return false;
      }

      // --------------------------------------------------------------------------------
      // 3. Choose and set the pixel format. This is not the ARB one.
      PIXELFORMATDESCRIPTOR pfd;
      ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));
      pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
      pfd.nVersion = 1;
      pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;

      int const pf = ChoosePixelFormat(dc, &pfd);

      if (pf==0) {
        show_last_error(L"sociarium_project_glew::initialize/ChoosePixelFormat");
        return false;
      }

      if (SetPixelFormat(dc, pf, &pfd)==FALSE) {
        /* "SetPixelFormat()" can be called once in the same window.
         * So, after initializing the GLEW environment,
         * the dummy window should be destroyed.
         */
        show_last_error(L"sociarium_project_glew::initialize/SetPixelFormat");
        return false;
      }

      // --------------------------------------------------------------------------------
      // 4. Create and enable the rendering context.
      HGLRC rc = wglCreateContext(dc);

      if (rc==0) {
        show_last_error(L"sociarium_project_glew::initialize/wglCreateContext");
        ReleaseDC(hwnd, dc);
        return false;
      }

      if (wglMakeCurrent(dc, rc)==FALSE) {
        show_last_error(L"sociarium_project_glew::initialize/wglMakeCurrent");
        ReleaseDC(hwnd, dc);
        wglDeleteContext(rc);
        return false;
      }

      /*
       * The above code is just for calling "glewInit()" below validly.
       */

      // --------------------------------------------------------------------------------
      // 5. Initialize the glew environment.
      GLenum err = glewInit();

      // --------------------------------------------------------------------------------
      // Termination.
      wglMakeCurrent(NULL, NULL);
      ReleaseDC(hwnd, dc);
      wglDeleteContext(rc);
      DestroyWindow(hwnd);
      UnregisterClass(L"GLEW", GetModuleHandle(NULL));

      if (err!=GLEW_OK) {
        show_last_error(L"sociarium_project_glew::initialize/glewInit");
        return false;
      }

      return true;
    }

  } // The end of the namespace "sociarium_project_glew"

} // The end of the namespace "hashimoto_ut"

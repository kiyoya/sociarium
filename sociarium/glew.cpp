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

#ifdef _MSC_VER
#include <windows.h>
#endif
#ifdef __APPLE__
#include <GL/glew.h>
#else
#include <gl/glew.h>
#endif
#ifdef _MSC_VER
#include <gl/wglew.h>
#endif
#include "glew.h"

namespace hashimoto_ut {

  namespace sociarium_project_glew {

    void initialize(void) {

#ifdef _MSC_VER
#warning Should it be called in each world instances?
      ////////////////////////////////////////////////////////////////////////////////
      // Make a dummy window, then initialize GLEW environment.

      WNDCLASS wc;
      ZeroMemory(&wc, sizeof(WNDCLASS));
      wc.hInstance = GetModuleHandle(NULL);
      wc.lpfnWndProc = DefWindowProc;
      wc.lpszClassName = L"GLEW";

      if (RegisterClass(&wc)==0)
        throw L"sociarium_project_glew::initialize/RegisterClass";

      // --------------------------------------------------------------------------------
      // 1. Make a dummy window.
      HWND hwnd
        = CreateWindow(L"GLEW", L"GLEW", 0,
                       CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                       NULL, NULL, GetModuleHandle(NULL), NULL);;

      if (hwnd==NULL)
        throw L"sociarium_project_glew::initialize/CreateWindow";

      ShowWindow(hwnd, SW_HIDE); // Don't show.

      // --------------------------------------------------------------------------------
      // 2. Get a device context.
      HDC dc = GetDC(hwnd);

      if (dc==NULL)
        throw L"sociarium_project_glew::initialize/GetDC";

      // --------------------------------------------------------------------------------
      // 3. Choose and set the pixel format. This is not the ARB one.
      PIXELFORMATDESCRIPTOR pfd;
      ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));
      pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
      pfd.nVersion = 1;
      pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;

      int const pf = ChoosePixelFormat(dc, &pfd);

      if (pf==0)
        throw L"sociarium_project_glew::initialize/ChoosePixelFormat";

      if (SetPixelFormat(dc, pf, &pfd)==FALSE)
        throw L"sociarium_project_glew::initialize/SetPixelFormat";
        /* "SetPixelFormat()" can be called once in the same window.
         * So, after initializing the GLEW environment,
         * the dummy window should be destroyed.
         */

      // --------------------------------------------------------------------------------
      // 4. Create and enable the rendering context.
      HGLRC rc = wglCreateContext(dc);

      if (rc==0) {
        ReleaseDC(hwnd, dc);
        throw L"sociarium_project_glew::initialize/wglCreateContext";
      }

      if (wglMakeCurrent(dc, rc)==FALSE) {
        ReleaseDC(hwnd, dc);
        wglDeleteContext(rc);
        throw L"sociarium_project_glew::initialize/wglMakeCurrent";
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

      if (err!=GLEW_OK)
        throw L"sociarium_project_glew::initialize/glewInit";
#endif // _MSC_VER
    }

  } // The end of the namespace "sociarium_project_glew"

} // The end of the namespace "hashimoto_ut"

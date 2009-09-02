// HASHIMOTO, Yasuhiro (E-mail: hy @ sys.t.u-tokyo.ac.jp)
// update: 2009/02/16

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

#ifndef INCLUDE_GUARD_SHARED_GL_GLBASE_H
#define INCLUDE_GUARD_SHARED_GL_GLBASE_H

#include <windows.h>
#include <GL/gl.h>

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

namespace hashimoto_ut {

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // OpenGLレンダリングコンテキストの生成と削除
  class GLRenderingContext {
  public:
    GLRenderingContext(HDC hdc, DWORD dwFlags, const int depth) : hdc_(hdc) {
      PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR), 1, PFD_SUPPORT_OPENGL|dwFlags,
        PFD_TYPE_RGBA, GetDeviceCaps(hdc_, BITSPIXEL), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        depth, 0, 0, PFD_MAIN_PLANE, 0, 0, 0, 0
        };
      unsigned int pf = ChoosePixelFormat(hdc_, &pfd);
      SetPixelFormat(hdc_, pf, &pfd);
      hrc_ = wglCreateContext(hdc_);
      wglMakeCurrent(hdc_, hrc_);
    }

    ~GLRenderingContext() {
      wglMakeCurrent(hdc_, NULL);
      wglDeleteContext(hrc_);
    }

    HDC get_dc(void) const { return hdc_; }
    HGLRC get_rc(void) const { return hrc_; }

  private:
    HDC hdc_;
    HGLRC hrc_;
  };


  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // 垂直同期の制御

  /* [How to use]
   *
   *   // After creating the OpenGL Rendering Context
   *   std::tr1::shared_ptr<Vsync> vsync(Vsync::create(0)); // 垂直同期をOFF
   *
   *  See http://steinsoft.net/index.php?site=Programming/Code%20Snippets/OpenGL/no7
   *  Notice that you can specify the interval of VSync'ing.
   *  If you set the interval to 1, the monitor is refreshed after one 'completed' frame.
   *  When the interval is set to 2 the monitor will be refreshed every two frames.
   *  But you will rarely need this 'precision'.
   */

  class Vsync {
  public:
    virtual ~Vsync() {}
    virtual bool init(int) = 0;
    static Vsync* create(void);
    static Vsync* create(int interval);
  };

} // The end of the namespace "hashimoto_ut"

#endif // INCLUDE_GUARD_SHARED_GL_GLBASE_H

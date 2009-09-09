﻿// HASHIMOTO, Yasuhiro (E-mail: hy @ sys.t.u-tokyo.ac.jp)
// update: 2008/11/28

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

#ifdef _MSC_VER
#include <windows.h>
#endif
#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#ifdef _MSC_VER
#include <GL/wglext.h>
#endif
#include "glbase.h"

namespace hashimoto_ut {

  namespace {

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // 垂直同期の制御
    class VsyncImpl : public Vsync {
    public:
      typedef void (APIENTRY *PFNWGLEXTSWAPCONTROLPROC)(int);
      typedef int (*PFNWGLEXTGETSWAPINTERVALPROC)(void);

      VsyncImpl(void) : init_(0), wglGetSwapIntervalEXT(NULL), wglSwapIntervalEXT(NULL) {}
      VsyncImpl(int interval) : init_(0) { init(interval); }
      ~VsyncImpl() { if (wglSwapIntervalEXT && wglGetSwapIntervalEXT) wglSwapIntervalEXT(init_); }

      bool init(int interval) {
        wglSwapIntervalEXT = (PFNWGLEXTSWAPCONTROLPROC)wglGetProcAddress("wglSwapIntervalEXT");
        wglGetSwapIntervalEXT = (PFNWGLEXTGETSWAPINTERVALPROC)wglGetProcAddress("wglGetSwapIntervalEXT");
        if (wglSwapIntervalEXT && wglGetSwapIntervalEXT) {
          init_ = wglGetSwapIntervalEXT();
          wglSwapIntervalEXT(interval);
          return true;
        }
        // 垂直同期制御は未対応
        return false;
      }

    private:
      int init_;
      PFNWGLEXTSWAPCONTROLPROC wglSwapIntervalEXT;
      PFNWGLEXTGETSWAPINTERVALPROC wglGetSwapIntervalEXT;
    };

  } // The end of the anonymous namespace

  Vsync* Vsync::create(void) { return new VsyncImpl; }
  Vsync* Vsync::create(int interval) { return new VsyncImpl(interval); }

} // The end of the namespace "hashimoto_ut"

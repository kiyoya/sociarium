// HASHIMOTO, Yasuhiro (E-mail: hy @ sys.t.u-tokyo.ac.jp)
// update: 2008/11/29

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

#ifndef INCLUDE_GUARD_SHARED_FPS_H
#define INCLUDE_GUARD_SHARED_FPS_H

#ifdef __APPLE__
#include <CoreFoundation/CFDate.h>
#elif _MSC_VER
#include <mmsystem.h>
#endif
#include "thread.h"

#ifdef _MSC_VER
#pragma comment(lib, "winmm.lib")
#endif

namespace hashimoto_ut {

  class FPS : public Thread {
  public:
#ifdef __APPLE__
    FPS(void) : fps_(0.0), count_(0), last_(CFAbsoluteTimeGetCurrent()) {}
#elif _MSC_VER
    FPS(void) : fps_(0.0), count_(0), last_(timeGetTime()) {}
#endif
    ~FPS() {}
    double get(void) const { return fps_; }
    void count(void) { ++count_; } // 描画部に挿入

    void operator()(void) {
      while (1) {
#ifdef __APPLE__
        sleep(1); // 1sec間隔でFPS値を更新
        fps_ = 1000.0*count_/(CFAbsoluteTimeGetCurrent()-last_);
        last_ = CFAbsoluteTimeGetCurrent();
#elif _MSC_VER
        Sleep(1000); // 1000msec間隔でFPS値を更新
        fps_ = 1000.0*count_/(timeGetTime()-last_);
        last_ = timeGetTime();
#endif
        count_ = 0;
        if (cancel_check()) break;
      }
    }

  private:
    double fps_;
    int count_;
#ifdef __APPLE__
    CFAbsoluteTime last_;
#elif _MSC_VER
    DWORD last_;
#endif
  };

} // The end of the namespace "hashimoto_ut"

#endif // INCLUDE_GUARD_SHARED_FPS_H

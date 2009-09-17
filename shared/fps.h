// fps.h
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

#ifndef INCLUDE_GUARD_SHARED_FPS_H
#define INCLUDE_GUARD_SHARED_FPS_H

#include <cassert>
#ifdef __APPLE__
#include <CoreFoundation/CFDate.h>
#include <iostream>
#elif _MSC_VER
#include <windows.h>
#endif
#include "thread.h"

#ifdef _MSC_VER
#pragma comment(lib, "winmm.lib")
#endif

namespace hashimoto_ut {

  ////////////////////////////////////////////////////////////////////////////////
  class FPSCounter : public Thread {
  public:
#ifdef __APPLE__
    FPSCounter(void) : fps_(0.0), count_(0), last_(CFAbsoluteTimeGetCurrent()) {}
#elif _MSC_VER
    FPSCounter(void) : fps_(0.0), count_(0), last_(timeGetTime()) {}
#endif

    ~FPSCounter() {}

    double get(void) const {
      return fps_;
    }

    // To be inserted into a target loop.
    void count(void) {
      ++count_;
    }

    void operator()(void) {
      for (;;) {
#ifdef __APPLE__
        usleep(1000000);
        fps_ = count_/(CFAbsoluteTimeGetCurrent()-last_);
        last_ = CFAbsoluteTimeGetCurrent();
#elif _MSC_VER
        Sleep(1000); // update every 1000 msec
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
#else
    unsigned long last_;
#endif
  };


  ////////////////////////////////////////////////////////////////////////////////
  class FPSKeeper {
  public:
#ifdef __APPLE__
    FPSKeeper(unsigned long fps=1) : frame_time_(1.0), last_(0), fps_(fps) {
#elif _MSC_VER
    FPSKeeper(unsigned long fps=1) : frame_time_(1000), last_(0), err_(0), fps_(fps) {
#endif
      if (fps_<1) fps_ = 1;
    }

    ~FPSKeeper() {}

    void set(unsigned long fps) {
      if (fps<1) fps = 1;
      fps_ = fps;
    }

    // To be inserted into a target loop.
    void wait(void) {
      
#ifdef __APPLE__
      CFAbsoluteTime now = CFAbsoluteTimeGetCurrent();
      CFTimeInterval const diff = now-last_;
#elif _MSC_VER
      unsigned long now = timeGetTime()*fps_;
      
      if (last_>now){
        now += ULONG_MAX-last_;
        last_ = 0;
      }

      unsigned long const diff = now-last_;
#endif

#ifdef __APPLE__
      // [TODO]
      if (0 && frame_time_>diff && last_ != 0){
        CFTimeInterval wait_time = frame_time_-diff;
        usleep(wait_time * 1000);
#elif _MSC_VER
      if (frame_time_+err_>diff){
        unsigned long wait_time = frame_time_+err_-diff;
        err_ = wait_time%fps_;
        wait_time -= err_;
        Sleep(wait_time/fps_);
#endif
        last_ = now+wait_time;
      }
      else{
        last_ = now;
      }
    }

  private:
#ifdef __APPLE__
    CFTimeInterval const frame_time_;
    CFAbsoluteTime last_;
#elif _MSC_VER
    unsigned long const frame_time_;
    unsigned long last_;
    unsigned long err_;
#endif
    unsigned long fps_;
  };

} // The end of the namespace "hashimoto_ut"

#endif // INCLUDE_GUARD_SHARED_FPS_H

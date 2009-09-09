// timer.h
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

#ifndef INCLUDE_GUARD_SHARED_TIMER_H
#define INCLUDE_GUARD_SHARED_TIMER_H

#ifdef _MSC_VER

#include <map>
#include <windows.h>

namespace hashimoto_ut {

  class Timer {
  public:
    Timer(HWND hwnd) : hwnd_(hwnd) {}

    ~Timer() {}

    bool start(UINT id) {
      std::map<UINT, std::pair<UINT, bool> >::iterator i = id2sec_.find(id);
      if (i==id2sec_.end()) return false;
      SetTimer(hwnd_, id, i->second.first, NULL);
      i->second.second = true;
      return true;
    }

    bool stop(UINT id) {
      std::map<UINT, std::pair<UINT, bool> >::iterator i = id2sec_.find(id);
      if (i==id2sec_.end()) return false;
      KillTimer(hwnd_, id);
      i->second.second = false;
      return true;
    }

    bool is_active(UINT id) const {
      std::map<UINT, std::pair<UINT, bool> >::const_iterator i = id2sec_.find(id);
      if (i==id2sec_.end()) return false;
      return i->second.second;
    }

    bool add(UINT id, UINT second) {
      if (id2sec_.find(id)!=id2sec_.end()) return false;
      id2sec_.insert(std::make_pair(id, std::make_pair(second, false)));
      return true;
    }

    bool reset(UINT id, UINT second) {
      std::map<UINT, std::pair<UINT, bool> >::iterator i = id2sec_.find(id);
      if (i==id2sec_.end()) return false;
      i->second = std::make_pair(second, i->second.second);
      return true;
    }

  private:
    std::map<UINT, std::pair<UINT, bool> > id2sec_;
    HWND hwnd_;
  };

} // The end of the namespace "hashimoto_ut"

#endif // _MSC_VER

#endif // INCLUDE_GUARD_SHARED_TIMER_H

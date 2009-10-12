// s.o.c.i.a.r.i.u.m: main.h
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

#ifndef INCLUDE_GUARD_SOCIARIUM_PROJECT_MAIN_H
#define INCLUDE_GUARD_SOCIARIUM_PROJECT_MAIN_H

#include <memory>
#include <windowsx.h>
#include "../shared/wndbase.h"
#include "../shared/vector2.h"

namespace hashimoto_ut {

  class Timer;
  class World;

  class MainWindow : public WindowBase {
  private:
    ////////////////////////////////////////////////////////////////////////////////
    void wmCreate(HWND hwnd);
    void wmDestroy(HWND hwnd);
    void wmCommand(HWND hwnd, WPARAM wp, LPARAM lp);
    void wmSize(HWND hwnd);
    void wmPaint(HWND hwnd);
    void wmTimer(HWND hwnd, WPARAM wp, LPARAM lp);
    void wmActivate(HWND hwnd, WPARAM wp, LPARAM lp);
    void wmInitMenu(HWND hwnd, WPARAM wp, LPARAM lp);
    void wmKeyUp(HWND hwnd, WPARAM wp, LPARAM lp);
    void wmLButtonDown(HWND hwnd, WPARAM wp, LPARAM lp);
    void wmRButtonDown(HWND hwnd, WPARAM wp, LPARAM lp);
    void wmLButtonUp(HWND hwnd, WPARAM wp, LPARAM lp);
    void wmRButtonUp(HWND hwnd, WPARAM wp, LPARAM lp);
    void wmLButtonDBL(HWND hwnd, WPARAM wp, LPARAM lp);
    void wmRButtonDBL(HWND hwnd, WPARAM wp, LPARAM lp);
    void wmMButtonDown(HWND hwnd, WPARAM wp, LPARAM lp);
    void wmMButtonUp(HWND hwnd, WPARAM wp, LPARAM lp);
    void wmMouseMove(HWND hwnd, WPARAM wp, LPARAM lp);
    void wmMouseWheel(HWND hwnd, WPARAM wp, LPARAM lp);
    void wmDropFiles(HWND hwnd, WPARAM wp, LPARAM lp);
    void wmClose(HWND hwnd);

    ////////////////////////////////////////////////////////////////////////////////
    LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
      switch (msg) {
      case WM_CREATE:        wmCreate(hwnd); break;
      case WM_DESTROY:       wmDestroy(hwnd); return 0;
      case WM_COMMAND:       wmCommand(hwnd, wp, lp); break;
      case WM_SIZE:          wmSize(hwnd); break;
      case WM_PAINT:         wmPaint(hwnd); break;
      case WM_TIMER:         wmTimer(hwnd, wp, lp); break;
      case WM_ACTIVATE:      wmActivate(hwnd, wp, lp); break;
      case WM_INITMENU:      wmInitMenu(hwnd, wp, lp); break;
      case WM_KEYUP:         wmKeyUp(hwnd, wp, lp); break;
      case WM_LBUTTONDOWN:   wmLButtonDown(hwnd, wp, lp); break;
      case WM_RBUTTONDOWN:   wmRButtonDown(hwnd, wp, lp); break;
      case WM_LBUTTONUP:     wmLButtonUp(hwnd, wp, lp); break;
      case WM_RBUTTONUP:     wmRButtonUp(hwnd, wp, lp); break;
      case WM_LBUTTONDBLCLK: wmLButtonDBL(hwnd, wp, lp); break;
      case WM_RBUTTONDBLCLK: wmRButtonDBL(hwnd, wp, lp); break;
      case WM_MBUTTONDOWN:   wmMButtonDown(hwnd, wp, lp); break;
      case WM_MBUTTONUP:     wmMButtonUp(hwnd, wp, lp); break;
      case WM_MOUSEMOVE:     wmMouseMove(hwnd, wp, lp); break;
      case WM_MOUSEWHEEL:    wmMouseWheel(hwnd, wp, lp); break;
      case WM_DROPFILES:     wmDropFiles(hwnd, wp, lp); break;
      case WM_CLOSE:         wmClose(hwnd); return 0;
      default: break;
      }

      return DefWindowProc(hwnd, msg, wp, lp);
    }

    ////////////////////////////////////////////////////////////////////////////////
    // Convert window coordinates (the origin is upper-left) to
    // viewport coordinates (the origin is lower-left)
    Vector2<int> window2viewport(LPARAM lp) const {
      return Vector2<int>(GET_X_LPARAM(lp), wsize_.y-GET_Y_LPARAM(lp));
    }

    Vector2<int> window2viewport(POINT const& pt) const {
      return Vector2<int>(pt.x, wsize_.y-pt.y);
    }

    int mousemodifier(WPARAM wp) const {
      return (wp&MK_CONTROL) ? MouseModifier::CONTROL
                             : (wp&MK_SHIFT) ? MouseModifier::SHIFT : MouseModifier::NONE;
    }

    ////////////////////////////////////////////////////////////////////////////////
    // Get a position of the mouse pointer (in window coordinates)
    POINT get_mouse_position(HWND hwnd) const {
      POINT pt;
      GetCursorPos(&pt);
      ScreenToClient(hwnd, &pt);
      return pt;
    }

  public:
    ////////////////////////////////////////////////////////////////////////////////
    MainWindow(void) {}
    ~MainWindow() {}

  private:
    Vector2<int> wsize_;
    std::tr1::shared_ptr<Timer> timer_;
    std::tr1::shared_ptr<World> world_;
    int zoom_; // for timer use (0:zoom-in, 1:zoom-out)
  };

} // The end of the namespace "hashimoto_ut"

#endif // INCLUDE_GUARD_SOCIARIUM_PROJECT_MAIN_H

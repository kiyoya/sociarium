// wndbase.h
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

#ifndef INCLUDE_GUARD_SHARED_WNDBASE_H
#define INCLUDE_GUARD_SHARED_WNDBASE_H

#ifdef _MSC_VER

#include <windows.h>

namespace hashimoto_ut {

  ////////////////////////////////////////////////////////////////////////////////
  // ウィンドウ生成の流れ

  /*
   * 1. メンバ関数create()でCreateWindowEx()が実行されWM_CREATEが送信される．
   *    送信される情報には最後の引数で自分自身へのポインタthisを与える．
   * 2. ウィンドウクラスの登録時に指定したプロシージャWndProcMap(※)がWM_CREATEを捕獲する．
   * 3. WndPropMapは，WM_CREATE取得時に得たポインタを用いてメンバ変数に自身のハンドルをセットし，
   *    ハンドルに自分自身へのポインタを属性として与える．
   * 4. 以後，WndProcMapはWndPropに処理を引き継ぐ．
   *
   * 使用するウィンドウクラスのプロシージャ項には常にWindowBase::WndProcMapを指定する．
   */


  ////////////////////////////////////////////////////////////////////////////////
  class WindowBase {
  public:
    WindowBase(void) : hwnd_(NULL) {}

    virtual ~WindowBase() {}

    HWND create(LPCTSTR classname,
                LPCTSTR windowname,
                DWORD style, DWORD exstyle,
                int x, int y,
                int w, int h,
                HWND parent,
                HMENU menu) {

      if (!hwnd_)
        return CreateWindowEx(exstyle,
                              classname,
                              windowname,
                              style,
                              x, y, w, h,
                              parent,
                              menu,
                              GetModuleHandle(NULL),
                              this);
      return NULL;
    }

    HWND handle(void) const { return hwnd_; }

    static LRESULT CALLBACK WndProcMap(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
      static LPWSTR window_pointer = L"THIS_POINTER";
      WindowBase* p = (WindowBase*)GetProp(hwnd, window_pointer);

      if (!p && msg==WM_CREATE)
        p = (WindowBase*)LPCREATESTRUCT(lp)->lpCreateParams;

      if (p) {
        if (!p->hwnd_) {
          p->hwnd_ = hwnd;
          SetProp(hwnd, window_pointer, p);
          SetWindowLong(hwnd, GWL_WNDPROC, LONG(WndProcMap));
        }

        return p->WndProc(hwnd, msg, wp, lp);
      }

      return DefWindowProc(hwnd, msg, wp, lp);
    }

    virtual LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) = 0;

  protected:
    HWND hwnd_;
  };


  ////////////////////////////////////////////////////////////////////////////////
  class DialogBase {
  public:
    DialogBase(void) : hwnd_(NULL) {}

    virtual ~DialogBase() {}

    HWND create(LPCTSTR dlgname, HWND parent) {
      if (!hwnd_)
        return CreateDialogParam(GetModuleHandle(NULL),
                                 dlgname,
                                 parent,
                                 DLGPROC(DialogBase::DlgProcMap),
                                 LPARAM(this));
      return NULL;
    }

    HWND handle(void) const { return hwnd_; }

    static LRESULT CALLBACK DlgProcMap(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
      static LPWSTR window_pointer = L"THIS_POINTER";
      DialogBase* p = (DialogBase*)GetProp(hwnd, window_pointer);

      if (!p && msg==WM_INITDIALOG)
        p = (DialogBase*)lp;

      if (p) {
        if (!p->hwnd_) {
          p->hwnd_ = hwnd;
          SetProp(hwnd, window_pointer, p);
          SetWindowLong(hwnd, DWL_DLGPROC, LONG(DlgProcMap));
        }

        return p->DlgProc(hwnd, msg, wp, lp);
      }

      return FALSE;
    }

    virtual LRESULT DlgProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) = 0;

  protected:
    HWND hwnd_;
  };


  ////////////////////////////////////////////////////////////////////////////////
  class RegisterWindowClass {
  public:
    RegisterWindowClass(LPCTSTR classname, UINT style,
                        int cbClsExtra, int cbWndExtra,
                        HINSTANCE hInstance, HICON hIcon, HCURSOR hCursor, HBRUSH hbrBackground,
                        LPCTSTR menuname, HICON hIconSm) {
      WNDCLASSEX wc = {
        sizeof(WNDCLASSEX),
        style,
        WindowBase::WndProcMap,
        cbClsExtra,
        cbWndExtra,
        hInstance,
        hIcon,
        hCursor,
        hbrBackground,
        menuname,
        classname,
        hIconSm
        };
      RegisterClassEx(&wc);
    }
  };

} // The end of the namespace "hashimoto_ut"

#endif // _MSC_VER

#endif // INCLUDE_GUARD_SHARED_WNDBASE_H

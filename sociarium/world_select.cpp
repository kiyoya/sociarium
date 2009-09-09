// s.o.c.i.a.r.i.u.m
// HASHIMOTO, Yasuhiro (E-mail: hy @ sys.t.u-tokyo.ac.jp)
// update: 2009/04/06

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
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif
#include "world.h"
#include "common.h"
#include "mouse_and_selection.h"
#include "../shared/GL/glview.h"

namespace hashimoto_ut {

  ///////////////////////////////////////////////////////////////////////////////////////////////////
  void World::select(Vector2<int> const& mpos) const {

#ifdef _MSC_VER
    HWND hwnd = sociarium_project_common::get_window_handle();
    if (hwnd==0)
      sociarium_project_common::show_last_error(L"World::select/get_window_handle");

    bool must_release_dc = false;
    HDC dc = wglGetCurrentDC();
    if (dc==0) {
      dc = GetDC(hwnd);
      if (dc==0)
        sociarium_project_common::show_last_error(L"World::select/GetDC");
      if (wglMakeCurrent(dc, rc_)!=TRUE)
        sociarium_project_common::show_last_error(L"World::select/wglMakeCurrent(dc)");
      must_release_dc = true;
    }

    assert(dc!=0);
#endif

    // セレクションバッファの作成（これは必ずglRenderMode(GL_SELECT)の前に実行）
    static int const SELECTION_BUFFER = 1000;
    static GLuint selection_buffer[SELECTION_BUFFER];
    glSelectBuffer(SELECTION_BUFFER, selection_buffer);
    // レンダリングモードをセレクションモードに切替え
    glRenderMode(GL_SELECT);
    glInitNames();
    glPushName(-1);
    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    // --------------------------------------------------------------------------------
    // 透視射影の描画に対するセレクション
    view_->push_matrix();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPickMatrix(mpos.x, mpos.y, 3.0, 3.0, vp);
    draw_perspective_part();
    view_->pop_matrix();
    // --------------------------------------------------------------------------------
    // 直交投影の描画に対するセレクション
    view_->push_matrix();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPickMatrix(mpos.x, mpos.y, 3.0, 3.0, vp);
    draw_orthogonal_part();
    view_->pop_matrix();
    // --------------------------------------------------------------------------------
    // レンダリングモードを元に戻す．返り値は選択されたオブジェクトの数．
    // （データがセレクションバッファに入りきらなければhits<0）
    GLint hits = glRenderMode(GL_RENDER);
    GLuint* s = selection_buffer;
    if (hits>0) {
      sociarium_project_mouse_and_selection::get_selection()->set_level(s[0]);
      sociarium_project_mouse_and_selection::get_selection()->set_depth_min(double(s[1])/double(0x7fffffff));
      sociarium_project_mouse_and_selection::get_selection()->set_depth_max(double(s[2])/double(0x7fffffff));
      sociarium_project_mouse_and_selection::get_selection()->set_category(s[5*(hits-1)+3]); // 最後にピックアップしたオブジェクト
      sociarium_project_mouse_and_selection::get_selection()->set_id(s[5*(hits-1)+4]);       // 最後にピックアップしたオブジェクト
    } else {
      sociarium_project_mouse_and_selection::initialize_selection();
    }

#ifdef _MSC_VER
    if (must_release_dc) {
      if (wglMakeCurrent(0, 0)==FALSE)
        sociarium_project_common::show_last_error(L"World::select/wglMakeCurrent(0)");
      if (ReleaseDC(hwnd, dc)==0)
        sociarium_project_common::show_last_error(L"World::select/ReleaseDC");
    }
#endif
  }

} // The end of the namespace "hashimoto_ut"

// s.o.c.i.a.r.i.u.m
// HASHIMOTO, Yasuhiro (E-mail: hy @ sys.t.u-tokyo.ac.jp)
// update: 2009/04/05

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
#include <FTGL/ftgl.h>
#include "world.h"
#include "common.h"
#include "message.h"
#include "font.h"
#include "texture.h"
#include "view.h"
#include "fps_counter.h"
#include "mouse_and_selection.h"
#include "sociarium_graph_time_series.h"
#include "module/creation.h"
#include "module/layout.h"
#include "module/community_detection.h"
#include "../shared/msgbox.h"
#include "../shared/GL/glview.h"
#include "../shared/GL/gltexture.h"

namespace hashimoto_ut {

  ////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef __APPLE__
  World::World(void * context) {
#else
  World::World(void) {
#endif
    sociarium_project_message::create();

    // レンダリングコンテキストの生成
#ifdef __APPLE__
    rc_ = context;
#elif _MSC_VER
    HWND hwnd = sociarium_project_common::get_window_handle();
    if (hwnd==NULL)
      sociarium_project_common::show_last_error(L"World::initialize/get_window_handle");
    HDC dc = GetDC(hwnd);
    if (dc==NULL)
      sociarium_project_common::show_last_error(L"World::initialize/GetDC");

    PIXELFORMATDESCRIPTOR pfd = {
      sizeof(PIXELFORMATDESCRIPTOR), 1, PFD_SUPPORT_OPENGL|PFD_DRAW_TO_WINDOW|PFD_DOUBLEBUFFER,
      PFD_TYPE_RGBA, GetDeviceCaps(dc, BITSPIXEL), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      32, 0, 0, PFD_MAIN_PLANE, 0, 0, 0, 0
      };

    unsigned int const pf = ChoosePixelFormat(dc, &pfd);
    if (pf==0)
      sociarium_project_common::show_last_error(L"World::initialize/ChoosePixelFormat");
    if (SetPixelFormat(dc, pf, &pfd)==FALSE)
      sociarium_project_common::show_last_error(L"World::initialize/SetPixelFormat");
    if ((rc_=wglCreateContext(dc))==0)
      sociarium_project_common::show_last_error(L"World::initialize/wglCreateContext");
    if (wglMakeCurrent(dc, rc_)==FALSE)
      sociarium_project_common::show_last_error(L"World::initialize/wglMakeCurrent(dc)");
#else
    rc_ = NULL;
#endif
    
    // OpenGL環境の初期化
    glDisable(GL_FOG);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glEnable(GL_ALPHA_TEST);
    glDisable(GL_STENCIL_TEST);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glAlphaFunc(GL_ALWAYS, 0.0f);

    // 視点の初期化
    center_.set(0.0, 0.0);
    view_.reset(new GLView());
    view_->set_angle(-DEGREEV_MAX, DEGREEV_MAX);
    view_->set_distance(sociarium_project_view::VIEW_DISTANCE);
    view_->set_fov(60.0);
    view_->set_znear(0.1);
    view_->set_zfar(5000.0);
    view_->initialize_matrix();
    view_->set_perspective_matrix();

    // セレクションの初期化
    sociarium_project_mouse_and_selection::create_selection();

    // フォントの初期化
    sociarium_project_font::create();

    // テクスチャの初期化
    sociarium_project_texture::set_default_node_texture(L"___system_node.png", GL_CLAMP);
    sociarium_project_texture::set_default_edge_texture(L"___system_edge.png", GL_REPEAT);
    sociarium_project_texture::set_default_community_texture(L"___system_community.png", GL_CLAMP);
    sociarium_project_texture::set_slider_texture(L"___system_slider.png", GL_REPEAT);

    // モジュールマネージャーの初期化
    sociarium_project_module_creation::create();
    sociarium_project_module_layout::create();
    sociarium_project_module_community_detection::create();

    // グラフ時系列の初期化
    time_series_ = SociariumGraphTimeSeries::create();

#ifdef _MSC_VER
    if (wglMakeCurrent(0, 0)==FALSE)
      sociarium_project_common::show_last_error(L"World::initialize/wglMakeCurrent(0)");
    if (ReleaseDC(hwnd, dc)==0)
      sociarium_project_common::show_last_error(L"World::initialize/ReleaseDC");
#endif

    // FPSカウンターの初期化と開始
    sociarium_project_fps_counter::start();
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  World::~World() {
#ifdef _MSC_VER
    if (wglDeleteContext(rc_)==FALSE)
      sociarium_project_common::show_last_error(L"~World/wglDeleteContext");
#endif
  }

} // The end of the namespace "hashimoto_ut"

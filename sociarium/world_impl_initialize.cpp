// s.o.c.i.a.r.i.u.m: world_impl_initialize.cpp
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

#include <windows.h>
#include <gl/glew.h>
#include <gl/wglew.h>
#include <FTGL/ftgl.h>
#include "world_impl.h"
#include "cvframe.h"
#include "glew.h"
#include "common.h"
#include "language.h"
#include "texture.h"
#include "font.h"
#include "view.h"
#include "selection.h"
#include "sociarium_graph_time_series.h"
#include "community_transition_diagram.h"
#include "thread.h"
#include "fps_manager.h"
#include "thread/force_direction.h"
#include "../shared/msgbox.h"
#include "../shared/GL/glview.h"
#include "../shared/GL/gltexture.h"

#pragma comment(lib, "glew32.lib")

#include "designtide.h"

namespace hashimoto_ut {

  using std::tr1::shared_ptr;

  using namespace sociarium_project_common;
  using namespace sociarium_project_language;


  ////////////////////////////////////////////////////////////////////////////////
  shared_ptr<World> World::create(void) {
    return shared_ptr<World>(new WorldImpl());
  }


  ////////////////////////////////////////////////////////////////////////////////
  WorldImpl::WorldImpl(void) {

    // --------------------------------------------------------------------------------
    // Initialize the OpenGL environment.

    bool const glew_is_available
      = sociarium_project_glew::initialize();

    HDC dc = get_device_context();

    PIXELFORMATDESCRIPTOR pfd = {
      sizeof(PIXELFORMATDESCRIPTOR),
      1,
      PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER,
      PFD_TYPE_RGBA,
      GetDeviceCaps(dc, BITSPIXEL),
      0, 0, 0, 0, 0, 0,
      1,
      0,
      0,
      0, 0, 0, 0,
      16,
      0,
      0,
      PFD_MAIN_PLANE,
      0,
      0, 0, 0
      };

    int pf;
    BOOL msaa_is_available = FALSE;

    if (glew_is_available) {

      UINT number_of_formats = 0;
      float fattr[] = { 0.0f, 0.0f };

      int iattr[] = {
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
        WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
        WGL_COLOR_BITS_ARB, 24,
        WGL_ALPHA_BITS_ARB, 8,
        WGL_DEPTH_BITS_ARB, 16,
        WGL_STENCIL_BITS_ARB, 0,
        WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
        WGL_SAMPLE_BUFFERS_ARB, 1,
        WGL_SAMPLES_ARB, 8,
        0, 0
        };

      // Try to use Multi-Sampling Anti-Aliasing (x8).
      msaa_is_available = wglChoosePixelFormatARB(
        dc, iattr, fattr, 1, &pf, &number_of_formats);

      if (msaa_is_available && number_of_formats>0) {
        message_box(
          get_window_handle(),
          MB_OK|MB_ICONASTERISK|MB_SYSTEMMODAL,
          APPLICATION_TITLE,
          get_message(Message::GLEW_MSAA_8));
      } else {
        // Try to use Multi-Sampling Anti-Aliasing (x4).
        iattr[19] = 4;

        msaa_is_available = wglChoosePixelFormatARB(
          dc, iattr, fattr, 1, &pf, &number_of_formats);

        if (msaa_is_available && number_of_formats>0) {
          message_box(
            get_window_handle(),
            MB_OK|MB_ICONASTERISK|MB_SYSTEMMODAL,
            APPLICATION_TITLE,
            get_message(Message::GLEW_MSAA_4));
        } else {
          // Try to use Multi-Sampling Anti-Aliasing (x2).
          iattr[19] = 2;

          msaa_is_available = wglChoosePixelFormatARB(
            dc, iattr, fattr, 1, &pf, &number_of_formats);

          if (msaa_is_available && number_of_formats>0) {
            message_box(
              get_window_handle(),
              MB_OK|MB_ICONASTERISK|MB_SYSTEMMODAL,
              APPLICATION_TITLE,
              get_message(Message::GLEW_MSAA_2));
          } else {
            message_box(
              get_window_handle(),
              MB_OK|MB_ICONASTERISK|MB_SYSTEMMODAL,
              APPLICATION_TITLE,
              get_message(Message::GLEW_FAILED_TO_ENABLE_MSAA));
            msaa_is_available = FALSE;
          }
        }
      }
    } else {
      message_box(
        get_window_handle(),
        MB_OK|MB_ICONERROR|MB_SYSTEMMODAL,
        APPLICATION_TITLE,
        get_message(Message::GLEW_FAILED_TO_INITIALIZE));
    }

    /*
     * If @msaa_is_available==TRUE here, @pf for Multi-Sampling Anti-Aliasing is ready.
     * Otherwise, choose another pixel format.
     */

    if (msaa_is_available==FALSE) {
      if ((pf=ChoosePixelFormat(dc, &pfd))==0) {
        show_last_error(L"WorldImpl::World/ChoosePixelFormat");
        exit(1);
      }
    }

    if (SetPixelFormat(dc, pf, &pfd)==FALSE) {
      show_last_error(L"WorldImpl::World/SetPixelFormat");
      exit(1);
    }

    // Create a rendering context for drawing.
    HGLRC rc_draw = wglCreateContext(dc);

    if (rc_draw==NULL) {
      show_last_error(L"WorldImpl::World/wglCreateContext (drawing)");
      exit(1);
    }

    set_rendering_context(
      RenderingContext::DRAW, rc_draw);

    // Create a rendering context for loading textures.
    HGLRC rc_texture = wglCreateContext(dc);

    if (rc_texture==NULL) {
      show_last_error(L"WorldImpl::World/wglCreateContext (loading textures)");
      exit(1);
    }

    set_rendering_context(
      RenderingContext::LOAD_TEXTURES, rc_texture);

    // Share textures between both contexts.
    if (wglShareLists(rc_draw, rc_texture)==FALSE) {
      show_last_error(L"WorldImpl::World/wglShareLists (drawing<->loading textures)");
      exit(1);
    }

    // Activate the drawing context in the main thread.
    if (wglMakeCurrent(dc, rc_draw)==FALSE) {
      show_last_error(L"WorldImpl::World/wglMakeCurrent");
      exit(1);
    }

    glDisable(GL_FOG);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glEnable(GL_ALPHA_TEST);
    glDisable(GL_STENCIL_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glAlphaFunc(GL_ALWAYS, 0.0f);

    if (msaa_is_available)
      glEnable(GL_MULTISAMPLE);

    // --------------------------------------------------------------------------------
    // Initialize view.
    center_.set(0.0, 0.0);
    view_.reset(new GLView());
    view_->set_angle(-DEGREEV_MAX, DEGREEV_MAX);
    view_->set_distance(sociarium_project_view::VIEW_DISTANCE);
    view_->set_fov(60.0);
    view_->set_znear(0.1);
    view_->set_zfar(5000.0);
    view_->initialize_matrix();
    view_->set_perspective_matrix();

    // --------------------------------------------------------------------------------
    // Create selection objects.
    sociarium_project_selection::initialize();

    // --------------------------------------------------------------------------------
    // Create font objects.
    sociarium_project_font::initialize();

    // --------------------------------------------------------------------------------
    // Create textures.
    sociarium_project_texture::set_slider_texture(L"___system_slider.png");

    // --------------------------------------------------------------------------------
    // Create a graph time series object.
    sociarium_project_graph_time_series::initialize();

    // --------------------------------------------------------------------------------
    // Create a Community Transition Diagram object.
    sociarium_project_community_transition_diagram::initialize();

    // --------------------------------------------------------------------------------
    // Initialize the thread executing environment.
    {
      using namespace sociarium_project_thread;

      initialize();

      shared_ptr<Thread> tf = ForceDirectionThread::create();
      if (!sociarium_project_force_direction::is_active())
        tf->suspend();
      invoke(FORCE_DIRECTION, tf);
    }

    // --------------------------------------------------------------------------------
    // Initialize FPS counter.
    sociarium_project_fps_manager::start(60);


    sociarium_project_designtide::initialize();
  }


  ////////////////////////////////////////////////////////////////////////////////
  WorldImpl::~WorldImpl() {

    // --------------------------------------------------------------------------------
    sociarium_project_thread::finalize();

    // --------------------------------------------------------------------------------
    sociarium_project_graph_time_series::finalize();

    // --------------------------------------------------------------------------------
#ifdef SOCIAIRUM_PROJECT_USES_OPENCV
    sociarium_project_cvframe::terminate();
#endif

    // --------------------------------------------------------------------------------
    if (wglMakeCurrent(0, 0)==FALSE)
      show_last_error(L"WorldImpl::~World/wglMakeCurrent");

    HWND hwnd = get_window_handle();

    if (hwnd==NULL)
      show_last_error(L"WorldImpl::~World/get_window_handle");

    HDC dc = get_device_context();

    if (ReleaseDC(hwnd, dc)==0)
      show_last_error(L"WorldImpl::~World/ReleaseDC");

    HGLRC rc_draw = get_rendering_context(
      RenderingContext::DRAW);

    if (wglDeleteContext(rc_draw)==FALSE)
      show_last_error(L"WorldImpl::~World/wglDeleteContext (drawing)");

#if 0
    HGLRC rc_textures = get_rendering_context(
      RenderingContext::LOAD_TEXTURES);

    if (wglDeleteContext(rc_textures)==FALSE)
      show_last_error(L"WorldImpl::~World/wglDeleteContext (loading textures)");
    /* In some environments, this block causes the "pure virtual function" error.
     * It's thought to be causally related to releasing the context that is
     * activated by another thread, but I don't know how should it be written...
     */
#endif
  }

} // The end of the namespace "hashimoto_ut"

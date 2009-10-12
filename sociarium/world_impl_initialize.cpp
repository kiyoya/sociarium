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

#ifdef _MSC_VER
#include <windows.h>
#endif
#ifdef __APPLE__
#include <GL/glew.h>
#else
#include <gl/glew.h>
#endif
#ifdef _MSC_VER
#include <gl/wglew.h>
#endif
#include <FTGL/ftgl.h>
#include "common.h"
#include "community_transition_diagram.h"
#include "font.h"
#include "fps_manager.h"
#include "glew.h"
#include "menu_and_message.h"
#include "selection.h"
#include "sociarium_graph_time_series.h"
#include "texture.h"
#include "thread.h"
#include "view.h"
#include "world_impl.h"
#include "thread/force_direction.h"
#include "../shared/msgbox.h"
#include "../shared/gl/glview.h"
#include "../shared/gl/texture.h"

// #include "cvframe.h"
// #include "designtide.h"
//#include "tamabi_library.h"

#ifdef _MSC_VER
#pragma comment(lib, "glew32.lib")
#endif

namespace hashimoto_ut {

  using std::tr1::shared_ptr;

  using namespace sociarium_project_common;
  using namespace sociarium_project_menu_and_message;


  ////////////////////////////////////////////////////////////////////////////////
#ifdef __APPLE__
  World* World::create(void* window, CGLContextObj context) {
    return new WorldImpl(window, context);
  }
  
  void World::destroy(World* world) {
    delete world;
  }
#elif _MSC_VER
  shared_ptr<World> World::create(HWND hwnd) {
    return shared_ptr<World>(new WorldImpl(hwnd));
  }
#else
#error Not implemented
#endif


  ////////////////////////////////////////////////////////////////////////////////
#ifdef __APPLE__
  WorldImpl::WorldImpl(void* hwnd, CGLContextObj context) : hwnd_(hwnd) {
#elif _MSC_VER
  WorldImpl::WorldImpl(HWND hwnd) : hwnd_(hwnd) {
#else
#error Not implemented
#endif

    // --------------------------------------------------------------------------------
    // Initialize the OpenGL environment.

    if (hwnd_==NULL) {
      show_last_error(NULL, L"Error in initialization: hwnd");
      exit(1);
    }

#ifdef __APPLE__
    if (context==NULL) {
      show_last_error(hwnd_, L"Error in initialization: context");
      exit(1);
    }
#elif _MSC_VER
    if ((dc_=GetDC(hwnd_))==NULL) {
      show_last_error(hwnd_, L"Error in initialization: dc");
      exit(1);
    }
#else
#error Not implemented
#endif

    bool glew_is_available = true;

    try {
      sociarium_project_glew::initialize();
    } catch (wchar_t const* errmsg) {
      show_last_error(hwnd_, errmsg);
      glew_is_available = false;
    }

#ifdef __APPLE__
#warning Not implemented
#elif _MSC_VER
    PIXELFORMATDESCRIPTOR pfd = {
      sizeof(PIXELFORMATDESCRIPTOR),
      1,
      PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER,
      PFD_TYPE_RGBA,
      GetDeviceCaps(dc_, BITSPIXEL),
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
      msaa_is_available
        = wglChoosePixelFormatARB(dc_, iattr, fattr, 1, &pf, &number_of_formats);

      if (msaa_is_available && number_of_formats>0) {
        message_box(hwnd_, mb_info, APPLICATION_TITLE,
                    get_message(Message::GLEW_MSAA_8));
      } else {
        // Try to use Multi-Sampling Anti-Aliasing (x4).
        iattr[19] = 4;

        msaa_is_available
          = wglChoosePixelFormatARB(dc_, iattr, fattr, 1, &pf, &number_of_formats);

        if (msaa_is_available && number_of_formats>0) {
          message_box(hwnd_, mb_info, APPLICATION_TITLE,
                      get_message(Message::GLEW_MSAA_4));
        } else {
          // Try to use Multi-Sampling Anti-Aliasing (x2).
          iattr[19] = 2;

          msaa_is_available
            = wglChoosePixelFormatARB(dc_, iattr, fattr, 1, &pf, &number_of_formats);

          if (msaa_is_available && number_of_formats>0) {
            message_box(hwnd_, mb_info, APPLICATION_TITLE,
                        get_message(Message::GLEW_MSAA_2));
          } else {
            message_box(hwnd_, mb_info, APPLICATION_TITLE,
                        get_message(Message::GLEW_FAILED_TO_ENABLE_MSAA));
            msaa_is_available = FALSE;
          }
        }
      }
    } else {
      message_box(hwnd_, mb_error, APPLICATION_TITLE,
                  get_message(Message::GLEW_FAILED_TO_INITIALIZE));
    }

    /*
     * If @msaa_is_available==TRUE here, @pf for Multi-Sampling Anti-Aliasing is ready.
     * Otherwise, choose another pixel format.
     */

    if (msaa_is_available==FALSE) {
      if ((pf=ChoosePixelFormat(dc_, &pfd))==0) {
        show_last_error(hwnd_, L"WorldImpl::World/ChoosePixelFormat");
        exit(1);
      }
    }

    if (SetPixelFormat(dc_, pf, &pfd)==FALSE) {
      show_last_error(hwnd_, L"WorldImpl::World/SetPixelFormat");
      exit(1);
    }
#else
#error Not implemented
#endif

    // Create a rendering contexts.
    rc_.resize(RenderingContext::NUMBER_OF_THREAD_CATEGORIES);

    for (int i=0; i<RenderingContext::NUMBER_OF_THREAD_CATEGORIES; ++i) {
#ifdef __APPLE__
      CGLError err = CGLCreateContext(CGLGetPixelFormat(context), context, &(rc_[i]));
      
      if (err != kCGLNoError) {
        show_last_error(hwnd, L"WorldImpl::World/CGLCreateContext");
        exit(1);
      }
#elif _MSC_VER
      rc_[i] = wglCreateContext(dc_);

      if (rc_[i]==NULL) {
        show_last_error(hwnd_, L"WorldImpl::World/wglCreateContext");
        exit(1);
      }
#else
#error Not implemented
#endif
    }

#ifdef __APPLE__
    // Activate the drawing context in the main thread.
    if (CGLSetCurrentContext(rc_[RenderingContext::DRAW]) != kCGLNoError) {
      show_last_error(hwnd_, L"WorldImpl::World/CGLSetCurrentContext");
      exit(1);
    }
#elif _MSC_VER
    // Share textures between the drawing context and other contexts.
    for (int i=RenderingContext::DRAW+1; i<RenderingContext::NUMBER_OF_THREAD_CATEGORIES; ++i) {
      if (wglShareLists(rc_[RenderingContext::DRAW], rc_[i])==FALSE) {
        show_last_error(hwnd_, L"WorldImpl::World/wglShareLists");
        exit(1);
      }
    }
    
    // Activate the drawing context in the main thread.
    if (wglMakeCurrent(dc_, rc_[RenderingContext::DRAW])==FALSE) {
      show_last_error(hwnd_, L"WorldImpl::World/wglMakeCurrent");
      exit(1);
    }
#else
#error Not implemented
#endif

    glDisable(GL_FOG);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glEnable(GL_ALPHA_TEST);
    glDisable(GL_STENCIL_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glAlphaFunc(GL_ALWAYS, 0.0f);

#ifdef __APPLE__
#warning Not implemented
#elif _MSC_VER
    if (msaa_is_available) glEnable(GL_MULTISAMPLE);
#else
#error Not implemented
#endif

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
    try {
      sociarium_project_font::initialize();
    } catch (wchar_t const* errmsg) {
      message_box(hwnd_, mb_error, APPLICATION_TITLE, errmsg);
      exit(1);
    }

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
      if (!sociarium_project_force_direction::is_active()) tf->suspend();
      invoke(FORCE_DIRECTION, tf);
    }

    // --------------------------------------------------------------------------------
    // Initialize FPS counter.
    sociarium_project_fps_manager::start(60);

    //sociarium_project_designtide::initialize();
    //sociarium_project_tamabi_library::initialize();
  }


  ////////////////////////////////////////////////////////////////////////////////
  WorldImpl::~WorldImpl() {

    //sociarium_project_cvframe::terminate();

    // --------------------------------------------------------------------------------
    sociarium_project_thread::finalize();

    // --------------------------------------------------------------------------------
    sociarium_project_graph_time_series::finalize();

    // --------------------------------------------------------------------------------
#ifdef __APPLE__
    if (CGLSetCurrentContext(NULL)!=kCGLNoError)
      show_last_error(hwnd_, L"WorldImpl::~World/CGLSetCurrentContext");
    
    for (int i=1; i<RenderingContext::NUMBER_OF_THREAD_CATEGORIES; ++i) {
      CGLReleaseContext(rc_[i]);
    }
#elif _MSC_VER
    if (wglMakeCurrent(0, 0)==FALSE)
      show_last_error(hwnd_, L"WorldImpl::~World/wglMakeCurrent");

    if (ReleaseDC(hwnd_, dc_)==0)
      show_last_error(hwnd_, L"WorldImpl::~World/ReleaseDC");

    if (wglDeleteContext(rc_[RenderingContext::DRAW])==FALSE)
      show_last_error(hwnd_, L"WorldImpl::~World/wglDeleteContext");
    
#if 0
    for (int i=1; i<RenderingContext::NUMBER_OF_THREAD_CATEGORIES; ++i) {
      if (wglDeleteContext(rc_[i])==FALSE)
        show_last_error(hwnd_, L"WorldImpl::~World/wglDeleteContext");
    }
    /* In some environments, this block causes the "pure virtual function" error.
     * It's thought to be causally related to releasing the context activated by
     * another thread, however, I have no idea how should it be correctly written...
     */
#endif
#else
#error Not implemented
#endif
  }

} // The end of the namespace "hashimoto_ut"

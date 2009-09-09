// s.o.c.i.a.r.i.u.m: designtide.cpp
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

#include <vector>
#include <memory>
#include <array>
#include <windows.h>
#include <gl/glew.h>
#include <boost/format.hpp>
#include "designtide.h"
#include "cvframe.h"
#include "common.h"
#include "texture.h"
#include "view.h"
#include "font.h"
#include "agent.h"
#include "sociarium_graph_time_series.h"
#include "timeline.h"
#include "algorithm_selector.h"
#include "thread/force_direction.h"
#include "../shared/vector2.h"
#include "../shared/vector3.h"
#include "../shared/predefined_color.h"
#include "../shared/msgbox.h"
#include "../shared/gl/gltexture.h"

#pragma comment(lib, "winmm.lib")

namespace hashimoto_ut {

  using std::vector;
  using std::wstring;
  using std::tr1::array;
  using std::tr1::shared_ptr;

  using namespace sociarium_project_common;
  using namespace sociarium_project_cvframe;
  using namespace sociarium_project_view;
  using namespace sociarium_project_agent;
  using namespace sociarium_project_font;
  using namespace sociarium_project_timeline;
  using namespace sociarium_project_algorithm_selector;

  namespace sociarium_project_designtide {

    namespace {
      unsigned long balloon_start_time = 0;
      Vector2<float> pos_balloon;
      array<float, 4> rgba;

      float myrand(void) {
        return rand()/(RAND_MAX+1.0f);
      }
    }

    int const wait_w = 6;

    struct WalkingManMini {
      Vector2<float> pos;
      Vector2<float> pos_next;
      StaticNodeProperty const* snp_dst;
      int action;
      int dir;
      unsigned long time;
      int frame;
      float vel;
      bool is_special;
      int life;

      WalkingManMini(void) : snp_dst(0), action(1), time(0), frame(0), is_special(false), life(10) {
        vel = 0.1f*(1.0f+myrand());
      }

      void do_action(float angleH, float angleV, unsigned long now_time) {

        if (action==0) {

          unsigned long const wait_time
            = 3000+(is_special?0:unsigned long(1000*myrand()));

          if (now_time-time>wait_time) {
            time = now_time;
            action = 1;
            return;
          }

          glEnable(GL_TEXTURE_2D);
          GLTexture const* texture
            = sociarium_project_texture::get_texture(L"agent.png");

          if (texture) {
            if (is_special) glColor4f(0.3f, 0.0f, 0.0f, 0.6f);
            else glColor4f(0.0f, 0.0f, 0.0f, 0.6f);
            Vector2<float> const sz(2.0f, 2.0f*texture->height()/texture->width());
            glPushMatrix();
            glTranslatef(pos.x, pos.y, 0.0f);
            glRotatef(angleH, 0.0f, 0.0f, 1.0f);
            glRotatef(-angleV, 1.0f, 0.0f, 0.0f);
            glBindTexture(GL_TEXTURE_2D, texture->get());
            glBegin(GL_TRIANGLE_STRIP);
            glTexCoord2f(0.0f, texture->ycoord());
            glVertex2f(-0.5f*sz.x, +0.5f*sz.y);
            glTexCoord2f(0.0f, 0.0f);
            glVertex2f(-0.5f*sz.x, -0.5f*sz.y);
            glTexCoord2f(texture->xcoord(), texture->ycoord());
            glVertex2f(+0.5f*sz.x, +0.5f*sz.y);
            glTexCoord2f(texture->xcoord(), 0.0f);
            glVertex2f(+0.5f*sz.x, -0.5f*sz.y);
            glEnd();
            glPopMatrix();
          }

          glDisable(GL_TEXTURE_2D);
        }

        else if (action==1) {

          RadiusVector2<float> const r(pos, pos_next);

          if (r.norm<0.2f) {
            action = 2;
            return;
          }

          pos += vel*r.u;

          if (r.u.x<0.0f) dir = WalkingMan::LEFT;
          else dir = WalkingMan::RIGHT;

          shared_ptr<WalkingMan> walking_man = get_walking_man();

          if (++frame>wait_w*walking_man->number_of_frames())
            frame = 0;

          if (is_special) glColor4f(0.3f, 0.0f, 0.0f, 0.6f);
          else glColor4f(0.0f, 0.0f, 0.0f, 0.6f);
          glPushMatrix();
          glTranslatef(pos.x, pos.y, 0.0f);
          glRotatef(angleH, 0.0f, 0.0f, 1.0f);
          glRotatef(-angleV, 1.0f, 0.0f, 0.0f);
          walking_man->draw(Vector2<float>(0.0f, 0.0f), 6.0f, frame/wait_w, dir);
          glPopMatrix();
        }

        else if (action==2) {

          shared_ptr<SociariumGraphTimeSeries> ts
            = sociarium_project_graph_time_series::get();

          ts->read_lock();

          shared_ptr<SociariumGraph> g
            = ts->get_graph(0, ts->index_of_current_layer());

          if (g->esize()==0) {
            ts->read_unlock();
            return;
          }

          Edge const* e = g->edge(size_t(g->esize()*myrand()));
          Node const* n = e->source();
          DynamicNodeProperty& dnp = g->property(n);
          StaticNodeProperty const* snp = dnp.get_static_property();

          action = 0;

          if (snp==snp_dst)
            return;

          if (is_special && snp_dst) {
            StaticNodeProperty::DynamicPropertyMap::const_iterator
              i = snp_dst->dynamic_property_begin();
            StaticNodeProperty::DynamicPropertyMap::const_iterator
              end = snp_dst->dynamic_property_end();

            for (; i!=end; ++i) {
              if (i->second==ts->index_of_current_layer()) {
                rgba[0] = predefined_color[i->first->get_color_id()].x;
                rgba[1] = predefined_color[i->first->get_color_id()].y;
                rgba[2] = predefined_color[i->first->get_color_id()].z;
                balloon_start_time = timeGetTime();
                pos_balloon = pos;

                HDC dc = get_device_context();
                HGLRC rc_cvframe = wglCreateContext(dc);

                if (rc_cvframe==0)
                  show_last_error(L"sociarium_project_designtide::popup/wglCreateContext");

                HGLRC rc = get_rendering_context(RenderingContext::DRAW);

                if (wglShareLists(rc, rc_cvframe)==FALSE)
                  show_last_error(L"sociarium_project_designtide::popup/wglShareLists (rendering<->cvframe)");

                shared_ptr<CVFrame> cvframe
                  = sociarium_project_cvframe::create(dc, rc_cvframe);

                assert(cvframe!=0);

                cvframe->set_movie("movie.avi");
                cvframe->set_masking_image("balloon.png");

                sociarium_project_cvframe::invoke(cvframe);
              }
            }
          }

          snp_dst = snp;
          pos_next = snp_dst->get_position();
          --life;

          ts->read_unlock();

          time = timeGetTime();
        }

      }
    };


    vector<WalkingManMini> wm;
    GLuint fbo_texture;
    GLuint depthbuffer;
    GLuint fbo;


    ////////////////////////////////////////////////////////////////////////////////
    void initialize(void) {
      vector<shared_ptr<GLTexture> > texture_walking_man(8);

      for (size_t i=0; i<texture_walking_man.size(); ++i) {
        wstring const filename
          = (boost::wformat(L"texture\\walking\\%02d.png")%i).str();
        texture_walking_man[i] = GLTexture::create();
        int const err
          = texture_walking_man[i]->create_mipmap(
            filename.c_str(), GL_CLAMP_TO_EDGE_EXT, GL_CLAMP_TO_EDGE_EXT);
        if (err!=GLTexture::SUCCEEDED)
          message_box(get_window_handle(),
                      MB_OK|MB_ICONERROR|MB_SYSTEMMODAL,
                      APPLICATION_TITLE,
                      L"Failed to make walking men");
      }

      set_walking_man(texture_walking_man);

      set_show_edge(false);
      set_show_community(false);
      set_show_community_edge(false);
      set_show_community_name(false);
      set_show_community_edge_name(false);
      set_show_layer_name(false);
      set_show_slider(false);
      set_auto_run_id(AutoRun::FORWARD_1);
      set_font_scale(FontCategory::NODE_NAME, FontScale::LARGE);
      set_force_direction_algorithm(RealTimeForceDirectionAlgorithm::DESIGNTIDE);

      if (!sociarium_project_force_direction::is_active())
        sociarium_project_force_direction::toggle_execution();

      wm.push_back(WalkingManMini());
      wm[0].is_special = true;


      glGenFramebuffersEXT(1, &fbo);
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);

      glGenRenderbuffersEXT(1, &depthbuffer);
      glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depthbuffer);
      glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, 512, 512);
      glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depthbuffer);
      
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      glGenTextures(1, &fbo_texture);
      glBindTexture(GL_TEXTURE_2D, fbo_texture);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 512, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE_EXT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE_EXT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glGenerateMipmapEXT(GL_TEXTURE_2D);
      glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, fbo_texture, 0);
      glBindTexture(GL_TEXTURE_2D, 0);

      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

      if (glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT)!=GL_FRAMEBUFFER_COMPLETE_EXT)
        message_box(get_window_handle(),
                    MB_OK|MB_ICONERROR|MB_SYSTEMMODAL,
                    APPLICATION_TITLE,
                    L"FBO");
    }


    ////////////////////////////////////////////////////////////////////////////////
    void update(void) {
      wm.resize(wm.size()+10);
    }


    ////////////////////////////////////////////////////////////////////////////////
    void draw_balloon(float size, float angleH, float angleV) {

      shared_ptr<CVFrame> cvframe = get_current_frame();

      if (!joinable()) return;

      cvframe->read_lock();

      GLTexture const* texture = cvframe->get_texture();
      GLTexture const* texture_boundary
        = sociarium_project_texture::get_texture(L"balloon_frame.png");

      if (texture && texture_boundary) {

          glPushAttrib(GL_VIEWPORT_BIT);
          glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);

          glViewport(0, 0, 512, 512);

          glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
          glClearDepth(1.0f);

          glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

          glMatrixMode(GL_PROJECTION);
          glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
          glMatrixMode(GL_MODELVIEW);
          gluLookAt(0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

          glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
          glBegin(GL_TRIANGLE_STRIP);
          glVertex2f(0.0f, 1.0f);
          glVertex2f(0.0f, 0.0f);
          glVertex2f(1.0f, 1.0f);
          glVertex2f(1.0f, 0.0f);
          glEnd();

          
//           glEnable(GL_TEXTURE_2D);
//           {
//             glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
//             glBindTexture(GL_TEXTURE_2D, texture->get());
//             glBegin(GL_TRIANGLE_STRIP);
//             glTexCoord2f(0.0f, texture->ycoord());
//             glVertex2f(0.0f, 1.0f);
//             glTexCoord2f(0.0f, 0.0f);
//             glVertex2f(0.0f, 0.0f);
//             glTexCoord2f(texture->xcoord(), texture->ycoord());
//             glVertex2f(1.0f, 1.0f);
//             glTexCoord2f(texture->xcoord(), 0.0f);
//             glVertex2f(1.0f, 0.0f);
//             glEnd();
//             glBindTexture(GL_TEXTURE_2D, 0);
//           }
//           {
//             glColor4f(rgba[0], rgba[1], rgba[2], 1.0f);
//             glBindTexture(GL_TEXTURE_2D, texture_boundary->get());
//             glBegin(GL_TRIANGLE_STRIP);
//             glTexCoord2f(0.0f, texture_boundary->ycoord());
//             glVertex2f(0.0f, 1.0f);
//             glTexCoord2f(0.0f, 0.0f);
//             glVertex2f(0.0f, 0.0f);
//             glTexCoord2f(texture_boundary->xcoord(), texture_boundary->ycoord());
//             glVertex2f(1.0f, 1.0f);
//             glTexCoord2f(texture_boundary->xcoord(), 0.0f);
//             glVertex2f(1.0f, 0.0f);
//             glEnd();
//             glBindTexture(GL_TEXTURE_2D, 0);
//           }
//           glDisable(GL_TEXTURE_2D);

          glPopAttrib();
          glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);


        float const h = size*texture->height()/texture->width();
        Vector2<float> const sz(size, size*texture->height()/texture->width());

        glPushMatrix();
        glTranslatef(pos_balloon.x+0.5f*sz.x, pos_balloon.y+0.5f*sz.y, 0.0f);
        glRotatef(angleH, 0.0f, 0.0f, 1.0f);
        glRotatef(-angleV, 1.0f, 0.0f, 0.0f);

        glColor4f(1.0f, 1.0f, 1.0f, rgba[3]);

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, fbo_texture);

        glBegin(GL_TRIANGLE_STRIP);
        glTexCoord2f(0.0f, 1.0f);
        glVertex2f(-0.5f*sz.x, +0.5f*sz.y);
        glTexCoord2f(0.0f, 0.0f);
        glVertex2f(-0.5f*sz.x, -0.5f*sz.y);
        glTexCoord2f(1.0f, 1.0f);
        glVertex2f(+0.5f*sz.x, +0.5f*sz.y);
        glTexCoord2f(1.0f, 0.0f);
        glVertex2f(+0.5f*sz.x, -0.5f*sz.y);
        glEnd();

        glBindTexture(GL_TEXTURE_2D, 0);

//         {
//           glColor4f(1.0f, 1.0f, 1.0f, rgba[3]);
//           glBindTexture(GL_TEXTURE_2D, texture->get());
//           glBegin(GL_TRIANGLE_STRIP);
//           glTexCoord2f(0.0f, texture->ycoord());
//           glVertex2f(-0.5f*sz.x, +0.5f*sz.y);
//           glTexCoord2f(0.0f, 0.0f);
//           glVertex2f(-0.5f*sz.x, -0.5f*sz.y);
//           glTexCoord2f(texture->xcoord(), texture->ycoord());
//           glVertex2f(+0.5f*sz.x, +0.5f*sz.y);
//           glTexCoord2f(texture->xcoord(), 0.0f);
//           glVertex2f(+0.5f*sz.x, -0.5f*sz.y);
//           glEnd();
//         }{
//           rgba[3] = 1.0f;
//           glColor4fv(rgba.data());
//           size *= 0.87f;
//           Vector2<float> const sz(size, size*texture_boundary->height()/texture_boundary->width());
//           glBindTexture(GL_TEXTURE_2D, texture_boundary->get());
//           glBegin(GL_TRIANGLE_STRIP);
//           glTexCoord2f(0.0f, texture_boundary->ycoord());
//           glVertex2f(-0.5f*sz.x, +0.5f*sz.y);
//           glTexCoord2f(0.0f, 0.0f);
//           glVertex2f(-0.5f*sz.x, -0.5f*sz.y);
//           glTexCoord2f(texture_boundary->xcoord(), texture_boundary->ycoord());
//           glVertex2f(+0.5f*sz.x, +0.5f*sz.y);
//           glTexCoord2f(texture_boundary->xcoord(), 0.0f);
//           glVertex2f(+0.5f*sz.x, -0.5f*sz.y);
//           glEnd();
//         }
        glDisable(GL_TEXTURE_2D);
        glPopMatrix();
      }

      cvframe->read_unlock();
    }


    ////////////////////////////////////////////////////////////////////////////////
    void draw_hall(void) {
      glPushMatrix();
      glTranslatef(-100.0f, -28.456f, 0.0f);

      glColor4f(0.3f, 0.3f, 0.3f, 0.5f);

      float const top = 56.913f;

      // --------------------------------------------------------------------------------
      // The A Hall

      // bottom

      float x = 14.519f;
      float y = 0.0f;
      float w = 10.968f;
      float h = 6.555f;

      for (int i=0; i<5; ++i) {
        glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x, y+h);
        glVertex2f(x+w, y+h);
        glVertex2f(x+w, y);
        glEnd();
        x += 17.805f;
      }

      // right bottom

      glBegin(GL_QUADS);
      glVertex2f(103.543f, y);
      glVertex2f(103.543f+7.251f, y);
      glVertex2f(103.543f+7.251f, y+h);
      glVertex2f(103.543f, y+h);
      glEnd();

      // right middle

      x = 105.79f;
      y = 13.397f;

      glBegin(GL_QUADS);
      glVertex2f(x, y);
      glVertex2f(x+5.004f, y);
      glVertex2f(x+5.004f, y+30.442f);
      glVertex2f(x, y+30.442f);
      glEnd();

      // top

      x = 14.519f;
      y = 51.944f;
      w = 10.968f;
      h = 4.969f;

      for (int i=0; i<5; ++i) {
        glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x, y+h);
        glVertex2f(x+w, y+h);
        glVertex2f(x+w, y);
        glEnd();
        x += 17.805f;
      }

      // top right

      x = 103.543f;
      w = 2.247f;

      glBegin(GL_QUADS);
      glVertex2f(x, y);
      glVertex2f(x+w, y);
      glVertex2f(x+w, top);
      glVertex2f(x, top);
      glEnd();

      x += w;
      w = 5.004f;

      glBegin(GL_QUADS);
      glVertex2f(x, 50.675f);
      glVertex2f(x+w, 50.675f);
      glVertex2f(x+w, top);
      glVertex2f(x, top);
      glEnd();

      x += w;

      glBegin(GL_QUADS);
      glVertex2f(x, 55.737f);
      glVertex2f(114.59f, 55.737f);
      glVertex2f(114.59f, top);
      glVertex2f(x, top);
      glEnd();

      // left

      glBegin(GL_QUADS);
      glVertex2f(0, 0);
      glVertex2f(5.004f, 0);
      glVertex2f(5.004f, top);
      glVertex2f(0, top);
      glEnd();

      glBegin(GL_QUADS);
      glVertex2f(5.004f, 0);
      glVertex2f(7.683f, 0);
      glVertex2f(7.683f, 6.555f);
      glVertex2f(5.004f, 6.555f);
      glEnd();

      glBegin(GL_QUADS);
      glVertex2f(5.004f, 51.944f);
      glVertex2f(7.683f, 51.944f);
      glVertex2f(7.683f, top);
      glVertex2f(5.004f, top);
      glEnd();

      // --------------------------------------------------------------------------------
      // The B Hall

      // bottom

      x = 140.0f;
      w = 11.254f;
      h = 4.365f;

      for (int i=0; i<3; ++i) {
        glBegin(GL_QUADS);
        glVertex2f(x, 0);
        glVertex2f(x, h);
        glVertex2f(x+w, h);
        glVertex2f(x+w, 0);
        glEnd();
        x += 17.693f;
      }

      // top

      x = 138.598f;
      y = 53.237f;
      w = 13.522f;

      for (int i=0; i<3; ++i) {
        glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x, top);
        glVertex2f(x+w, top);
        glVertex2f(x+w, y);
        glEnd();
        x += 17.693f;
      }

      // left bottom

      x = 126.277f;
      w = 3.876f;

      glBegin(GL_QUADS);
      glVertex2f(x, 0);
      glVertex2f(x+w, 0);
      glVertex2f(x+w, 7.273f);
      glVertex2f(x, 7.273f);
      glEnd();

      glBegin(GL_QUADS);
      glVertex2f(x+w, 0);
      glVertex2f(x+7.285f, 0);
      glVertex2f(x+7.285f, 4.365f);
      glVertex2f(x+w, 4.365f);
      glEnd();

      // left middle

      y = 14.126f;
      h = 29.697f;

      glBegin(GL_QUADS);
      glVertex2f(x, y);
      glVertex2f(x+w, y);
      glVertex2f(x+w, y+h);
      glVertex2f(x, y+h);
      glEnd();

      // left top

      x = 121.294f;
      w = 4.983f;

      glBegin(GL_QUADS);
      glVertex2f(x, 55.737f);
      glVertex2f(x+w, 55.737f);
      glVertex2f(x+w, top);
      glVertex2f(x, top);
      glEnd();

      x += w;
      w = 3.876f;

      glBegin(GL_QUADS);
      glVertex2f(x, 50.676f);
      glVertex2f(x+w, 50.676f);
      glVertex2f(x+w, top);
      glVertex2f(x, top);
      glEnd();

      x += w;
      w = 4.085f;

      glBegin(GL_QUADS);
      glVertex2f(x, 53.237f);
      glVertex2f(x+w, 53.237f);
      glVertex2f(x+w, top);
      glVertex2f(x, top);
      glEnd();

      // right bottom

      x = 196.193f;
      w = 3.807f;

      glBegin(GL_QUADS);
      glVertex2f(193.078f, 0);
      glVertex2f(x, 0);
      glVertex2f(x, 4.365f);
      glVertex2f(193.078f, 4.365f);
      glEnd();

      glBegin(GL_QUADS);
      glVertex2f(x, 0);
      glVertex2f(200.0f, 0);
      glVertex2f(200.0f, 7.273f);
      glVertex2f(x, 7.273f);
      glEnd();

      // right middle

      y = 14.126f;
      h = 29.697f;

      glBegin(GL_QUADS);
      glVertex2f(x, y);
      glVertex2f(x+w, y);
      glVertex2f(x+w, y+h);
      glVertex2f(x, y+h);
      glEnd();

      // right top

      x = 192.246f;
      y = 53.237f;
      w = 3.807f;

      glBegin(GL_QUADS);
      glVertex2f(x, y);
      glVertex2f(196.193f, y);
      glVertex2f(196.193f, top);
      glVertex2f(x, top);
      glEnd();

      glBegin(GL_QUADS);
      glVertex2f(196.193f, 50.676f);
      glVertex2f(200.0f, 50.676f);
      glVertex2f(200.0f, top);
      glVertex2f(196.193f, top);
      glEnd();

      glPopMatrix();
    }


    ////////////////////////////////////////////////////////////////////////////////
    void draw(float angleH, float angleV) {

      unsigned long now_time = timeGetTime();
      unsigned long diff_time = now_time-balloon_start_time;

      rgba[3] = (11000-diff_time)/1000.0f;

      draw_hall();

      for (size_t i=0; i<wm.size(); ++i)
        wm[i].do_action(angleH, angleV, now_time);

      for (size_t i=0; i<wm.size();) {
        if (wm[i].life<0) {
          std::swap(wm[i], wm.back());
          wm.pop_back();
        } else ++i;
      }

      if (diff_time<10000) {
        if (diff_time>1000)
          draw_balloon(60.0f, angleH, angleV);
        else {
          float const size = 30.0f*(diff_time-500.0f)*(diff_time-750.0f)
            *(diff_time-250.0f)/(500.0f*750.0f*250.0f)+30.0f;
          draw_balloon(size, angleH, angleV);
        }
      } else if (diff_time<11000) {
        draw_balloon(60.0f, angleH, angleV);
      }
    }


    ////////////////////////////////////////////////////////////////////////////////
    // Captured frame drawing.
    void draw_captured_frame(float angleH, float angleV) {
#if 0
      using namespace sociarium_project_cvframe;

      read_lock();
      glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
      glEnable(GL_TEXTURE_2D);
      GLTexture const* texture = get_frame();
      GLTexture const* texture_boundary = sociarium_project_texture::get_texture(L"balloon_frame.png");

      if (texture) {
#if 0 // 1: 50 locations
        for (int i=0; i<10; ++i) {
          for (int j=0; j<5; ++j) {
            Vector2<float> const pos(20.0f*(i-5)+10, 25.0f*(j-2));
            Vector2<float> const size(10.0f, 10.0f*texture->height()/texture->width());
            glBindTexture(GL_TEXTURE_2D, texture->get());
            glBegin(GL_TRIANGLE_STRIP);
            glTexCoord2f(0.0f, texture->ycoord());
            glVertex2f(pos.x-0.5f*size.x, pos.y+0.5f*size.y);
            glTexCoord2f(0.0f, 0.0f);
            glVertex2f(pos.x-0.5f*size.x, pos.y-0.5f*size.y);
            glTexCoord2f(texture->xcoord(), texture->ycoord());
            glVertex2f(pos.x+0.5f*size.x, pos.y+0.5f*size.y);
            glTexCoord2f(texture->xcoord(), 0.0f);
            glVertex2f(pos.x+0.5f*size.x, pos.y-0.5f*size.y);
            glEnd();
          }
        }
#elif 0
        float const w = 20.0f;
        float const h = w*texture->height()/texture->width();
        float const d = 1.0f;

        float const ww = 1.7f*w;

        {
          float y = 1.5f*(h+d);

          for (int i=0; i<2; ++i) {
            float x = -1.5f*(w+d);
            for (int j=0; j<4; ++j) {
              {
                glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
                Vector2<float> const size(w, w*texture->height()/texture->width());
                glBindTexture(GL_TEXTURE_2D, texture->get());
                glBegin(GL_TRIANGLE_STRIP);
                glTexCoord2f(0.0f, texture->ycoord());
                glVertex2f(x-0.5f*size.x, y+0.5f*size.y);
                glTexCoord2f(0.0f, 0.0f);
                glVertex2f(x-0.5f*size.x, y-0.5f*size.y);
                glTexCoord2f(texture->xcoord(), texture->ycoord());
                glVertex2f(x+0.5f*size.x, y+0.5f*size.y);
                glTexCoord2f(texture->xcoord(), 0.0f);
                glVertex2f(x+0.5f*size.x, y-0.5f*size.y);
                glEnd();
              }{
                Vector2<float> const size(ww, ww*texture_boundary->height()/texture_boundary->width());
                glBindTexture(GL_TEXTURE_2D, texture_boundary->get());
                glBegin(GL_TRIANGLE_STRIP);
                glTexCoord2f(0.0f, texture_boundary->ycoord());
                glVertex2f(x-0.5f*size.x, y+0.5f*size.y);
                glTexCoord2f(0.0f, 0.0f);
                glVertex2f(x-0.5f*size.x, y-0.5f*size.y);
                glTexCoord2f(texture_boundary->xcoord(), texture_boundary->ycoord());
                glVertex2f(x+0.5f*size.x, y+0.5f*size.y);
                glTexCoord2f(texture_boundary->xcoord(), 0.0f);
                glVertex2f(x+0.5f*size.x, y-0.5f*size.y);
                glEnd();
              }
              x += w+d;
            }
            y -= 3*(h+d);
          }
        }{
          float y = 0.5f*(h+d);

          for (int i=0; i<2; ++i) {
            float x = -1.5f*(w+d);
            for (int j=0; j<2; ++j) {
              {
                glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
                Vector2<float> const size(w, w*texture->height()/texture->width());
                glBindTexture(GL_TEXTURE_2D, texture->get());
                glBegin(GL_TRIANGLE_STRIP);
                glTexCoord2f(0.0f, texture->ycoord());
                glVertex2f(x-0.5f*size.x, y+0.5f*size.y);
                glTexCoord2f(0.0f, 0.0f);
                glVertex2f(x-0.5f*size.x, y-0.5f*size.y);
                glTexCoord2f(texture->xcoord(), texture->ycoord());
                glVertex2f(x+0.5f*size.x, y+0.5f*size.y);
                glTexCoord2f(texture->xcoord(), 0.0f);
                glVertex2f(x+0.5f*size.x, y-0.5f*size.y);
                glEnd();
              }{
                Vector2<float> const size(ww, ww*texture_boundary->height()/texture_boundary->width());
                glBindTexture(GL_TEXTURE_2D, texture_boundary->get());
                glBegin(GL_TRIANGLE_STRIP);
                glTexCoord2f(0.0f, texture_boundary->ycoord());
                glVertex2f(x-0.5f*size.x, y+0.5f*size.y);
                glTexCoord2f(0.0f, 0.0f);
                glVertex2f(x-0.5f*size.x, y-0.5f*size.y);
                glTexCoord2f(texture_boundary->xcoord(), texture_boundary->ycoord());
                glVertex2f(x+0.5f*size.x, y+0.5f*size.y);
                glTexCoord2f(texture_boundary->xcoord(), 0.0f);
                glVertex2f(x+0.5f*size.x, y-0.5f*size.y);
                glEnd();
              }
              x += 3.0f*(w+d);
            }
            y -= h+d;
          }{
            {
              glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
              Vector2<float> const size(2.0f*w+d, 2.0f*w*texture->height()/texture->width()+d);
              glBindTexture(GL_TEXTURE_2D, texture->get());
              glBegin(GL_TRIANGLE_STRIP);
              glTexCoord2f(0.0f, texture->ycoord());
              glVertex2f(-0.5f*size.x, +0.5f*size.y);
              glTexCoord2f(0.0f, 0.0f);
              glVertex2f(-0.5f*size.x, -0.5f*size.y);
              glTexCoord2f(texture->xcoord(), texture->ycoord());
              glVertex2f(+0.5f*size.x, +0.5f*size.y);
              glTexCoord2f(texture->xcoord(), 0.0f);
              glVertex2f(+0.5f*size.x, -0.5f*size.y);
              glEnd();
            }{
              Vector2<float> const size(2.0f*ww, 2.0f*ww*texture_boundary->height()/texture_boundary->width()+d);
              glBindTexture(GL_TEXTURE_2D, texture_boundary->get());
              glBegin(GL_TRIANGLE_STRIP);
              glTexCoord2f(0.0f, texture_boundary->ycoord());
              glVertex2f(-0.5f*size.x, +0.5f*size.y);
              glTexCoord2f(0.0f, 0.0f);
              glVertex2f(-0.5f*size.x, -0.5f*size.y);
              glTexCoord2f(texture_boundary->xcoord(), texture_boundary->ycoord());
              glVertex2f(+0.5f*size.x, +0.5f*size.y);
              glTexCoord2f(texture_boundary->xcoord(), 0.0f);
              glVertex2f(+0.5f*size.x, -0.5f*size.y);
              glEnd();
            }
          }
        }
#endif
      }

      glDisable(GL_TEXTURE_2D);
      read_unlock();
#endif
    }


    ////////////////////////////////////////////////////////////////////////////////
    // Agent drawing.
    void draw_agents(float angleH, float angleV) {
#if 0
      shared_ptr<DancingMan> dancing_man = sociarium_project_agent::get_dancing_man();

      int const wait_d = 4;

      glColor4f(0.2f, 0.4f, 1.0f, 0.9f);

      {
        static int frame_id_d = 0;
        if (++frame_id_d>wait_d*dancing_man->number_of_frames())
          frame_id_d = 0;
        glPushMatrix();
        glTranslatef(0.0f, 10.0f, 0.0f);
        glRotatef(angleH, 0.0f, 0.0f, 1.0f);
        glRotatef(-angleV, 1.0f, 0.0f, 0.0f);
        dancing_man->draw(Vector2<float>(0.0f, 0.0f), 5.0f, frame_id_d/wait_d);
        glPopMatrix();
      }
#endif
    }

  } // The end of the namespace "sociarium_project_draw_detail"

} // The end of the namespace "hashimoto_ut"

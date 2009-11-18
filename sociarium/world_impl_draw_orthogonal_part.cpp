// s.o.c.i.a.r.i.u.m: world_impl_draw_orthogonal_part.cpp
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
#include <deque>
#include <memory>
#include <string>
#include <boost/format.hpp>
#include <windows.h>
#include <FTGL/ftgl.h>
#include "color.h"
#include "community_transition_diagram.h"
#include "draw.h"
#include "font.h"
#include "fps_manager.h"
#include "selection.h"
#include "sociarium_graph_time_series.h"
#include "texture.h"
#include "thread.h"
#include "../shared/predefined_color.h"
#include "../shared/GL/glview.h"
#include "../shared/GL/texture.h"

namespace hashimoto_ut {

  using std::vector;
  using std::deque;
  using std::wstring;
  using std::pair;
  using std::make_pair;
  using std::tr1::shared_ptr;

  using namespace sociarium_project_draw;
  using namespace sociarium_project_color;
  using namespace sociarium_project_font;
  using namespace sociarium_project_selection;

  namespace sociarium_project_draw_detail {

    ////////////////////////////////////////////////////////////////////////////////
    void draw_selection_result(Vector2<float> const& scale) {

      shared_ptr<SociariumGraphTimeSeries> ts
        = sociarium_project_graph_time_series::get();

      TimeSeriesLock lock(ts, TimeSeriesLock::Read);

      size_t const index_of_current_layer
        = ts->index_of_current_layer();

      shared_ptr<SociariumGraph const> g0
        = ts->get_graph(0, index_of_current_layer);

      shared_ptr<SociariumGraph const> g1
        = ts->get_graph(1, index_of_current_layer);

      DynamicNodeProperty const* n[2] = { 0, 0 };

      if (is_selected(SelectionCategory::NODE)
          || is_selected(SelectionCategory::COMMUNITY)) {
        n[0] = static_cast<DynamicNodeProperty*>(get_selected_dynamic_object());
      } else if (is_selected(SelectionCategory::EDGE)) {
        Edge* e = static_cast<Edge*>(get_selected_object());
        n[0] = &g0->property(e->source());
        n[1] = &g0->property(e->target());
      } else if (is_selected(SelectionCategory::COMMUNITY_EDGE)) {
        Edge* e = static_cast<Edge*>(get_selected_object());
        n[0] = &g1->property(e->source());
        n[1] = &g1->property(e->target());
      }

      float const aspect = scale.x==0.0f?0.0f:scale.y/scale.x;

      if (n[0]!=0) {
        float const top_margin = scale.y*get_frame_offset().y;
        float const left_margin = scale.x*get_frame_offset().x;
        float yy = 0.0f;

        glPushMatrix();
        Vector2<float> const frame_position(left_margin, 1.0f-top_margin); // upper-left
        Vector2<float> const frame_size(0.1f, float(0.11*aspect)); // vertically long
        float const texture_ratio = 0.8f; // texture_width/frame_width
        Vector2<float> const texture_size(
          texture_ratio*frame_size.x, texture_ratio*frame_size.x*aspect);
        Vector2<float> const margin = frame_size-texture_size;
        Vector2<float> const texture_position(0.5f*margin.x, -0.5f*margin.x);
        glTranslatef(frame_position.x, frame_position.y, 0.0f);

        for (int i=0; i<2; ++i) {
          if (n[i]==0) break;
          DynamicNodeProperty const* dnp = n[i];
          StaticNodeProperty const* snp = dnp->get_static_property();
          wstring const text = snp->get_name();

          glPushMatrix();
          glTranslatef(0.0f, -yy, 0.0f);

          { // Draw a frame.
            if (dnp->number_of_upper_nodes()==0 && dnp->number_of_lower_nodes()==0) {
              glColor4fv(get_color(ColorCategory::SELECTION_FRAME).data());
            } else {
              Vector3<float> const& rgb = predefined_color[dnp->get_color_id()];
              float rgba[4] = { 0.8f*rgb.x, 0.8f*rgb.y, 0.8f*rgb.z, 0.8f };
              glColor4fv(rgba);
            }

            glBegin(GL_TRIANGLE_STRIP);
            glVertex2f(0.0f, 0.0f);
            glVertex2f(0.0f, -frame_size.y);
            glVertex2f(frame_size.x, 0.0f);
            glVertex2f(frame_size.x, -frame_size.y);
            glEnd();
          }

          glPushMatrix();
          glTranslatef(texture_position.x, texture_position.y, 0.0f);

          {
            // --------------------------------------------------------------------------------
            // Draw a texture of the selected object.
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
            glEnable(GL_TEXTURE_2D);
            Texture const* texture = snp->get_texture();
            float const texture_aspect = float(texture->width())/texture->height();

            Vector2<float> const sz(
              texture_aspect<1.0f?
              Vector2<float>(texture_aspect*texture_size.x, texture_size.y)
              :Vector2<float>(texture_size.x, texture_size.y/texture_aspect));

            float const left   = 0.5f*(texture_size.x-sz.x);
            float const right  = 0.5f*(texture_size.x+sz.x);
            float const top    = -0.5f*(texture_size.y-sz.y);
            float const bottom = -0.5f*(texture_size.y+sz.y);

            glBindTexture(GL_TEXTURE_2D, texture->get());
            glBegin(GL_TRIANGLE_STRIP);
            glTexCoord2f(0.0f, 0.0f);
            glVertex2f(left, bottom);
            glTexCoord2f(0.0f, texture->ycoord());
            glVertex2f(left, top);
            glTexCoord2f(texture->xcoord(), 0.0f);
            glVertex2f(right, bottom);
            glTexCoord2f(texture->xcoord(), texture->ycoord());
            glVertex2f(right, top);
            glEnd();
            glBindTexture(GL_TEXTURE_2D, 0);
            glDisable(GL_TEXTURE_2D);
          }

          {
            // --------------------------------------------------------------------------------
            // Draw a name of the selected object.
            shared_ptr<FTFont> f = get_font(FontCategory::MISC);
            float llx, lly, llz, urx, ury, urz;
            f->BBox(text.c_str(), llx, lly, llz, urx, ury, urz);

            Vector2<float> const text_size(urx-llx, ury-lly);

            float const w_max = texture_size.x;
            float const h_max = 0.8f*(margin.y-0.5f*margin.x);
            /* Adjust text size to the remaining margin.
             */

            float w = scale.x*(urx-llx);
            float h = scale.y*(ury-lly);
            float adjust = 1.0f;

            if (h>h_max) {
              adjust *= h_max/h;
              w *= h_max/h;
              h = h_max;
            }

            if (w>w_max) {
              adjust *= w_max/w;
              h *= w_max/w;
              w = w_max;
            }

            glTranslatef(
              0.5f*(texture_size.x-w), -texture_size.y-0.5f*(h_max+h), 0.0f);
            glScalef(adjust*scale.x, adjust*scale.y, 0.0f);
            glColor4fv(get_color(ColorCategory::SELECTION_NAME).data());
            f->Render(text.c_str());
          }

          glPopMatrix();
          glPopMatrix();
          yy = frame_size.y+top_margin;
        }

        glPopMatrix();
      }
    }


    ////////////////////////////////////////////////////////////////////////////////
    void draw_layer_name(Vector2<float> const& scale) {

      shared_ptr<SociariumGraphTimeSeries> ts
        = sociarium_project_graph_time_series::get();

      TimeSeriesLock lock(ts, TimeSeriesLock::Read);

      size_t const index_of_current_layer = ts->index_of_current_layer();

      shared_ptr<SociariumGraph const> g0 =
        ts->get_graph(0, index_of_current_layer);

      shared_ptr<FTFont> f = get_font(FontCategory::MISC);
      float llx, lly, llz, urx, ury, urz;

      wstring const text_tmp = L"Yy\',"; // adjust height.
      f->BBox(text_tmp.c_str(), llx, lly, llz, urx, ury, urz);

      float const h_adjust = ury-lly;

      using namespace sociarium_project_selection;

      wstring const text = is_selected(SelectionCategory::LAYER_NAME)?
        (boost::wformat(L"L:%d, N:%d, E:%d")
         %(index_of_current_layer+1)
         %g0->nsize()%g0->esize()).str()
          :ts->get_layer_name(index_of_current_layer);

      f->BBox(text.c_str(), llx, lly, llz, urx, ury, urz);

      float const w = urx-llx;
      float h = ury-lly;
      if (h_adjust>h) h = h_adjust;

      // --------------------------------------------------------------------------------
      // Draw a frame.
      glLoadName(SelectionCategory::LAYER_NAME);
      glPushName(0);
      glColor4fv(get_color(ColorCategory::LAYER_NAME_FRAME).data());
      glBegin(GL_TRIANGLE_STRIP);
      glVertex2f(0.0f, 0.0f);
      glVertex2f(0.0f, 1.5f*scale.y*h);
      glVertex2f(1.0f, 0.0f);
      glVertex2f(1.0f, 1.5f*scale.y*h);
      glEnd();
      glPopName();

      GLint mode;
      glGetIntegerv(GL_RENDER_MODE, &mode);

      // --------------------------------------------------------------------------------
      // Draw a layer name.
      if (mode==GL_RENDER) {
        glPushMatrix();
        glTranslatef(0.5f*(1.0f-scale.x*w), 0.4f*scale.y*h, 0.0f);
        glScalef(scale.x, scale.y, 0.0f);
        glColor4fv(get_color(ColorCategory::LAYER_NAME).data());
        f->Render(text.c_str());
        glPopMatrix();
      }
    }


    ////////////////////////////////////////////////////////////////////////////////
    void draw_time_slider(Vector2<float> const& scale) {

      shared_ptr<SociariumGraphTimeSeries> ts
        = sociarium_project_graph_time_series::get();

      TimeSeriesLock lock(ts, TimeSeriesLock::Read);

      static float const transparent[] = { 0.0f, 0.0f, 0.0f, 0.0f, };

      float const xoff = scale.x*get_slider_offset().x;
      float const yoff = scale.y*get_slider_offset().y;
      float const w = 0.5f*xoff;
      float const h = 1.0f-2*yoff;
      float const x0 = 1.0f-xoff;
      float const x1 = x0+0.25f*w;
      float const x2 = x0+0.75f*w;
      float const x3 = x0+w;
      float const top = yoff+h;

      pair<int, int> const c = (
        is_selected(SelectionCategory::TIME_SLIDER)?
        make_pair(ColorCategory::TIME_SLIDER_UPPER_ON,
                  ColorCategory::TIME_SLIDER_LOWER_ON):
        make_pair(ColorCategory::TIME_SLIDER_UPPER_OFF,
                  ColorCategory::TIME_SLIDER_LOWER_OFF));

      glLoadName(SelectionCategory::TIME_SLIDER);
      glPushName(0);

      if (ts->number_of_layers()>100) {
        size_t const num = ts->number_of_layers()-1;
        float const y = (h*ts->index_of_current_layer())/num+yoff;

        // Upper part of the slider.
        glBegin(GL_TRIANGLE_STRIP);
        glColor4fv(transparent);
        glVertex2f(x0, yoff);
        glVertex2f(x0, y);
        glColor4fv(get_color(c.second).data());
        glVertex2f(x1, yoff);
        glVertex2f(x1, y);
        glVertex2f(x2, yoff);
        glVertex2f(x2, y);
        glColor4fv(transparent);
        glVertex2f(x3, yoff);
        glVertex2f(x3, y);
        glEnd();

        // Lower part of the slider.
        glBegin(GL_TRIANGLE_STRIP);
        glColor4fv(transparent);
        glVertex2f(x0, y);
        glVertex2f(x0, top);
        glColor4fv(get_color(c.first).data());
        glVertex2f(x1, y);
        glVertex2f(x1, top);
        glVertex2f(x2, y);
        glVertex2f(x2, top);
        glColor4fv(transparent);
        glVertex2f(x3, y);
        glVertex2f(x3, top);
        glEnd();
      }

      else {
        // If the number of layers is small, draw separators between each layer.
        size_t const num = ts->number_of_layers();
        size_t const num_lower = ts->index_of_current_layer()+1;
        float const interval_scale = 0.1f;
        float const dy = h/(interval_scale*(num-1)+num);
        float const my = interval_scale*dy;
        float yb = yoff;

        for (size_t i=0; i<num_lower; ++i) {
          float yt = yb+dy;
          glBegin(GL_TRIANGLE_STRIP);
          glColor4fv(transparent);
          glVertex2f(x0, yb);
          glVertex2f(x0, yt);
          glColor4fv(get_color(c.second).data());
          glVertex2f(x1, yb);
          glVertex2f(x1, yt);
          glVertex2f(x2, yb);
          glVertex2f(x2, yt);
          glColor4fv(transparent);
          glVertex2f(x3, yb);
          glVertex2f(x3, yt);
          glEnd();
          yb = yt+my;
        }

        for (size_t i=num_lower; i<num; ++i) {
          float const yt = yb+dy;
          glBegin(GL_TRIANGLE_STRIP);
          glColor4fv(transparent);
          glVertex2f(x0, yb);
          glVertex2f(x0, yt);
          glColor4fv(get_color(c.first).data());
          glVertex2f(x1, yb);
          glVertex2f(x1, yt);
          glVertex2f(x2, yb);
          glVertex2f(x2, yt);
          glColor4fv(transparent);
          glVertex2f(x3, yb);
          glVertex2f(x3, yt);
          glEnd();
          yb = yt+my;
        }
      }

      glPopName();
    }


    ////////////////////////////////////////////////////////////////////////////////
    void draw_fps(Vector2<float> const& scale) {

      using namespace sociarium_project_fps_manager;

      shared_ptr<FTFont> f = get_font(FontCategory::MISC);
      wstring const text = (boost::wformat(L"FPS=%.1f")%get_fps()).str();
      float llx, lly, llz, urx, ury, urz;
      f->BBox(text.c_str(), llx, lly, llz, urx, ury, urz);

      float const w = scale.x*(urx-llx);
      float const h = scale.y*(ury-lly);

      glPushMatrix();
      glTranslatef(1.0f-w-scale.x*get_frame_offset().x,
                   1.0f-h-scale.y*get_frame_offset().y, 0.0f);
      glScalef(scale.x, scale.y, 0.0f);
      glColor4fv(get_color(ColorCategory::FPS).data());
      f->Render(text.c_str());
      glPopMatrix();
    }


    ////////////////////////////////////////////////////////////////////////////////
    void draw_mouse_drag_region(void) {
      glColor4fv(get_color(ColorCategory::MOUSE_DRAG_REGION).data());
      glBegin(GL_TRIANGLE_STRIP);
      glVertex2dv(get_drag_region(0).data);
      glVertex2dv(get_drag_region(1).data);
      glVertex2dv(get_drag_region(2).data);
      glVertex2dv(get_drag_region(3).data);
      glEnd();
    }


    ////////////////////////////////////////////////////////////////////////////////
    void draw_thread_status(Vector2<float> const& scale) {

      using namespace sociarium_project_thread;

      Vector2<float> const sub_scale(0.8f*scale);
      shared_ptr<FTFont> f = get_font(FontCategory::MISC);

      static float const upper_margin = 4.0f;
      static float const lower_margin = 8.0f;

      float const interval = scale.y*4;

      struct StatusBlock {
        wchar_t const* status0;
        wchar_t const* status1;
        float w0, h0, w1, h1;
        float block_height;
        StatusBlock(void) : status0(0), status1(0),
        w0(0.0f), h0(0.0f), w1(0.0f), h1(0.0f), block_height(0.0f) {}
      };

      vector<StatusBlock> status_block;

      float whole_height = 0.0f;

      for (int i=0; i<NUMBER_OF_THREAD_CATEGORIES; ++i) {

        deque<wstring> const& status = get_status(i);

        if (status[0]!=L"") {

          StatusBlock sb;

          sb.status0 = status[0].c_str();

          float llx, lly, llz, urx, ury, urz;
          f->BBox(sb.status0, llx, lly, llz, urx, ury, urz);

          sb.w0 = scale.x*(urx-llx);
          sb.h0 = scale.y*(ury-lly);

          float middle_margin = 0.0f;

          if (!status[1].empty()) {

            sb.status1 = status[1].c_str();

            f->BBox(sb.status1, llx, lly, llz, urx, ury, urz);

            sb.w1 = sub_scale.x*(urx-llx);
            sb.h1 = sub_scale.y*(ury-lly);

            middle_margin = lower_margin;
          }

          sb.block_height = sb.h0+sb.h1+scale.y*(upper_margin+middle_margin+lower_margin);

          status_block.push_back(sb);

          whole_height += sb.block_height+interval;
        }
      }

      // Centering messages to the middle.
      float base = -0.5f*whole_height;

      for (size_t i=0; i<status_block.size(); ++i) {

        StatusBlock const& sb = status_block[i];

        float const bottom = 0.5f+base;
        float const top    = bottom+sb.block_height;

        base += sb.block_height+interval;

        // --------------------------------------------------------------------------------
        // Draw a frame.
        glColor4fv(get_color(ColorCategory::THREAD_MESSAGE_FRAME).data());
        glBegin(GL_TRIANGLE_STRIP);
        glVertex2f(-0.1f, bottom);
        glVertex2f(-0.1f, top);
        glVertex2f(+1.1f, bottom);
        glVertex2f(+1.1f, top);
        glEnd();

        glColor4fv(get_color(ColorCategory::THREAD_MESSAGE).data());

        // --------------------------------------------------------------------------------
        // Draw a main message.
        glPushMatrix();
        glTranslatef(0.5f*(1.0f-sb.w0), top-sb.h0-scale.y*upper_margin, 0.0f);
        glScalef(scale.x, scale.y, 0.0f);
        f->Render(sb.status0);
        glPopMatrix();

        if (sb.status1) {
          // Draw a sub message.
          glPushMatrix();
          glTranslatef(0.5f*(1.0f-sb.w1), bottom+scale.y*lower_margin, 0.0f);
          glScalef(sub_scale.x, sub_scale.y, 0.0f);
          f->Render(sb.status1);
          glPopMatrix();
        }
      }
    }


    ////////////////////////////////////////////////////////////////////////////////
    void draw_community_transition_diagram(Vector2<float> const& scale) {

      GLint mode;
      glGetIntegerv(GL_RENDER_MODE, &mode);

      shared_ptr<CommunityTransitionDiagram> diagram
        = sociarium_project_community_transition_diagram::get();

      assert(diagram!=0);

      if (diagram->empty()) return;

      using namespace sociarium_project_community_transition_diagram;

      Vector2<float> const& pos  = get_frame_position();
      Vector2<float> const& size = get_frame_size();

      size_t const resolution = get_resolution();
      pair<int, int> const& scope = get_scope();
      size_t const number_of_points = scope.second-scope.first;
      float const dx = size.x/number_of_points;
      int const mid_point = (scope.first+scope.second)/2;

      diagram->read_lock();
      /*
       * Don't forget to call read_unlock().
       */

      float const scy = size.y*(diagram->get_max_bundle_size()>0
                                ?1.0f/diagram->get_max_bundle_size():0.0f);

      glPushMatrix();
      glTranslatef(0.0f, pos.y+0.5f*size.y, 0.0f);
      glLoadName(SelectionCategory::TRAJECTORY);

      float y_max = 0.0f;
      float y_min = 1000.0f;

      shared_ptr<SociariumGraphTimeSeries> ts
        = sociarium_project_graph_time_series::get();

      // Communities where the selected node belongs to in each layer.
      vector<vector<StaticNodeProperty const*> > upper(ts->number_of_layers());

      if (sociarium_project_selection::is_selected(SelectionCategory::NODE)) {
        StaticNodeProperty const* snp = static_cast<StaticNodeProperty const*>(get_selected_static_object());

        typedef StaticNodeProperty::DynamicPropertyMap DynamicPropertyMap;

        DynamicPropertyMap::const_iterator j = snp->dynamic_property_begin();
        DynamicPropertyMap::const_iterator jend = snp->dynamic_property_end();

        for (; j!=jend; ++j) {

          vector<DynamicNodeProperty*>::const_iterator k
            = j->first->upper_nbegin();
          vector<DynamicNodeProperty*>::const_iterator kend
            = j->first->upper_nend();

          for (; k!=kend; ++k)
            upper[j->second].push_back((*k)->get_static_property());
        }
      }

      // Start drawing.
      vector<shared_ptr<Trajectory> >::const_iterator i
        = diagram->trajectory_begin();
      vector<shared_ptr<Trajectory> >::const_iterator end
        = diagram->trajectory_end();

      for (; i!=end; ++i) {

        pair<int, int> const duration(
          int(resolution*(*i)->get_duration().first),
          int(resolution*(*i)->get_duration().second));

        if (scope.first>=duration.second || scope.second<=duration.first)
          continue;

        StaticNodeProperty const* snp = (*i)->get_snp();

        // The indices of interpolated points in both ends of the diagram scope.
        int const lhs = scope.first>duration.first?scope.first:duration.first;
        int const rhs = scope.second<duration.second?scope.second:duration.second;

        assert(lhs>=0);
        assert(lhs<=rhs);

        float x = pos.x+(lhs-scope.first)*dx;

        // Both ends of the diagram scope fade out.
        float const d_fade = 0.1f*size.x;
        float const x_fade0 = pos.x+d_fade;
        float const x_fade1 = pos.x+size.x-d_fade;

        glPushName(snp->get_id());
        glBegin(GL_TRIANGLE_STRIP);

        for (int j=lhs; j<=rhs; ++j) {

          pair<float, float> y = (*i)->get_interpolated_position(j);

          if (j==mid_point) {
            if (y_max<y.second) y_max = y.second;
            if (y_min>y.first) y_min = y.first;
          }

          int const index_of_layer = j/get_resolution();

          {
            Vector3<float> rgb = (*i)->get_interpolated_color(j);

            using namespace sociarium_project_selection;

            bool b = false;

            for (size_t k=0; k<upper[index_of_layer].size(); ++k) {
              if (upper[index_of_layer][k]==snp) {
                b = true;
                break;
              }
            }

            if (is_selected(snp) || b) {
              rgb.x *= 0.4f;
              rgb.y *= 0.4f;
              rgb.z *= 0.4f;

              if (sociarium_project_community_transition_diagram::get_diagram_mode()!=0
                  && !is_selected(SelectionCategory::TRAJECTORY)) {
                y.first  -= 2.0f;
                y.second += 2.0f;
              }
            }

            if (x_fade0<x && x<x_fade1)
              glColor3fv(rgb.data);
            else {
              float const alpha = 1.0f-(x<=x_fade0?x_fade0-x:x-x_fade1)/d_fade;
              glColor4f(rgb.x, rgb.y, rgb.z, alpha);
            }
          }

          glVertex2f(x, scy*y.first);
          glVertex2f(x, scy*y.second);
          x += dx;
        }

        glEnd();
        glPopName();
#if 0
        {
          if (mode==GL_RENDER) {
            x = pos.x+(lhs-scope.first)*dx;
            glPointSize(3.0);
            glBegin(GL_POINTS);
            for (size_t j=lhs; j<=rhs; ++j) {
              pair<float, float> const& y = (*i)->get_interpolated_position(j);
              glVertex2f(x, scy*y.first);
              glVertex2f(x, scy*y.second);
              x += dx;
            }
            glEnd();
          }
        }
#endif
      }

      if (mode==GL_RENDER && !diagram->empty()) {
        if (y_min<y_max) {
          static float const w = 0.005f;
          glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
          glBegin(GL_TRIANGLE_STRIP);
          glVertex2f(0.5f-w, scy*y_min);
          glVertex2f(0.5f-w, scy*y_max);
          glVertex2f(0.5f+w, scy*y_min);
          glVertex2f(0.5f+w, scy*y_max);
          glEnd();
        }
      }

      glPopMatrix();

      diagram->read_unlock();
    }


    ////////////////////////////////////////////////////////////////////////////////
    void draw_community_transition_diagram_frame(Vector2<float> const& scale) {

      using namespace sociarium_project_community_transition_diagram;

      Vector2<float> const& pos  = get_frame_position();
      Vector2<float> const& size = get_frame_size();

      // Drawing area.
      glColor3fv(predefined_color[PredefinedColor::WHITE].data);
      glLoadName(SelectionCategory::DIAGRAM_FRAME);
      glPushName(0);
      glBegin(GL_TRIANGLE_STRIP);
      glVertex2f(pos.x, pos.y);
      glVertex2f(pos.x, pos.y+size.y);
      glVertex2f(pos.x+size.x, pos.y);
      glVertex2f(pos.x+size.x, pos.y+size.y);
      glEnd();
      glPopName();

      glColor4f(1.0f, 1.0f, 1.0f, 0.0f);

      // Top edge of the frame.
      glLoadName(SelectionCategory::DIAGRAM_FRAME_BORDER);
      glPushName(SelectionCategory::FrameBorder::TOP);
      glBegin(GL_LINES);
      glVertex2f(pos.x, pos.y+size.y);
      glVertex2f(pos.x+size.x, pos.y+size.y);
      glEnd();
      glPopName();
      // Bottom edge of the frame.
      glPushName(SelectionCategory::FrameBorder::BOTTOM);
      glBegin(GL_LINES);
      glVertex2f(pos.x, pos.y);
      glVertex2f(pos.x+size.x, pos.y);
      glEnd();
      glPopName();
      // Right edge of the frame.
      glPushName(SelectionCategory::FrameBorder::RIGHT);
      glBegin(GL_LINES);
      glVertex2f(pos.x+size.x, pos.y);
      glVertex2f(pos.x+size.x, pos.y+size.y);
      glEnd();
      glPopName();
      // Left edge of the frame.
      glPushName(SelectionCategory::FrameBorder::LEFT);
      glBegin(GL_LINES);
      glVertex2f(pos.x, pos.y);
      glVertex2f(pos.x, pos.y+size.y);
      glEnd();
      glPopName();

      // Top-right corner of the frame.
      glPushName(SelectionCategory::FrameBorder::TOP_RIGHT);
      glBegin(GL_LINE_STRIP);
      glVertex2f(pos.x+size.x-3*scale.x, pos.y+size.y);
      glVertex2f(pos.x+size.x, pos.y+size.y);
      glVertex2f(pos.x+size.x, pos.y+size.y-3*scale.y);
      glEnd();
      glPopName();
      // Top-left corner of the frame.
      glPushName(SelectionCategory::FrameBorder::TOP_LEFT);
      glBegin(GL_LINE_STRIP);
      glVertex2f(pos.x, pos.y+size.y-3*scale.y);
      glVertex2f(pos.x, pos.y+size.y);
      glVertex2f(pos.x+3*scale.x, pos.y+size.y);
      glEnd();
      glPopName();
      // Bottom-right corner of the frame.
      glPushName(SelectionCategory::FrameBorder::BOTTOM_RIGHT);
      glBegin(GL_LINE_STRIP);
      glVertex2f(pos.x+size.x-3*scale.x, pos.y);
      glVertex2f(pos.x+size.x, pos.y);
      glVertex2f(pos.x+size.x, pos.y+3*scale.y);
      glEnd();
      glPopName();
      // Bottom-left corner of the frame.
      glPushName(SelectionCategory::FrameBorder::BOTTOM_LEFT);
      glBegin(GL_LINE_STRIP);
      glVertex2f(pos.x, pos.y+3*scale.y);
      glVertex2f(pos.x, pos.y);
      glVertex2f(pos.x+3*scale.x, pos.y);
      glEnd();
      glPopName();
    }

  } // The end of the namespace "sociarium_project_draw_detail"

} // The end of the namespace "hashimoto_ut"

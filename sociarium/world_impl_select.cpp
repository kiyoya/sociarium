// s.o.c.i.a.r.i.u.m: world_impl_select.cpp
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

#include <cassert>
#include <memory>
#include <windows.h>
#include <GL/glu.h>
#include "community_transition_diagram.h"
#include "selection.h"
#include "sociarium_graph_time_series.h"
#include "world_impl.h"
#include "../shared/gl/glview.h"

namespace hashimoto_ut {

  using std::tr1::shared_ptr;

  namespace {
    int const SELECTION_BUFFER = 1000;
    GLuint selection_buffer[SELECTION_BUFFER];
  }


  ////////////////////////////////////////////////////////////////////////////////
  void WorldImpl::select(Vector2<int> const& mpos) const {

    // --------------------------------------------------------------------------------
    // Prepare the selection buffer.
    glSelectBuffer(SELECTION_BUFFER, selection_buffer);
    /* This should be done before calling glRenderMode(GL_SELECT).
     */

    // --------------------------------------------------------------------------------
    // Switch to the seleciton mode.
    glRenderMode(GL_SELECT);
    glInitNames();
    glPushName(-1);
    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);

    // --------------------------------------------------------------------------------
    // Selection in the perspective drawing part.
    view_->push_matrix();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPickMatrix(mpos.x, mpos.y, 3.0, 3.0, vp);
    draw_perspective_part();
    view_->pop_matrix();

    // --------------------------------------------------------------------------------
    // Selection in the orthogonal drawing part.
    view_->push_matrix();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPickMatrix(mpos.x, mpos.y, 3.0, 3.0, vp);
    draw_orthogonal_part();
    view_->pop_matrix();

    // --------------------------------------------------------------------------------
    // Switch to the rendering mode.
    GLint const hits = glRenderMode(GL_RENDER);
    GLuint const* buf = selection_buffer;
    assert(hits<SELECTION_BUFFER);

    // --------------------------------------------------------------------------------
    // Parse the result.
    if (hits>0) {
      using namespace sociarium_project_selection;

      shared_ptr<Selection> s = get();
      s->set_level(buf[0]);
      s->set_depth_min(double(buf[1])/double(0x7fffffff));
      s->set_depth_max(double(buf[2])/double(0x7fffffff));
      s->set_category(buf[5*(hits-1)+3]);
      s->set_id(buf[5*(hits-1)+4]);

      if (is_selected(SelectionCategory::NODE)) {
        shared_ptr<SociariumGraphTimeSeries> ts = sociarium_project_graph_time_series::get();
        shared_ptr<SociariumGraph> g = ts->get_graph(0, ts->index_of_current_layer());
        Node* n = g->node(s->get_id());
        DynamicNodeProperty const& dnp = g->property(n);

        set_selected_object((void*)n);
        set_selected_dynamic_object((void*)&dnp);
        set_selected_static_object((void*)dnp.get_static_property());
      }

      else if (is_selected(SelectionCategory::EDGE)) {
        shared_ptr<SociariumGraphTimeSeries> ts = sociarium_project_graph_time_series::get();
        shared_ptr<SociariumGraph> g = ts->get_graph(0, ts->index_of_current_layer());
        Edge* e = g->edge(s->get_id());
        DynamicEdgeProperty const& dep = g->property(e);

        set_selected_object((void*)e);
        set_selected_dynamic_object((void*)&dep);
        set_selected_static_object((void*)dep.get_static_property());
      }

      else if (is_selected(SelectionCategory::COMMUNITY)) {
        shared_ptr<SociariumGraphTimeSeries> ts = sociarium_project_graph_time_series::get();
        shared_ptr<SociariumGraph> g = ts->get_graph(1, ts->index_of_current_layer());
        Node* n = g->node(s->get_id());
        DynamicNodeProperty const& dnp = g->property(n);

        set_selected_object((void*)n);
        set_selected_dynamic_object((void*)&dnp);
        set_selected_static_object((void*)dnp.get_static_property());
      }

      else if (is_selected(SelectionCategory::COMMUNITY_EDGE)) {
        shared_ptr<SociariumGraphTimeSeries> ts = sociarium_project_graph_time_series::get();
        shared_ptr<SociariumGraph> g = ts->get_graph(1, ts->index_of_current_layer());
        Edge* e = g->edge(s->get_id());
        DynamicEdgeProperty const& dep = g->property(e);

        set_selected_object((void*)e);
        set_selected_dynamic_object((void*)&dep);
        set_selected_static_object((void*)dep.get_static_property());
      }

      else if (is_selected(SelectionCategory::TRAJECTORY)) {
        using namespace sociarium_project_community_transition_diagram;
        shared_ptr<SociariumGraphTimeSeries> ts = sociarium_project_graph_time_series::get();
        int const level = get_diagram_mode()!=0?0:1;
        SociariumGraphTimeSeries::StaticNodePropertySet::const_iterator i
          = ts->find_static_node(level, s->get_id());
        assert(i!=ts->static_node_property_end(level));
        set_selected_static_object((void*)&*i);
      }
#if 0
       if (s->get_category()==SelectionCategory::DIAGRAM_FRAME_BORDER) {
         if (s->get_id()==SelectionCategory::FrameBorder::TOP ||
             s->get_id()==SelectionCategory::FrameBorder::BOTTOM) {
           HCURSOR hcur = HCURSOR(LoadImage(NULL, IDC_SIZENS, IMAGE_CURSOR, 0, 0, LR_SHARED|LR_DEFAULTSIZE));
           SetCursor(hcur);
         } else if (s->get_id()==SelectionCategory::FrameBorder::RIGHT ||
                    s->get_id()==SelectionCategory::FrameBorder::LEFT) {
           HCURSOR hcur = HCURSOR(LoadImage(NULL, IDC_SIZEWE, IMAGE_CURSOR, 0, 0, LR_SHARED|LR_DEFAULTSIZE));
           SetCursor(hcur);
         } else if (s->get_id()==SelectionCategory::FrameBorder::TOP_RIGHT ||
                    s->get_id()==SelectionCategory::FrameBorder::BOTTOM_LEFT) {
           HCURSOR hcur = HCURSOR(LoadImage(NULL, IDC_SIZENESW, IMAGE_CURSOR, 0, 0, LR_SHARED|LR_DEFAULTSIZE));
           SetCursor(hcur);
         } else if (s->get_id()==SelectionCategory::FrameBorder::TOP_LEFT ||
                    s->get_id()==SelectionCategory::FrameBorder::BOTTOM_RIGHT) {
           HCURSOR hcur = HCURSOR(LoadImage(NULL, IDC_SIZENWSE, IMAGE_CURSOR, 0, 0, LR_SHARED|LR_DEFAULTSIZE));
           SetCursor(hcur);
         }
       } else if (s->get_category()==SelectionCategory::LAYOUT_FRAME) {
         HCURSOR hcur = HCURSOR(LoadImage(NULL, IDC_SIZEALL, IMAGE_CURSOR, 0, 0, LR_SHARED|LR_DEFAULTSIZE));
         SetCursor(hcur);
       }
#endif
    } else {
      sociarium_project_selection::clear();
#if 0
      HCURSOR hcur = HCURSOR(LoadImage(NULL, IDC_ARROW, IMAGE_CURSOR, 0, 0, LR_SHARED|LR_DEFAULTSIZE));
      SetCursor(hcur);
#endif
    }
  }

} // The end of the namespace "hashimoto_ut"

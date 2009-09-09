// s.o.c.i.a.r.i.u.m
// HASHIMOTO, Yasuhiro (E-mail: hy @ sys.t.u-tokyo.ac.jp)

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

#include "mouse_and_selection.h"
#include "../graph/graph.h"

namespace hashimoto_ut {

  using std::tr1::shared_ptr;

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  Selection::Selection(void) { initialize(); }
  Selection::~Selection() {}
  unsigned int Selection::get_level(void) const { return level_; }
  void Selection::set_level(unsigned int level) { level_ = level; }
  double Selection::get_depth_min(void) const { return depth_min_; }
  void Selection::set_depth_min(double depth_min) { depth_min_ = depth_min; }
  double Selection::get_depth_max(void) const { return depth_max_; }
  void Selection::set_depth_max(double depth_max) { depth_max_ = depth_max; }
  unsigned int Selection::get_category(void) const { return category_; }
  void Selection::set_category(unsigned int category) { category_ = category; }
  unsigned int Selection::get_id(void) const { return id_; }
  void Selection::set_id(unsigned int id) { id_ = id; }

  void Selection::initialize(void) {
    level_     = unsigned int(-1);
    depth_min_ = 0.0;
    depth_max_ = 0.0;
    category_  = unsigned int(-1);
    id_        = unsigned int(-1);
  }

  namespace sociarium_project_mouse_and_selection {

    namespace {
      ////////////////////////////////////////////////////////////////////////////////////////////////////
      // ローカル変数
      Vector2<double> drag_region[4];
      bool drag_status = false;
      shared_ptr<Selection> selection;
      void* captured_object = 0;
    }

    const shared_ptr<Selection>& get_selection(void) { return selection; }
    void create_selection(void) { selection.reset(new Selection); }
    void initialize_selection(void) { selection->initialize(); captured_object = 0; }

    bool node_is_selected(void) { return selection->get_category()==SelectionCategory::NODE; }
    bool edge_is_selected(void) { return selection->get_category()==SelectionCategory::EDGE; }
    bool community_is_selected(void) { return selection->get_category()==SelectionCategory::COMMUNITY; }
    bool community_edge_is_selected(void) { return selection->get_category()==SelectionCategory::COMMUNITY_EDGE; }

    bool node_is_selected(Node const* n) { return node_is_selected()&&selection->get_id()==n->index(); }
    bool edge_is_selected(Edge const* e) { return edge_is_selected()&&selection->get_id()==e->index(); }
    bool community_is_selected(Node const* n) { return community_is_selected()&&selection->get_id()==n->index(); }
    bool community_edge_is_selected(Edge const* e) { return community_edge_is_selected()&&selection->get_id()==e->index(); }

    bool time_slider_is_selected(void) { return selection->get_category()==SelectionCategory::TIME_SLIDER; }
    bool layer_label_is_selected(void) { return selection->get_category()==SelectionCategory::LAYER_LABEL; }
    bool layout_frame_is_selected(void) { return selection->get_category()==SelectionCategory::LAYOUT_FRAME; }
    bool layout_frame_border_is_selected(void) { return selection->get_category()==SelectionCategory::LAYOUT_FRAME_BORDER; }

    Vector2<double> const& get_drag_region(int index) { return drag_region[index]; }
    void set_drag_region(int index, Vector2<double> const& pos) { drag_region[index] = pos; }

    const bool get_drag_status(void) { return drag_status; }
    void set_drag_status(bool status) { drag_status = status; }

    void* get_captured_object(void) { return captured_object; }
    void set_captured_object(void* p) { captured_object = p; }

  } // The end of the namespace "sociarium_project_mouse_and_selection"

} // The end of the namespace "hashimoto_ut"

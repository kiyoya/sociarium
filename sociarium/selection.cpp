// s.o.c.i.a.r.i.u.m: selection.cpp
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
#include "selection.h"
#include "../graph/graph.h"

namespace hashimoto_ut {

  using std::tr1::shared_ptr;

  ////////////////////////////////////////////////////////////////////////////////
  Selection::Selection(void) {
    clear();
  }

  Selection::~Selection() {}

  unsigned int Selection::get_level(void) const {
    return level_;
  }

  void Selection::set_level(unsigned int level) {
    level_ = level;
  }

  double Selection::get_depth_min(void) const {
    return depth_min_;
  }

  void Selection::set_depth_min(double depth_min) {
    depth_min_ = depth_min;
  }

  double Selection::get_depth_max(void) const {
    return depth_max_;
  }

  void Selection::set_depth_max(double depth_max) {
    depth_max_ = depth_max;
  }

  unsigned int Selection::get_category(void) const {
    return category_;
  }

  void Selection::set_category(unsigned int category) {
    category_ = category;
  }

  unsigned int Selection::get_id(void) const {
    return id_;
  }

  void Selection::set_id(unsigned int id) {
    id_ = id;
  }

  void Selection::clear(void) {
    level_     = unsigned int(-1);
    depth_min_ = 0.0;
    depth_max_ = 0.0;
    category_  = unsigned int(-1);
    id_        = unsigned int(-1);
  }


  namespace sociarium_project_selection {

    namespace {
      Vector2<double> drag_region[4];
      bool drag_status = false;
      shared_ptr<Selection> selection;
      void* captured_object = 0;
      void* selected_object = 0;
      void* selected_dynamic_object = 0;
      void* selected_static_object = 0;
    }

    ////////////////////////////////////////////////////////////////////////////////
    shared_ptr<Selection> get(void) {
      return selection;
    }

    void initialize(void) {
      selection.reset(new Selection);
    }

    void clear(void) {
      selection->clear();
      captured_object = 0;
      selected_object = 0;
      selected_dynamic_object = 0;
      selected_static_object = 0;
    }


    ////////////////////////////////////////////////////////////////////////////////
    bool is_selected(int category) {
      assert(0<=category && category<SelectionCategory::NUMBER_OF_CATEGORIES);
      return selection->get_category()==category;
    }

    bool is_selected(Node const* n) {
      return selected_object==n;
    }

    bool is_selected(Edge const* e) {
      return selected_object==e;
    }

    bool is_selected(DynamicNodeProperty const* dnp) {
      return selected_dynamic_object==dnp;
    }

    bool is_selected(DynamicEdgeProperty const* dep) {
      return selected_dynamic_object==dep;
    }

    bool is_selected(StaticNodeProperty const* snp) {
      return selected_static_object==snp;
    }

    bool is_selected(StaticEdgeProperty const* sep) {
      return selected_static_object==sep;
    }


    ////////////////////////////////////////////////////////////////////////////////
    Vector2<double> const& get_drag_region(int index) {
      assert(0<=index && index<4);
      return drag_region[index];
    }

    void set_drag_region(int index, Vector2<double> const& pos) {
      assert(0<=index && index<4);
      drag_region[index] = pos;
    }

    bool get_drag_status(void) {
      return drag_status;
    }

    void set_drag_status(bool status) {
      drag_status = status;
    }


    ////////////////////////////////////////////////////////////////////////////////
    void* get_captured_object(void) {
      return captured_object;
    }

    void set_captured_object(void* p) {
      captured_object = p;
    }

    void* get_selected_object(void) {
      return selected_object;
    }

    void set_selected_object(void* p) {
      selected_object = p;
    }

    void* get_selected_dynamic_object(void) {
      return selected_dynamic_object;
    }

    void set_selected_dynamic_object(void* p) {
      selected_dynamic_object = p;
    }

    void* get_selected_static_object(void) {
      return selected_static_object;
    }

    void set_selected_static_object(void* p) {
      selected_static_object = p;
    }

  } // The end of the namespace "sociarium_project_selection"

} // The end of the namespace "hashimoto_ut"

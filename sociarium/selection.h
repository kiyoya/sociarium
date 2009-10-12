// s.o.c.i.a.r.i.u.m: selection.h
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

#ifndef INCLUDE_GUARD_SOCIARIUM_PROJECT_SELECTION_H
#define INCLUDE_GUARD_SOCIARIUM_PROJECT_SELECTION_H

#include <memory>
#include "../shared/vector2.h"

namespace hashimoto_ut {

  class Node;
  class Edge;
  class DynamicNodeProperty;
  class DynamicEdgeProperty;
  class StaticNodeProperty;
  class StaticEdgeProperty;

  ////////////////////////////////////////////////////////////////////////////////
  namespace MouseAction {
    enum {
      LBUTTON_UP = 0,
      LBUTTON_DOWN,
      LBUTTON_DBL,
      RBUTTON_UP,
      RBUTTON_DOWN,
      RBUTTON_DBL,
      MBUTTON_UP,
      MBUTTON_DOWN,
      WHEEL,
      MOVE,
      NUMBER_OF_ACTIONS
    };
  }

  ////////////////////////////////////////////////////////////////////////////////
  namespace SelectionCategory {
    enum {
      NODE = 0,
      EDGE,
      COMMUNITY,
      COMMUNITY_EDGE,
      TIME_SLIDER,
      LAYER_NAME,
      LAYOUT_FRAME,
      LAYOUT_FRAME_BORDER,
      TRAJECTORY,
      DIAGRAM_FRAME,
      DIAGRAM_FRAME_BORDER,
      NUMBER_OF_CATEGORIES
    };

    namespace FrameBorder {
      enum {
        TOP,
        BOTTOM,
        RIGHT,
        LEFT,
        TOP_RIGHT,
        TOP_LEFT,
        BOTTOM_RIGHT,
        BOTTOM_LEFT,
      };
    }
  }

  ////////////////////////////////////////////////////////////////////////////////
  class Selection {
  public:
    Selection(void);
    ~Selection();
    unsigned int get_level(void) const;
    void set_level(unsigned int level);
    double get_depth_min(void) const;
    void set_depth_min(double depth_min);
    double get_depth_max(void) const;
    void set_depth_max(double depth_max);
    unsigned int get_category(void) const;
    void set_category(unsigned int category);
    unsigned int get_id(void) const;
    void set_id(unsigned int id);
    void clear(void);

  private:
    unsigned int level_;
    double depth_min_;
    double depth_max_;
    unsigned int category_;
    unsigned int id_;
  };


  namespace sociarium_project_selection {

    ////////////////////////////////////////////////////////////////////////////////
    std::tr1::shared_ptr<Selection> get(void);
    void initialize(void);
    void clear(void);

    ////////////////////////////////////////////////////////////////////////////////
    bool is_selected(int category);

    bool is_selected(Node const* n);
    bool is_selected(Edge const* e);

    bool is_selected(DynamicNodeProperty const* dnp);
    bool is_selected(DynamicEdgeProperty const* dep);

    bool is_selected(StaticNodeProperty const* snp);
    bool is_selected(StaticEdgeProperty const* sep);

    ////////////////////////////////////////////////////////////////////////////////
    Vector2<double> const& get_drag_region(int index);
    void set_drag_region(int index, Vector2<double> const& pos);

    bool get_drag_status(void);
    void set_drag_status(bool status);

    ////////////////////////////////////////////////////////////////////////////////
    void* get_captured_object(void);
    void set_captured_object(void* p);

    void* get_selected_object(void);
    void set_selected_object(void* p);

    void* get_selected_dynamic_object(void);
    void set_selected_dynamic_object(void* p);

    void* get_selected_static_object(void);
    void set_selected_static_object(void* p);
    
  } // The end of the namespace "sociarium_project_selection"

} // The end of the namespace "hashimoto_ut"

#endif // INCLUDE_GUARD_SOCIARIUM_PROJECT_SELECTION_H

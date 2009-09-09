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

#ifndef INCLUDE_GUARD_SOCIARIUM_PROJECT_MOUSE_AND_SELECTION_H
#define INCLUDE_GUARD_SOCIARIUM_PROJECT_MOUSE_AND_SELECTION_H

#ifdef _MSC_VER
#include <memory>
#else
#include <tr1/memory>
#endif
#include "../shared/vector2.h"

namespace hashimoto_ut {

  ///////////////////////////////////////////////////////////////////////////////////////////////////
  struct MouseAction {
    enum {
      LBUTTON_UP = 0,
      LBUTTON_DOWN,
      LBUTTON_DBL,
      LBUTTON_DRAG,
      RBUTTON_UP,
      RBUTTON_DOWN,
      RBUTTON_DBL,
      RBUTTON_DRAG,
      MBUTTON_UP,
      MBUTTON_DOWN,
      WHEEL,
      MOVE,
      SIZE
    };
  };

  struct MouseModifier {
    enum {
      NONE = 0,
      CONTROL,
      SHIFT,
    };
  };

  ///////////////////////////////////////////////////////////////////////////////////////////////////
  struct SelectionCategory {
    enum {
      NODE = 0,
      EDGE,
      COMMUNITY,
      COMMUNITY_EDGE,
      TIME_SLIDER,
      LAYER_LABEL,
      LAYOUT_FRAME,
      LAYOUT_FRAME_BORDER,
      SIZE
    };
  };

  ///////////////////////////////////////////////////////////////////////////////////////////////////
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
    void initialize(void);

  private:
    unsigned int level_;    // ピクセル階層
    double depth_min_;      // ピクセル階層の最も手前のdepth値
    double depth_max_;      // ピクセル階層の最も奥のdepth値
    unsigned int category_; // セレクション対象のカテゴリー
    unsigned int id_;       // セレクション対象のID
  };

  class Node;
  class Edge;

  ///////////////////////////////////////////////////////////////////////////////////////////////////
  namespace sociarium_project_mouse_and_selection {

    std::tr1::shared_ptr<Selection> const& get_selection(void);
    void create_selection(void);
    void initialize_selection(void);

    bool node_is_selected(void);
    bool edge_is_selected(void);
    bool community_is_selected(void);
    bool community_edge_is_selected(void);

    bool node_is_selected(Node const* n);
    bool edge_is_selected(Edge const* e);
    bool community_is_selected(Node const* n);
    bool community_edge_is_selected(Edge const* e);

    bool time_slider_is_selected(void);
    bool layer_label_is_selected(void);
    bool layout_frame_is_selected(void);
    bool layout_frame_border_is_selected(void);

    Vector2<double> const& get_drag_region(int index);
    void set_drag_region(int index, Vector2<double> const& pos);

    const bool get_drag_status(void);
    void set_drag_status(bool status);

    void* get_captured_object(void);
    void set_captured_object(void* p);

  } // The end of the namespace "sociarium_project_mouse_and_selection"

} // The end of the namespace "hashimoto_ut"

#endif // INCLUDE_GUARD_SOCIARIUM_PROJECT_MOUSE_AND_SELECTION_H

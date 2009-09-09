// s.o.c.i.a.r.i.u.m: view.h
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

#ifndef INCLUDE_GUARD_SOCIARIUM_PROJECT_VIEW_H
#define INCLUDE_GUARD_SOCIARIUM_PROJECT_VIEW_H

#include "../shared/vector2.h"

namespace hashimoto_ut {

  ////////////////////////////////////////////////////////////////////////////////
  namespace NodeStyle {
    enum {
      POLYGON = 0x01,
      TEXTURE = 0x02,
      MASK1   = 0x03,
      MASK2   = 0x04
    };
  }

  ////////////////////////////////////////////////////////////////////////////////
  namespace EdgeStyle {
    enum {
      LINE     = 0x01,
      POLYGON  = 0x02,
      MASK1    = 0x03,
      MASK2    = 0x04
    };
  }

  ////////////////////////////////////////////////////////////////////////////////
  namespace CommunityStyle {
    enum {
      POLYGON_CIRCLE = 0x01,
      TEXTURE        = 0x02,
      POLYGON_CURVE  = 0x04,
      MASK1          = 0x07,
      MASK2          = 0x08
    };
  }

  ////////////////////////////////////////////////////////////////////////////////
  namespace CommunityEdgeStyle {
    enum {
      LINE     = 0x01,
      POLYGON  = 0x02,
      MASK1    = 0x03,
      MASK2    = 0x04
    };
  }

  namespace sociarium_project_view {

    namespace {
      // Initial distance between the center of the world and the eyepoint.
      float const VIEW_DISTANCE = 100.0f;
      float const VIEW_DISTANCE_MAX = 500.0f;
      float const VIEW_DISTANCE_MIN = 1.0f;
    }

    ////////////////////////////////////////////////////////////////////////////////
    // Draw or not each graph element.
    bool get_show_node(void);
    void set_show_node(bool b);

    bool get_show_edge(void);
    void set_show_edge(bool b);
    
    bool get_show_community(void);
    void set_show_community(bool b);

    bool get_show_community_edge(void);
    void set_show_community_edge(bool b);

    ////////////////////////////////////////////////////////////////////////////////
    // Draw or not the name of each graph element.
    bool get_show_node_name(void);
    void set_show_node_name(bool b);
    
    bool get_show_edge_name(void);
    void set_show_edge_name(bool b);
    
    bool get_show_community_name(void);
    void set_show_community_name(bool b);
    
    bool get_show_community_edge_name(void);
    void set_show_community_edge_name(bool b);

    ////////////////////////////////////////////////////////////////////////////////
    // The drawing size of the name is variable or not.
    bool get_node_name_size_variable(void);
    void set_node_name_size_variable(bool b);

    bool get_edge_name_size_variable(void);
    void set_edge_name_size_variable(bool b);

    bool get_community_name_size_variable(void);
    void set_community_name_size_variable(bool b);

    bool get_community_edge_name_size_variable(void);
    void set_community_edge_name_size_variable(bool b);

    ////////////////////////////////////////////////////////////////////////////////
    // The drawing style of each graph element.
    unsigned int get_node_style(void);
    void set_node_style(unsigned int value);
    void shift_node_style(void);

    unsigned int get_edge_style(void);
    void set_edge_style(unsigned int value);
    void shift_edge_style(void);

    unsigned int get_community_style(void);
    void set_community_style(unsigned int value);
    void shift_community_style(void);

    unsigned int get_community_edge_style(void);
    void set_community_edge_style(unsigned int value);
    void shift_community_edge_style(void);

    ////////////////////////////////////////////////////////////////////////////////
    // Draw or not other elements.
    bool get_show_slider(void);
    void set_show_slider(bool b);

    bool get_show_grid(void);
    void set_show_grid(bool b);
    
    bool get_show_fps(void);
    void set_show_fps(bool b);
    
    bool get_show_center(void);
    void set_show_center(bool b);
    
    bool get_show_layer_name(void);
    void set_show_layer_name(bool b);

    bool get_show_layout_frame(void);
    void set_show_layout_frame(bool b);

    bool get_show_diagram(void);
    void set_show_diagram(bool b);

  } // The end of the namespace "sociarium_project_view"

} // The end of the namespace "hashimoto_ut"

#endif // INCLUDE_GUARD_SOCIARIUM_PROJECT_VIEW_H

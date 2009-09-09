// s.o.c.i.a.r.i.u.m: view.cpp
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

#include "view.h"

namespace hashimoto_ut {

  namespace sociarium_project_view {

    namespace {
      ////////////////////////////////////////////////////////////////////////////////
      // Default values.
      bool show_node = true;
      bool show_edge = true;
      bool show_community = true;
      bool show_community_edge = true;

      bool show_node_name = true;
      bool show_edge_name = false;
      bool show_community_name = true;
      bool show_community_edge_name = false;

      bool node_name_size_variable = true;
      bool edge_name_size_variable = true;
      bool community_name_size_variable = false;
      bool community_edge_name_size_variable = false;

      unsigned int node_style = NodeStyle::TEXTURE;
      unsigned int edge_style = EdgeStyle::POLYGON;
      unsigned int community_style = CommunityStyle::TEXTURE;
      unsigned int community_edge_style = CommunityEdgeStyle::POLYGON;

      bool show_slider = true;
      bool show_grid = false;
      bool show_fps = false;
      bool show_center = false;
      bool show_layer_name = true;
      bool show_layout_frame = false;
      bool show_diagram = false;
    }


    ////////////////////////////////////////////////////////////////////////////////
    bool get_show_node(void) {
      return show_node;
    }

    void set_show_node(bool b) {
      show_node = b;
    }

    bool get_show_edge(void) {
      return show_edge;
    }

    void set_show_edge(bool b) {
      show_edge = b;
    }

    bool get_show_community(void) {
      return show_community;
    }

    void set_show_community(bool b) {
      show_community = b;
    }

    bool get_show_community_edge(void) {
      return show_community_edge;
    }

    void set_show_community_edge(bool b) {
      show_community_edge = b;
    }


    ////////////////////////////////////////////////////////////////////////////////
    bool get_show_node_name(void) {
      return show_node_name;
    }

    void set_show_node_name(bool b) {
      show_node_name = b;
    }

    bool get_show_edge_name(void) {
      return show_edge_name;
    }

    void set_show_edge_name(bool b) {
      show_edge_name = b;
    }

    bool get_show_community_name(void) {
      return show_community_name;
    }

    void set_show_community_name(bool b) {
      show_community_name = b;
    }

    bool get_show_community_edge_name(void) {
      return show_community_edge_name;
    }

    void set_show_community_edge_name(bool b) {
      show_community_edge_name = b;
    }


    ////////////////////////////////////////////////////////////////////////////////
    bool get_node_name_size_variable(void) {
      return node_name_size_variable;
    }

    void set_node_name_size_variable(bool b) {
      node_name_size_variable = b;
    }

    bool get_edge_name_size_variable(void) {
      return edge_name_size_variable;
    }

    void set_edge_name_size_variable(bool b) {
      edge_name_size_variable = b;
    }

    bool get_community_name_size_variable(void) {
      return community_name_size_variable;
    }

    void set_community_name_size_variable(bool b) {
      community_name_size_variable = b;
    }

    bool get_community_edge_name_size_variable(void) {
      return community_edge_name_size_variable;
    }

    void set_community_edge_name_size_variable(bool b) {
      community_edge_name_size_variable = b;
    }


    ////////////////////////////////////////////////////////////////////////////////
    unsigned int get_node_style(void) {
      return node_style;
    }

    unsigned int get_edge_style(void) {
      return edge_style;
    }

    unsigned int get_community_style(void) {
      return community_style;
    }
    unsigned int get_community_edge_style(void) {
      return community_edge_style;
    }

    void set_node_style(unsigned int value) {
      if (value<NodeStyle::MASK1) {
        unsigned int const s = node_style&NodeStyle::MASK2;
        node_style = value|s;
      }
    }

    void set_edge_style(unsigned int value) {
      if (value<EdgeStyle::MASK1) {
        unsigned int const s = edge_style&EdgeStyle::MASK2;
        edge_style = value|s;
      } else {
        unsigned int const s = edge_style&EdgeStyle::MASK1;
        edge_style = value|s;
      }
    }

    void set_community_style(unsigned int value) {
      if (value<CommunityStyle::MASK1) {
        unsigned int const s = community_style&CommunityStyle::MASK2;
        community_style = value|s;
      }
    }

    void set_community_edge_style(unsigned int value) {
      if (value<CommunityEdgeStyle::MASK1) {
        unsigned int const s = community_edge_style&CommunityEdgeStyle::MASK2;
        community_edge_style = value|s;
      } else {
        unsigned int const s = community_edge_style&CommunityEdgeStyle::MASK1;
        community_edge_style = value|s;
      }
    }

    void shift_node_style(void) {
      unsigned int s1 = (node_style&NodeStyle::MASK1)<<1;
      unsigned int s2 = (node_style&NodeStyle::MASK2);

      if (s1>NodeStyle::MASK1) {
        s1 = 1;
        unsigned int const s2x = s2<<1;
        if ((s2x&NodeStyle::MASK2)==0)
          s2 = NodeStyle::MASK1+1;
        else s2 = s2x;
      }

      node_style = s1+s2;
    }

    void shift_edge_style(void) {
      unsigned int s1 = (edge_style&EdgeStyle::MASK1)<<1;
      unsigned int s2 = (edge_style&EdgeStyle::MASK2);

      if (s1>EdgeStyle::MASK1) {
        s1 = 1;
        unsigned int const s2x = s2<<1;
        if ((s2x&EdgeStyle::MASK2)==0)
          s2 = EdgeStyle::MASK1+1;
        else s2 = s2x;
      }

      edge_style = s1+s2;
    }

    void shift_community_style(void) {
      unsigned int s1 = (community_style&CommunityStyle::MASK1)<<1;
      unsigned int s2 = (community_style&CommunityStyle::MASK2);

      if (s1>CommunityStyle::MASK1) {
        s1 = 1;
        unsigned int const s2x = s2<<1;
        if ((s2x&CommunityStyle::MASK2)==0)
          s2 = CommunityStyle::MASK1+1;
        else s2 = s2x;
      }

      community_style = s1+s2;
    }

    void shift_community_edge_style(void) {
      unsigned int s1 = (community_edge_style&CommunityEdgeStyle::MASK1)<<1;
      unsigned int s2 = (community_edge_style&CommunityEdgeStyle::MASK2);

      if (s1>CommunityEdgeStyle::MASK1) {
        s1 = 1;
        unsigned int const s2x = s2<<1;
        if ((s2x&CommunityEdgeStyle::MASK2)==0)
          s2 = CommunityEdgeStyle::MASK1+1;
        else s2 = s2x;
      }

      community_edge_style = s1+s2;
    }


    ////////////////////////////////////////////////////////////////////////////////
    bool get_show_slider(void) {
      return show_slider;
    }

    void set_show_slider(bool b) {
      show_slider = b;
    }

    bool get_show_grid(void) {
      return show_grid;
    }

    void set_show_grid(bool b) {
      show_grid = b;
    }

    bool get_show_fps(void) {
      return show_fps;
    }

    void set_show_fps(bool b) {
      show_fps = b;
    }

    bool get_show_center(void) {
      return show_center;
    }

    void set_show_center(bool b) {
      show_center = b;
    }

    bool get_show_layer_name(void) {
      return show_layer_name;
    }

    void set_show_layer_name(bool b) {
      show_layer_name = b;
    }

    bool get_show_layout_frame(void) {
      return show_layout_frame;
    }

    void set_show_layout_frame(bool b) {
      show_layout_frame = b;
    }

    bool get_show_diagram(void) {
      return show_diagram;
    }

    void set_show_diagram(bool b) {
      show_diagram = b;
    }

  } // The end of the namespace "sociarium_project_view"

} // The end of the namespace "hashimoto_ut"

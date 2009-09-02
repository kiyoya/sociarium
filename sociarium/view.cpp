// s.o.c.i.a.r.i.u.m
// HASHIMOTO, Yasuhiro (E-mail: hy @ sys.t.u-tokyo.ac.jp)
// update: 2009/05/10

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

#include "view.h"

namespace hashimoto_ut {

  namespace sociarium_project_view {

    namespace {
      ////////////////////////////////////////////////////////////////////////////////////////////////////
      // ローカル変数

      bool show_node = true;
      bool show_edge = true;
      bool show_community = true;
      bool show_community_edge = true;

      bool show_node_label = true;
      bool show_edge_label = true;
      bool show_community_label = true;
      bool show_community_edge_label = false;

      unsigned int node_style = NodeView::Style::TEXTURE;
      //unsigned int edge_style = EdgeView::Style::LINE|EdgeView::Style::HCURVE;
      unsigned int edge_style = EdgeView::Style::LINE;
      unsigned int community_style = CommunityView::Style::TEXTURE;
      //unsigned int community_edge_style = CommunityEdgeView::Style::LINE|CommunityEdgeView::Style::HCURVE;
      unsigned int community_edge_style = CommunityEdgeView::Style::LINE;

      unsigned int node_size_factor = NodeView::SizeFactor::DEGREE_CENTRALITY;

      bool show_slider = true;
			bool show_grid = false;
      bool show_fps = true;
      bool show_center = false;
      bool show_layout_frame = true;
      bool show_time_label = true;
    }

    // 描画のON/OFF
    bool get_show_node(void) { return show_node; }
    bool get_show_edge(void) { return show_edge; }
    bool get_show_community(void) { return show_community; }
    bool get_show_community_edge(void) { return show_community_edge; }
    void set_show_node(bool b) { show_node = b; }
    void set_show_edge(bool b) { show_edge = b; }
    void set_show_community(bool b) { show_community = b; }
    void set_show_community_edge(bool b) { show_community_edge = b; }

    // ラベル描画のON/OFF
    bool get_show_node_label(void) { return show_node_label; }
    bool get_show_edge_label(void) { return show_edge_label; }
    bool get_show_community_label(void) { return show_community_label; }
    bool get_show_community_edge_label(void) { return show_community_edge_label; }
    void set_show_node_label(bool b) { show_node_label = b; }
    void set_show_edge_label(bool b) { show_edge_label = b; }
    void set_show_community_label(bool b) { show_community_label = b; }
    void set_show_community_edge_label(bool b) { show_community_edge_label = b; }

    // 描画スタイル
    unsigned int get_node_style(void) { return node_style; }
    unsigned int get_edge_style(void) { return edge_style; }
    unsigned int get_community_style(void) { return community_style; }
    unsigned int get_community_edge_style(void) { return community_edge_style; }

    void set_node_style(unsigned int item) {
      if (item<NodeView::Style::MASK1) {
        unsigned int const s = node_style&NodeView::Style::MASK2;
        node_style = item|s;
      }
    }

    void set_edge_style(unsigned int item) {
      if (item<EdgeView::Style::MASK1) {
        unsigned int const s = edge_style&EdgeView::Style::MASK2;
        edge_style = item|s;
      } else {
        unsigned int const s = edge_style&EdgeView::Style::MASK1;
        edge_style = item|s;
      }
    }

    void set_community_style(unsigned int item) {
      if (item<CommunityView::Style::MASK1) {
        unsigned int const s = community_style&CommunityView::Style::MASK2;
        community_style = item|s;
      }
    }

    void set_community_edge_style(unsigned int item) {
      if (item<CommunityEdgeView::Style::MASK1) {
        unsigned int const s = community_edge_style&CommunityEdgeView::Style::MASK2;
        community_edge_style = item|s;
      } else {
        unsigned int const s = community_edge_style&CommunityEdgeView::Style::MASK1;
        community_edge_style = item|s;
      }
    }

    void shift_node_style(void) {
      unsigned int s1 = (node_style&NodeView::Style::MASK1)*2;
      unsigned int s2 = (node_style&NodeView::Style::MASK2);
      if (s1>NodeView::Style::MASK1) {
        s1 = 1;
        unsigned int const s2x = s2*2;
        if ((s2x&NodeView::Style::MASK2)==0) s2 = NodeView::Style::MASK1+1;
        else s2 = s2x;
      }
      node_style = s1+s2;
    }

    void shift_edge_style(void) {
      unsigned int s1 = (edge_style&EdgeView::Style::MASK1)*2;
      unsigned int s2 = (edge_style&EdgeView::Style::MASK2);
      if (s1>EdgeView::Style::MASK1) {
        s1 = 1;
        unsigned int const s2x = s2*2;
        if ((s2x&EdgeView::Style::MASK2)==0) s2 = EdgeView::Style::MASK1+1;
        else s2 = s2x;
      }
      edge_style = s1+s2;
    }

    void shift_community_style(void) {
      unsigned int s1 = (community_style&CommunityView::Style::MASK1)*2;
      unsigned int s2 = (community_style&CommunityView::Style::MASK2);
      if (s1>CommunityView::Style::MASK1) {
        s1 = 1;
        unsigned int const s2x = s2*2;
        if ((s2x&CommunityView::Style::MASK2)==0) s2 = CommunityView::Style::MASK1+1;
        else s2 = s2x;
      }
      community_style = s1+s2;
    }

    void shift_community_edge_style(void) {
      unsigned int s1 = (community_edge_style&CommunityEdgeView::Style::MASK1)*2;
      unsigned int s2 = (community_edge_style&CommunityEdgeView::Style::MASK2);
      if (s1>CommunityEdgeView::Style::MASK1) {
        s1 = 1;
        unsigned int const s2x = s2*2;
        if ((s2x&CommunityEdgeView::Style::MASK2)==0) s2 = CommunityEdgeView::Style::MASK1+1;
        else s2 = s2x;
      }
      community_edge_style = s1+s2;
    }

    bool get_show_slider(void) { return show_slider; }
    bool get_show_grid(void) { return show_grid; }
    bool get_show_fps(void) { return show_fps; }
    bool get_show_center(void) { return show_center; }
    bool get_show_layout_frame(void) { return show_layout_frame; }
    bool get_show_time_label(void) { return show_time_label; }

    void set_show_slider(bool b) { show_slider = b; }
    void set_show_grid(bool b) { show_grid = b; }
    void set_show_fps(bool b) { show_fps = b; }
    void set_show_center(bool b) { show_center = b; }
    void set_show_layout_frame(bool b) { show_layout_frame = b; }
    void set_show_time_label(bool b) { show_time_label = b; }

    unsigned int get_node_size_factor(void) { return node_size_factor; }
    void set_node_size_factor(unsigned int factor) { node_size_factor = factor; }

  } // The end of the namespace "sociarium_project_view"

} // The end of the namespace "hashimoto_ut"

// s.o.c.i.a.r.i.u.m
// HASHIMOTO, Yasuhiro (E-mail: hy @ sys.t.u-tokyo.ac.jp)
// update: 2009/03/11

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

#ifndef INCLUDE_GUARD_SOCIARIUM_PROJECT_VIEW_H
#define INCLUDE_GUARD_SOCIARIUM_PROJECT_VIEW_H

#include "../shared/vector2.h"

namespace hashimoto_ut {

  namespace sociarium_project_view {

    namespace {
      ////////////////////////////////////////////////////////////////////////////////////////////////////
      // ローカル変数

      // 視点の初期値
      double const VIEW_DISTANCE = 100.0;
      double const VIEW_DISTANCE_MAX = 500.0;
      double const VIEW_DISTANCE_MIN = 1.0;
    }

    struct NodeView {
      struct Style {
        enum {
          POLYGON = 0x01,
          TEXTURE = 0x02,
          MASK1   = 0x03,
          MASK2   = 0x04
        };
      };
      struct SizeFactor {
        enum {
          UNIFORM = 0,
          DEGREE_CENTRALITY,
          CLOSENESS_CENTRALITY,
          BETWEENNESS_CENTRALITY,
          PAGERANK,
          NONE,
          SIZE
        };
      };
    };

    struct EdgeView {
      struct Style {
//         enum {
//           STRAIGHT = 0x01,
//           HCURVE   = 0x02,
//           VCURVE   = 0x04,
//           MASK1    = 0x07,
//           LINE     = 0x08,
//           TEXTURE  = 0x10,
//           MASK2    = 0x18
//         };
        enum {
          LINE     = 0x01,
          TEXTURE  = 0x02,
          MASK1    = 0x03,
          MASK2    = 0x04
        };
      };
    };

    struct CommunityView {
      struct Style {
        enum {
          POLYGON_CIRCLE = 0x01,
          POLYGON_CURVE  = 0x02,
          TEXTURE        = 0x04,
          MASK1          = 0x07,
          MASK2          = 0x08
        };
      };
    };

    struct CommunityEdgeView {
      struct Style {
//         enum {
//           STRAIGHT = 0x01,
//           HCURVE   = 0x02,
//           VCURVE   = 0x04,
//           MASK1    = 0x07,
//           LINE     = 0x08,
//           TEXTURE  = 0x10,
//           MASK2    = 0x18
//         };
        enum {
          LINE     = 0x01,
          TEXTURE  = 0x02,
          MASK1    = 0x03,
          MASK2    = 0x04
        };
      };
    };

    // 描画のON/OFF
    bool get_show_node(void);
    bool get_show_edge(void);
    bool get_show_community(void);
    bool get_show_community_edge(void);
    void set_show_node(bool b);
    void set_show_edge(bool b);
    void set_show_community(bool b);
    void set_show_community_edge(bool b);

    // ラベル描画のON/OFF
    bool get_show_node_label(void);
    bool get_show_edge_label(void);
    bool get_show_community_label(void);
    bool get_show_community_edge_label(void);
    void set_show_node_label(bool b);
    void set_show_edge_label(bool b);
    void set_show_community_label(bool b);
    void set_show_community_edge_label(bool b);

    // 描画スタイル
    unsigned int get_node_style(void);
    unsigned int get_edge_style(void);
    unsigned int get_community_style(void);
    unsigned int get_community_edge_style(void);
    void set_node_style(unsigned int item);
    void set_edge_style(unsigned int item);
    void set_community_style(unsigned int item);
    void set_community_edge_style(unsigned int item);

    void shift_node_style(void);
    void shift_edge_style(void);
    void shift_community_style(void);
    void shift_community_edge_style(void);

    // その他の描画のON/OFF
    bool get_show_slider(void);
    bool get_show_grid(void);
    bool get_show_fps(void);
    bool get_show_center(void);
    bool get_show_layout_frame(void);
    bool get_show_time_label(void);
    void set_show_slider(bool b);
    void set_show_grid(bool b);
    void set_show_fps(bool b);
    void set_show_center(bool b);
    void set_show_layout_frame(bool b);
    void set_show_time_label(bool b);

    // ノードサイズ
    unsigned int get_node_size_factor(void);
    void set_node_size_factor(unsigned int factor);

  } // The end of the namespace "sociarium_project_view"

} // The end of the namespace "hashimoto_ut"

#endif // INCLUDE_GUARD_SOCIARIUM_PROJECT_VIEW_H

// s.o.c.i.a.r.i.u.m: color.cpp
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

#include "color.h"
#include "sociarium_graph.h"
#include "../shared/predefined_color.h"

namespace hashimoto_ut {

  using std::tr1::array;

  namespace sociarium_project_color {

    ////////////////////////////////////////////////////////////////////////////////
    namespace {
      array<float, 4> const color_array[ColorCategory::NUMBER_OF_CATEGORIES] = {
        { 0.92f, 0.93f, 0.93f, 1.00f },  // BACKGROUND
        { 0.80f, 0.80f, 0.60f, 0.90f },  // LAYOUT_FRAME_BORDER
        { 0.80f, 0.80f, 0.60f, 0.30f },  // LAYOUT_FRAME_AREA
        { 0.00f, 0.00f, 0.00f, 0.60f },  // GRID
        { 0.30f, 0.30f, 0.30f, 0.90f },  // TIME_SLIDER_UPPER_ON
        { 0.30f, 0.30f, 0.30f, 0.50f },  // TIME_SLIDER_UPPER_OFF
        { 0.10f, 0.10f, 1.00f, 0.95f },  // TIME_SLIDER_LOWER_ON
        { 0.20f, 0.20f, 1.00f, 0.60f },  // TIME_SLIDER_LOWER_OFF
        { 0.00f, 0.00f, 0.00f, 0.20f },  // MOUSE_DRAG_REGION
        { 0.20f, 0.20f, 0.20f, 0.80f },  // SELECTION_FRAME
        { 1.00f, 1.00f, 1.00f, 1.00f },  // SELECTION_NAME
        { 0.00f, 0.00f, 0.00f, 0.50f },  // FPS
        { 1.00f, 1.00f, 1.00f, 0.90f },  // THREAD_MESSAGE
        { 0.00f, 0.00f, 0.00f, 0.50f },  // THREAD_MESSAGE_FRAME
        { 1.00f, 1.00f, 1.00f, 0.90f },  // LAYER_NAME
        { 0.00f, 0.00f, 0.00f, 0.50f },  // LAYER_NAME_FRAME
        { 0.50f, 1.00f, 0.30f, 0.80f },  // THREAD
      };

      int const default_node_color_id           = PredefinedColor::GRAY;
      int const default_edge_color_id           = PredefinedColor::GRAY;
      int const default_community_color_id      = PredefinedColor::GRAY;
      int const default_community_edge_color_id = PredefinedColor::GRAY;

      int const independent_node_color_id = PredefinedColor::LIGHT_GRAY;
      int const independent_edge_color_id = PredefinedColor::LIGHT_GRAY;

      int const default_node_name_color_id           = PredefinedColor::BLACK;
      int const default_edge_name_color_id           = PredefinedColor::BLACK;
      int const default_community_name_color_id      = PredefinedColor::BLACK;
      int const default_community_edge_name_color_id = PredefinedColor::BLACK;
    }


    ////////////////////////////////////////////////////////////////////////////////
    array<float, 4> const& get_color(int color_id) {
      assert(0<=color_id && color_id<ColorCategory::NUMBER_OF_CATEGORIES);
      return color_array[color_id];
    }


    ////////////////////////////////////////////////////////////////////////////////
    int get_default_node_color_id(void) {
      return default_node_color_id;
    }

    int get_default_edge_color_id(void) {
      return default_edge_color_id;
    }

    int get_default_community_color_id(void) {
      return default_community_color_id;
    }

    int get_default_community_edge_color_id(void) {
      return default_community_edge_color_id;
    }


    ////////////////////////////////////////////////////////////////////////////////
    int get_default_node_name_color_id(void) {
      return default_node_name_color_id;
    }

    int get_default_edge_name_color_id(void) {
      return default_edge_name_color_id;
    }

    int get_default_community_name_color_id(void) {
      return default_community_name_color_id;
    }

    int get_default_community_edge_name_color_id(void) {
      return default_community_edge_name_color_id;
    }


    ////////////////////////////////////////////////////////////////////////////////
    void highlight_effect(array<float, 4>& rgba) {
      rgba[0] = 0.9f;
      rgba[1] *= 0.5f;
      rgba[2] *= 0.5f;
      rgba[3] = 0.6f;
    }


    ////////////////////////////////////////////////////////////////////////////////
    void selection_effect(array<float, 4>& rgba) {
      rgba[3] *= 1.5f;
    }


    ////////////////////////////////////////////////////////////////////////////////
    void marking_effect(array<float, 4>& rgba) {
      rgba[0] *= 0.4f;
      rgba[1] *= 0.4f;
      rgba[2] *= 0.4f;
      rgba[3] = 0.7f;
    }


    ////////////////////////////////////////////////////////////////////////////////
    namespace {

      template <typename DynamicProperty>
      int get_independent_color_id(void);

      template <>
      int get_independent_color_id<DynamicNodeProperty>(void) {
        return independent_node_color_id;
      }

      template <>
      int get_independent_color_id<DynamicEdgeProperty>(void) {
        return independent_edge_color_id;
      }

      template <typename DynamicProperty>
      void set_color(DynamicProperty& dp) {
        if (dp.number_of_upper_nodes()==0)
          // If the graph element doesn't belong to any community.
          dp.set_color_id(get_independent_color_id<DynamicProperty>());
        else if (dp.number_of_upper_nodes()==1)
          // If the graph element belongs to only one community.
          dp.set_color_id((*dp.upper_nbegin())->get_color_id());
        else
          // If the edge belongs to multiple communities.
          dp.set_color_id(PredefinedColor::WHITE);
      }
    }

    void update_color_under_community_information(DynamicNodeProperty& dnp) {
      set_color<DynamicNodeProperty>(dnp);
    }

    void update_color_under_community_information(DynamicEdgeProperty& dep) {
      set_color<DynamicEdgeProperty>(dep);
    }

  } // The end of the namespace "sociarium_project_color"

} // The end of the namespace "hashimoto_ut"

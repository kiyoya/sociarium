// s.o.c.i.a.r.i.u.m: algorithm_selector.cpp
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
#include "algorithm_selector.h"
#include "thread/force_direction.h"

namespace hashimoto_ut {

  namespace sociarium_project_algorithm_selector {

    namespace {
      ////////////////////////////////////////////////////////////////////////////////
      int layout_algorithm
        = LayoutAlgorithm::KAMADA_KAWAI_METHOD;

      int force_direction_algorithm
        = RealTimeForceDirectionAlgorithm::KAMADA_KAWAI_METHOD;

      int community_detection_algorithm
        = CommunityDetectionAlgorithm::MODULARITY_MAXIMIZATION_USING_GREEDY_METHOD;

      int node_size_update_algorithm
        = NodeSizeUpdateAlgorithm::WEIGHT;

      int edge_width_update_algorithm
        = EdgeWidthUpdateAlgorithm::WEIGHT;

      bool weighted_modularity = true;
    }


    ////////////////////////////////////////////////////////////////////////////////
    int get_layout_algorithm(void) {
      return layout_algorithm;
    }

    void set_layout_algorithm(int algorithm) {
      assert(0<=algorithm && algorithm<LayoutAlgorithm::NUMBER_OF_ALGORITHMS);
      layout_algorithm = algorithm;
    }


    ////////////////////////////////////////////////////////////////////////////////
    int get_force_direction_algorithm(void) {
      return force_direction_algorithm;
    }

    void set_force_direction_algorithm(int algorithm) {
      assert(0<=algorithm && algorithm<RealTimeForceDirectionAlgorithm::NUMBER_OF_ALGORITHMS);
      force_direction_algorithm = algorithm;
      sociarium_project_force_direction::should_be_updated();
    }


    ////////////////////////////////////////////////////////////////////////////////
    int get_community_detection_algorithm(void) {
      return community_detection_algorithm;
    }

    void set_community_detection_algorithm(int algorithm) {
      assert(0<=algorithm && algorithm<CommunityDetectionAlgorithm::NUMBER_OF_ALGORITHMS);
      community_detection_algorithm = algorithm;
    }

    bool use_weighted_modularity(void) {
      return weighted_modularity;
    }

    void use_weighted_modularity(bool b) {
      weighted_modularity = b;
    }


    ////////////////////////////////////////////////////////////////////////////////
    int get_node_size_update_algorithm(void) {
      return node_size_update_algorithm;
    }

    void set_node_size_update_algorithm(int algorithm) {
      assert(0<=algorithm && algorithm<NodeSizeUpdateAlgorithm::NUMBER_OF_ALGORITHMS);
      node_size_update_algorithm = algorithm;
    }


    ////////////////////////////////////////////////////////////////////////////////
    int get_edge_width_update_algorithm(void) {
      return edge_width_update_algorithm;
    }

    void set_edge_width_update_algorithm(int algorithm) {
      assert(0<=algorithm && algorithm<EdgeWidthUpdateAlgorithm::NUMBER_OF_ALGORITHMS);
      edge_width_update_algorithm = algorithm;
    }

  } // The end of the namespace "sociarium_project_algorithm_selector"

} // The end of the namespace "hashimoto_ut"

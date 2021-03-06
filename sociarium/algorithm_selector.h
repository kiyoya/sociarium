﻿// s.o.c.i.a.r.i.u.m: algorithm_selector.h
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

#ifndef INCLUDE_GUARD_SOCIARIUM_PROJECT_ALGORITHM_SELECTOR_H
#define INCLUDE_GUARD_SOCIARIUM_PROJECT_ALGORITHM_SELECTOR_H

namespace hashimoto_ut {

  namespace sociarium_project_algorithm_selector {

    ////////////////////////////////////////////////////////////////////////////////
    namespace LayoutAlgorithm {
      enum {
        KAMADA_KAWAI_METHOD = 0,
        HIGH_DIMENSIONAL_EMBEDDING_1_2,
        HIGH_DIMENSIONAL_EMBEDDING_1_3,
        HIGH_DIMENSIONAL_EMBEDDING_2_3,
        CIRCLE,
        CIRCLE_IN_SIZE_ORDER,
        RANDOM,
        LATTICE,
        CARTOGRAMS,
        NUMBER_OF_ALGORITHMS
      };
    }

    ////////////////////////////////////////////////////////////////////////////////
    namespace RealTimeForceDirectionAlgorithm {
      enum {
        KAMADA_KAWAI_METHOD = 0,
        KAMADA_KAWAI_METHOD_WITH_COMMUNITY_SEPARATION,
        COMMUNITY_ORIENTED,
        SIMPLE_SPRING, // for refinement of HDE in the lattice structure.
        SPRING_AND_REPULSIVE_FORCE,
        LATTICE_GAS_METHOD,
        DESIGNTIDE,
        NUMBER_OF_ALGORITHMS
      };
    }

    ////////////////////////////////////////////////////////////////////////////////
    namespace CommunityDetectionAlgorithm {
      enum {
        CONNECTED_COMPONENTS = 0,
        STRONGLY_CONNECTED_COMPONENTS,
        CLIQUE_PERCOLATION_3,
        CLIQUE_PERCOLATION_4,
        CLIQUE_PERCOLATION_5,
        MODULARITY_MAXIMIZATION_USING_GREEDY_METHOD,
        MODULARITY_MAXIMIZATION_USING_TEO_METHOD,
        BETWEENNESS_CENTRALITY_SEPARATION,
        INFORMATION_FLOW_MAPPING,
        NUMBER_OF_ALGORITHMS
      };
    }

    ////////////////////////////////////////////////////////////////////////////////
    namespace NodeSizeUpdateAlgorithm {
      enum {
        UNIFORM = 0,
        WEIGHT,
        DEGREE_CENTRALITY,
        CLOSENESS_CENTRALITY,
        BETWEENNESS_CENTRALITY,
        PAGERANK,
        POINT,
        NUMBER_OF_ALGORITHMS
      };
    }

    ////////////////////////////////////////////////////////////////////////////////
    namespace EdgeWidthUpdateAlgorithm {
      enum {
        UNIFORM = 0,
        WEIGHT,
        BETWEENNESS_CENTRALITY,
        NUMBER_OF_ALGORITHMS
      };
    }

    ////////////////////////////////////////////////////////////////////////////////
    int get_layout_algorithm(void);
    void set_layout_algorithm(int algorithm);

    ////////////////////////////////////////////////////////////////////////////////
    int get_force_direction_algorithm(void);
    void set_force_direction_algorithm(int algorithm);

    ////////////////////////////////////////////////////////////////////////////////
    int get_community_detection_algorithm(void);
    void set_community_detection_algorithm(int algorithm);

    bool use_weighted_modularity(void);
    void use_weighted_modularity(bool b);

    ////////////////////////////////////////////////////////////////////////////////
    int get_node_size_update_algorithm(void);
    void set_node_size_update_algorithm(int algorithm);

    ////////////////////////////////////////////////////////////////////////////////
    int get_edge_width_update_algorithm(void);
    void set_edge_width_update_algorithm(int algorithm);

  } // The end of the namespace "sociarium_project_algorithm_selector"

} // The end of the namespace "hashimoto_ut"

#endif // INCLUDE_GUARD_SOCIARIUM_PROJECT_ALGORITHM_SELECTOR_H

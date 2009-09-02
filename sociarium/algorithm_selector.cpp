// s.o.c.i.a.r.i.u.m
// HASHIMOTO, Yasuhiro (E-mail: hy @ sys.t.u-tokyo.ac.jp)
// update: 2009/03/04

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

#include "algorithm_selector.h"

namespace hashimoto_ut {

  namespace sociarium_project_algorithm_selector {

    namespace {
      ////////////////////////////////////////////////////////////////////////////////////////////////////
      // ローカル変数
      int layout_algorithm = LayoutAlgorithm::KAMADA_KAWAI_METHOD;
      int community_detection_algorithm = CommunityDetectionAlgorithm::MODULARITY_MAXIMIZATION_USING_GREEDY_METHOD;
      bool weighted_modularity = false;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    int get_graph_layout_algorithm(void) {
      return layout_algorithm;
    }

    void set_graph_layout_algorithm(int algorithm) {
      if (algorithm<0) algorithm = 0;
      layout_algorithm = algorithm%LayoutAlgorithm::SIZE;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    int get_community_detection_algorithm(void) {
      return community_detection_algorithm;
    }

    void set_community_detection_algorithm(int algorithm) {
      if (algorithm<0) algorithm = 0;
      community_detection_algorithm = algorithm%CommunityDetectionAlgorithm::SIZE;
    }

    bool use_weighted_modularity(void) {
      return weighted_modularity;
    }
    
    void use_weighted_modularity(bool b) {
      weighted_modularity = b;
    }

  } // The end of the namespace "sociarium_project_algorithm_selector"

} // The end of the namespace "hashimoto_ut"

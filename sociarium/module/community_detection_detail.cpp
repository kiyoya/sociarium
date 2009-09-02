// s.o.c.i.a.r.i.u.m
// HASHIMOTO, Yasuhiro (E-mail: hy @ sys.t.u-tokyo.ac.jp)
// update: 2009/05/11

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

#include "community_detection_detail.h"
#include "../../graph/graph.h"

namespace hashimoto_ut {

  namespace sociarium_project_module_community_detection_detail {

    using std::vector;
    using std::tr1::shared_ptr;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // フィットネス
    vector<double> calc_fitness(shared_ptr<Graph const> const& g,
                                vector<Node*> const& nodes,
                                vector<int> const& grouping, // 1 or 0
                                double const& total_weight,
                                vector<double> const& node_weight,
                                vector<double> const& edge_weight) {

      size_t const gsz = nodes.size();
      vector<size_t> index(g->nsize(), size_t(-1));
      for (size_t i=0; i<gsz; ++i) index[nodes[i]->index()] = i;

      double w_sum[2] = { 0, 0 };
      vector<double> w_in(gsz, 0);

      for (size_t i=0; i<gsz; ++i) {
        Node const* n = nodes[i];
        int const b = grouping[i];
        double& w = w_in[i];
        w_sum[b] += node_weight[n->index()];
        for (adjacency_list_iterator j=n->obegin(), jend=n->oend(); j!=jend; ++j) {
          size_t const& jj = index[(*j)->target()->index()];
          if (jj!=size_t(-1) && b==grouping[jj]) w += edge_weight[(*j)->index()];
        }
        for (adjacency_list_iterator j=n->ibegin(), jend=n->iend(); j!=jend; ++j) {
          size_t const& jj = index[(*j)->source()->index()];
          if (jj!=size_t(-1) && b==grouping[jj]) w += edge_weight[(*j)->index()];
        }
      }

      double const a[2] = { 0.5*w_sum[0]/total_weight, 0.5*w_sum[1]/total_weight };
      vector<double> retval(gsz, 0.0);
      for (size_t i=0; i<gsz; ++i) {
        double const& nw = node_weight[nodes[i]->index()];
        if (nw>0.0) retval[i] = w_in[i]/nw-a[grouping[i]];
      }

      return retval;
    }

  } // The end of the namespace "sociarium_project_module_community_detection_detail"
} // The end of the namespace "hashimoto_ut"

// C++ GRAPH LIBRARY
// HASHIMOTO, Yasuhiro (E-mail: hy @ sys.t.u-tokyo.ac.jp)
// update: 2009/03/25

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

#ifndef INCLUDE_GUARD_GRAPH_UTIL_STATISTICS_H
#define INCLUDE_GUARD_GRAPH_UTIL_STATISTICS_H

#include <vector>
#include <utility>
#include "traverser.h"

namespace hashimoto_ut {

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  std::vector<double> pagerank(std::tr1::shared_ptr<Graph> const& g, double err=1e-5, double df=0.85);
  /* returns the array of pageranks for each node in @g.
   * @g is forcely treated as a directed graph.
   * The calculation of a principal component is based on the power method,
   * and @err is a criterion for convergence. @df is a dumping factor (see Ref.).
   *
   * [Reference]
   *   L. Page, S. Brin, R. Motwani, T. Winograd,
   *   "The PageRank citation ranking: Bringing order to the Web", 1999.
   */

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  std::pair<std::vector<double>, std::vector<double> >
    betweenness_centrality(std::tr1::shared_ptr<BFSRecordingTraverser> const& t);
  /* returns "Betweenness Centrality (BC)" of all nodes and all edges in the associated
   * graph of @t; the first value is that of nodes, and the second is that of edges.
   * BC of the node/edge is defined as the number of the shortest paths that
   * pass through the node/edge (in detail, see Ref.). The calculation of BC
   * (or determining the shortest path) depends on the traversing algorithm of @t.
   *
   * [Reference]
   *   M. E. J. Newman, M. Girvan, "Finding and evaluating community structure in networks",
   *   Physical Review E, 69, 026113, 2004.
   */

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  std::vector<double> closeness_centrality(std::tr1::shared_ptr<BFSTraverser> const& t);

} // The end of the namespace "hashimoto_ut"

#endif // INCLUDE_GUARD_GRAPH_UTIL_STATISTICS_H

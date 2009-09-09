// s.o.c.i.a.r.i.u.m: graph_utility.h
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

#ifndef INCLUDE_GUARD_SOCIARIUM_PROJECT_GRAPH_UTILITY_H
#define INCLUDE_GUARD_SOCIARIUM_PROJECT_GRAPH_UTILITY_H

#include <vector>
#include <string>
#include <utility>
#include "../graph/graph.h"

namespace hashimoto_ut {

  class Thread;
  class BFSTraverser;
  class DFSTraverser;
  class BFSRecordingTraverser;
  class Message;

  namespace sociarium_project_graph_utility {

    /*
     * If the first return value is false, it means the thread was cancelled.
     */

    ////////////////////////////////////////////////////////////////////////////////
    // Closeness Centrality
    std::pair<bool, std::vector<double> >
      closeness_centrality(
        Thread* parent, std::wstring* status,
        Message const* message,
        std::tr1::shared_ptr<BFSTraverser> t);

    ////////////////////////////////////////////////////////////////////////////////
    // Mean Shortest Path Length
    std::pair<bool, double>
      mean_shortest_path_length(
        Thread* parent, std::wstring* status,
        Message const* message,
        std::tr1::shared_ptr<BFSTraverser> t,
        node_iterator first, node_iterator last);

    ////////////////////////////////////////////////////////////////////////////////
    // All Connected Components
    std::pair<bool, std::vector<std::vector<Node*> > >
      connected_components(
        Thread* parent, std::wstring* status,
        Message const* message,
        std::tr1::shared_ptr<BFSTraverser> t,
        size_t minimum_size=0);

    ////////////////////////////////////////////////////////////////////////////////
    // All Strongly Connected Components
    std::pair<bool, std::vector<std::vector<Node*> > >
      strongly_connected_components(
        Thread* parent, std::wstring* status,
        Message const* message,
        std::tr1::shared_ptr<DFSTraverser> t_forward,
        std::tr1::shared_ptr<BFSTraverser> t_backward,
        size_t minimum_size=0);

    ////////////////////////////////////////////////////////////////////////////////
    // Connected Component (starting from the node @n)
    std::pair<bool, std::vector<Node*> >
      connected_component(
        Thread* parent, std::wstring* status,
        Message const* message,
        std::tr1::shared_ptr<BFSTraverser> t,
        Node const* n);

    ////////////////////////////////////////////////////////////////////////////////
    // Betweenness Centrality
    std::pair<bool, std::pair<std::vector<double>, std::vector<double> > >
      betweenness_centrality(
        Thread* parent, std::wstring* status,
        Message const* message,
        std::tr1::shared_ptr<BFSRecordingTraverser> t);

    ////////////////////////////////////////////////////////////////////////////////
    // PageRank
    std::pair<bool, std::vector<double> >
      pagerank(
        Thread* parent, std::wstring* status,
        Message const* message,
        std::tr1::shared_ptr<Graph const> g,
        double dumping_factor,
        double err);

    ////////////////////////////////////////////////////////////////////////////////
    // Modularity
    std::pair<bool, double>
      modularity(
        Thread* parent, std::wstring* status,
        Message const* message,
        std::tr1::shared_ptr<Graph const> g,
        std::vector<std::vector<Node*> > const& community,
        std::vector<double> const& edge_weight);

    ////////////////////////////////////////////////////////////////////////////////
    // All Largest Cliques
    std::pair<bool, std::vector<std::vector<Node*> > >
      largest_cliques(
        Thread* parent, std::wstring* status,
        Message const* message,
        std::tr1::shared_ptr<Graph const> g);

    ////////////////////////////////////////////////////////////////////////////////
    // 3-Clique Communities
    std::pair<bool, std::vector<std::vector<Node*> > >
      clique_communities_3(
        Thread* parent, std::wstring* status,
        Message const* message,
        std::tr1::shared_ptr<Graph const> g);

  } // The end of the namespace "sociarium_project_graph_utility"

} // The end of the namespace "hashimoto_ut"

#endif // INCLUDE_GUARD_SOCIARIUM_PROJECT_GRAPH_UTILITY_H

// s.o.c.i.a.r.i.u.m: graph_extractor.cpp
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

#include <boost/format.hpp>
#include "graph_extractor.h"
#include "menu_and_message.h"
#include "thread.h"
#include "../shared/thread.h"

namespace hashimoto_ut {

  using std::vector;
  using std::deque;
  using std::pair;
  using std::make_pair;
  using std::wstring;
  using std::tr1::shared_ptr;
  using std::tr1::unordered_map;

  typedef SociariumGraph::node_property_iterator node_property_iterator;
  typedef SociariumGraph::edge_property_iterator edge_property_iterator;

  using namespace sociarium_project_menu_and_message;

  namespace sociarium_project_graph_extractor {

    ////////////////////////////////////////////////////////////////////////////////
    pair<bool, shared_ptr<Graph> > get(
      Thread* th, wstring* status,
      shared_ptr<SociariumGraph const> graph,
      unordered_map<Node*, Node const*>& node2node,
      unordered_map<Edge*, Edge const*>& edge2edge,
      unsigned int flag) {

      shared_ptr<Graph> retval(Graph::create(graph->is_directed()));

      size_t const nsz = graph->nsize();
      size_t const esz = graph->esize();

      // Map the indices of nodes in @graph to nodes in @retval.
      vector<Node*> index2node(nsz);

      {
        ////////////////////////////////////////////////////////////////////////////////
        // Extract nodes.
        node_property_iterator i   = graph->node_property_begin();
        node_property_iterator end = graph->node_property_end();

        for (int count=1; i!=end; ++i, ++count) {

          // **********  Catch a termination signal  **********
          if (th && th->cancel_check())
            return make_pair(false, shared_ptr<Graph>());

          if (status)
            *status
              = (boost::wformat(L"%s: %d%%")
                 %get_message(Message::GRAPH_EXTRACTOR_EXTRACTING_NODES)
                 %int(100.0*count/nsz)).str();

          DynamicNodeProperty const& dnp = i->second;

          if (dnp.get_flag()&flag) {
            Node* n = retval->add_node();
            node2node[n] = i->first;
            index2node[i->first->index()] = n;
          }
        }

        if (retval->nsize()==0)
          return make_pair(true, Graph::create());
      }

      {
        ////////////////////////////////////////////////////////////////////////////////
        // Extract edges.
        edge_property_iterator i   = graph->edge_property_begin();
        edge_property_iterator end = graph->edge_property_end();

        for (int count=1; i!=end; ++i, ++count) {

          // **********  Catch a termination signal  **********
          if (th && th->cancel_check())
            return make_pair(false, shared_ptr<Graph>());

          if (status)
            *status
              = (boost::wformat(L"%s: %d%%")
                 %get_message(Message::GRAPH_EXTRACTOR_EXTRACTING_EDGES)
                 %int(100.0*count/esz)).str();

          Node const* n0 = i->first->source();
          Node const* n1 = i->first->target();
          DynamicNodeProperty const& dnp0 = graph->property(n0);
          DynamicNodeProperty const& dnp1 = graph->property(n1);
          DynamicEdgeProperty const& dep = i->second;

          if (dep.get_flag()&flag
              && dnp0.get_flag()&flag
              && dnp1.get_flag()&flag) {
            Node* m0 = index2node[n0->index()];
            Node* m1 = index2node[n1->index()];
            assert(m0!=0);
            assert(m1!=0);
            Edge* e = retval->add_edge(m0, m1);
            edge2edge[e] = i->first;
          }
        }
      }

      return make_pair(true, retval);
    }

  }// The end of the namespace "sociarium_project_graph_extractor"

} // The end of the namespace "hashimoto_ut"

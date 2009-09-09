// C++ GRAPH LIBRARY: graphex.h
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

#ifndef INCLUDE_GUARD_GRAPHEX_H
#define INCLUDE_GUARD_GRAPHEX_H

#include <cassert>
#include <set>
#include "graph.h"

namespace hashimoto_ut {

  namespace {

    ////////////////////////////////////////////////////////////////////////////////
    std::tr1::shared_ptr<Graph> copy(std::tr1::shared_ptr<Graph const> g) {
      assert(g!=0);
      std::tr1::shared_ptr<Graph> retval = g->clone();

      for (node_iterator i=g->nbegin(), end=g->nend(); i!=end; ++i)
        retval->add_node();

      for (edge_iterator i=g->ebegin(), end=g->eend(); i!=end; ++i)
        retval->add_edge(retval->node((*i)->source()->index()),
                         retval->node((*i)->target()->index()));

      return retval;
    }


    ////////////////////////////////////////////////////////////////////////////////
    void copy(std::tr1::shared_ptr<Graph const> const& g0,
              std::tr1::shared_ptr<Graph> const& g1) {
      assert(g0!=0);
      assert(g1!=0);
      g1->clear();

      for (node_iterator i=g0->nbegin(), end=g0->nend(); i!=end; ++i)
        g1->add_node();

      for (edge_iterator i=g0->ebegin(), end=g0->eend(); i!=end; ++i)
        g1->add_edge(g1->node((*i)->source()->index()),
                     g1->node((*i)->target()->index()));
    }


    ////////////////////////////////////////////////////////////////////////////////
    void remove_loop_edges(std::tr1::shared_ptr<Graph> g, Node* n) {
      assert(g!=0);
      assert(n!=0 && g->node(n->index())==n);
      std::vector<Edge*> removed_edges;
      for (adjacency_list_iterator i=n->obegin(), end=n->oend(); i!=end; ++i)
        if ((*i)->target()==n) removed_edges.push_back(*i);
      g->remove_edges(removed_edges.begin(), removed_edges.end());
    }


    ////////////////////////////////////////////////////////////////////////////////
    void remove_loop_edges(std::tr1::shared_ptr<Graph> g) {
      assert(g!=0);
      std::vector<Edge*> removed_edges;

      for (edge_iterator i=g->ebegin(), end=g->eend(); i!=end; ++i)
        if ((*i)->source()==(*i)->target())
          removed_edges.push_back(*i);

      g->remove_edges(removed_edges.begin(), removed_edges.end());
    }


    ////////////////////////////////////////////////////////////////////////////////
    void remove_parallel_edges(std::tr1::shared_ptr<Graph> g, Edge const* e) {
      assert(g!=0);
      Node* source = e->source();
      Node* target = e->target();
      std::vector<Edge*> removed_edges;

      for (adjacency_list_iterator i=source->obegin(), oend=source->oend(); i!=oend; ++i)
        if (*i!=e && target==(*i)->target()) removed_edges.push_back(*i);

      if (g->is_directed())
        for (adjacency_list_iterator i=source->ibegin(), iend=source->iend(); i!=iend; ++i)
          if (*i!=e && target==(*i)->source())
            removed_edges.push_back(*i);

      g->remove_edges(removed_edges.begin(), removed_edges.end());
    }


    ////////////////////////////////////////////////////////////////////////////////
    void contract(std::tr1::shared_ptr<Graph> g, Node* n_remain, Node* n_vanish) {
      assert(g!=0);
      assert(n_remain!=0 && g->node(n_remain->index())==n_remain);
      assert(n_vanish!=0 && g->node(n_vanish->index())==n_vanish);
      std::vector<Edge*> oedges(n_vanish->obegin(), n_vanish->oend());

      for (std::vector<Edge*>::iterator i=oedges.begin(), end=oedges.end(); i!=end; ++i)
        g->change_source(*i, n_remain);

      std::vector<Edge*> iedges(n_vanish->ibegin(), n_vanish->iend());

      for (std::vector<Edge*>::iterator i=iedges.begin(), end=iedges.end(); i!=end; ++i)
        g->change_target(*i, n_remain);

      g->remove_node(n_vanish);
    }
    /* The total number of edges doesn't change after contracting.
     * The edges between @n_remain and @n_vanish remain as loop edges on @n_remain.
     */


    ////////////////////////////////////////////////////////////////////////////////
    void contract(std::tr1::shared_ptr<Graph> g, Edge* e) {
      assert(g!=0);
      assert(e!=0 && g->edge(e->index())==e);
      Node* n_remain = e->source();
      Node* n_vanish = e->target();
      std::vector<Edge*> oedges(n_vanish->obegin(), n_vanish->oend());

      for (std::vector<Edge*>::iterator i=oedges.begin(), end=oedges.end(); i!=end; ++i)
        if ((*i)->target()!=n_remain)
          g->change_source(*i, n_remain);

      std::vector<Edge*> iedges(n_vanish->ibegin(), n_vanish->iend());

      for (std::vector<Edge*>::iterator i=iedges.begin(), end=iedges.end(); i!=end; ++i)
        if ((*i)->source()!=n_remain)
          g->change_target(*i, n_remain);

      g->remove_node(n_vanish);
    }
    /* @e and its parallel edges are removed.
     */


    ////////////////////////////////////////////////////////////////////////////////
    std::vector<Node*> complementary_nodes(
      std::tr1::shared_ptr<Graph const> g,
      node_iterator first, node_iterator last) {

      assert(g!=0);
      size_t const num = size_t(last-first);
      assert(num<=g->nsize());
      std::vector<Node*> retval;
      retval.reserve(g->nsize()-num);
      std::vector<int> flag(g->nsize(), 0);

      for (; first!=last; ++first) {
        assert(*first==g->node((*first)->index()));
        flag[(*first)->index()] = 1;
      }

      for (size_t i=0, sz=g->nsize(); i<sz; ++i)
        if (flag[i]==0)
          retval.push_back(g->node(i));

      return retval;
    }


    ////////////////////////////////////////////////////////////////////////////////
    std::vector<Edge*> complementary_edges(
      std::tr1::shared_ptr<Graph const> g,
      edge_iterator first, edge_iterator last) {

      assert(g!=0);
      size_t const num = size_t(last-first);
      assert(num<=g->esize());
      std::vector<Edge*> retval;
      retval.reserve(g->esize()-num);
      std::vector<int> flag(g->esize(), 0);

      for (; first!=last; ++first) {
        assert(*first==g->edge((*first)->index()));
        flag[(*first)->index()] = 1;
      }

      for (size_t i=0, esz=g->esize(); i<esz; ++i)
        if (flag[i]==0)
          retval.push_back(g->edge(i));

      return retval;
    }


    ////////////////////////////////////////////////////////////////////////////////
    // Return edges whose either end falls in the range [first:last).
    std::vector<Edge*> incident_edges(node_iterator first, node_iterator last) {
      std::vector<Edge*> retval;
      std::set<Edge*> e;

      for (node_iterator i=first; i!=last; ++i) {
        for (adjacency_list_iterator j=(*i)->begin(), end=(*i)->end(); j!=end; ++j) {
          if (e.find(*j)==e.end()) {
            retval.push_back(*j);
            e.insert(*j);
          }
        }
      }

      return retval;
    }


    ////////////////////////////////////////////////////////////////////////////////
    // Return edges whose both ends fall in the range [first:last).
    std::vector<Edge*> induced_edges(node_iterator first, node_iterator last) {
      std::vector<Edge*> retval;
      std::set<Node*> n;
      std::set<Edge*> e;

      for (node_iterator i=first; i!=last; ++i)
        n.insert(*i);

      for (node_iterator i=first; i!=last; ++i) {
        for (adjacency_list_iterator j=(*i)->obegin(), end=(*i)->oend(); j!=end; ++j) {
          if (n.find((*j)->target())!=n.end() && e.find(*j)==e.end()) {
            retval.push_back(*j);
            e.insert(*j);
          }
        }
      }

      return retval;
    }

  } // The end of the anonymous namespace

} // The end of the namespace "hashimoto_ut"

#endif

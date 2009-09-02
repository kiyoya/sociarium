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

#include <cassert>
#include <deque>
#include <utility>
#include <numeric>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include "statistics.h"

namespace hashimoto_ut {

  using std::vector;
  using std::deque;
  using std::pair;
  using std::make_pair;
  using std::tr1::shared_ptr;
  using boost::numeric::ublas::matrix;

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  vector<double> pagerank(shared_ptr<Graph> const& g, double err, double df) {
    assert(g!=0);
    assert(!g->empty());
    size_t const sz = g->nsize();
    double const jump = (1.0-df)/sz;
    matrix<double> m(boost::numeric::ublas::scalar_matrix<double>(sz, sz, 0.0));
    for (size_t i=0; i<sz; ++i) {
      Node const* n = g->node(i);
      size_t const odeg = n->odegree();
      if (odeg) {
        double const p = df/odeg;
        for (adjacency_list_iterator j=n->obegin(), end=n->oend(); j!=end; ++j) m((*j)->target()->index(), i) = p;
        for (size_t j=0; j<sz; ++j) m(j, i) += jump;
      } else {
        double const p = 1.0/sz;
        for (size_t j=0; j<sz; ++j) m(j, i) = p;
      }
    }
    for (size_t i=0; i<sz; ++i) m(i,i) += 1.0; // origin shift
    boost::numeric::ublas::vector<double, vector<double> > v(1, vector<double>(sz, 1.0));
    while (1) {
      boost::numeric::ublas::vector<double, vector<double> > v_tmp(v);
      static int const iteration = 10;
      for (int j=0; j<iteration; ++j) v = prod(m, v);
      v /= norm_2(v);
      v_tmp -= v;
      double e = 0.0;
      for (size_t i=0; i<sz; ++i) e += v_tmp[i]*v_tmp[i];
      if (e<err) break;
    }
    double sum = accumulate(v.begin(), v.end(), 0.0);
    assert(sum!=0.0);
    vector<double> retval(v.data());
    for (vector<double>::iterator i=retval.begin(), end=retval.end(); i!=end; ++i) *i /= sum;
    return retval;
  }


  ////////////////////////////////////////////////////////////////////////////////////////////////////
  pair<vector<double>, vector<double> > betweenness_centrality(shared_ptr<BFSRecordingTraverser> const& t) {
    assert(t!=0);
    shared_ptr<Graph> const& g = t->graph();
    assert(g!=0);
    assert(!g->empty());
    vector<double> node_centrality(g->nsize(), 0.0);
    vector<double> edge_centrality(g->esize(), 0.0);
    for (node_iterator i=g->nbegin(), nend=g->nend(); i!=nend; ++i) {
      Node const* snk = *i;
      vector<Node const*> src_array;
      vector<int> weight(g->nsize(), 0);
      weight[snk->index()] = 1;
      t->reset();
      for (t->start(snk, 0.0), t->advance(); !t->end(); t->advance()) {
        src_array.push_back(t->node());
        for (vector<pair<Edge*, Node*> >::const_iterator p=t->pbegin(src_array.back()), pend=t->pend(src_array.back()); p!=pend; ++p) {
          weight[t->node()->index()] += weight[p->second->index()];
        }
      }
      vector<double> u(g->nsize(), 0.0);
      for (vector<Node const*>::const_reverse_iterator s=src_array.rbegin(), send=src_array.rend(); s!=send; ++s) {
        Node const* src = *s;
        size_t const sid = src->index();
        double const u_base = (u[sid]+1.0)/weight[sid];
        for (vector<pair<Edge*, Node*> >::const_iterator p=t->pbegin(src), pend=t->pend(src); p!=pend; ++p) {
          size_t const nid = p->second->index();
          size_t const eid = p->first->index();
          double const uw = u_base*weight[nid];
          edge_centrality[eid] += uw;
          if (p->second!=snk) node_centrality[nid] += uw;
          u[nid] += uw;
        }
      }
    }
    return make_pair(node_centrality, edge_centrality);
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  vector<double> closeness_centrality(shared_ptr<BFSTraverser> const& t) {
    assert(t!=0);
    shared_ptr<Graph> const& g = t->graph();
    assert(g!=0);
    assert(!g->empty());
    vector<double> retval(g->nsize(), 0.0);
    for (node_iterator i=g->nbegin(), nend=g->nend(); i!=nend; ++i) {
      t->reset();
      for (t->start(*i, 0.0); !t->end(); t->advance()) retval[(*i)->index()] += pow(2.0, -t->distance());
    }
    return retval;
  }

} // The end of the namespace "hashimoto_ut"

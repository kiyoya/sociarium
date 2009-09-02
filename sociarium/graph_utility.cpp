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

#include <boost/format.hpp>
#include "graph_utility.h"
#include "message.h"
#include "../shared/thread.h"
#include "../graph/util/traverser.h"

namespace hashimoto_ut {

  using std::vector;
  using std::wstring;
  using std::pair;
  using std::make_pair;
  using std::tr1::shared_ptr;

  namespace sociarium_project_graph_utility {

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    pair<bool, double> mean_shortest_path_length(
      Thread* parent,
      wstring* message,
      shared_ptr<BFSTraverser> const& t,
      node_iterator first, node_iterator last) {

      assert(t!=0);
      shared_ptr<Graph const> const g = t->graph();
      assert(g!=0);

      size_t const nsz = last-first;
      if (nsz<2) return make_pair(true, -1.0);

      double distance_sum = 0.0;

      for (size_t count=0; first!=last; ++first) {

        // ----------------------------------------------------------------------------------------------------
        if (parent!=0 && parent->cancel_check()) return make_pair(false, 0.0);
        // ----------------------------------------------------------------------------------------------------

        if (message!=0)
          *message = (boost::wformat(L"%s: %d%%")
                      %sociarium_project_message::STATISTICS_MEAN_SHORTEST_PATH_LENGTH
                      %int(100.0*(float(++count)/nsz))).str();

        assert(*first==g->node((*first)->index()));
        size_t sz_chk = 0;

        for (t->reset(), t->start(*first, 0.0); !t->end(); t->advance()) {

          // ----------------------------------------------------------------------------------------------------
          if (parent!=0 && parent->cancel_check()) return make_pair(false, 0.0);
          // ----------------------------------------------------------------------------------------------------

          distance_sum += t->distance();
          ++sz_chk;
        }

        if (sz_chk!=nsz) return make_pair(true, -1.0);
      }

      return make_pair(true, distance_sum/(nsz*(nsz-1.0)));
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////
    pair<bool, vector<vector<Node*> > > connected_components(
      Thread* parent,
      wstring* message,
      shared_ptr<BFSTraverser> const& t) {

      vector<vector<Node*> > retval;
      shared_ptr<Graph const> const g = t->graph();
      assert(g!=0);

      for (node_iterator i=g->nbegin(), end=g->nend(); i!=end; ++i) {

        // ----------------------------------------------------------------------------------------------------
        if (parent!=0 && parent->cancel_check()) return make_pair(false, vector<vector<Node*> >());
        // ----------------------------------------------------------------------------------------------------

        if (message!=0)
          *message = (boost::wformat(L"%s: %d%%")
                      %sociarium_project_message::STATISTICS_CONNECTED_COMPONENT
                      %int(100.0*((*i)->index()+1.0)/g->nsize())).str();

        vector<Node*> c;
        for (t->start(*i, 0.0); !t->end(); t->advance()) {

          // ----------------------------------------------------------------------------------------------------
          if (parent!=0 && parent->cancel_check()) return make_pair(false, vector<vector<Node*> >());
          // ----------------------------------------------------------------------------------------------------

          c.push_back(const_cast<Node*>(t->node()));
        }

        if (!c.empty()) retval.push_back(c);
      }

      return make_pair(true, retval);
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////
    pair<bool, vector<Node*> > connected_component(
      Thread* parent,
      wstring* message,
      shared_ptr<BFSTraverser> const& t,
      Node const* n) {

      assert(t!=0);
      shared_ptr<Graph const> const g = t->graph();
      assert(g!=0);
      assert(n==g->node(n->index()));

      vector<Node*> retval;
      for (t->start(n, 0.0); !t->end(); t->advance()) {

        // ----------------------------------------------------------------------------------------------------
        if (parent!=0 && parent->cancel_check()) return make_pair(false, vector<Node*>());
        // ----------------------------------------------------------------------------------------------------

        if (message!=0) *message = (boost::wformat(L"%.2f")%t->distance()).str();

        retval.push_back(const_cast<Node*>(t->node()));
      }

      return make_pair(true, retval);
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////
    pair<bool, pair<vector<double>, vector<double> > > betweenness_centrality(
      Thread* parent,
      wstring* message,
      shared_ptr<BFSRecordingTraverser> const& t) {

      assert(t!=0);
      shared_ptr<Graph const> const g = t->graph();
      assert(g!=0);
      if (g->empty()) return make_pair(true, make_pair(vector<double>(), vector<double>()));

      vector<double> node_centrality(g->nsize(), 0.0);
      vector<double> edge_centrality(g->esize(), 0.0);

      for (node_iterator i=g->nbegin(), nend=g->nend(); i!=nend; ++i) {

        // ----------------------------------------------------------------------------------------------------
        if (parent!=0 && parent->cancel_check()) return make_pair(false, make_pair(vector<double>(), vector<double>()));
        // ----------------------------------------------------------------------------------------------------

        if (message!=0)
          *message = (boost::wformat(L"%s: %d%%")
                      %sociarium_project_message::STATISTICS_BETWEENNESS_CENTRALITY
                      %int(100.0*((*i)->index()+1.0)/g->nsize())).str();

        Node const* snk = *i;
        vector<Node const*> src_array;
        vector<int> weight(g->nsize(), 0);
        weight[snk->index()] = 1;
        t->reset();

        for (t->start(snk, 0.0), t->advance(); !t->end(); t->advance()) {

          // ----------------------------------------------------------------------------------------------------
          if (parent!=0 && parent->cancel_check()) return make_pair(false, make_pair(vector<double>(), vector<double>()));
          // ----------------------------------------------------------------------------------------------------

          src_array.push_back(t->node());
          for (vector<pair<Edge*, Node*> >::const_iterator p=t->pbegin(src_array.back()), pend=t->pend(src_array.back()); p!=pend; ++p) {
            weight[t->node()->index()] += weight[p->second->index()];
          }
        }

        vector<double> u(g->nsize(), 0.0);
        for (vector<Node const*>::const_reverse_iterator s=src_array.rbegin(), send=src_array.rend(); s!=send; ++s) {

          // ----------------------------------------------------------------------------------------------------
          if (parent!=0 && parent->cancel_check()) return make_pair(false, make_pair(vector<double>(), vector<double>()));
          // ----------------------------------------------------------------------------------------------------

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

      return make_pair(true, make_pair(node_centrality, edge_centrality));
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // モジュラリティ
    pair<bool, double> modularity(
      Thread* parent,
      wstring* message,
      shared_ptr<Graph const> const& g,
      vector<vector<Node*> > const& community,
      vector<double> const& edge_weight) {

      vector<size_t> cid(g->nsize(), 0);
      for (size_t i=0; i<community.size(); ++i)
        for (size_t j=0; j<community[i].size(); ++j)
          cid[community[i][j]->index()] = i;

      vector<double> eii(community.size(), 0);
      vector<double> ai(community.size(), 0);
      double retval = 0.0;

      if (edge_weight.empty()) {
        // 重みなし
        for (edge_iterator i=g->ebegin(); i!=g->eend(); ++i) {

          // ----------------------------------------------------------------------------------------------------
          if (parent!=0 && parent->cancel_check()) return make_pair(false, 0.0);
          // ----------------------------------------------------------------------------------------------------

          size_t const s = cid[(*i)->source()->index()];
          size_t const t = cid[(*i)->target()->index()];
          if (s==t) {
            ++eii[s];
            ai[s] += 2;
          } else {
            ++ai[s];
            ++ai[t];
          }
        }

        for (size_t i=0; i<community.size(); ++i) {

          // ----------------------------------------------------------------------------------------------------
          if (parent!=0 && parent->cancel_check()) return make_pair(false, 0.0);
          // ----------------------------------------------------------------------------------------------------

          double const a = (0.5*ai[i])/g->esize();
          retval += double(eii[i])/g->esize()-a*a;
        }

      } else {
        // 重み付き
        double w_sum = 0.0;
        for (edge_iterator i=g->ebegin(); i!=g->eend(); ++i) {

          // ----------------------------------------------------------------------------------------------------
          if (parent!=0 && parent->cancel_check()) return make_pair(false, 0.0);
          // ----------------------------------------------------------------------------------------------------

          double const& w = edge_weight[(*i)->index()];
          w_sum += w;
          size_t const s = cid[(*i)->source()->index()];
          size_t const t = cid[(*i)->target()->index()];
          if (s==t) {
            eii[s] += w;
            ai[s] += 2.0*w;
          } else {
            ai[s] += w;
            ai[t] += w;
          }
        }

        for (size_t i=0; i<community.size(); ++i) {

          // ----------------------------------------------------------------------------------------------------
          if (parent!=0 && parent->cancel_check()) return make_pair(false, 0.0);
          // ----------------------------------------------------------------------------------------------------

          double const a = (0.5*ai[i])/w_sum;
          retval += double(eii[i])/w_sum-a*a;
        }
      }

      return make_pair(true, retval);
    }


    namespace {
      ////////////////////////////////////////////////////////////////////////////////////////////////////
      // Return true if "lhs" is a subset of "rhs".
      template <typename T>
      bool is_included(vector<T> const& lhs, vector<T> const& rhs) {
        for (typename vector<T>::const_iterator i=lhs.begin(), lend=lhs.end(); i!=lend; ++i) {
          for (typename vector<T>::const_iterator j=rhs.begin(), rend=rhs.end(); j!=rend; ++j)
            if (*i==*j) goto detected;
          return false;
        detected:;
        }
        return true;
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////
      // Return true if "lhs" is a subset of either of element in "rhs".
      template <typename T>
      bool is_included(vector<T> const& lhs, vector<vector<T> > const& rhs) {
        for (typename  vector<vector<T> >::const_iterator i=rhs.begin(), end=rhs.end(); i!=end; ++i)
          if (is_included(lhs, *i)) return true;
        return false;
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////
      // Return true if "n" is adjacent to all nodes in "rhs".
      bool is_completely_adjacent(Node const* n, vector<Node*> const& rhs) {
        for (vector<Node*>::const_iterator i=rhs.begin(), end=rhs.end(); i!=end; ++i)
          if (n->find(n->begin(), n->end(), *i)==n->end()) return false;
        return true;
      }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    pair<bool, vector<vector<Node*> > > clique(
      Thread* parent,
      wstring* message,
      shared_ptr<Graph const> const& g) {

      vector<vector<Node*> > retval;
      for (node_iterator i=g->nbegin(), end=g->nend(); i!=end; ++i) {

        // ----------------------------------------------------------------------------------------------------
        if (parent!=0 && parent->cancel_check()) return make_pair(false, vector<vector<Node*> >());
        // ----------------------------------------------------------------------------------------------------

        Node* n = *i;
        vector<Node*> stk_n(1, n);
        vector<adjacency_list_iterator> stk_c;
        adjacency_list_iterator c = n->begin();
        adjacency_list_iterator cend = n->end();

        for (;;) {
          bool newly_added = false;
          for (; c!=cend; ++c) {
            Node* m = c<n->iend()?(*c)->source():(*c)->target();
            if (m->index()>n->index() && is_completely_adjacent(m, stk_n)) {
              stk_n.push_back(m);
              stk_c.push_back(c);
              newly_added = true;
            }
          }
          if (newly_added && stk_n.size()>0 && !is_included(stk_n, retval)) retval.push_back(stk_n);
          stk_n.pop_back();
          if (stk_n.empty()) break;
          c = ++stk_c.back();
          stk_c.pop_back();
        }
      }

      return make_pair(true, retval);
    }

  } // The end of the namespace "sociarium_project_graph_utility"
} // The end of the namespace "hashimoto_ut"

// s.o.c.i.a.r.i.u.m: graph_utility.cpp
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

#include <map>
#include <numeric>
#include <boost/format.hpp>
#include <boost/numeric/ublas/matrix_sparse.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include "graph_utility.h"
#include "menu_and_message.h"
#include "../graph/util/traverser.h"
#include "../shared/thread.h"

namespace hashimoto_ut {

  using std::vector;
  using std::multimap;
  using std::wstring;
  using std::make_pair;
  using std::pair;
  using std::greater;
  using std::accumulate;
  using std::tr1::shared_ptr;
  using boost::numeric::ublas::mapped_matrix;

  using namespace sociarium_project_menu_and_message;

  namespace sociarium_project_graph_utility {

    ////////////////////////////////////////////////////////////////////////////////
    pair<bool, vector<double> > closeness_centrality(
      Thread* parent, wstring* status,
      Message const* message,
      shared_ptr<BFSTraverser> t) {

      assert(t!=0);
      shared_ptr<Graph const> g = t->graph();
      assert(g!=0);

      size_t const nsz = g->nsize();

      vector<double> retval(nsz, 0.0);

      for (size_t i=0; i<nsz; ++i) {

        // **********  Catch a termination signal  **********
        if (parent && parent->cancel_check())
          return make_pair(false, vector<double>());

        if (status && message)
          *status
            = (boost::wformat(L"%s: %d%%")
               %message->get(Message::CALCULATING_CLOSENESS_CENTRALITY)
               %int(100.0*i/nsz)).str();

        Node* n = g->node(i);

        // Traversing starts from the neighboring node of @n.
        t->reset();
        t->start(n);
        t->advance();

        for (; !t->end(); t->advance()) {

          // **********  Catch a termination signal  **********
          if (parent && parent->cancel_check())
            return make_pair(false, vector<double>());

          retval[i] += pow(2.0, -t->distance());
          //cc += 1.0/(t->distance()*t->distance());
        }
      }

      return make_pair(true, retval);
    }


    ////////////////////////////////////////////////////////////////////////////////
    pair<bool, double> mean_shortest_path_length(
      Thread* parent, wstring* status,
      Message const* message,
      shared_ptr<BFSTraverser> t,
      node_iterator first, node_iterator last) {

      assert(t!=0);
      shared_ptr<Graph const> g = t->graph();
      assert(g!=0);

      size_t const nsz = last-first;
      if (nsz<2) return make_pair(true, -1.0);

      double distance_sum = 0.0;

      for (size_t count=0; first!=last; ++first) {

        // **********  Catch a termination signal  **********
        if (parent && parent->cancel_check())
          return make_pair(false, 0.0);

        if (status && message)
          *status
            = (boost::wformat(L"%s: %d%%")
               %message->get(Message::CALCULATING_MEAN_SHORTEST_PATH_LENGTH)
               %int(100.0*(++count)/nsz)).str();

        Node const* n = *first;

        assert(n==g->node(n->index()));
        size_t sz_chk = 0;

        t->reset();
        t->start(n);

        for (; !t->end(); t->advance()) {

          // **********  Catch a termination signal  **********
          if (parent && parent->cancel_check())
            return make_pair(false, 0.0);

          distance_sum += t->distance();
          ++sz_chk;
        }

        if (sz_chk!=nsz)
          return make_pair(true, -1.0);
      }

      return make_pair(true, distance_sum/(nsz*(nsz-1.0)));
    }


    ////////////////////////////////////////////////////////////////////////////////
    pair<bool, vector<vector<Node*> > > connected_components(
      Thread* parent, wstring* status,
      Message const* message,
      shared_ptr<BFSTraverser> t,
      size_t threshold_size) {

      vector<vector<Node*> > retval;
      shared_ptr<Graph const> g = t->graph();
      assert(g!=0);

      node_iterator i   = g->nbegin();
      node_iterator end = g->nend();

      for (; i!=end; ++i) {

        // **********  Catch a termination signal  **********
        if (parent && parent->cancel_check())
          return make_pair(false, vector<vector<Node*> >());

        if (status && message)
          *status
            = (boost::wformat(L"%s: %d%%")
               %message->get(Message::CALCULATING_CONNECTED_COMPONENTS)
               %int(100.0*((*i)->index()+1.0)/g->nsize())).str();

        vector<Node*> c;

        for (t->start(*i); !t->end(); t->advance()) {

          // **********  Catch a termination signal  **********
          if (parent && parent->cancel_check())
            return make_pair(false, vector<vector<Node*> >());

          c.push_back(const_cast<Node*>(t->node()));
        }

        if (c.size()>threshold_size)
          retval.push_back(c);
      }

      return make_pair(true, retval);
    }


    ////////////////////////////////////////////////////////////////////////////////
    pair<bool, vector<vector<Node*> > > strongly_connected_components(
      Thread* parent, wstring* status,
      Message const* message,
      shared_ptr<DFSTraverser> t_forward,
      shared_ptr<BFSTraverser> t_backward,
      size_t thread_size) {

      vector<vector<Node*> > retval;
      shared_ptr<Graph const> g = t_forward->graph();
      assert(g==t_backward->graph());
      assert(g!=0);

      size_t const nsz = g->nsize();
      size_t const esz = g->esize();

      if (nsz==0 || esz==0)
        return make_pair(false, vector<vector<Node*> >());

      typedef multimap<int, Node*, greater<int> > BacktrackOrder;
      BacktrackOrder backtrack_order;

      size_t count = 0;
      double const counter_denom = 3.0*nsz;

      { // Sort nodes in descending DFS-backtrack-order.
        node_iterator i   = g->nbegin();
        node_iterator end = g->nend();

        for (; i!=end; ++i) {

          if (status && message)
            *status
              = (boost::wformat(L"%s: %d%%")
                 %message->get(Message::CALCULATING_STRONGLY_CONNECTED_COMPONENTS)
                 %int(100.0*(++count)/counter_denom)).str();

          for (t_forward->start(*i); !t_forward->end(); t_forward->advance()) {
            // **********  Catch a termination signal  **********
            if (parent->cancel_check())
              return make_pair(false, vector<vector<Node*> >());
          }
        }

        for (i=g->nbegin(); i!=end; ++i) {

          // **********  Catch a termination signal  **********
          if (parent && parent->cancel_check())
            return make_pair(false, vector<vector<Node*> >());

          if (status && message)
            *status
              = (boost::wformat(L"%s: %d%%")
                 %message->get(Message::CALCULATING_STRONGLY_CONNECTED_COMPONENTS)
                 %int(100.0*(++count)/counter_denom)).str();

          backtrack_order.insert(make_pair(t_forward->backtrack_order(*i), *i));
        }
      }

      { // The nodes that reverse-search can reach compose SCC.
        BacktrackOrder::const_iterator i = backtrack_order.begin();
        BacktrackOrder::const_iterator end = backtrack_order.end();

        for (; i!=end; ++i) {
          vector<Node*> c;
          t_backward->start(i->second);

          for (; !t_backward->end(); t_backward->advance()) {

            // **********  Catch a termination signal  **********
            if (parent && parent->cancel_check())
              return make_pair(false, vector<vector<Node*> >());

            if (status && message)
              *status
                = (boost::wformat(L"%s: %d%%")
                   %message->get(Message::CALCULATING_STRONGLY_CONNECTED_COMPONENTS)
                   %int(100.0*(++count)/counter_denom)).str();

            c.push_back(const_cast<Node*>(t_backward->node()));
          }

          if (c.size()>thread_size)
            retval.push_back(c);
        }

        return make_pair(true, retval);
      }
    }


    ////////////////////////////////////////////////////////////////////////////////
    pair<bool, vector<Node*> > connected_component(
      Thread* parent, wstring* status,
      Message const* message,
      shared_ptr<BFSTraverser> t,
      Node const* n) {

      assert(t!=0);
      shared_ptr<Graph const> g = t->graph();
      assert(g!=0);
      assert(n==g->node(n->index()));

      vector<Node*> retval;

      for (t->start(n); !t->end(); t->advance()) {

        // **********  Catch a termination signal  **********
        if (parent && parent->cancel_check())
          return make_pair(false, vector<Node*>());

        if (status && message)
          *status
            = (boost::wformat(L"%s: %.2f")
               %message->get(Message::CALCULATING_CONNECTED_COMPONENTS)
               %t->distance()).str();

        retval.push_back(const_cast<Node*>(t->node()));
      }

      return make_pair(true, retval);
    }


    ////////////////////////////////////////////////////////////////////////////////
    pair<bool, pair<vector<double>, vector<double> > > betweenness_centrality(
      Thread* parent, wstring* status,
      Message const* message,
      shared_ptr<BFSRecordingTraverser> t) {

      assert(t!=0);
      shared_ptr<Graph const> g = t->graph();
      assert(g!=0);

      if (g->empty())
        return make_pair(true, make_pair(vector<double>(), vector<double>()));

      vector<double> node_centrality(g->nsize(), 0.0);
      vector<double> edge_centrality(g->esize(), 0.0);

      for (node_iterator i=g->nbegin(), nend=g->nend(); i!=nend; ++i) {

        // **********  Catch a termination signal  **********
        if (parent && parent->cancel_check())
          return make_pair(false, make_pair(vector<double>(), vector<double>()));


        if (status && message)
          *status
            = (boost::wformat(L"%s: %d%%")
               %message->get(Message::CALCULATING_BETWEENNESS_CENTRALITY)
               %int(100.0*((*i)->index()+1.0)/g->nsize())).str();

        Node const* snk = *i;
        vector<Node const*> src_array;
        vector<int> weight(g->nsize(), 0);
        weight[snk->index()] = 1;

        // Traversing starts from the neighboring node of &snk.
        t->reset();
        t->start(snk);
        t->advance();

        for (; !t->end(); t->advance()) {

          // **********  Catch a termination signal  **********
          if (parent && parent->cancel_check())
            return make_pair(false, make_pair(vector<double>(), vector<double>()));

          src_array.push_back(t->node());

          vector<pair<Edge*, Node*> >::const_iterator p = t->pbegin(src_array.back());
          vector<pair<Edge*, Node*> >::const_iterator pend = t->pend(src_array.back());

          for (; p!=pend; ++p)
            weight[t->node()->index()] += weight[p->second->index()];
        }

        vector<double> u(g->nsize(), 0.0);

        vector<Node const*>::const_reverse_iterator s = src_array.rbegin();
        vector<Node const*>::const_reverse_iterator send = src_array.rend();

        for (; s!=send; ++s) {

          // **********  Catch a termination signal  **********
          if (parent && parent->cancel_check())
            return make_pair(false, make_pair(vector<double>(), vector<double>()));

          Node const* src = *s;
          size_t const sid = src->index();
          double const u_base = (u[sid]+1.0)/weight[sid];

          vector<pair<Edge*, Node*> >::const_iterator p = t->pbegin(src);
          vector<pair<Edge*, Node*> >::const_iterator pend = t->pend(src);

          for (; p!=pend; ++p) {
            size_t const nid = p->second->index();
            size_t const eid = p->first->index();
            double const uw = u_base*weight[nid];

            edge_centrality[eid] += uw;

            if (p->second!=snk)
              node_centrality[nid] += uw;

            u[nid] += uw;
          }
        }
      }

      return make_pair(true, make_pair(node_centrality, edge_centrality));
    }


    ////////////////////////////////////////////////////////////////////////////////
    pair<bool, vector<double> > pagerank(
      Thread* parent, wstring* status,
      Message const* message,
      shared_ptr<Graph const> g,
      double dumping_factor,
      double err) {

      size_t const nsz = g->nsize();
      double const jump = (1.0-dumping_factor)/nsz;
      mapped_matrix<double> m(nsz, nsz);

      for (size_t i=0; i<nsz; ++i) {
        Node const* n = g->node(i);
        size_t const odeg = n->odegree();

        // **********  Catch a termination signal  **********
        if (parent && parent->cancel_check())
          return make_pair(false, vector<double>());

        if (odeg) {
          double const p = dumping_factor/odeg;
          adjacency_list_iterator c = n->obegin();
          adjacency_list_iterator end = n->oend();
          for (; c!=end; ++c)
            m((*c)->target()->index(), i) = p;
          for (size_t j=0; j<nsz; ++j)
            m(j, i) += jump;
        } else {
          double const p = 1.0/nsz;
          for (size_t j=0; j<nsz; ++j)
            m(j, i) = p;
        }
      }

      for (size_t i=0; i<nsz; ++i)
        m(i,i) += 1.0; // origin shift

      boost::numeric::ublas::vector<double, vector<double> >
        v(1, vector<double>(nsz, 1.0));

      for (;;) {

        // **********  Catch a termination signal  **********
        if (parent && parent->cancel_check())
          return make_pair(false, vector<double>());

        boost::numeric::ublas::vector<double, vector<double> > v_tmp(v);
        static int const iteration = 10; // heuristic

        for (int j=0; j<iteration; ++j)
          v = prod(m, v);

        v /= norm_2(v); // normalize
        v_tmp -= v;
        double e = 0.0; // square sum

        for (size_t i=0; i<nsz; ++i)
          e += v_tmp[i]*v_tmp[i];

        if (e<err) break; // Calculation converged.

        if (status && message)
          *status
            = (boost::wformat(L"%s: %.3f")
               %message->get(Message::CALCULATING_PAGERANK)
               %e).str();
      }

      vector<double> retval(v.data());
      return make_pair(true, retval);
    }


    ////////////////////////////////////////////////////////////////////////////////
    pair<bool, double> modularity(
      Thread* parent, wstring* status,
      Message const* message,
      shared_ptr<Graph const> g,
      vector<vector<Node*> > const& community,
      vector<double> const& edge_weight) {

      size_t const nsz = g->nsize();
      size_t const esz = g->esize();
      size_t const csz = community.size();

      vector<size_t> cid(nsz, 0);
      for (size_t i=0; i<csz; ++i)
        for (size_t j=0; j<community[i].size(); ++j)
          cid[community[i][j]->index()] = i;

      vector<double> eii(csz, 0);
      vector<double> ai(csz, 0);
      double retval = 0.0;

      if (edge_weight.empty()) {
        // Unweighted modularity.

        int count = 0;

        edge_iterator i   = g->ebegin();
        edge_iterator end = g->eend();

        for (; i!=end; ++i) {

          // **********  Catch a termination signal  **********
          if (parent && parent->cancel_check())
            return make_pair(false, 0.0);

          if (status && message)
            *status
              = (boost::wformat(L"%s: %d%%")
                 %message->get(Message::CALCULATING_MODULARITY)
                 %int(100.0*(++count)/esz)).str();

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

        for (size_t i=0; i<csz; ++i) {

          // **********  Catch a termination signal  **********
          if (parent && parent->cancel_check())
            return make_pair(false, 0.0);

          double const a = (0.5*ai[i])/esz;
          retval += double(eii[i])/esz-a*a;
        }

      } else {
        // Weighted modularity.

        int count = 0;
        double w_sum = 0.0;

        edge_iterator i   = g->ebegin();
        edge_iterator end = g->eend();

        for (; i!=end; ++i) {

          // **********  Catch a termination signal  **********
          if (parent && parent->cancel_check())
            return make_pair(false, 0.0);

          if (status && message)
            *status
              = (boost::wformat(L"%s: %d%%")
                 %message->get(Message::CALCULATING_MODULARITY)
                 %int(100.0*(++count)/esz)).str();

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

        for (size_t i=0; i<csz; ++i) {

          // **********  Catch a termination signal  **********
          if (parent && parent->cancel_check())
            return make_pair(false, 0.0);

          double const a = (0.5*ai[i])/w_sum;
          retval += double(eii[i])/w_sum-a*a;
        }
      }

      return make_pair(true, retval);
    }


    namespace {

      ////////////////////////////////////////////////////////////////////////////////
      // Return true if @lhs is a subset of @rhs.
      template <typename T>
      bool is_subset(vector<T> const& lhs, vector<T> const& rhs) {

        typename vector<T>::const_iterator i    = lhs.begin();
        typename vector<T>::const_iterator lend = lhs.end();

        for (; i!=lend; ++i) {

          typename vector<T>::const_iterator j    = rhs.begin();
          typename vector<T>::const_iterator rend = rhs.end();

          for (; j!=rend; ++j)
            if (*i==*j)
              goto detected;
          return false;
        detected:;
        }

        return true;
      }

      ////////////////////////////////////////////////////////////////////////////////
      // Return true if @lhs is a subset of either of element in @rhs.
      template <typename T>
      bool is_subset(vector<T> const& lhs, vector<vector<T> > const& rhs) {

        typename vector<vector<T> >::const_iterator i   = rhs.begin();
        typename vector<vector<T> >::const_iterator end = rhs.end();

        for (; i!=end; ++i)
          if (is_subset(lhs, *i))
            return true;

        return false;
      }

      ////////////////////////////////////////////////////////////////////////////////
      // Return true if @n is adjacent to all nodes in @rhs.
      bool is_completely_adjacent(Node const* n, vector<Node*> const& rhs) {

        vector<Node*>::const_iterator i   = rhs.begin();
        vector<Node*>::const_iterator end = rhs.end();

        for (; i!=end; ++i)
          if (n->find(n->begin(), n->end(), *i)==n->end())
            return false;

        return true;
      }

    } // The end of the anonymous namespace


    ////////////////////////////////////////////////////////////////////////////////
    pair<bool, vector<vector<Node*> > > largest_cliques(
      Thread* parent, wstring* status,
      Message const* message,
      shared_ptr<Graph const> g) {

      size_t const esz = g->esize();

      vector<vector<Node*> > retval;

      int count = 0;

      node_iterator i   = g->nbegin();
      node_iterator end = g->nend();

      for (; i!=end; ++i) {

        // **********  Catch a termination signal  **********
        if (parent && parent->cancel_check())
          return make_pair(false, vector<vector<Node*> >());

        if (status && message)
          *status
            = (boost::wformat(L"%s: %d%%")
               %message->get(Message::CALCULATING_LARGEST_CLIQUES)
               %int(100.0*(++count)/esz)).str();

        Node* n = *i;
        vector<Node*> stk_n(1, n);
        vector<adjacency_list_iterator> stk_c;
        adjacency_list_iterator c    = n->begin();
        adjacency_list_iterator cend = n->end();

        for (;;) {

          bool newly_added = false;

          for (; c!=cend; ++c) {

            // **********  Catch a termination signal  **********
            if (parent && parent->cancel_check())
              return make_pair(false, vector<vector<Node*> >());

            Node* m = get_pair(n, c);

            if (m->index()>n->index()
                && is_completely_adjacent(m, stk_n)) {
              stk_n.push_back(m);
              stk_c.push_back(c);
              newly_added = true;
            }
          }

          if (newly_added && stk_n.size()>0 && !is_subset(stk_n, retval))
            retval.push_back(stk_n);

          stk_n.pop_back();

          if (stk_n.empty()) break;

          c = ++stk_c.back();
          stk_c.pop_back();
        }
      }

      return make_pair(true, retval);
    }


    ////////////////////////////////////////////////////////////////////////////////
    pair<bool, vector<vector<Node*> > > clique_communities_3(
      Thread* parent, wstring* status,
      Message const* message,
      shared_ptr<Graph const> g) {

      size_t const nsz = g->nsize();
      size_t const esz = g->esize();

      if (nsz==0 || esz==0)
        return make_pair(false, vector<vector<Node*> >());

      vector<int> nflag(nsz, -1);
      vector<int> eflag(esz, -1);

      int cid = 0;
      int count = 0;

      vector<vector<Node*> > retval;

      for (size_t i=0; i<esz; ++i) {

        // **********  Catch a termination signal  **********
        if (parent && parent->cancel_check())
          return make_pair(false, vector<vector<Node*> >());

        if (eflag[i]!=-1) continue;

        Edge const* e = g->edge(i);

        if (e->source()==e->target()) {
          ++count;
          continue;
        }

        vector<Edge const*> estack(1, e);

        while (!estack.empty()) {

          // **********  Catch a termination signal  **********
          if (parent && parent->cancel_check())
            return make_pair(false, vector<vector<Node*> >());

          e = estack.back();
          estack.pop_back();

          eflag[e->index()] = cid;

          if (status && message)
            *status
              = (boost::wformat(L"%s: %d%%")
                 %message->get(Message::CALCULATING_3_CLIQUE_COMMUNITIES)
                 %int(100.0*(++count)/esz)).str();

          Node const* s = e->source();
          Node const* t = e->target();

          adjacency_list_iterator j    = s->begin();
          adjacency_list_iterator send = s->end();

          for (; j!=send; ++j) {

            // **********  Catch a termination signal  **********
            if (parent && parent->cancel_check())
              return make_pair(false, vector<vector<Node*> >());

            Edge const* e0 = *j;
            Node const* n0 = s==(*j)->source()?(*j)->target():(*j)->source();

            if (e0==e) continue;
            if (n0==s) continue;

            adjacency_list_iterator k    = t->begin();
            adjacency_list_iterator tend = t->end();

            for (; k!=tend; ++k) {

              // **********  Catch a termination signal  **********
              if (parent && parent->cancel_check())
                return make_pair(false, vector<vector<Node*> >());

              Edge const* e1 = *k;
              Node const* n1 = t==(*k)->target()?(*k)->source():(*k)->target();

              if (e1==e) continue;
              if (n1==t) continue;
              if (n0!=n1) continue;

              if (nflag[n0->index()]<cid)
                nflag[n0->index()] = cid;

              if (eflag[e0->index()]<cid) {
                eflag[e0->index()] = cid;
                estack.push_back(e0);
              }

              if (eflag[e1->index()]<cid) {
                eflag[e1->index()] = cid;
                estack.push_back(e1);
              }
            }
          }
        }

        vector<Node*> c;

        for (size_t j=0; j<nsz; ++j)
          if (nflag[j]==cid)
            c.push_back(g->node(j));

        if (!c.empty())
          retval.push_back(c);

        ++cid;
      }

      return make_pair(true, retval);
    }

  } // The end of the namespace "sociarium_project_graph_utility"

} // The end of the namespace "hashimoto_ut"

// s.o.c.i.a.r.i.u.m: module/community_detection_modularity_maximization_using_greedy_method.cpp
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

#include <list>
#include <numeric>
#include <boost/format.hpp>
#include <boost/numeric/ublas/matrix_sparse.hpp>
#include "community_detection.h"
#include "../graph_utility.h"
#include "../menu_and_message.h"
#include "../../shared/thread.h"
#include "../../graph/graph.h"

BOOL WINAPI DllMain (HINSTANCE hinstDll, DWORD fdwReason, LPVOID lpvReserved) {
  return TRUE;
}

namespace hashimoto_ut {

  using std::vector;
  using std::list;
  using std::wstring;
  using std::pair;
  using std::accumulate;
  using std::tr1::shared_ptr;
  using boost::numeric::ublas::mapped_matrix;

  using namespace sociarium_project_module_community_detection;
  using namespace sociarium_project_menu_and_message;

  ////////////////////////////////////////////////////////////////////////////////
  // Modularity Maximization with Greedy Method
  extern "C" __declspec(dllexport)
    void __cdecl detect_community(

      Thread& parent,
      wstring& status,
      Message const& message,
      vector<vector<Node*> >& community,
      bool& is_canceled,
      shared_ptr<Graph const> g,
      vector<double> const& edge_weight) {

      is_canceled = false;
      size_t const nsz = g->nsize();
      size_t const esz = g->esize();
      if (nsz==0 || esz==0) return;

      vector<vector<Node*> >(nsz, vector<Node*>(1)).swap(community);
      vector<size_t> cid(nsz);

      for (size_t i=0; i<nsz; ++i) {
        community[i][0] = g->node(i);
        cid[i] = i;
      }

      // Make a modularity matrix.
      mapped_matrix<double> mm(nsz, nsz);

      if (edge_weight.empty()) {
        // normal modularity
        double const w = 0.5/esz;

        for (edge_iterator i=g->ebegin(), end=g->eend(); i!=end; ++i) {
          size_t const s = (*i)->source()->index();
          size_t const t = (*i)->target()->index();
          mm(s, t) += w;
          mm(t, s) += w;
        }
      } else{
        // weighted modularity
        double w_sum = 0.0;

        for (edge_iterator i=g->ebegin(), end=g->eend(); i!=end; ++i)
          w_sum += edge_weight[(*i)->index()];

        for (edge_iterator i=g->ebegin(), end=g->eend(); i!=end; ++i) {
          size_t const s = (*i)->source()->index();
          size_t const t = (*i)->target()->index();
          double const w = 0.5*edge_weight[(*i)->index()]/w_sum;
          mm(s, t) += w;
          mm(t, s) += w;
        }
      }

      vector<double> a(nsz, 0.0);
      for (size_t i=0; i<nsz; ++i)
        for (size_t j=0; j<nsz; ++j)
          a[i] += mm(i, j);

      list<Edge const*> e_target(g->ebegin(), g->eend());

      // Calculate initial modularity.
      pair<bool, double> qq
        = sociarium_project_graph_utility::modularity(
          &parent, 0, &message, g, community, edge_weight);

      if (qq.first==false) {
        is_canceled = true;
        return;
      }

      double q = qq.second;

      for (size_t i=0; community.size()>1; ++i) {

        // **********  Catch a termination signal  **********
        if (parent.cancel_check()) {
          is_canceled = true;
          return;
        }

        status
          = (boost::wformat(L"%s: Q=%.03f [%d]")
             %message.get(Message::MODULARITY_MAXIMIZATION_USING_GREEDY_METHOD)
             %q%community.size()).str();

        ////////////////////////////////////////////////////////////////////////////////
        // Find the edge having the max value of dq.
        pair<double, Edge const*> dq_max(-1.0, 0);

        for (list<Edge const*>::iterator j=e_target.begin(); j!=e_target.end();) {

          // **********  Catch a termination signal  **********
          if (parent.cancel_check()) {
            is_canceled = true;
            return;
          }

          Edge const* e = *j;
          size_t const s = cid[e->source()->index()];
          size_t const t = cid[e->target()->index()];

          if (s==t) {
            j = e_target.erase(j);
            continue;
          }

          double dq = 2.0*(mm(s, t)-a[s]*a[t]);

          if (dq>dq_max.first) {
            dq_max.first = dq;
            dq_max.second = *j;
          }

          ++j;
        }

        if (dq_max.first<0.0) return;

        q += dq_max.first;
        Edge const* e_max = dq_max.second;

        status
          = (boost::wformat(L"%s: Q=%.03f [%d]")
             %message.get(Message::MODULARITY_MAXIMIZATION_USING_GREEDY_METHOD)
             %q%community.size()).str();

        ////////////////////////////////////////////////////////////////////////////////
        // Merge two clusters
        size_t const from = cid[e_max->source()->index()];
        size_t const to   = cid[e_max->target()->index()];
        size_t const n = nsz-i;

        a[to] += a[from];
        a[from] = a[n-1];

        for (size_t j=0; j<n; ++j) {

          // **********  Catch a termination signal  **********
          if (parent.cancel_check()) {
            is_canceled = true;
            return;
          }

          mm(to, j) += mm(from, j);
        }

        for (size_t j=0; j<n; ++j) {

          // **********  Catch a termination signal  **********
          if (parent.cancel_check()) {
            is_canceled = true;
            return;
          }

          mm(j, to) += mm(j, from);
        }

        for (size_t j=0; j<n; ++j) {

          // **********  Catch a termination signal  **********
          if (parent.cancel_check()) {
            is_canceled = true;
            return;
          }

          mm(from, j) = mm(n-1, j);
        }

        for (size_t j=0; j<n; ++j) {

          // **********  Catch a termination signal  **********
          if (parent.cancel_check()) {
            is_canceled = true;
            return;
          }

          mm(j, from) = mm(j, n-1);
        }

        size_t const from2 = cid[community[from][0]->index()];
        size_t const to2   = cid[community[to][0]->index()];

        for (node_iterator j=community[from].begin(); j!=community[from].end(); ++j)
          cid[(*j)->index()] = to2;

        community[to].insert(community[to].end(), community[from].begin(), community[from].end());
        community[from].swap(community.back());
        community.pop_back();

        if (from!=community.size())
          for (node_iterator j=community[from].begin(); j!=community[from].end(); ++j)
            cid[(*j)->index()] = from2;
      }
    }

} // The end of the namespace "hashimoto_ut"

// s.o.c.i.a.r.i.u.m: module/community_detection_modularity_maximization_using_t-eo_method.cpp
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

#include <ctime>
#include <cmath>
#include <vector>
#include <map>
#include <boost/format.hpp>
#include <boost/random.hpp>
#include "community_detection.h"
#include "../language.h"
#include "../../shared/thread.h"
#include "../../graph/graph.h"

BOOL WINAPI DllMain (HINSTANCE hinstDll, DWORD fdwReason, LPVOID lpvReserved) {
  return TRUE;
}

namespace hashimoto_ut {

  using std::vector;
  using std::multimap;
  using std::make_pair;
  using std::wstring;
  using std::pair;
  using std::tr1::shared_ptr;

  using namespace sociarium_project_module_community_detection;
  using namespace sociarium_project_language;

  namespace {
    int const alpha = 50;

    // fitness
    vector<double> calc_fitness(
      shared_ptr<Graph const> g,
      vector<Node*> const& nodes,
      vector<int> const& grouping, // 1 or 0
      double const& total_weight,
      vector<double> const& node_weight,
      vector<double> const& edge_weight) {

      size_t const csz = nodes.size();
      vector<size_t> index(g->nsize(), size_t(-1));
      for (size_t i=0; i<csz; ++i)
        index[nodes[i]->index()] = i;

      double w_sum[2] = { 0, 0 };
      vector<double> w_in(csz, 0);

      for (size_t i=0; i<csz; ++i) {
        Node const* n = nodes[i];
        int const b = grouping[i];
        double& w = w_in[i];
        w_sum[b] += node_weight[n->index()];

        for (adjacency_list_iterator j=n->obegin(); j!=n->oend(); ++j) {
          size_t const& jj = index[(*j)->target()->index()];
          if (jj!=size_t(-1) && b==grouping[jj])
            w += edge_weight[(*j)->index()];
        }

        for (adjacency_list_iterator j=n->ibegin(); j!=n->iend(); ++j) {
          size_t const& jj = index[(*j)->source()->index()];
          if (jj!=size_t(-1) && b==grouping[jj])
            w += edge_weight[(*j)->index()];
        }
      }

      double const a[2] = { 0.5*w_sum[0]/total_weight, 0.5*w_sum[1]/total_weight };
      vector<double> retval(csz, 0.0);
      for (size_t i=0; i<csz; ++i) {
        double const& nw = node_weight[nodes[i]->index()];
        if (nw>0.0)
          retval[i] = w_in[i]/nw-a[grouping[i]];
      }

      return retval;
    }
  }


  ////////////////////////////////////////////////////////////////////////////////
  // Modularity Maximization with t-EO Method
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

      time_t tm;
      boost::mt19937 generator((unsigned long)time(&tm));
      boost::uniform_real<> distribution(0.0, 1.0);
      boost::variate_generator<boost::mt19937, boost::uniform_real<> > rand(generator, distribution);

      vector<vector<Node*> > c(1, vector<Node*>(g->nbegin(), g->nend()));
      community.clear();

      vector<double> node_weight(nsz, 0.0);
      vector<double> const ew(esz, 1.0);
      double total_weight = 0.0;

      if (edge_weight.empty()) {
        for (size_t i=0; i<nsz; ++i)
          node_weight[i] = double(g->node(i)->degree());
        total_weight = double(g->esize());
      } else {
        assert(esz==edge_weight.size());
        for (edge_iterator i=g->ebegin(), end=g->eend(); i!=end; ++i) {
          double const& w = edge_weight[(*i)->index()];
          total_weight += w;
          node_weight[(*i)->source()->index()] += w;
          node_weight[(*i)->target()->index()] += w;
        }
      }

      while (!c.empty()) {
        vector<Node*> const& target_group = c.back();
        int const csz = int(target_group.size());
        assert(csz>1);
        vector<int> grouping(csz, 0);
        vector<int> grouping_max; // the best grouping maximizing the modularity

        // **********  Catch a termination signal  **********
        if (parent.cancel_check()) {
          is_canceled = true;
          return;
        }

        vector<double> f
          = calc_fitness(g, target_group, grouping, total_weight,
                         node_weight, edge_weight.empty()?ew:edge_weight);

        double q_max = 0.0;

        for (int i=0; i<csz; ++i)
          q_max += f[i]*node_weight[target_group[i]->index()];

        double const q_init = q_max;

        // Divide @target_group into two groups: '0' and '1'.
        grouping.resize(csz/2);
        grouping.resize(csz, 1);

        // Repeat moving nodes for sufficient times between two groups.
        for (int trial=0, number_of_trials=alpha*csz; trial<number_of_trials; ++trial) {

          // **********  Catch a termination signal  **********
          if (parent.cancel_check()) {
            is_canceled = true;
            return;
          }

          status
            = (boost::wformat(L"%s: Q=%.3f [%d]")
               %message.get(Message::MODULARITY_MAXIMIZATION_USING_TEO_METHOD)
               %(0.5*q_max/g->esize())%community.size()).str();

          // Calculate the rank from the fitness;
          // worse the fitness, smaller the rank.
          multimap<double, int> m; // sort in ascending order of the fitness.

          for (int i=0; i<csz; ++i)
            m.insert(make_pair(f[i], i));

          assert(int(m.size())==csz);
          vector<double> p(csz, 0.0); // rank
          int counter = 0;

          for (multimap<double, int>::const_iterator i=m.begin(), mend=m.end(); i!=mend; ++i)
            p[i->second] = double(++counter);

          // Calculate the probability from the rank;
          // smaller the rank, higher the probability.
          double p_cum = 0.0;

          //double const exp = 1.0+1.0/log(double(csz)); ??
          double const exp = 1.0+1.0/log(double(nsz));

          for (int i=0; i<csz; ++i) {
            p[i] = pow(p[i], -exp);
            p_cum += p[i];
            p[i] = p[i]/p_cum;
          }

          // Choose one node according to the rated probability,
          // and change its belonging group.
          while (p.back()<rand())
            p.pop_back();

          int const selected = int(p.size())-1;
          grouping[selected] = (~grouping[selected])&0x01; // flip a bit

          // Calculate the fitness of post-moving state,
          // and if the value is the highest in past, memorize the value.

          // **********  Catch a termination signal  **********
          if (parent.cancel_check()) {
            is_canceled = true;
            return;
          }

          f = calc_fitness(g, target_group, grouping, total_weight,
                           node_weight, edge_weight.empty()?ew:edge_weight);

          double q = 0.0;

          for (int i=0; i<csz; ++i)
            q += f[i]*node_weight[target_group[i]->index()];

          if (q>q_max) {
            q_max = q;
            grouping_max = grouping;
          }
        }

        // If the modularity of post-divided state is higher than
        // pre-divided state, the division is adopted.
        if (q_max>q_init) {
          vector<Node*> divided_group[2];

          for (int i=0; i<csz; ++i)
            divided_group[grouping_max[i]].push_back(target_group[i]);

          c.pop_back();

          for (int i=0; i<2; ++i) {
            if (divided_group[i].size()==1)
              community.push_back(divided_group[i]);
            else if (divided_group[i].size()>1)
              c.push_back(divided_group[i]);
            else assert(0 && "never reach");
          }
        } else {
          community.push_back(target_group);
          c.pop_back();
        }
      }
    }

} // The end of the namespace "hashimoto_ut"

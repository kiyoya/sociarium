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

#include <ctime>
#include <cmath>
#include <vector>
#include <map>
#include <boost/format.hpp>
#include <boost/random.hpp>
#include "community_detection.h"
#include "community_detection_detail.h"
#include "../message.h"
#include "../../shared/thread.h"
#include "../../graph/graph.h"

BOOL WINAPI DllMain (HINSTANCE hinstDll, DWORD fdwReason, LPVOID lpvReserved) { return TRUE; }

namespace hashimoto_ut {

  using std::vector;
  using std::multimap;
  using std::make_pair;
  using std::wstring;
  using std::pair;
  using std::tr1::shared_ptr;
  using namespace sociarium_project_module_community_detection_detail;

  namespace {
    int const alpha = 50;
  }


  extern "C" __declspec(dllexport)
    void __cdecl detect_community(
      Thread* parent,
      wstring* message,
      shared_ptr<Graph const> const& g,
      vector<double> const& edge_weight,
      vector<vector<Node*> >& community,
      bool& is_canceled) {

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
        for (size_t i=0; i<nsz; ++i) node_weight[i] = double(g->node(i)->degree());
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
        int const gsz = int(target_group.size());
        assert(gsz>1);
        vector<int> grouping(gsz, 0);
        vector<int> grouping_max; // the best grouping maximizing the modularity

        // ----------------------------------------------------------------------------------------------------
        if (parent->cancel_check()) { is_canceled = true; return; }
        // ----------------------------------------------------------------------------------------------------

        vector<double> f = calc_fitness(g, target_group, grouping, total_weight,
                                        node_weight, edge_weight.empty()?ew:edge_weight);

        double q_max = 0.0;
        for (int i=0; i<gsz; ++i) q_max += f[i]*node_weight[target_group[i]->index()];
        double const q_init = q_max;
        // divide @target_group into two groups: '0' and '1'.
        grouping.resize(gsz/2);
        grouping.resize(gsz, 1);
        // repeat moving nodes for sufficient times between two groups
        for (int trial=0, number_of_trials=alpha*gsz; trial<number_of_trials; ++trial) {

          if (message)
            *message= (boost::wformat(L"%s: Q=%.3f [%d]")
                       %sociarium_project_message::MODULARITY_MAXIMIZATION_USING_TEO_METHOD
                       %(0.5*q_max/g->esize())%community.size()).str();

          // ----------------------------------------------------------------------------------------------------
          if (parent->cancel_check()) { is_canceled = true; return; }
          // ----------------------------------------------------------------------------------------------------

          // calculate the rank from the fitness; worse the fitness, smaller the rank.
          multimap<double, int> m; // sort in ascending order of the fitness.
          for (int i=0; i<gsz; ++i) m.insert(make_pair(f[i], i));
          assert(int(m.size())==gsz);
          vector<double> p(gsz, 0.0); // rank
          int counter = 0;
          for (multimap<double, int>::const_iterator i=m.begin(), mend=m.end(); i!=mend; ++i)
            p[i->second] = double(++counter);

          // calculate the probability from the rank; smaller the rank, higher the probability.
          double p_cum = 0.0;
          //double const exp = 1.0+1.0/log(double(gsz));
          double const exp = 1.0+1.0/log(double(nsz));
          for (int i=0; i<gsz; ++i) {
            p[i] = pow(p[i], -exp);
            p_cum += p[i];
            p[i] = p[i]/p_cum;
          }
          // choose one node according to the probability, and change its belonging group.
          while (p.back()<rand()) p.pop_back();
          int const selected = int(p.size())-1;
          grouping[selected] = (~grouping[selected])&0x01; // 0->1, 1->0
          // calculate the fitness of post-moving state,
          // and if the value is the highest value in past, memorize that value.

          // ----------------------------------------------------------------------------------------------------
          if (parent->cancel_check()) { is_canceled = true; return; }
          // ----------------------------------------------------------------------------------------------------

          f = calc_fitness(g, target_group, grouping, total_weight,
                           node_weight, edge_weight.empty()?ew:edge_weight);

          double q = 0.0;
          for (int i=0; i<gsz; ++i) q += f[i]*node_weight[target_group[i]->index()];
          if (q>q_max) {
            q_max = q;
            grouping_max = grouping;
          }
        }

        // if the modularity of the post-divided state is higher than
        // the pre-divided state, the division is adopted.
        if (q_max>q_init) {
          vector<Node*> divided_group[2];
          for (int i=0; i<gsz; ++i) divided_group[grouping_max[i]].push_back(target_group[i]);
          c.pop_back();
          for (int i=0; i<2; ++i) {
            if (divided_group[i].size()==1) community.push_back(divided_group[i]);
            else if (divided_group[i].size()>1) c.push_back(divided_group[i]);
            else assert(0 && "never reach");
          }
        } else {
          community.push_back(target_group);
          c.pop_back();
        }
      }
    }

} // The end of the namespace "hashimoto_ut"

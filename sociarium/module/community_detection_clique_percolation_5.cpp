// s.o.c.i.a.r.i.u.m: module/community_detection_clique_percolation_5.cpp
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

#ifdef _MSC_VER
#include <unordered_set>
#else
#include <tr1/unordered_set>
#endif
#include <boost/format.hpp>
#include "community_detection.h"
#include "../graph_utility.h"
#include "../../graph/graphex.h"

#ifdef _MSC_VER
BOOL WINAPI DllMain (HINSTANCE hinstDll, DWORD fdwReason, LPVOID lpvReserved) {
  return TRUE;
}
#endif

namespace hashimoto_ut {

  using std::vector;
  using std::wstring;
  using std::pair;
  using std::tr1::shared_ptr;
  using std::tr1::unordered_set;

  namespace {

    ////////////////////////////////////////////////////////////////////////////////
    bool is_percolatable(vector<Node*> const& lhs, vector<Node*> const& rhs, int num) {

      vector<Node*>::const_iterator i    = lhs.begin();
      vector<Node*>::const_iterator iend = lhs.end();

      for (; i!=iend; ++i) {

        vector<Node*>::const_iterator j0   = rhs.begin();
        vector<Node*>::const_iterator jend = rhs.end();

        for (vector<Node*>::const_iterator j=j0; j!=jend; ++j)
          if (*i==*j && --num<=0)
            return true;
      }

      return false;
    }

  } // The end of the anonymous namespace


  ////////////////////////////////////////////////////////////////////////////////
#ifdef _MSC_VER
  extern "C" __declspec(dllexport)
    void __cdecl detect_community(
#else
  extern "C" void detect_community(
#endif
      Thread& parent,
      wstring& status,
      Message const& message,
      vector<vector<Node*> >& community,
      bool& is_canceled,
      shared_ptr<Graph const> g,
      vector<double> const& edge_weight) {

      is_canceled = false;

      size_t const threshold = 4;

      // Extract 3 clique communities.
      pair<bool, vector<vector<Node*> > >
        cc3 = sociarium_project_graph_utility::clique_communities_3(
          &parent, &status, &message, g);

      if (cc3.first==false) {
        is_canceled = true;
        return;
      }

      vector<vector<Node*> > const& c3 = cc3.second;
      unordered_set<Node*> extracted_nodes_s;

      for (size_t i=0; i<c3.size(); ++i)
        if (c3[i].size()>threshold)
          extracted_nodes_s.insert(c3[i].begin(), c3[i].end());

      if (extracted_nodes_s.empty())
        return;

      vector<Node*> extracted_nodes(extracted_nodes_s.begin(), extracted_nodes_s.end());

      // Rebuild a graph using extracted nodes.
      shared_ptr<Graph> g_tmp
        = g->copy_induced_subgraph(extracted_nodes.begin(), extracted_nodes.end());

      // Extract the largest cliques.
      pair<bool, vector<vector<Node*> > >
        cc_tmp = sociarium_project_graph_utility::largest_cliques(
          &parent, &status, &message, g_tmp);

      if (cc_tmp.first==false) {
        is_canceled = true;
        return;
      }

      vector<vector<Node*> > c_tmp = cc_tmp.second;
      size_t const csz_tmp = c_tmp.size();
      vector<int> flag(csz_tmp, 0);

      community.clear();

      // Merge the largest cliques.
      for (size_t i=0; i<csz_tmp; ++i) {

        vector<Node*>& ci = c_tmp[i];

        if (flag[i]!=0 || ci.size()<=threshold) continue;

        for (size_t j=i+1; j<csz_tmp; ++j) {

          vector<Node*>& cj = c_tmp[j];

          if (flag[j]!=0 || cj.size()<=threshold) continue;

          if (is_percolatable(ci, cj, threshold)) {
            // Merge two cliques and eliminate duplicated nodes.
            unordered_set<Node*> k(ci.begin(), ci.end());
            k.insert(cj.begin(), cj.end());
            ci.clear();
            cj.clear();
            ci.insert(ci.end(), k.begin(), k.end());
            flag[j] = 1;
          }
        }

        size_t const csz = ci.size();
        vector<Node*> c(csz);

        for (size_t j=0; j<csz; ++j)
          c[j] = extracted_nodes[ci[j]->index()];

        community.push_back(c);
      }
    }

} // The end of the namespace "hashimoto_ut"

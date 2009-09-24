// s.o.c.i.a.r.i.u.m: module/community_detection_betweenness_centrality_clustering.cpp
// HASHIMOTO, Yasuhiro (E-mail: hy @ sys.t.u-tokyo.ac.jp)
// update: 2009/08/31

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
#include <boost/format.hpp>
#include "community_detection.h"
#include "../graph_utility.h"
#include "../language.h"
#include "../../shared/thread.h"
#include "../../graph/graph.h"
#include "../../graph/util/traverser.h"

BOOL WINAPI DllMain (HINSTANCE hinstDll, DWORD fdwReason, LPVOID lpvReserved) {
  return TRUE;
}

namespace hashimoto_ut {

  using std::vector;
  using std::multimap;
  using std::wstring;
  using std::pair;
  using std::make_pair;
  using std::tr1::shared_ptr;

  using namespace sociarium_project_module_community_detection;
  using namespace sociarium_project_language;

  extern "C" __declspec(dllexport)
    void __cdecl detect_community(

      Thread* parent,
      wstring* status,
      Message const* message,
      vector<vector<Node*> >& community,
      bool& is_canceled,
      shared_ptr<Graph const> g,
      vector<double> const& edge_weight) {

      is_canceled = false;
      size_t const nsz = g->nsize();
      size_t const esz = g->esize();

      if (nsz==0 || esz==0) return;

      shared_ptr<BFSRecordingTraverser> t =
        g->is_directed()?
          BFSRecordingTraverser::create<downward_tag>(g)
            :BFSRecordingTraverser::create<bidirectional_tag>(g);

      shared_ptr<CircumventSpecifiedEdges>
        cond(new CircumventSpecifiedEdges(g, g->eend(), g->eend()));

      t->set_condition(cond);

      pair<bool, vector<vector<Node*> > > c
        = sociarium_project_graph_utility::connected_components(parent, 0, message, t);

      size_t number_of_communities = 0;

      if (c.first)
        number_of_communities = c.second.size();
      else {
        // **********  Catch a termination signal  **********
        is_canceled = true;
        return;
      }

      typedef multimap<double, Edge*, std::greater<double> > BC2Edge;
      size_t count = 0;

      for (; count<esz; ++count) {

        // **********  Catch a termination signal  **********
        if (parent->cancel_check()) {
          is_canceled = true;
          return;
        }

        if (status)
          *status
            = (boost::wformat(L"%s: %d/%d")
               %message->get(Message::BETWEENNESS_CENTRALITY_CLUSTERING)
               %(count+1)%esz).str();

        t->reset();

        pair<bool, pair<vector<double>, vector<double> > > bc
          = sociarium_project_graph_utility::betweenness_centrality(parent, 0, message, t);

        if (bc.first==false) {
          // **********  Catch a termination signal  **********
          is_canceled = true;
          return;
        }

        BC2Edge bc2edge;

        for (size_t i=0; i<esz; ++i)
          bc2edge.insert(make_pair(bc.second.second[i], g->edge(i)));

        cond->flag_[bc2edge.begin()->second->index()] = ConditionalPass::CLOSED;

        // **********  Catch a termination signal  **********
        if (parent->cancel_check()) {
          is_canceled = true;
          return;
        }

        t->reset();

        pair<bool, vector<vector<Node*> > > c
          = sociarium_project_graph_utility::connected_components(parent, 0, message, t);

        if (c.first==false) {
          // **********  Catch a termination signal  **********
          is_canceled = true;
          return;
        }

        if (c.second.size()>number_of_communities) {

          vector<vector<Node*> >::const_iterator i   = c.second.begin();
          vector<vector<Node*> >::const_iterator end = c.second.end();

          for (; i!=end; ++i)
            if (i->size()>1)
              community.push_back(*i);

          break;
        }
      }
    }

} // The end of the namespace "hashimoto_ut"

// s.o.c.i.a.r.i.u.m
// HASHIMOTO, Yasuhiro (E-mail: hy @ sys.t.u-tokyo.ac.jp)
// update: 2009/05/10

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

#include <map>
#include <boost/format.hpp>
#include "community_detection.h"
#include "../message.h"
#include "../../shared/thread.h"
#include "../../shared/msgbox.h"
#include "../../graph/graph.h"
#include "../../graph/util/traverser.h"

BOOL WINAPI DllMain (HINSTANCE hinstDll, DWORD fdwReason, LPVOID lpvReserved) { return TRUE; }

namespace hashimoto_ut {

  using std::vector;
  using std::multimap;
  using std::wstring;
  using std::make_pair;
  using std::tr1::shared_ptr;

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

      typedef multimap<int, Node*, std::greater<int> > BacktrackOrder;
      BacktrackOrder backtrack_order;

      size_t count = 0;
      double const denom = 3.0*nsz;

      { // Sort nodes in descending DFS-backtrack-order.
        shared_ptr<DFSTraverser> t(DFSTraverser::create<downward_tag>(g));
        for (node_iterator i=g->nbegin(), end=g->nend(); i!=end; ++i) {

          if (message)
            *message = (boost::wformat(L"%s: %d%%")
                        %sociarium_project_message::STRONGLY_CONNECTED_COMPONENTS
                        %int(100.0*(++count)/denom)).str();

          for (t->start(*i); !t->end(); t->advance()) {
            // ----------------------------------------------------------------------------------------------------
            if (parent->cancel_check()) { is_canceled = true; return; }
            // ----------------------------------------------------------------------------------------------------
          }
        }

        for (node_iterator i=g->nbegin(), end=g->nend(); i!=end; ++i) {

          // ----------------------------------------------------------------------------------------------------
          if (parent->cancel_check()) { is_canceled = true; return; }
          // ----------------------------------------------------------------------------------------------------

          if (message)
            *message = (boost::wformat(L"%s: %d%%")
                        %sociarium_project_message::STRONGLY_CONNECTED_COMPONENTS
                        %int(100.0*(++count)/denom)).str();

          backtrack_order.insert(make_pair(t->backtrack_order(*i), *i));
        }
      }

      { // The nodes that reverse-search can reach compose SCC.
        shared_ptr<BFSTraverser> t(BFSTraverser::create<upward_tag>(g));
        for (BacktrackOrder::const_iterator i=backtrack_order.begin(), end=backtrack_order.end(); i!=end; ++i) {
          vector<Node*> c;
          for (t->start(i->second); !t->end(); t->advance()) {

            // ----------------------------------------------------------------------------------------------------
            if (parent->cancel_check()) { is_canceled = true; return; }
            // ----------------------------------------------------------------------------------------------------

            if (message)
              *message = (boost::wformat(L"%s: %d%%")
                          %sociarium_project_message::STRONGLY_CONNECTED_COMPONENTS
                          %int(100.0*(++count)/denom)).str();

            c.push_back(const_cast<Node*>(t->node()));
          }

          if (c.size()>1) community.push_back(c);
        }
      }
    }

} // The end of the namespace "hashimoto_ut"

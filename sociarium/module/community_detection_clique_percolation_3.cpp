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

#include <boost/format.hpp>
#include "community_detection.h"
#include "../message.h"
#include "../../shared/thread.h"
#include "../../shared/msgbox.h"
#include "../../graph/graph.h"

BOOL WINAPI DllMain (HINSTANCE hinstDll, DWORD fdwReason, LPVOID lpvReserved) { return TRUE; }

namespace hashimoto_ut {

  using std::vector;
  using std::wstring;
  using std::pair;
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

      vector<int> nflag(nsz, -1);
      vector<int> eflag(esz, -1);

      int cid = 0;

      for (size_t i=0; i<esz; ++i) {

        // ----------------------------------------------------------------------------------------------------
        if (parent->cancel_check()) { is_canceled = true; return; }
        // ----------------------------------------------------------------------------------------------------

        if (message)
          *message = (boost::wformat(L"3-%s: %d%%")
                      %sociarium_project_message::CLIQUE_PERCOLATION
                      %int(100.0*(i+1)/esz)).str();

        if (eflag[i]!=-1) continue;
        Edge const* e = g->edge(i);
        if (e->source()==e->target()) continue;

        vector<Edge const*> estack(1, e);
        while (!estack.empty()) {

          // ----------------------------------------------------------------------------------------------------
          if (parent->cancel_check()) { is_canceled = true; return; }
          // ----------------------------------------------------------------------------------------------------

          e = estack.back();
          estack.pop_back();
          eflag[e->index()] = cid;
          Node const* s = e->source();
          Node const* t = e->target();

          for (adjacency_list_iterator j=s->begin(), jend=s->end(); j!=jend; ++j) {

            // ----------------------------------------------------------------------------------------------------
            if (parent->cancel_check()) { is_canceled = true; return; }
            // ----------------------------------------------------------------------------------------------------

            Edge const* e0 = *j;
            Node const* n0 = s==(*j)->source()?(*j)->target():(*j)->source();
            if (e0==e) continue;
            if (n0==s) continue;

            for (adjacency_list_iterator k=t->begin(), kend=t->end(); k!=kend; ++k) {

              // ----------------------------------------------------------------------------------------------------
              if (parent->cancel_check()) { is_canceled = true; return; }
              // ----------------------------------------------------------------------------------------------------

              Edge const* e1 = *k;
              Node const* n1 = t==(*k)->source()?(*k)->target():(*k)->source();
              if (e1==e) continue;
              if (n1==t) continue;
              if (n0!=n1) continue;

              if (nflag[n0->index()]<cid) nflag[n0->index()] = cid;

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
        for (size_t j=0; j<nsz; ++j) if (nflag[j]==cid) c.push_back(g->node(j));
        if (!c.empty()) community.push_back(c);
        ++cid;
      }
    }

} // The end of the namespace "hashimoto_ut"

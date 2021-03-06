﻿// s.o.c.i.a.r.i.u.m: module/community_detection_strongly_connected_components.cpp
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

#include <boost/format.hpp>
#include "community_detection.h"
#include "../graph_utility.h"
#include "../../graph/util/traverser.h"

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

      shared_ptr<DFSTraverser> t_forward(DFSTraverser::create<downward_tag>(g));
      shared_ptr<BFSTraverser> t_backward(BFSTraverser::create<upward_tag>(g));

      pair<bool, vector<vector<Node*> > > cc = 
        sociarium_project_graph_utility::strongly_connected_components(
          &parent, &status, &message, t_forward, t_backward, 1);

      if (cc.first==false) {
        is_canceled = true;
        return;
      }

      community.swap(cc.second);
    }

} // The end of the namespace "hashimoto_ut"

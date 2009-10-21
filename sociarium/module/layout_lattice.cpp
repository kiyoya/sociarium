// s.o.c.i.a.r.i.u.m: module/layout_lattice.cpp
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

#include <cassert>
#include <vector>
#include <map>
#ifdef _MSC_VER
#include <windows.h>
#endif
#include "layout.h"
#include "../../shared/thread.h"
#include "../../graph/graph.h"

#ifdef _MSC_VER
BOOL WINAPI DllMain (HINSTANCE hinstDll, DWORD fdwReason, LPVOID lpvReserved) {
  return TRUE;
}
#endif

namespace hashimoto_ut {

  using std::vector;
  using std::deque;
  using std::multimap;
  using std::wstring;
  using std::pair;
  using std::make_pair;
  using std::greater;
  using std::tr1::shared_ptr;

#ifdef _MSC_VER
  extern "C" __declspec(dllexport)
    void __cdecl layout(
#else
  extern "C" void layout(
#endif
      Thread& parent,
      wstring& status,
      Message const& message,
      vector<Vector2<double> >& position,
      shared_ptr<Graph const> graph,
      vector<double> const& hint) {

      size_t const nsz = graph->nsize();
      assert(nsz==position.size());
      assert(nsz==hint.size());

      typedef multimap<double, size_t, greater<double> > SizeOrderMap;
      SizeOrderMap m;
      for (node_iterator i=graph->nbegin(), end=graph->nend(); i!=end; ++i)
        m.insert(make_pair(hint[(*i)->index()], (*i)->index()));

      double const sect = sqrt(double(nsz));
      size_t const num = double(size_t(sect))==sect?size_t(sect):size_t(sect)+1;
      SizeOrderMap::const_iterator i = m.begin();
      if (nsz==1) {
        position[i->second].set(0.0, 0.0);
      } else if (nsz==2) {
        position[i->second].set(-1.0, 0.0); ++i;
        position[i->second].set( 1.0, 0.0);
      } else {
        for (size_t j=0; j<num; ++j)
          for (size_t k=0; k<num && i!=m.end(); ++k, ++i)
            position[i->second].set((2.0*k)/(num-1)-1.0, -(2.0*j)/(num-1)+1.0);
      }
    }

} // The end of the namespace "hashimoto_ut"

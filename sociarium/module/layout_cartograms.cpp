// s.o.c.i.a.r.i.u.m: module/layout_cartograms.cpp
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
#include "../menu_and_message.h"
#include "../../cartograms/cartograms.h"
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

  using namespace sociarium_project_menu_and_message;

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
      assert(nsz+4==hint.size());

      double const lbx = hint[nsz];
      double const lby = hint[nsz+1];
      double const rtx = hint[nsz+2];
      double const rty = hint[nsz+3];
      double const w = rtx-lbx;
      double const h = rty-lby;

      assert(w==h);

      int const resolution = 256; // The division number of transforming space.
      size_t const sz = resolution*resolution;
      vector<double> density(sz, 0.0);
      vector<int> index(nsz, -1);

      for (size_t i=0; i<nsz; ++i) {

        Vector2<double> const& pos = position[i];

        if (lbx<=pos.x && pos.x<=rtx && lby<=pos.y && pos.y<=rty) {
          int x = int(resolution*(pos.x-lbx)/w);
          int y = int(resolution*(pos.y-lby)/h);
          if (x==resolution) x = resolution-1;
          if (y==resolution) y = resolution-1;
          density[resolution*x+y] += 10.0*hint[i];
          index[i] = resolution*x+y;
        }
      }

      status = message.get(Message::CARTOGRAMS);

      vector<Vector2<double> > c
        = cartograms::generate(&parent, &status, density, resolution, resolution, 1.0);

      if (!c.empty()) {

        assert(c.size()==(resolution+1)*(resolution+1));

        vector<Vector2<double> > cc(sz);

        for (size_t x=0, i=0; x<resolution; ++x) {
          for (size_t y=0; y<resolution; ++y, ++i) {

            size_t const i0 = x*(resolution+1)+y;
            size_t const i1 = x*(resolution+1)+y+1;
            size_t const i2 = (x+1)*(resolution+1)+y;
            size_t const i3 = (x+1)*(resolution+1)+y+1;

            assert(i0<(resolution+1)*(resolution+1));
            assert(i1<(resolution+1)*(resolution+1));
            assert(i2<(resolution+1)*(resolution+1));
            assert(i3<(resolution+1)*(resolution+1));

            cc[i] = 0.25*(c[i0]+c[i1]+c[i2]+c[i3]);
          }
        }

        for (size_t i=0; i<nsz; ++i) {
          if (index[i]!=-1) {
            Vector2<double> const& pos = cc[index[i]];
            position[i].set(lbx+w*pos.x/resolution, lby+h*pos.y/resolution);
          }
        }
      }
    }

} // The end of the namespace "hashimoto_ut"

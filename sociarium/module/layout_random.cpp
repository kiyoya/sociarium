// s.o.c.i.a.r.i.u.m
// HASHIMOTO, Yasuhiro (E-mail: hy @ sys.t.u-tokyo.ac.jp)
// update: 2009/03/04

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
#include <vector>
#include <boost/random.hpp>
#ifdef _MSC_VER
#include <windows.h>
#endif
#include "layout.h"
#include "../../shared/thread.h"
#include "../../shared/math.h"
#include "../../graph/graph.h"

BOOL WINAPI DllMain (HINSTANCE hinstDll, DWORD fdwReason, LPVOID lpvReserved) { return TRUE; }

namespace hashimoto_ut {

  using std::vector;
  using std::wstring;
  using std::pair;
  using std::tr1::shared_ptr;

  extern "C" __declspec(dllexport)
    void __cdecl layout_graph(
      Thread* parent,
      shared_ptr<pair<wstring, wstring> > const& message,
      shared_ptr<Graph const> const& graph,
      vector<double> const& input_values,
      vector<Vector2<double> >& position) {

      size_t const nsz = graph->nsize();
      assert(nsz==position.size());
      if (nsz<2) return;

      time_t t;
      boost::mt19937 generator((unsigned long)time(&t));
      boost::uniform_real<> distribution(0.0, 1.0);
      boost::variate_generator<boost::mt19937, boost::uniform_real<> > rand(generator, distribution);

      for (size_t i=0; i<nsz; ++i) position[i].set(2.0*rand()-1.0, 2.0*rand()-1.0);
    }

} // The end of the namespace "hashimoto_ut"

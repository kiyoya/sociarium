// s.o.c.i.a.r.i.u.m
// module/graph_creation_read_sierpinski_gasket_model.cpp
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
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#ifdef _MSC_VER
#include <windows.h>
#endif
#include "graph_creation.h"
#include "../menu_and_message.h"
#include "../../shared/thread.h"
#include "../../shared/util.h"
#include "../../graph/graph.h"
#include "../../graph/graphex.h"

#ifdef _MSC_VER
BOOL WINAPI DllMain (HINSTANCE hinstDll, DWORD fdwReason, LPVOID lpvReserved) {
  return TRUE;
}
#endif

namespace hashimoto_ut {

  using std::vector;
  using std::deque;
  using std::string;
  using std::wstring;
  using std::pair;
  using std::tr1::shared_ptr;
  using std::tr1::unordered_map;

  using namespace sociarium_project_menu_and_message;
  using namespace sociarium_project_module_graph_creation;

#ifdef _MSC_VER
  extern "C" __declspec(dllexport)
    void __cdecl create_graph_time_series(
#else
  extern "C" void create_graph_time_series(
#endif
      Thread& parent,
      deque<wstring>& status,
      Message const& message,
      vector<shared_ptr<Graph> >& graph,
      vector<vector<NodeProperty> >& node_property,
      vector<vector<EdgeProperty> >& edge_property,
      vector<wstring>& layer_name,
      unordered_map<wstring, pair<wstring, int> > const& params,
      vector<pair<wstring, int> > const& data,
      wstring const& filename) {

      assert(graph.empty());
      assert(node_property.empty());
      assert(edge_property.empty());
      assert(layer_name.empty());
      assert(status.size()==2);


      ////////////////////////////////////////////////////////////////////////////////
      // Read parameters.

      bool directed = false;
      wstring title;
      size_t depth = 0;

      unordered_map<wstring, pair<wstring, int> >::const_iterator pos;

      if ((pos=params.find(L"directed"))!=params.end())
        directed = true;

      if ((pos=params.find(L"title"))!=params.end() && !pos->second.first.empty())
        title =pos->second.first;

      if ((pos=params.find(L"depth"))!=params.end()) {
        try {
          depth = boost::lexical_cast<size_t>(pos->second.first);
        } catch (...) {
          throw (boost::wformat(L"bad data: line=%d\n%s")
                 %pos->second.second%filename.c_str()).str();
        }
      }


      ////////////////////////////////////////////////////////////////////////////////
      // Make a graph.

      vector<shared_ptr<Graph> >(1, Graph::create(directed)).swap(graph);
      shared_ptr<Graph>& g = graph[0];

      status[0]
        = (boost::wformat(L"%s")
           %message.get(Message::MAKING_GRAPH_SNAPSHOT)).str();

      status[1]
        = (boost::wformat(L"Sierpinski Gasket [D=%d, N=%d, E=%d]")
           %depth%g->nsize()%g->esize()).str();

      g->add_node();
      g->add_node();
      g->add_node();
      g->add_edge(g->node(0), g->node(1));
      g->add_edge(g->node(1), g->node(2));
      g->add_edge(g->node(2), g->node(0));

      // Merging nodes which degree is two makes a larger triangle.
      for (size_t d=0; d<depth; ++d) {

        status[1]
          = (boost::wformat(L"%s: Sierpinski Gasket [D=%d, N=%d, E=%d]")
             %message.get(Message::MAKING_GRAPH_SNAPSHOT)
             %depth%g->nsize()%g->esize()).str();

        // **********  Catch a termination signal  **********
        if (parent.cancel_check()) {
          graph.clear();
          return;
        }

        shared_ptr<Graph> g1 = copy(g);

        // **********  Catch a termination signal  **********
        if (parent.cancel_check()) {
          graph.clear();
          return;
        }

        shared_ptr<Graph> g2 = copy(g);

        // **********  Catch a termination signal  **********
        if (parent.cancel_check()) {
          graph.clear();
          return;
        }

        g->merge(g1);
        g->merge(g2);

        // **********  Catch a termination signal  **********
        if (parent.cancel_check()) {
          graph.clear();
          return;
        }

        vector<Node*> v;
        for (node_iterator i=g->nbegin(); i!=g->nend(); ++i)
          if ((*i)->degree()==2)
            v.push_back(*i);

        assert(v.size()==9);

        contract(g, v[0], v[3]);
        contract(g, v[1], v[6]);
        contract(g, v[4], v[7]);

        status[1]
          = (boost::wformat(L"Sierpinski Gasket [D=%d, N=%d, E=%d]")
             %depth%g->nsize()%g->esize()).str();
      }


      ////////////////////////////////////////////////////////////////////////////////
      // Set properties.

      node_property.resize(1, vector<NodeProperty>(g->nsize()));
      edge_property.resize(1, vector<EdgeProperty>(g->esize()));

      int const digit = int(log10(double(g->nsize())))+1;

      for (size_t i=0, nsz=g->nsize(); i<nsz; ++i) {

        // **********  Catch a termination signal  **********
        if (parent.cancel_check()) {
          graph.clear();
          return;
        }

        sociarium_project_module_graph_creation::NodeProperty& np = node_property[0][i];
        if (digit==1) np.identifier = (boost::wformat(L"n%d")%i).str();
        else if (digit==2) np.identifier = (boost::wformat(L"n%02d")%i).str();
        else if (digit==3) np.identifier = (boost::wformat(L"n%03d")%i).str();
        else if (digit==4) np.identifier = (boost::wformat(L"n%04d")%i).str();
        else if (digit==5) np.identifier = (boost::wformat(L"n%05d")%i).str();
        else np.identifier = (boost::wformat(L"n%d")%i).str();
        np.name = np.identifier;
        np.weight = float(g->node(i)->degree());
      }

      for (size_t i=0, esz=g->esize(); i<esz; ++i) {

        // **********  Catch a termination signal  **********
        if (parent.cancel_check()) {
          graph.clear();
          return;
        }

        Edge* e = g->edge(i);
        wstring const& source = node_property[0][e->source()->index()].name;
        wstring const& target = node_property[0][e->target()->index()].name;
        sociarium_project_module_graph_creation::EdgeProperty& ep = edge_property[0][i];
        ep.identifier = (directed||source<target)?(source+L'~'+target):(target+L'~'+source);
        ep.name = ep.name;
        ep.weight = 1.0f;
      }


      ////////////////////////////////////////////////////////////////////////////////
      // Set a layer name.
      layer_name.push_back(title);
    }

} // The end of the namespace "hashimoto_ut"

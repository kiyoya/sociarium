// s.o.c.i.a.r.i.u.m: module/graph_creation_read_sierpinski_gasket_model.cpp
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
#include <windows.h>
#include "graph_creation.h"
#include "../common.h"
#include "../language.h"
#include "../../shared/thread.h"
#include "../../shared/general.h"
#include "../../shared/win32api.h"
#include "../../shared/msgbox.h"
#include "../../graph/graph.h"
#include "../../graph/graphex.h"

BOOL WINAPI DllMain (HINSTANCE hinstDll, DWORD fdwReason, LPVOID lpvReserved) {
  return TRUE;
}

namespace hashimoto_ut {

  using std::vector;
  using std::deque;
  using std::string;
  using std::wstring;
  using std::pair;
  using std::tr1::shared_ptr;
  using std::tr1::unordered_map;

  using namespace sociarium_project_common;
  using namespace sociarium_project_module_graph_creation;
  using namespace sociarium_project_language;

  extern "C" __declspec(dllexport)
    void __cdecl create_graph_time_series(

      Thread* parent,
      deque<wstring>& status,
      Message const* message,
      vector<shared_ptr<Graph> >& graph,
      vector<vector<NodeProperty> >& node_property,
      vector<vector<EdgeProperty> >& edge_property,
      vector<wstring>& layer_name,
      unordered_map<string, pair<string, int> > const& params,
      vector<pair<string, int> > const& data,
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

      unordered_map<string, pair<string, int> >::const_iterator pos;

      if ((pos=params.find("directed"))!=params.end())
        directed = true;

      if ((pos=params.find("title"))!=params.end() && !pos->second.first.empty())
        title = mbcs2wcs(pos->second.first.c_str(), pos->second.first.size());

      if ((pos=params.find("depth"))!=params.end()) {
        try {
          depth = boost::lexical_cast<size_t>(pos->second.first);
        } catch (...) {
          message_box(
            get_window_handle(),
            MessageType::CRITICAL,
            APPLICATION_TITLE,
            L"bad data: %s [line=%d]",
            filename.c_str(), pos->second.second);
        }
      }


      ////////////////////////////////////////////////////////////////////////////////
      // Make a graph.

      vector<shared_ptr<Graph> >(1, Graph::create(directed)).swap(graph);
      shared_ptr<Graph>& g = graph[0];

      status[0]
        = (boost::wformat(L"%s")
           %message->get(Message::MAKING_GRAPH_SNAPSHOT)).str();

      status[1]
        = (boost::wformat(L"Sierpinski Gasket [D=%d, N=%d, E=%d]")
           %depth%g->nsize()%g->esize()).str();

      g->add_node();
      g->add_node();
      g->add_node();
      g->add_edge(g->node(0), g->node(1));
      g->add_edge(g->node(1), g->node(2));
      g->add_edge(g->node(2), g->node(0));

      // 三角形の次数2の頂点同士を融合して大きな三角形を作成
      for (size_t d=0; d<depth; ++d) {

        status[0]
          = (boost::wformat(L"%s: Sierpinski Gasket [D=%d, N=%d, E=%d]")
             %message->get(Message::MAKING_GRAPH_SNAPSHOT)
             %depth%g->nsize()%g->esize()).str();

        // **********  Catch a termination signal  **********
        if (parent->cancel_check()) {
          graph.clear();
          return;
        }

        shared_ptr<Graph> g1 = copy(g);

        // **********  Catch a termination signal  **********
        if (parent->cancel_check()) {
          graph.clear();
          return;
        }

        shared_ptr<Graph> g2 = copy(g);

        // **********  Catch a termination signal  **********
        if (parent->cancel_check()) {
          graph.clear();
          return;
        }

        g->merge(g1); // マージされたグラフ要素はコンテナ後方に追加される
        g->merge(g2); // マージされたグラフ要素はコンテナ後方に追加される

        // **********  Catch a termination signal  **********
        if (parent->cancel_check()) {
          graph.clear();
          return;
        }

        vector<Node*> v;
        for (node_iterator i=g->nbegin(); i!=g->nend(); ++i)
          if ((*i)->degree()==2)
            v.push_back(*i);

        assert(v.size()==9); // v[0:2], v[3:5], v[6:8]が融合した3つの三角形の頂点に対応

        // 三角形の頂点同士を縮約して三角形を連結
        contract(g, v[0], v[3]);
        contract(g, v[1], v[6]);
        contract(g, v[4], v[7]);

        status[0]
          = (boost::wformat(L"%s")
             %message->get(Message::MAKING_GRAPH_SNAPSHOT)).str();

        status[1]
          = (boost::wformat(L"Sierpinski Gasket [D=%d, N=%d, E=%d]")
             %depth%g->nsize()%g->esize()).str();
      }


      ////////////////////////////////////////////////////////////////////////////////
      // Set properties.
      node_property.resize(1, vector<sociarium_project_module_graph_creation::NodeProperty>(g->nsize()));

      int const digit = int(log10(double(g->nsize())))+1;

      for (size_t i=0, nsz=g->nsize(); i<nsz; ++i) {

        // **********  Catch a termination signal  **********
        if (parent->cancel_check()) {
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

      edge_property.resize(1, vector<sociarium_project_module_graph_creation::EdgeProperty>(g->esize()));

      for (size_t i=0, esz=g->esize(); i<esz; ++i) {

        // **********  Catch a termination signal  **********
        if (parent->cancel_check()) {
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

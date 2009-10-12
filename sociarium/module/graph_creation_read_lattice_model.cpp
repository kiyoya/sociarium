// s.o.c.i.a.r.i.u.m
// module/graph_creation_read_lattice.cpp
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
#include <unordered_map>
#include <boost/format.hpp>
#include <windows.h>
#include "graph_creation.h"
#include "../menu_and_message.h"
#include "../../shared/thread.h"
#include "../../shared/util.h"
#include "../../graph/graph.h"

BOOL WINAPI DllMain (HINSTANCE hinstDll, DWORD fdwReason, LPVOID lpvReserved) {
  return TRUE;
}

namespace hashimoto_ut {

  using std::vector;
  using std::deque;
  using std::string;
  using std::wstring;
  using std::make_pair;
  using std::pair;
  using std::getline;
  using std::tr1::shared_ptr;
  using std::tr1::unordered_map;

  using namespace sociarium_project_menu_and_message;
  using namespace sociarium_project_module_graph_creation;

  extern "C" __declspec(dllexport)
    void __cdecl create_graph_time_series(

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

      wstring title;
      size_t xsize = 0;
      size_t ysize = 0;

      unordered_map<wstring, pair<wstring, int> >::const_iterator pos;

      if ((pos=params.find(L"title"))!=params.end() && !pos->second.first.empty())
        title = pos->second.first;

      if ((pos=params.find(L"xsize"))!=params.end()) {
        try {
          xsize = boost::lexical_cast<size_t>(pos->second.first);
          if (xsize<2) {
            throw (boost::wformat(L"xsize<2: line=%d\n%s")
                   %pos->second.second%filename.c_str()).str();
          }
        } catch (...) {
          throw (boost::wformat(L"bad data: line=%d\n%s")
                 %pos->second.second%filename.c_str()).str();
        }
      }

      if ((pos=params.find(L"ysize"))!=params.end()) {
        try {
          ysize = boost::lexical_cast<size_t>(pos->second.first);
          if (ysize<2) {
            throw (boost::wformat(L"ysize<2: line=%d\n%s")
                   %pos->second.second%filename.c_str()).str();
          }
        } catch (...) {
          throw (boost::wformat(L"bad data: line=%d\n%s")
                 %pos->second.second%filename.c_str()).str();
        }
      }


      ////////////////////////////////////////////////////////////////////////////////
      // Make a graph.

      vector<shared_ptr<Graph> >(1, Graph::create(false)).swap(graph);
      shared_ptr<Graph>& g = graph[0];

      status[0]
        = (boost::wformat(L"%s")
           %message.get(Message::MAKING_GRAPH_SNAPSHOT)).str();

      vector<vector<Node* > > n(xsize, vector<Node*>(ysize));

      size_t const nsz = xsize*ysize;
      size_t const esz = 2*nsz-xsize-ysize;

      status[1]
        = (boost::wformat(L"Lattice [N=%d%%, E=%d%%]")
           %int(100.0*g->nsize()/nsz)%int(100.0*g->esize()/esz)).str();

      for (size_t x=0, i=0; x<xsize; ++x) {
        for (size_t y=0; y<ysize; ++y) {
          n[x][y] = g->add_node();

          if (parent.cancel_check()) {
            graph.clear();
            return;
          }

          status[1]
            = (boost::wformat(L"Lattice [N=%d%%, E=%d%%]")
               %int(100.0*g->nsize()/nsz)%int(100.0*g->esize()/esz)).str();
        }
      }

      for (size_t x=0; x<xsize-1; ++x) {
        for (size_t y=0; y<ysize-1; ++y) {
          g->add_edge(n[x][y], n[x+1][y]);
          g->add_edge(n[x][y], n[x][y+1]);

          if (parent.cancel_check()) {
            graph.clear();
            return;
          }

          status[1]
            = (boost::wformat(L"Lattice [N=%d%%, E=%d%%]")
               %int(100.0*g->nsize()/nsz)%int(100.0*g->esize()/esz)).str();
        }
      }

      for (size_t x=0; x<xsize-1; ++x) {
        g->add_edge(n[x][ysize-1], n[x+1][ysize-1]);

        if (parent.cancel_check()) {
          graph.clear();
          return;
        }

        status[1]
          = (boost::wformat(L"Lattice [N=%d%%, E=%d%%]")
             %int(100.0*g->nsize()/nsz)%int(100.0*g->esize()/esz)).str();
      }

      for (size_t y=0; y<ysize-1; ++y) {
        g->add_edge(n[xsize-1][y], n[xsize-1][y+1]);

        if (parent.cancel_check()) {
          graph.clear();
          return;
        }

        status[1]
          = (boost::wformat(L"Lattice [N=%d%%, E=%d%%]")
             %int(100.0*g->nsize()/nsz)%int(100.0*g->esize()/esz)).str();
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
        ep.identifier = (source<target)?(source+L'~'+target):(target+L'~'+source);
        ep.name = ep.name;
        ep.weight = 1.0f;
      }


      ////////////////////////////////////////////////////////////////////////////////
      // Set a layer name.
      layer_name.push_back(title);
    }

} // The end of the namespace "hashimoto_ut"

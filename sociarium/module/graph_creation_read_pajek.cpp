// s.o.c.i.a.r.i.u.m: module/graph_creation_read_pajek.cpp
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
  using std::pair;
  using std::make_pair;
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

      bool directed = false;
      wstring title;

      unordered_map<wstring, pair<wstring, int> >::const_iterator pos;

      if ((pos=params.find(L"directed"))!=params.end())
        directed = true;

      if ((pos=params.find(L"title"))!=params.end() && !pos->second.first.empty())
        title = pos->second.first;


      ////////////////////////////////////////////////////////////////////////////////
      // Make a graph and set properties.

      vector<shared_ptr<Graph> >(1, Graph::create(directed)).swap(graph);
      shared_ptr<Graph>& g = graph[0];

      node_property.resize(1);
      edge_property.resize(1);

      unordered_map<wstring, Node*> identifier2node;
      unordered_map<wstring, Edge*> identifier2edge;

      int mode = -1;

      for (size_t count=0; count<data.size(); ++count) {

        // **********  Catch a termination signal  **********
        if (parent.cancel_check()) {
          graph.clear();
          return;
        }

        status[0]
          = (boost::wformat(L"%s: %d%%")
             %message.get(Message::MAKING_GRAPH_SNAPSHOT)
             %int(100.0*(count+1)/data.size())).str();

        wstring const line = data[count].first;

        if (line.find(L"*Vertices")==0
            || line.find(L"*vertices")==0
            || line.find(L"*VERTICES")==0)
          mode = 0;
        else if (line.find(L"*Arcs")==0
                 || line.find(L"*arcs")==0
                 || line.find(L"*ARCS")==0)
          mode = 1;

        else if (mode==0) {
          // Make nodes.
          size_t head = line.find_first_of(L'\"');
          size_t const tail = line.find_last_of(L'\"');
          wstring id = line.substr(0, head);
          trim(id);

          if (identifier2node.find(id)!=identifier2node.end())
            throw (boost::wformat(L"bad data: line=%d\n%s")
                 %data[count].second%filename.c_str()).str();

          ++head;
          wstring const identifier = line.substr(head, tail-head);
          Node* n = g->add_node();
          identifier2node.insert(make_pair(id, n));
          NodeProperty np;
          np.identifier = identifier;
          np.name = np.identifier;
          np.weight = 0.0f;
          node_property[0].push_back(np);
        }

        else if (mode==1) {
          // Make edges.
          vector<wstring> tok = tokenize(line, ' ');
          if (tok.size()<2)
            throw (boost::wformat(L"bad data: line=%d\n%s")
                 %data[count].second%filename.c_str()).str();

          trim(tok[0]);
          trim(tok[1]);
          float weight = 1.0f;

          if (tok.size()>2) {
            trim(tok[2]);
            try {
              weight = boost::lexical_cast<float>(tok[2]);
            } catch (...) {
              weight = 1.0f;
            }
          }

          unordered_map<wstring, Node*>::const_iterator m0
            = identifier2node.find(tok[0]);
          unordered_map<wstring, Node*>::const_iterator m1
            = identifier2node.find(tok[1]);

          if (m0==identifier2node.end() || m1==identifier2node.end())
            throw (boost::wformat(L"bad data: line=%d\n%s")
                   %data[count].second%filename.c_str()).str();

          Node* n0 = m0->second;
          Node* n1 = m1->second;
          if (!directed) {
            n0 = m0->first<m1->first?m0->second:m1->second;
            n1 = m0->first<m1->first?m1->second:m0->second;
          }

          wstring const& node_name0 = node_property[0][n0->index()].name;
          wstring const& node_name1 = node_property[0][n1->index()].name;
          wstring const identifier = L'\"'+node_name0+L"\"~"+node_name1+L'\"';
          unordered_map<wstring, Edge*>::const_iterator m
            = identifier2edge.find(identifier);

          if (m==identifier2edge.end()) {
            Edge* e = g->add_edge(n0, n1);
            identifier2edge.insert(make_pair(identifier, e));
            EdgeProperty ep;
            ep.identifier = identifier;
            ep.name = node_name0+L'~'+node_name1;
            ep.weight = weight;
            edge_property[0].push_back(ep);
          } else
            edge_property[0][m->second->index()].weight += weight;

          node_property[0][n0->index()].weight += weight;
          node_property[0][n1->index()].weight += weight;
        }

        else assert(0 && "never reach");
      }


      ////////////////////////////////////////////////////////////////////////////////
      // Set a layer name.

      layer_name.push_back(title);
    }

} // The end of the namespace "hashimoto_ut"

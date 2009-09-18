// s.o.c.i.a.r.i.u.m: module/graph_creation_read_adjacency_list.cpp
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
#include "../common.h"
#include "../language.h"
#include "../../shared/thread.h"
#include "../../shared/general.h"
#include "../../shared/win32api.h"
#include "../../shared/msgbox.h"
#include "../../graph/graph.h"

BOOL WINAPI DllMain (HINSTANCE hinstDll, DWORD fdwReason, LPVOID lpvReserved) {
  return TRUE;
}

namespace hashimoto_ut {

  using std::vector;
  using std::deque;
  using std::map;
  using std::string;
  using std::wstring;
  using std::pair;
  using std::make_pair;
  using std::getline;
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
      char delimiter = '\0';
      wstring title;

      unordered_map<string, pair<string, int> >::const_iterator pos;

      if ((pos=params.find("directed"))!=params.end())
        directed = true;

      if ((pos=params.find("title"))!=params.end() && !pos->second.first.empty())
        title = mbcs2wcs(pos->second.first.c_str(), pos->second.first.size());

      if ((pos=params.find("delimiter"))!=params.end() && !pos->second.first.empty())
        delimiter = pos->second.first[0];

      if (delimiter=='\0') {
        message_box(
          get_window_handle(),
          MessageType::CRITICAL,
          APPLICATION_TITLE,
          L"%s: %s",
          message->get(Message::UNCERTAIN_DELIMITER),
          filename.c_str());
        return;
      }


      ////////////////////////////////////////////////////////////////////////////////
      // Make a graph and set properties.

      vector<shared_ptr<Graph> >(1, Graph::create(directed)).swap(graph);
      shared_ptr<Graph>& g = graph[0];

      struct EdgeProp {
        size_t index;
        string name;
        float weight;
      };

      unordered_map<string, Node*> identifier2node;
      unordered_map<string, Edge*> identifier2edge;

      node_property.resize(1);
      edge_property.resize(1);

      for (size_t count=0; count<data.size(); ++count) {

        // **********  Catch a termination signal  **********
        if (parent->cancel_check()) {
          graph.clear();
          return;
        }

        if (!status.empty()) {
          status[0]
            = (boost::wformat(L"%s: %d%%")
               %message->get(Message::MAKING_GRAPH_SNAPSHOT)
               %int(100.0*(count+1)/data.size())).str();
        }

        vector<string> tok = tokenize(data[count].first, delimiter);

        Node* n0 = 0;

        for (size_t i=0; i<tok.size(); ++i) {
          trim(tok[i]);
          string const& node_identifier = tok[i];
          unordered_map<string, Node*>::iterator j = identifier2node.find(node_identifier);
          Node* n1 = 0;

          if (j==identifier2node.end()) {
            n1 = g->add_node();
            NodeProperty np;
            np.identifier = mbcs2wcs(node_identifier.c_str(), node_identifier.size());
            np.name = np.identifier;
            np.weight = 0.0f;
            node_property[0].push_back(np);
            identifier2node.insert(make_pair(node_identifier, n1));
          } else {
            n1 = j->second;
          }

          if (n0==0)
            n0 = n1;
          else {
            string const& source = tok[0];
            string const& target = tok[i];

            string const edge_identifier
              = (directed||source<target)?source+delimiter+target:target+delimiter+source;

            unordered_map<string, Edge*>::iterator j = identifier2edge.find(edge_identifier);

            if (j==identifier2edge.end()) {
              Edge* e = g->add_edge(n0, n1);
              EdgeProperty ep;
              ep.identifier = mbcs2wcs(edge_identifier.c_str(), edge_identifier.size());
              string const name = (directed||source<target)?source+'~'+target:target+'~'+source;
              ep.name = mbcs2wcs(name.c_str(), name.size());
              ep.weight = 1.0f;
              edge_property[0].push_back(ep);
              identifier2edge.insert(make_pair(edge_identifier, e));
            } else {
              edge_property[0][j->second->index()].weight += 1.0f;
            }

            node_property[0][identifier2node[source]->index()].weight += 1.0f;
            node_property[0][identifier2node[target]->index()].weight += 1.0f;
          }
        }
      }


      ////////////////////////////////////////////////////////////////////////////////
      // Set a layer name.
      layer_name.push_back(title);
    }

} // The end of the namespace "hashimoto_ut"

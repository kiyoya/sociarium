// s.o.c.i.a.r.i.u.m: module/graph_creation_read_adjacency_matrix.cpp
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
#include <boost/numeric/ublas/matrix_sparse.hpp>
#include <windows.h>
#include "graph_creation.h"
#include "../common.h"
#include "../language.h"
#include "../../shared/thread.h"
#include "../../shared/general.h"
#include "../../shared/msgbox.h"
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
  using std::tr1::shared_ptr;
  using std::tr1::unordered_map;
  using boost::numeric::ublas::mapped_matrix;

  using namespace sociarium_project_common;
  using namespace sociarium_project_language;
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
      wchar_t delimiter = L'\0';
      wstring title;
      float threshold = 0.0f;
      wstring node_name_line;

      unordered_map<wstring, pair<wstring, int> >::const_iterator pos;

      if ((pos=params.find(L"directed"))!=params.end())
        directed = true;

      if ((pos=params.find(L"title"))!=params.end() && !pos->second.first.empty())
        title = pos->second.first;

      if ((pos=params.find(L"delimiter"))!=params.end() && !pos->second.first.empty())
        delimiter = pos->second.first[0];

      if ((pos=params.find(L"node_name"))!=params.end())
        node_name_line = pos->second.first;

      if ((pos=params.find(L"threshold"))!=params.end()) {
        try {
          threshold = boost::lexical_cast<float>(pos->second.first);
        } catch (...) {
          message_box(get_window_handle(), mb_error, APPLICATION_TITLE,
                      L"bad data: %s [line=%d]",
                      filename.c_str(), pos->second.second);
        }
      }

      if (delimiter==L'\0') {
        message_box(get_window_handle(), mb_error, APPLICATION_TITLE,
                    L"%s: %s", message.get(Message::UNCERTAIN_DELIMITER),
                    filename.c_str());
        return;
      }


      ////////////////////////////////////////////////////////////////////////////////
      // Parse data.

      size_t const nsz = data.size();
      mapped_matrix<float> weight_matrix(nsz, nsz);
      vector<wstring> node_name = tokenize(node_name_line, delimiter);

      for (size_t i=0; i<nsz; ++i) {

        // **********  Catch a termination signal  **********
        if (parent.cancel_check()) {
          graph.clear();
          return;
        }

        status[0]
          = (boost::wformat(L"%s: %d%%")
             %message.get(Message::PARSING_DATA)
             %int(100.0*(i+1)/data.size())).str();

        vector<wstring> tok = tokenize(data[i].first, delimiter);

        if (tok.size()!=nsz) {
          message_box(get_window_handle(), mb_error, APPLICATION_TITLE,
                      L"%s: %s [line=%d]",
                      message.get(Message::INVALID_NUMBER_OF_ITEMS),
                      filename.c_str(), data[i].second);
          graph.clear();
          return;
        }

        for (size_t j=0; j<nsz; ++j) {

          trim(tok[j]);

          try {
            float const w = boost::lexical_cast<float>(tok[j]);
            weight_matrix(j,i) = w;
          } catch (...) {
            message_box(get_window_handle(), mb_error, APPLICATION_TITLE,
                        L"bad data: %s [line=%d]",
                        filename.c_str(), data[i].second);
            graph.clear();
            return;
          }
        }
      }


      ////////////////////////////////////////////////////////////////////////////////
      // Make a graph.

      vector<shared_ptr<Graph> >(1, Graph::create(directed)).swap(graph);
      shared_ptr<Graph>& g = graph[0];
      vector<float> node_weight(nsz, 0.0f);
      vector<float> edge_weight;

      for (size_t i=0; i<nsz; ++i) {
        g->add_node();
        if (i>=node_name.size())
          // If the number of node names is less than the number of nodes,
          // the serial number is automatically assigned to the failed nodes.
          node_name.push_back((boost::wformat(L"n%02d")%i).str());
      }

      for (size_t i=0; i<nsz; ++i) {

        // **********  Catch a termination signal  **********
        if (parent.cancel_check()) {
          graph.clear();
          return;
        }

        status[0]
          = (boost::wformat(L"%s: %d%%")
             %message.get(Message::MAKING_GRAPH_SNAPSHOT)
             %int(100.0*(i+1)/data.size())).str();

        if (g->is_directed()) {
          for (size_t j=0; j<nsz; ++j) {
            float const w = weight_matrix(j,i);
            if (w>threshold) {
              g->add_edge(g->node(j), g->node(i));
              edge_weight.push_back(w);
              node_weight[i] += w;
              node_weight[j] += w;
            }
          }
        } else {
          // If the graph is undirected, weight of each edge is equal to the sum of
          // the values of upper and lower triangular elements.
          float const w = weight_matrix(i,i);
          if (w>threshold) {
            // for diagonal elements.
            g->add_edge(g->node(i), g->node(i));
            edge_weight.push_back(w);
            node_weight[i] += 2.0f*w;
          }
          // for non diagonal elements.
          for (size_t j=i+1; j<nsz; ++j) {
            float const w = weight_matrix(i,j)+weight_matrix(j,i);
            if (w>threshold) {
              g->add_edge(g->node(j), g->node(i));
              edge_weight.push_back(w);
              node_weight[i] += w;
              node_weight[j] += w;
            }
          }
        }
      }


      ////////////////////////////////////////////////////////////////////////////////
      // Set properties.
      {
        size_t const nsz = g->nsize();
        size_t const esz = g->esize();

        assert(node_name.size()==nsz);
        assert(node_weight.size()==nsz);
        assert(edge_weight.size()==esz);

        node_property.resize(1, vector<NodeProperty>(nsz));

        for (size_t i=0; i<nsz; ++i) {
          NodeProperty& np = node_property[0][i];
          np.identifier = node_name[i];
          np.name = np.identifier;
          np.weight = node_weight[i];
        }

        edge_property.resize(1, vector<EdgeProperty>(esz));

        for (size_t i=0; i<esz; ++i) {
          Edge* e = g->edge(i);
          wstring const source = node_name[e->source()->index()];
          wstring const target = node_name[e->target()->index()];
          wstring const identifier
            = (directed||source<target)?source+delimiter+target:target+delimiter+source;
          wstring const name
            = (directed||source<target)?source+L'~'+target:target+L'~'+source;
          EdgeProperty& ep = edge_property[0][i];
          ep.identifier = identifier;
          ep.name = name;
          ep.weight = edge_weight[i];
        }
      }


      ////////////////////////////////////////////////////////////////////////////////
      // Set a layer name.

      layer_name.push_back(title);
    }

} // The end of the namespace "hashimoto_ut"

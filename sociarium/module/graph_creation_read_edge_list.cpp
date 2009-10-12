// s.o.c.i.a.r.i.u.m: module/graph_creation_read_edge_list.cpp
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

      bool directed = false;
      wchar_t delimiter = L'\0';
      wstring title;
      float threshold = 0.0f;

      unordered_map<wstring, pair<wstring, int> >::const_iterator pos;

      if ((pos=params.find(L"directed"))!=params.end())
        directed = true;

      if ((pos=params.find(L"title"))!=params.end() && !pos->second.first.empty())
        title = pos->second.first;

      if ((pos=params.find(L"delimiter"))!=params.end() && !pos->second.first.empty())
        delimiter = pos->second.first[0];

      if ((pos=params.find(L"threshold"))!=params.end()) {
        try {
          threshold = boost::lexical_cast<float>(pos->second.first);
        } catch (...) {
          throw (boost::wformat(L"bad data: line=%d\n%s")
                 %pos->second.second%filename.c_str()).str();
        }
      }

      if (delimiter==L'\0')
        throw message.get(Message::UNCERTAIN_DELIMITER)+wstring(L": ")+filename;

      size_t number_of_columns = 0;

      size_t source_column = 0;
      size_t target_column = 1;
      size_t source_texture_column = -1;
      size_t target_texture_column = -1;
      size_t weight_column = -1;
      size_t name_column = -1;

      if ((pos=params.find(L"columns"))!=params.end()) {
        vector<wstring> row = tokenize(pos->second.first, delimiter);

        source_column = -1;
        target_column = -1;

        for (size_t i=0; i<row.size(); ++i)
          trim(row[i]);

        for (size_t i=0; i<row.size(); ++i) {
          if (row[i]==L"source") {
            source_column = i;
            ++number_of_columns;
          } else if (row[i]==L"target") {
            target_column = i;
            ++number_of_columns;
          } else if (row[i]==L"source_texture") {
            source_texture_column = i;
            ++number_of_columns;
          } else if (row[i]==L"target_texture") {
            target_texture_column = i;
            ++number_of_columns;
          } else if (row[i]==L"weight") {
            weight_column = i;
            ++number_of_columns;
          } else if (row[i]==L"name") {
            name_column = i;
            ++number_of_columns;
          } else {
            // ignore
            ++number_of_columns;
          }
        }

        if (source_column==-1 || target_column==-1)
          throw (boost::wformat(L"bad data: line=%d\n%s")
                 %pos->second.second%filename.c_str()).str();
      }


      ////////////////////////////////////////////////////////////////////////////////
      // Parse data.

      unordered_map<wstring, pair<wstring, float> > id2property;
      unordered_map<wstring, wstring> node_name2texture_file_name;

      for (size_t count=0; count<data.size(); ++count) {

        // **********  Catch a termination signal  **********
        if (parent.cancel_check())
          return;

        status[0]
          = (boost::wformat(L"%s: %d%%")
             %message.get(Message::PARSING_DATA)
             %int(100.0*(count+1)/data.size())).str();

        vector<wstring> tok = tokenize(data[count].first, delimiter);

        // Get a name of a node.
        if ((number_of_columns==0 && tok.size()<2)
            || (tok.size()<number_of_columns))
          throw (boost::wformat(L"%s: line=%d\n%s")
                 %message.get(Message::INVALID_NUMBER_OF_ITEMS)
                 %data[count].second%filename.c_str()).str();

        trim(tok[source_column]);
        trim(tok[target_column]);

        wstring const source = tok[source_column]; // The name of the source node.
        wstring const target = tok[target_column]; // The name of the target node.

        if (source.empty() || target.empty())
          throw (boost::wformat(L"bad data: line=%d\n%s")
                 %data[count].second%filename.c_str()).str();

        // If the graph is undirected, the direction of the edge is ignored.
        wstring identifier
          = (directed||source<target)?(source+delimiter+target):(target+delimiter+source);

        pair<wstring, float>* ep = 0;

        // Get a name of an edge.
        // If the name column is not specified, the name of the edge
        // is made from the names of nodes.
        if (number_of_columns==0 || name_column==-1) {
          ep = &id2property[identifier];
          ep->first
            = (directed||source<target)?(source+L'~'+target):(target+L'~'+source);
        } else {
          if (tok.size()<number_of_columns)
            throw (boost::wformat(L"%s: line=%d\n%s")
                   %message.get(Message::INVALID_NUMBER_OF_ITEMS)
                   %data[count].second%filename.c_str()).str();

          trim(tok[name_column]);
          identifier += delimiter+tok[name_column];
          ep = &id2property[identifier];

          if (!ep->first.empty() && ep->first!=tok[name_column])
            throw (boost::wformat(L"name confliction: line=%d [%s]\n%s")
                   %data[count].second%tok[name_column].c_str()
                   %filename.c_str()).str();

          ep->first = tok[name_column];
        }

        { // Get a weight value of the edge.
          wstring w;
          if (number_of_columns==0) {
            // If the weight column is not specified, the default column is used.
            // If the number of columns is less than the value of the default column,
            // the weight is set to "1.0".
            if (tok.size()>weight_column) {
              trim(tok[weight_column]);
              w = tok[weight_column];
            } else
              // In case the default weight column is invalid.
              w = L"1.0";
          } else if (weight_column==-1) {
            // The weight column is not specified.
            w = L"1.0";
          } else {
            if (tok.size()<number_of_columns)
              throw (boost::wformat(L"%s: line=%d\n%s")
                     %message.get(Message::INVALID_NUMBER_OF_ITEMS)
                     %data[count].second%filename.c_str()).str();

            trim(tok[weight_column]);
            w = tok[weight_column];
          }

          try {
            ep->second += boost::lexical_cast<float>(w);
          } catch (...) {
            throw (boost::wformat(L"bad data: line=%d\n%s")
                   %data[count].second%filename.c_str()).str();
          }
        }

        { // Get a texture filename.
          if (source_texture_column!=-1 && !source.empty()) {
            if (tok.size()<number_of_columns)
              throw (boost::wformat(L"%s: line=%d\n%s")
                     %message.get(Message::INVALID_NUMBER_OF_ITEMS)
                     %data[count].second%filename.c_str()).str();

            trim(tok[source_texture_column]);
            wstring& texture_file_name = node_name2texture_file_name[source];

            if (!texture_file_name.empty()
                && texture_file_name!=tok[source_texture_column])
              throw (boost::wformat(L"texture confliction: line=%d [%s]\n%s")
                     %data[count].second%tok[source_texture_column].c_str()
                     %filename.c_str()).str();

            texture_file_name = tok[source_texture_column];
          }

          if (target_texture_column!=-1 && !target.empty()) {
            if (tok.size()<number_of_columns)
              throw (boost::wformat(L"%s: line=%d\n%s")
                     %message.get(Message::INVALID_NUMBER_OF_ITEMS)
                     %data[count].second%filename.c_str()).str();

            trim(tok[target_texture_column]);
            wstring& texture_file_name = node_name2texture_file_name[target];

            if (!texture_file_name.empty()
                && texture_file_name!=tok[target_texture_column])
              throw (boost::wformat(L"texture confliction: line=%d [%s]\n%s")
                     %data[count].second%tok[source_texture_column].c_str()
                     %filename.c_str()).str();

            texture_file_name = tok[target_texture_column];
          }
        }
      }


      ////////////////////////////////////////////////////////////////////////////////
      // Make a graph and set properties.

      vector<shared_ptr<Graph> >(1, Graph::create(directed)).swap(graph);
      shared_ptr<Graph>& g = graph[0];

      node_property.resize(1);
      edge_property.resize(1);

      unordered_map<wstring, Node*> name2node;

      unordered_map<wstring, pair<wstring, float> >::const_iterator i = id2property.begin();
      unordered_map<wstring, pair<wstring, float> >::const_iterator end = id2property.end();

      for (int count=0; i!=end; ++i, ++count) {

        // **********  Catch a termination signal  **********
        if (parent.cancel_check()) {
          graph.clear();
          return;
        }

        status[0]
          = (boost::wformat(L"%s: %d%%")
             %message.get(Message::MAKING_GRAPH_SNAPSHOT)
             %int(100.0*count/data.size())).str();

        wstring const& identifier = i->first;
        float const w = i->second.second;

        if (w>threshold) {
          Node* n[2];
          vector<wstring> const tok = tokenize(identifier, delimiter);

          for (int j=0; j<2; ++j) {

            unordered_map<wstring, Node*>::const_iterator k = name2node.find(tok[j]);

            if (k==name2node.end()) {
              n[j] = g->add_node();
              name2node.insert(make_pair(tok[j], n[j]));
              NodeProperty np;
              np.identifier = tok[j];
              np.name = np.identifier;
              np.weight = w;
              node_property[0].push_back(np);
            } else {
              n[j] = k->second;
              NodeProperty& np = node_property[0][n[j]->index()];
              np.weight += w;
            }
          }

          Edge* e = g->add_edge(n[0], n[1]);
          EdgeProperty ep;
          ep.identifier = identifier;
          ep.name = i->second.first;
          ep.weight = w;
          edge_property[0].push_back(ep);
        }
      }


      ////////////////////////////////////////////////////////////////////////////////
      // Set a layer name.

      layer_name.push_back(title);
    }

} // The end of the namespace "hashimoto_ut"

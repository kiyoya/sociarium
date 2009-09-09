﻿// s.o.c.i.a.r.i.u.m: module/graph_creation_read_edge_list.cpp
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
#include "../../shared/msgbox.h"
#include "../../shared/win32api.h"
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
      float threshold = 0.0f;

      unordered_map<string, pair<string, int> >::const_iterator pos;

      if ((pos=params.find("directed"))!=params.end())
        directed = true;

      if ((pos=params.find("title"))!=params.end() && !pos->second.first.empty())
        title = mbcs2wcs(pos->second.first.c_str(), pos->second.first.size());

      if ((pos=params.find("delimiter"))!=params.end() && !pos->second.first.empty())
        delimiter = pos->second.first[0];

      if ((pos=params.find("threshold"))!=params.end()) {
        try {
          threshold = boost::lexical_cast<float>(pos->second.first);
        } catch (...) {
          message_box(
            get_window_handle(),
            MB_OK|MB_ICONERROR|MB_SYSTEMMODAL,
            APPLICATION_TITLE,
            L"bad data: %s [line=%d]",
            filename.c_str(), pos->second.second);
        }
      }

      if (delimiter=='\0') {
        message_box(
          get_window_handle(),
          MB_OK|MB_ICONERROR|MB_SYSTEMMODAL,
          APPLICATION_TITLE,
          L"%s: %s",
          message->get(Message::UNCERTAIN_DELIMITER),
          filename.c_str());
        return;
      }

      size_t number_of_columns = 0;

      size_t source_column = 0;
      size_t target_column = 1;
      size_t weight_column = 2;
      size_t name_column = -1; // Unavailable in a default setting.

      if ((pos=params.find("columns"))!=params.end()) {
        vector<string> row = tokenize(pos->second.first, delimiter);

        for (size_t i=0; i<row.size(); ++i)
          trim(row[i]);

        for (size_t i=0; i<row.size(); ++i) {
          if (row[i]=="source") {
            source_column = i;
            ++number_of_columns;
          } else if (row[i]=="target") {
            target_column = i;
            ++number_of_columns;
          } else if (row[i]=="weight") {
            weight_column = i;
            ++number_of_columns;
          } else if (row[i]=="name") {
            name_column = i;
            ++number_of_columns;
          } else {
            // ignore
            ++number_of_columns;
          }
        }
      }


      ////////////////////////////////////////////////////////////////////////////////
      // Parse data.

      unordered_map<string, pair<string, float> > id2property;

      for (size_t count=0; count<data.size(); ++count) {

        // **********  Catch a termination signal  **********
        if (parent->cancel_check())
          return;

        status[0]
          = (boost::wformat(L"%s: %d%%")
             %message->get(Message::PARSING_DATA)
             %int(100.0*(count+1)/data.size())).str();

        vector<string> tok = tokenize(data[count].first, delimiter);

        // Get a name of a node.
        if ((number_of_columns==0 && tok.size()<2) || (tok.size()<number_of_columns)) {
          message_box(
            get_window_handle(),
            MB_OK|MB_ICONERROR|MB_SYSTEMMODAL,
            APPLICATION_TITLE,
            L"%s: %s [line=%d]",
            message->get(Message::INVALID_NUMBER_OF_ITEMS),
            filename.c_str(), data[count].second);
          return;
        }

        trim(tok[source_column]);
        trim(tok[target_column]);

        string const source = tok[source_column]; // The name of the source node.
        string const target = tok[target_column]; // The name of the target node.

        if (source.empty() || target.empty()) {
          message_box(
            get_window_handle(),
            MB_OK|MB_ICONERROR|MB_SYSTEMMODAL,
            APPLICATION_TITLE,
            L"bad data: %s [line=%d]",
            filename.c_str(), data[count].second);
          return;
        }

        // If the graph is undirected, the direction of the edge is ignored.
        string const identifier
          = (directed||source<target)?(source+delimiter+target):(target+delimiter+source);

        pair<string, float>& ep = id2property[identifier];

        { // Get a name of an edge.
          // If the name column is not specified, the name of the edge
          // is made from the names of nodes.
          if (number_of_columns==0 || name_column==-1)
            ep.first = (directed||source<target)?(source+'~'+target):(target+'~'+source);

          else {
            if (tok.size()<number_of_columns) {
              message_box(
                get_window_handle(),
                MB_OK|MB_ICONERROR|MB_SYSTEMMODAL,
                APPLICATION_TITLE,
                L"%s: %s [line=%d]",
                message->get(Message::INVALID_NUMBER_OF_ITEMS),
                filename.c_str(), data[count].second);
              return;
            }

            trim(tok[name_column]);

            if (!ep.first.empty() && ep.first!=tok[name_column]) {
              message_box(
                get_window_handle(),
                MB_OK|MB_ICONERROR|MB_SYSTEMMODAL,
                APPLICATION_TITLE,
                L"name confliction: %s [line=%d]",
                filename.c_str(), data[count].second);
            }

            ep.first = tok[name_column];
          }
        }

        { // Get weight of an edge.
          string w;

          // If the weight column is not specified, the default column is used.
          // If the number of columns is less than the value of the default column,
          // the weight is set to 1.0.
          if (number_of_columns==0)
            if (tok.size()>weight_column) {
              trim(tok[weight_column]);
              w = tok[weight_column];
            } else
              // The default weight column is invalid.
              w = "1.0";
          else if (weight_column==-1)
            // The weight column is not specified.
            w = "1.0";
          else {
            if (tok.size()<number_of_columns) {
              message_box(
                get_window_handle(),
                MB_OK|MB_ICONERROR|MB_SYSTEMMODAL,
                APPLICATION_TITLE,
                L"%s: %s [line=%d]",
                message->get(Message::INVALID_NUMBER_OF_ITEMS),
                filename.c_str(), data[count].second);
              return;
            }

            trim(tok[weight_column]);
            w = tok[weight_column];
          }

          try {
            ep.second += boost::lexical_cast<float>(w);
          } catch (...) {
            message_box(
              get_window_handle(),
              MB_OK|MB_ICONERROR|MB_SYSTEMMODAL,
              APPLICATION_TITLE,
              L"bad data: %s [line=%d]",
              filename.c_str(), data[count].second);
            return;
          }
        }
      }


      ////////////////////////////////////////////////////////////////////////////////
      // Make a graph and set properties.

      vector<shared_ptr<Graph> >(1, Graph::create(directed)).swap(graph);
      shared_ptr<Graph>& g = graph[0];

      node_property.resize(1);
      edge_property.resize(1);

      unordered_map<string, Node*> name2node;

      unordered_map<string, pair<string, float> >::const_iterator i = id2property.begin();
      unordered_map<string, pair<string, float> >::const_iterator end = id2property.end();

      for (int count=0; i!=end; ++i, ++count) {

        // **********  Catch a termination signal  **********
        if (parent->cancel_check()) {
          graph.clear();
          return;
        }

        status[0]
          = (boost::wformat(L"%s: %d%%")
             %message->get(Message::MAKING_GRAPH_SNAPSHOT)
             %int(100.0*count/data.size())).str();

        string const& identifier = i->first;
        float const w = i->second.second;

        if (w>threshold) {
          Node* n[2];
          vector<string> const tok = tokenize(identifier, delimiter);

          for (int j=0; j<2; ++j) {
            unordered_map<string, Node*>::const_iterator k = name2node.find(tok[j]);
            if (k==name2node.end()) {
              n[j] = g->add_node();
              name2node.insert(make_pair(tok[j], n[j]));
              NodeProperty np;
              np.identifier = mbcs2wcs(tok[j].c_str(), tok[j].size());
              np.name = np.identifier;
              np.weight = w;
              node_property[0].push_back(np);
            } else {
              n[j] = k->second;
              NodeProperty& np
                = node_property[0][n[j]->index()];
              np.weight += w;
            }
          }

          Edge* e = g->add_edge(n[0], n[1]);
          EdgeProperty ep;
          ep.identifier = mbcs2wcs(identifier.c_str(), identifier.size());
          ep.name = mbcs2wcs(i->second.first.c_str(), i->second.first.size());
          ep.weight = w;
          edge_property[0].push_back(ep);
        }
      }


      ////////////////////////////////////////////////////////////////////////////////
      // Set a layer name.

      layer_name.push_back(title);
    }

} // The end of the namespace "hashimoto_ut"
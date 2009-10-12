// s.o.c.i.a.r.i.u.m: module/graph_creation_read_time_series_rect.cpp
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
#include <map>
#include <unordered_map>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <windows.h>
#include "graph_creation.h"
#include "../menu_and_message.h"
#include "../../shared/thread.h"
#include "../../shared/util.h"
#include "../../graph/graph.h"

BOOL WINAPI DllMain (HINSTANCE hinstDll, DWORD fdwReason, LPVOID lpvReserved) {
  return TRUE;
}

#ifdef NDEBUG
#pragma comment(lib, "libboost_regex-vc90-mt-1_37.lib")
#else
#pragma comment(lib, "libboost_regex-vc90-mt-gd-1_37.lib")
#endif

namespace hashimoto_ut {

  using std::vector;
  using std::deque;
  using std::multimap;
  using std::pair;
  using std::string;
  using std::wstring;
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


      ////////////////////////////////////////////////////////////////////////////////
      // Read parameters.

      bool directed = false;
      wchar_t delimiter = L'\0';
      wstring title;
      wstring time_format;
      time_t interval = -1;
      time_t characteristic_time = -1;
      double threshold = 0.0;

      unordered_map<wstring, pair<wstring, int> >::const_iterator pos;

      if ((pos=params.find(L"directed"))!=params.end())
        directed = true;

      if ((pos=params.find(L"title"))!=params.end() && !pos->second.first.empty())
        title = pos->second.first;

      if ((pos=params.find(L"delimiter"))!=params.end() && !pos->second.first.empty())
        delimiter = pos->second.first[0];

      if ((pos=params.find(L"time_format"))!=params.end() && !pos->second.first.empty())
        time_format = pos->second.first;

      if ((pos=params.find(L"interval"))!=params.end()) {
        try {
          interval = boost::lexical_cast<time_t>(pos->second.first);
        } catch (...) {
          throw (boost::wformat(L"bad data: line=%d\n%s")
                 %pos->second.second%filename.c_str()).str();
        }
      }

      if ((pos=params.find(L"characteristic_time"))!=params.end()) {
        try {
          characteristic_time = boost::lexical_cast<time_t>(pos->second.first);
        } catch (...) {
          throw (boost::wformat(L"bad data: line=%d\n%s")
                 %pos->second.second%filename.c_str()).str();
        }
      }

      if ((pos=params.find(L"threshold"))!=params.end()) {
        try {
          threshold = boost::lexical_cast<double>(pos->second.first);
        } catch (...) {
          throw (boost::wformat(L"bad data: line=%d\n%s")
                 %pos->second.second%filename.c_str()).str();
        }
      }

      if (delimiter==L'\0')
        throw message.get(Message::UNCERTAIN_DELIMITER)+wstring(L": ")+filename;

      if (interval==-1)
        throw L"no @interval: "+filename;

      if (characteristic_time==-1)
        throw L"no @characteristic_time: "+filename;

      size_t number_of_columns = 0;

      size_t time_column = 0;
      size_t source_column = 1;
      size_t target_column = 2;
      size_t source_texture_column = -1; // Unavailable in a default format.
      size_t target_texture_column = -1; // Unavailable in a default format.
      size_t weight_column = -1; // Unavailable in a default format.
      size_t name_column = -1; // Unavailable in a default format.

      if ((pos=params.find(L"columns"))!=params.end()) {
        vector<wstring> row = tokenize(pos->second.first, delimiter);

        time_column = -1;
        source_column = -1;
        target_column = -1;

        for (size_t i=0; i<row.size(); ++i)
          trim(row[i]);

        for (size_t i=0; i<row.size(); ++i) {
          if (row[i]==L"time") {
            time_column = i;
            ++number_of_columns;
          } else if (row[i]==L"source") {
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
            // ignore;
            ++number_of_columns;
          }
        }

        if (time_column==-1 || source_column==-1 || target_column==-1)
          throw (boost::wformat(L"bad data: line=%d\n%s")
                 %pos->second.second%filename.c_str()).str();
      }

      unordered_map<wstring, Vector2<float> > node_name2position;

      if ((pos=params.find(L"position"))!=params.end() && !pos->second.first.empty()) {
        vector<wstring> tok = tokenize(pos->second.first, delimiter);
        boost::wregex r(L"^(.+?)\\((.+?),(.+?)\\)$");
        boost::wsmatch m;
        for (size_t i=0; i<tok.size(); ++i) {
          try {
            if (boost::regex_search(tok[i], m, r)) {
              float const x = boost::lexical_cast<float>(m.str(2));
              float const y = boost::lexical_cast<float>(m.str(3));
              node_name2position[m.str(1)] = Vector2<float>(x, y);
            } else {
              throw;
            }
          } catch (...) {
            throw (boost::wformat(L"bad data: line=%d\n%s")
                   %pos->second.second%filename.c_str()).str();
          }
        }
      }


      ////////////////////////////////////////////////////////////////////////////////
      // Parse data.

      // <time, <identifier, weight> >
      typedef multimap<time_t, pair<wstring, double> > TimeSeries;

      TimeSeries node_time_series;
      TimeSeries edge_time_series;

      unordered_map<wstring, wstring> identifier2edge_name;
      unordered_map<wstring, wstring> node_name2texture_file_name;

      for (size_t count=0; count<data.size(); ++count) {

        // **********  Catch a termination signal  **********
        if (parent.cancel_check()) return;

        status[0]
          = (boost::wformat(L"%s: %d%%")
             %message.get(Message::PARSING_DATA)
             %int(100.0*(count+1)/data.size())).str();

        vector<wstring> tok = tokenize(data[count].first, delimiter);

        time_t time;
        wstring source;
        wstring target;
        double weight;

        { // Get a time and a pair of node names.
          if ((number_of_columns==0 && tok.size()<3)
              || (tok.size()<number_of_columns))
            throw (boost::wformat(L"%s: line=%d\n%s")
                   %message.get(Message::INVALID_NUMBER_OF_ITEMS)
                   %data[count].second%filename.c_str()).str();

          trim(tok[time_column]);
          trim(tok[source_column]);
          trim(tok[target_column]);

          source = tok[source_column]; // The name of the source node.
          target = tok[target_column]; // The name of the target node.

          if (tok[time_column].empty() || source.empty() && target.empty())
            throw (boost::wformat(L"bad data: line=%d\n%s")
                   %data[count].second%filename.c_str()).str();

          try {
            time = boost::lexical_cast<time_t>(tok[time_column]);
          } catch (...) {
            throw (boost::wformat(L"bad data: line=%d\n%s")
                   %data[count].second%filename.c_str()).str();
          }
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
            weight = boost::lexical_cast<double>(w);
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

        // Make an identifier and a name of the edge.
        if (source.empty() || target.empty()) {
          if (source.empty())
            node_time_series.insert(make_pair(time, make_pair(target, weight)));
          else
            node_time_series.insert(make_pair(time, make_pair(source, weight)));
        } else {
          wstring identifier
            = (directed||source<target)?source+delimiter+target:target+delimiter+source;

          if (number_of_columns==0 || name_column==-1)
            identifier2edge_name[identifier]
              = (directed||source<target)?source+L'~'+target:target+L'~'+source;
          else {
            if (tok.size()<number_of_columns)
              throw (boost::wformat(L"%s: line=%d\n%s")
                     %message.get(Message::INVALID_NUMBER_OF_ITEMS)
                     %data[count].second%filename.c_str()).str();

            trim(tok[name_column]);
            identifier += delimiter+tok[name_column];
            wstring& name = identifier2edge_name[identifier];

            if (!name.empty() && name!=tok[name_column])
              throw (boost::wformat(L"name confliction: line=%d [%s]\n%s")
                     %data[count].second%tok[name_column].c_str()
                     %filename.c_str()).str();

            name = tok[name_column];
          }

          edge_time_series.insert(make_pair(time, make_pair(identifier, weight)));
        }
      }


      ////////////////////////////////////////////////////////////////////////////////
      // Make a graph time-series and set properties.

      if (node_time_series.empty() && edge_time_series.empty())
        throw (boost::wformat(L"%s\n%s")
               %message.get(Message::NO_VALID_DATA)
               %filename.c_str()).str();

      // Start time.
      time_t const start_time
        = node_time_series.empty()
          ?edge_time_series.begin()->first
            :(edge_time_series.empty()
              ?node_time_series.begin()->first
              :(node_time_series.begin()->first<edge_time_series.begin()->first
                ?node_time_series.begin()->first
                :edge_time_series.begin()->first));

      // End time.
      time_t const end_time
        = node_time_series.empty()
          ?edge_time_series.rbegin()->first
            :(edge_time_series.empty()
              ?node_time_series.rbegin()->first
              :(node_time_series.rbegin()->first>edge_time_series.rbegin()->first
                ?node_time_series.rbegin()->first
                :edge_time_series.rbegin()->first));

      for (time_t t=start_time+interval; t<=end_time; t+=interval) {

        // **********  Catch a termination signal  **********
        if (parent.cancel_check()) {
          graph.clear();
          return;
        }

        status[0]
          = (boost::wformat(L"%s: %d%%")
             %message.get(Message::MAKING_GRAPH_TIME_SERIES)
             %int(100.0*(t-start_time)/(end_time-start_time))).str();

        typedef unordered_map<wstring, double> ID2Weight;
        ID2Weight node_weight; // Node weights at time @t.
        ID2Weight edge_weight; // Edge weights at time @t.

        // Calculate node and wdge weights at time @t.

        { // Accumulate node weights between @t-@characteristic_time and @t.
          TimeSeries::const_iterator now = node_time_series.upper_bound(t);
          TimeSeries::const_iterator i = node_time_series.upper_bound(t-characteristic_time);
          for (; i!=now; ++i) node_weight[i->second.first] += i->second.second;
        }{// Accumulate edge weights between @t-@characteristic_time and @t.
          TimeSeries::const_iterator now = edge_time_series.upper_bound(t);
          TimeSeries::const_iterator i = edge_time_series.upper_bound(t-characteristic_time);
          for (; i!=now; ++i) edge_weight[i->second.first] += i->second.second;
        }

        // **********  Catch a termination signal  **********
        if (parent.cancel_check()) {
          graph.clear();
          return;
        }

        // --------------------------------------------------------------------------------
        // Add a layer.

        // graph
        graph.push_back(shared_ptr<Graph>(Graph::create(directed)));
        shared_ptr<Graph>& g = graph.back();

        // node property
        node_property.push_back(vector<NodeProperty>());
        vector<NodeProperty>& np_now = node_property.back();

        // edge property
        edge_property.push_back(vector<EdgeProperty>());
        vector<EdgeProperty>& ep_now = edge_property.back();

        unordered_map<wstring, Node*> identifier2node;
        unordered_map<wstring, Edge const*> identifier2edge;

        // --------------------------------------------------------------------------------
        // Make nodes, edges, and their properties.

        for (ID2Weight::const_iterator i=node_weight.begin(); i!=node_weight.end(); ++i) {

          if (i->second<threshold) continue;

          Node* n = g->add_node();
          identifier2node.insert(make_pair(i->first, n));
          np_now.push_back(NodeProperty());
          NodeProperty& np = np_now.back();

          // name and identifier
          np.identifier = i->first;
          np.name = np.identifier;

          // texture
          unordered_map<wstring, wstring>::iterator n2t
            = node_name2texture_file_name.find(i->first);
          if (n2t!=node_name2texture_file_name.end())
            np.texture_file_name = n2t->second;

          // position
          unordered_map<wstring, Vector2<float> >::iterator n2p
            = node_name2position.find(i->first);
          if (n2p!=node_name2position.end())
            np.position.reset(new Vector2<float>(n2p->second));

          // weight
          np.weight = float(i->second);
        }

        // **********  Catch a termination signal  **********
        if (parent.cancel_check()) {
          graph.clear();
          return;
        }

        for (ID2Weight::const_iterator i=edge_weight.begin(); i!=edge_weight.end(); ++i) {

          if (i->second<threshold) continue;

          // Get node names by splitting the edge name.
          vector<wstring> tok = tokenize(i->first, delimiter);
          assert(tok.size()==2);
          Node* n[2];

          for (int j=0; j<2; ++j) {
            unordered_map<wstring, Node*>::iterator id2n = identifier2node.find(tok[j]);
            if (id2n==identifier2node.end()) {
              identifier2node.insert(make_pair(tok[j], (n[j]=g->add_node())));
              np_now.push_back(NodeProperty());
              NodeProperty& np = np_now.back();

              // name and identifier
              np.identifier = tok[j];
              np.name = np.identifier;

              // texture
              unordered_map<wstring, wstring>::iterator n2t
                = node_name2texture_file_name.find(tok[j]);
              if (n2t!=node_name2texture_file_name.end())
                np.texture_file_name = n2t->second;

              // position
              unordered_map<wstring, Vector2<float> >::iterator n2p
                = node_name2position.find(tok[j]);
              if (n2p!=node_name2position.end())
                np.position.reset(new Vector2<float>(n2p->second));

              // weight
              np.weight = float(i->second);
            } else {
              n[j] = id2n->second;
              np_now[n[j]->index()].weight += float(i->second);
            }
          }

          unordered_map<wstring, Edge const*>::iterator id2e
            = identifier2edge.find(i->first);
          EdgeProperty* ep = 0;

          if (id2e==identifier2edge.end()) {
            // Newly appeared edge.
            Edge const* e = g->add_edge(n[0], n[1]);
            identifier2edge.insert(make_pair(i->first, e));
            ep_now.push_back(EdgeProperty());
            ep = &ep_now.back();
          } else
            ep = &ep_now[id2e->second->index()];

          ep->identifier = i->first;
          ep->name = identifier2edge_name[i->first];
          ep->weight = float(i->second);
        }

        // --------------------------------------------------------------------------------
        // Set a layer name.

        struct tm tm0;
        localtime_s(&tm0, &t);
        if (time_format.empty()) {
          layer_name.push_back(
            title+L": "
            +(boost::wformat(L"%04d.%02d.%02d %02d:%02d")
              %(1900+tm0.tm_year)%(1+tm0.tm_mon)%tm0.tm_mday%tm0.tm_hour%tm0.tm_min).str());
        } else {
          wstring s;
          for (size_t i=0; i<time_format.size(); ++i) {
            if (time_format[i]=='Y') {
              s += (boost::wformat(L"%04d")%(1900+tm0.tm_year)).str();
            } else if (time_format[i]=='M') {
              s += (boost::wformat(L"%02d")%(1+tm0.tm_mon)).str();
            } else if (time_format[i]=='D') {
              s += (boost::wformat(L"%02d")%(tm0.tm_mday)).str();
            } else if (time_format[i]=='h') {
              s += (boost::wformat(L"%02d")%(tm0.tm_hour)).str();
            } else if (time_format[i]=='m') {
              s += (boost::wformat(L"%02d")%(tm0.tm_min)).str();
            } else if (time_format[i]=='s') {
              s += (boost::wformat(L"%02d")%(tm0.tm_sec)).str();
            } else s += time_format[i];
          }

          if (!title.empty()) layer_name.push_back(title+L": "+s);
          else layer_name.push_back(s);
        }
      }
    }

} // The end of the namespace "hashimoto_ut"

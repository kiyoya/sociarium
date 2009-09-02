// s.o.c.i.a.r.i.u.m
// HASHIMOTO, Yasuhiro (E-mail: hy @ sys.t.u-tokyo.ac.jp)
// update: 2009/05/01

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

#include <cassert>
#include <unordered_map>
#include <fstream>
#include <boost/format.hpp>
#include <windows.h>
#include "creation.h"
#include "creation_detail.h"
#include "../common.h"
#include "../message.h"
#include "../../shared/thread.h"
#include "../../shared/general.h"
#include "../../shared/msgbox.h"
#include "../../shared/win32api.h"
#include "../../graph/graph.h"

BOOL WINAPI DllMain (HINSTANCE hinstDll, DWORD fdwReason, LPVOID lpvReserved) { return TRUE; }

namespace hashimoto_ut {

  using std::vector;
  using std::string;
  using std::wstring;
  using std::pair;
  using std::ifstream;
  using std::getline;
  using std::tr1::shared_ptr;
  using std::tr1::unordered_map;

  using namespace sociarium_project_module_creation_detail;

  extern "C" __declspec(dllexport)
    void __cdecl create_graph_time_series(
      Thread* parent,
      wstring* message,
      vector<shared_ptr<Graph> >& graph,
      vector<vector<sociarium_project_user_defined_property::NodeProperty> >& node_property,
      vector<vector<sociarium_project_user_defined_property::EdgeProperty> >& edge_property,
      vector<wstring>& time_label,
      char const* filename) {

      assert(graph.empty());
      assert(node_property.empty());
      assert(edge_property.empty());
      assert(time_label.empty());

      ////////////////////////////////////////////////////////////////////////////////////////////////////
      // 区切り文字と諸属性の読み込み＋行数のカウント
      char delimiter;
      wstring title;
      string nlabel; // 使わない
      bool directed = false;
      int const number_of_lines = get_items(parent, filename, delimiter, title, nlabel, directed);
      if (number_of_lines<0) return;

      ////////////////////////////////////////////////////////////////////////////////////////////////////
      // グラフの構築
      vector<shared_ptr<Graph> >(1, Graph::create(directed)).swap(graph);
      shared_ptr<Graph>& g = graph[0];
      vector<string> node_name;
      vector<float> edge_weight;
      wstring const filename_w = mbcs2wcs(filename, strlen(filename));

      unordered_map<string, Node*> label2node;
      unordered_map<string, Edge*> label2edge;

      ifstream ifs(filename);
      string line;

      for (size_t count=1; getline(ifs, line); ++count) {

        // ----------------------------------------------------------------------------------------------------
        if (parent->cancel_check()) { graph.clear(); return; }
        // ----------------------------------------------------------------------------------------------------

        if (message)
          *message = (boost::wformat(L"%s: %d%%")
                      %sociarium_project_message::MAKING_GRAPH_SNAPSHOT
                      %int(100.0*count/number_of_lines)).str();

        line = get_body(line);
        if (line.empty()) continue;
        if (line[0]=='#') continue;

        // 行のパース
        vector<string> tok = tokenize(line, delimiter);

        if (tok.size()<2) {
          MsgBox(sociarium_project_common::get_window_handle(), sociarium_project_common::APPLICATION_TITLE,
                 L"%s [%s:%d]", sociarium_project_message::ERROR_INVALID_NUMBER_OF_ITEMS, filename_w.c_str(), count);
          graph.clear();
          return;
        }

        if (tok[0].empty() || tok[1].empty()) {
          MsgBox(sociarium_project_common::get_window_handle(), sociarium_project_common::APPLICATION_TITLE,
                 L"bad value: [%s, %d]", filename_w.c_str(), count);
            continue;
        }

        // n[0]がsource, n[1]がtarget
        Node* n[2];
        for (int i=0; i<2; ++i) {
          unordered_map<string, Node*>::const_iterator pos = label2node.find(tok[i]);
          if (pos==label2node.end()) {
            n[i] = g->add_node();
            label2node.insert(make_pair(tok[i], n[i]));
            node_name.push_back(tok[i]);
          } else {
            n[i] = pos->second;
          }
        }

        string const edge_label = directed?(tok[0]+delimiter+tok[1]):
        (tok[0]<tok[1]?tok[0]+delimiter+tok[1]:tok[1]+delimiter+tok[0]);
        unordered_map<string, Edge*>::const_iterator pos = label2edge.find(edge_label);
        if (pos==label2edge.end()) {
          Edge* e = g->add_edge(n[0], n[1]);
          label2edge.insert(make_pair(edge_label, e));
          if (tok.size()>=3) {
            try {
              float const w = boost::lexical_cast<float>(tok[2]);
              edge_weight.push_back(w);
            } catch (...) {
              MsgBox(sociarium_project_common::get_window_handle(), sociarium_project_common::APPLICATION_TITLE,
                     L"bad value: [%s, %d]", filename_w.c_str(), count);
              edge_weight.push_back(1.0f);
            }
          } else {
            edge_weight.push_back(1.0f);
          }
        } else {
          Edge* e = pos->second;
          if (tok.size()>=3) {
            try {
              float const w = boost::lexical_cast<float>(tok[2]);
              edge_weight[e->index()] += w;
            } catch (...) {
              MsgBox(sociarium_project_common::get_window_handle(), sociarium_project_common::APPLICATION_TITLE,
                     L"bad value: [%s, %d]", filename_w.c_str(), count);
              edge_weight[e->index()] += 1.0f;
            }
          } else {
            edge_weight[e->index()] += 1.0f;
          }
        }
      }

      // ----------------------------------------------------------------------------------------------------
      if (parent->cancel_check()) { graph.clear(); return; }
      // ----------------------------------------------------------------------------------------------------

      ////////////////////////////////////////////////////////////////////////////////////////////////////
      // プロパティの設定
      set_properties_for_snapshot(g, node_property, edge_property, node_name, edge_weight);

      ////////////////////////////////////////////////////////////////////////////////////////////////////
      // 時刻ラベルの設定
      time_label.push_back(title);
    }

} // The end of the namespace "hashimoto_ut"

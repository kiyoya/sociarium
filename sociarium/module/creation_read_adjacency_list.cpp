// s.o.c.i.a.r.i.u.m
// HASHIMOTO, Yasuhiro (E-mail: hy @ sys.t.u-tokyo.ac.jp)

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
#include <fstream>
#include <boost/format.hpp>
#ifdef _MSC_VER
#include <unordered_map>
#include <windows.h>
#else
#include <tr1/unordered_map>
#endif
#include "creation.h"
#include "creation_detail.h"
#include "../common.h"
#include "../message.h"
#include "../../shared/thread.h"
#include "../../shared/general.h"
#include "../../shared/win32api.h"
#include "../../shared/msgbox.h"
#include "../../graph/graph.h"

BOOL WINAPI DllMain (HINSTANCE hinstDll, DWORD fdwReason, LPVOID lpvReserved) { return TRUE; }

namespace hashimoto_ut {

  using std::vector;
  using std::map;
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

        Node* source = 0;
        for (size_t i=0; i<tok.size(); ++i) {
          string const& label = tok[i];
          unordered_map<string, Node*>::const_iterator j = label2node.find(label);
          Node* target = 0;
          if (j==label2node.end()) {
            target = g->add_node();
            label2node.insert(make_pair(label, target));
            node_name.push_back(label);
          } else {
            target = j->second;
          }

          if (source==0) source = target;
          else {
            string const edge_label = directed?(tok[0]+delimiter+tok[i]):
            (tok[0]<tok[i]?tok[0]+delimiter+tok[i]:tok[i]+delimiter+tok[0]);
            unordered_map<string, Edge*>::const_iterator pos = label2edge.find(edge_label);
            if (pos==label2edge.end()) {
              Edge* e = g->add_edge(source, target);
              label2edge.insert(make_pair(edge_label, e));
              edge_weight.push_back(1.0f);
            } else {
              ++edge_weight[pos->second->index()];
            }
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

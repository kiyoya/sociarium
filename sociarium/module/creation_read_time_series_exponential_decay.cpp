// s.o.c.i.a.r.i.u.m
// HASHIMOTO, Yasuhiro (E-mail: hy @ sys.t.u-tokyo.ac.jp)
// update: 2009/04/24

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
#include <map>
#include <fstream>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
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
  using std::multimap;
  using std::pair;
  using std::string;
  using std::wstring;
  using std::ifstream;
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

      wstring filename_w = mbcs2wcs(filename, strlen(filename));

      ////////////////////////////////////////////////////////////////////////////////////////////////////
      // 区切り文字とタイトルの読み込み＋行数のカウント
      char delimiter;
      wstring title;
      string tlabel;
      bool directed = false;
      time_t interval;
      time_t characteristic_time;
      double threshold;
      int const number_of_lines = get_items(parent, filename, delimiter, title, tlabel, directed, interval, characteristic_time, threshold);
      if (number_of_lines<0) return;

      if (characteristic_time==0) {
        MsgBox(sociarium_project_common::get_window_handle(), sociarium_project_common::APPLICATION_TITLE,
               L"bad characteristic_time: [%s]", filename_w.c_str());
        return;
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////
      // 時系列データの読み込み

      multimap<time_t, pair<string, double> > node_time_series; // <時刻, <ノードラベル, 重み> >
      multimap<time_t, pair<string, double> > edge_time_series; // <時刻, <エッジラベル, 重み> >

      ifstream ifs(filename);
      string line;

      for (int count=1; getline(ifs, line); ++count) {

        if (message)
          *message = (boost::wformat(L"%s: %d%%")
                      %sociarium_project_message::READING_GRAPH_TIME_SERIES
                      %int(100.0*count/number_of_lines)).str();

        // ----------------------------------------------------------------------------------------------------
        if (parent->cancel_check()) return;
        // ----------------------------------------------------------------------------------------------------

        if (line.empty()) continue;
        if (line[0]=='#') continue;
        // 行をトークン分割してvector<string>に格納
        vector<string> tok = tokenize(line, delimiter);
        // 時系列データは"時刻[delim]ノードラベル1[delim]ノードラベル2[delim]重み"という形式を要求
        if (tok.size()<4) {
          wstring filename_w = mbcs2wcs(filename, strlen(filename));
          MsgBox(sociarium_project_common::get_window_handle(), sociarium_project_common::APPLICATION_TITLE,
                 L"%s [%s, %d]", sociarium_project_message::ERROR_INVALID_NUMBER_OF_ITEMS, filename_w.c_str(), count);
          return;
        }

        // ----------------------------------------------------------------------------------------------------
        if (parent->cancel_check()) return;
        // ----------------------------------------------------------------------------------------------------

        time_t time;
        try {
          time = boost::lexical_cast<time_t>(tok[0]);
        } catch (...) {
          MsgBox(sociarium_project_common::get_window_handle(), sociarium_project_common::APPLICATION_TITLE,
                 L"bad time [%s, %d]", filename_w.c_str(), count);
          return;
        }

        string const& source = tok[1]; // ノード1
        string const& target = tok[2]; // ノード2

        double w;
        try {
          w = boost::lexical_cast<double>(tok[3]);
        } catch (...) {
          MsgBox(sociarium_project_common::get_window_handle(), sociarium_project_common::APPLICATION_TITLE,
                 L"bad weight [%s, %d]", filename_w.c_str(), count);
          w = 0.0f;
        }

        if (tok[2].empty()) {
          // 2つ目のノードが空文字列の場合，エッジではなくてノードが出現したとみなす
          node_time_series.insert(make_pair(time, make_pair(source, w)));
        } else {
          // エッジのラベルは"ノードラベル[delim]ノードラベル"とする
          string const edge_label = directed?source+delimiter+target:(source<target?source+delimiter+target:target+delimiter+source);
          edge_time_series.insert(make_pair(time, make_pair(edge_label, w)));
        }
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////
      // グラフ時系列の構築とグラフ要素の動的属性の設定

      if (node_time_series.empty() && edge_time_series.empty()) {
        wstring filename_w = mbcs2wcs(filename, strlen(filename));
        MsgBox(sociarium_project_common::get_window_handle(), sociarium_project_common::APPLICATION_TITLE,
               L"%s [%s]", sociarium_project_message::ERROR_NO_VALID_DATA, filename_w.c_str());
        return;
      }

      // 先頭データの時刻
      time_t const start_time =
        node_time_series.empty()?
          edge_time_series.begin()->first
            :(edge_time_series.empty()?
              node_time_series.begin()->first
              :(node_time_series.begin()->first<edge_time_series.begin()->first?
                node_time_series.begin()->first
                :edge_time_series.begin()->first));

      // 末尾データの時刻
      time_t const end_time =
        node_time_series.empty()?
          edge_time_series.rbegin()->first
            :(edge_time_series.empty()?
              node_time_series.rbegin()->first
              :(node_time_series.rbegin()->first>edge_time_series.rbegin()->first?
                node_time_series.rbegin()->first
                :edge_time_series.rbegin()->first));

      time_t const range = 3*characteristic_time; // カットオフ値=1e-3

      // 時系列に沿ってグラフを構築
      for (time_t t=start_time; t<=end_time; t+=interval) {

        if (message)
          *message = (boost::wformat(L"%s: %d%%")
                      %sociarium_project_message::MAKING_GRAPH_TIME_SERIES
                      %int(100.0*(t-start_time)/(end_time-start_time))).str();

        // ----------------------------------------------------------------------------------------------------
        if (parent->cancel_check()) { graph.clear(); return; }
        // ----------------------------------------------------------------------------------------------------

        unordered_map<string, double> node_weight; // 時刻tにおけるノードの重み
        unordered_map<string, double> edge_weight; // 時刻tにおけるエッジの重み
        // 時刻tにおけるノードとエッジの重みを計算
        { // 時刻t-range〜tのノードについて重みを積算
          multimap<time_t, pair<string, double> >::const_iterator now = node_time_series.upper_bound(t);
          multimap<time_t, pair<string, double> >::const_iterator i = node_time_series.upper_bound(t-range);
          for (; i!=now; ++i) node_weight[i->second.first] += (i->second.second)*exp(-double(t-i->first)/characteristic_time);
        }{// 時刻t-interval〜tのエッジについて重みを積算
          multimap<time_t, pair<string, double> >::const_iterator now = edge_time_series.upper_bound(t);
          multimap<time_t, pair<string, double> >::const_iterator i = edge_time_series.upper_bound(t-range);
          for (; i!=now; ++i) edge_weight[i->second.first] += (i->second.second)*exp(-double(t-i->first)/characteristic_time);
        }

        // ----------------------------------------------------------------------------------------------------
        if (parent->cancel_check()) { graph.clear(); return; }
        // ----------------------------------------------------------------------------------------------------

        // レイヤーの追加
        graph.push_back(shared_ptr<Graph>(Graph::create(directed)));
        node_property.push_back(vector<sociarium_project_user_defined_property::NodeProperty>());
        edge_property.push_back(vector<sociarium_project_user_defined_property::EdgeProperty>());
        shared_ptr<Graph>& g = graph.back();
        vector<sociarium_project_user_defined_property::NodeProperty>& np_now = node_property.back();
        vector<sociarium_project_user_defined_property::EdgeProperty>& ep_now = edge_property.back();
        unordered_map<string, Node*> label2node;
        unordered_map<string, Edge const*> label2edge;

        // ノードの作成と動的属性の設定
        for (unordered_map<string, double>::const_iterator i=node_weight.begin(), wend=node_weight.end(); i!=wend; ++i) {
          if (i->second<threshold) continue;
          Node* n = g->add_node();
          label2node.insert(make_pair(i->first, n));
          np_now.push_back(sociarium_project_user_defined_property::NodeProperty());
          sociarium_project_user_defined_property::NodeProperty& np = np_now.back();
          np.name = mbcs2wcs(i->first.c_str(), i->first.size());
          np.text = L"";
          np.weight = float(i->second);
        }

        // ----------------------------------------------------------------------------------------------------
        if (parent->cancel_check()) { graph.clear(); return; }
        // ----------------------------------------------------------------------------------------------------

        // エッジの作成と動的属性の設定
        for (unordered_map<string, double>::const_iterator i=edge_weight.begin(), wend=edge_weight.end(); i!=wend; ++i) {
          if (i->second<threshold) continue;
          // エッジのラベルを再分割して2つのノードラベルを取り出す
          vector<string> tok = tokenize(i->first, delimiter);
          assert(tok.size()==2);
          Node* n[2];

          for (int j=0; j<2; ++j) {
            unordered_map<string, Node*>::iterator pos = label2node.find(tok[j]);
            if (pos==label2node.end()) {
              // 初出のノードであれば新たに追加
              label2node.insert(make_pair(tok[j], (n[j]=g->add_node())));
              np_now.push_back(sociarium_project_user_defined_property::NodeProperty());
              sociarium_project_user_defined_property::NodeProperty& np = np_now.back();
              np.name = mbcs2wcs(tok[j].c_str(), tok[j].size());
              np.text = L"";
              np.weight = float(i->second);
            } else {
              n[j] = pos->second;
            }
          }

          unordered_map<string, Edge const*>::iterator pos = label2edge.find(i->first);
          if (pos==label2edge.end()) {
            Edge const* e = g->add_edge(n[0], n[1]);
            label2edge.insert(make_pair(i->first, e));
            ep_now.push_back(sociarium_project_user_defined_property::EdgeProperty());
            sociarium_project_user_defined_property::EdgeProperty& ep = ep_now.back();
            ep.text = L"";
            ep.weight = float(i->second);
          } else {
            sociarium_project_user_defined_property::EdgeProperty& ep = ep_now[pos->second->index()];
            ep.weight += float(i->second);
          }
        }

        if (tlabel=="time_t") {
          // 西暦表示のラベルの作成
          struct tm tm0;
          localtime_s(&tm0, &t);
          time_label.push_back(title+(boost::wformat(L": %04d.%02d.%02d %02d:%02d")
                                      %(1900+tm0.tm_year)%(1+tm0.tm_mon)%tm0.tm_mday%tm0.tm_hour%tm0.tm_min).str());
        } else {
          try {
            wstring const label = boost::lexical_cast<wstring>(t);
            time_label.push_back(title+L": "+label);
          } catch (...) {
            MsgBox(sociarium_project_common::get_window_handle(), sociarium_project_common::APPLICATION_TITLE,
                   L"bad tlabel [%s]", filename_w.c_str());
            time_label.push_back(L"");
          }
        }
      }
    }
} // The end of the namespace "hashimoto_ut"

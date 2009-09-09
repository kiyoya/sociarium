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
#include <boost/random.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <windows.h>
#include "creation.h"
#include "../common.h"
#include "../message.h"
#include "../../shared/thread.h"
#include "../../shared/general.h"
#include "../../shared/win32api.h"
#include "../../shared/msgbox.h"
#include "../../graph/graph.h"
#include "../../graph/graphex.h"

BOOL WINAPI DllMain (HINSTANCE hinstDll, DWORD fdwReason, LPVOID lpvReserved) { return TRUE; }

namespace hashimoto_ut {

  using std::vector;
  using std::string;
  using std::wstring;
  using std::pair;
  using std::ifstream;
  using std::tr1::shared_ptr;

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

      time_t t;
      boost::mt19937 generator((unsigned long)time(&t));
      boost::uniform_real<> distribution(0.0, 1.0);
      boost::variate_generator<boost::mt19937, boost::uniform_real<> > rand(generator, distribution);

      ////////////////////////////////////////////////////////////////////////////////////////////////////
      // 階層の深さとタイトルの読み込み＋行数のカウント
      size_t depth = 0;
      wstring title = L"";
      bool directed = false;
      int number_of_lines = 0;
      {
        string const pre_depth = "#depth=";
        string const pre_title = "#title=";
        string const pre_directed = "#directed";

        ifstream ifs(filename);
        string line;

        while (getline(ifs, line)) {

          // ----------------------------------------------------------------------------------------------------
          if (parent->cancel_check()) return;
          // ----------------------------------------------------------------------------------------------------

          ++number_of_lines;
          if (line.find(pre_title)==0) {
            if (line.size()<pre_title.size()+1) continue;
            string const t = line.substr(pre_title.size());
            title = mbcs2wcs(t.c_str(), t.size());
          } else if (line.find(pre_directed)==0) {
            directed = true;
          } else if (line.find(pre_depth)==0 && depth==0) {
            try {
              depth = boost::lexical_cast<size_t>(line.substr(pre_depth.size()));
            } catch (...) {
              MsgBox(sociarium_project_common::get_window_handle(), sociarium_project_common::APPLICATION_TITLE,
                     L"bad number of nodes: [%s, %d]", filename_w.c_str(), number_of_lines);
              depth = 0;
              continue;
            }
          }
        }

        if (number_of_lines==0) {
          MsgBox(sociarium_project_common::get_window_handle(), sociarium_project_common::APPLICATION_TITLE,
                 L"%s [%s]", sociarium_project_message::ERROR_NO_LINE, filename_w.c_str());
          return;
        }

        if (depth==0) {
          MsgBox(sociarium_project_common::get_window_handle(), sociarium_project_common::APPLICATION_TITLE,
                 L"%s [%s]", sociarium_project_message::ERROR_FAILED_TO_READ_DATA, filename_w.c_str());
          return;
        }
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////
      // グラフの構築

      vector<shared_ptr<Graph> >(1, Graph::create(directed)).swap(graph);
      shared_ptr<Graph>& g = graph[0];

      if (message)
        *message = (boost::wformat(L"%s: Sierpinski Gasket [D=%d, N=%d, E=%d]")
                    %sociarium_project_message::MAKING_GRAPH_SNAPSHOT
                    %depth%g->nsize()%g->esize()).str();

      g->add_node();
      g->add_node();
      g->add_node();
      g->add_edge(g->node(0), g->node(1));
      g->add_edge(g->node(1), g->node(2));
      g->add_edge(g->node(2), g->node(0));

      // 三角形の次数2の頂点同士を融合して大きな三角形を作成
      for (size_t d=0; d<depth; ++d) {

        if (message)
          *message = (boost::wformat(L"%s: Sierpinski Gasket [D=%d, N=%d, E=%d]")
                      %sociarium_project_message::MAKING_GRAPH_SNAPSHOT
                      %depth%g->nsize()%g->esize()).str();

        // ----------------------------------------------------------------------------------------------------
        if (parent->cancel_check()) { graph.clear(); return; }
        // ----------------------------------------------------------------------------------------------------

        shared_ptr<Graph> g1 = copy(g);

        // ----------------------------------------------------------------------------------------------------
        if (parent->cancel_check()) { graph.clear(); return; }
        // ----------------------------------------------------------------------------------------------------

        shared_ptr<Graph> g2 = copy(g);

        // ----------------------------------------------------------------------------------------------------
        if (parent->cancel_check()) { graph.clear(); return; }
        // ----------------------------------------------------------------------------------------------------

        g->merge(g1); // マージされたグラフ要素はコンテナ後方に追加される
        g->merge(g2); // マージされたグラフ要素はコンテナ後方に追加される

        // ----------------------------------------------------------------------------------------------------
        if (parent->cancel_check()) { graph.clear(); return; }
        // ----------------------------------------------------------------------------------------------------

        vector<Node*> v;
        for (node_iterator i=g->nbegin(); i!=g->nend(); ++i) if ((*i)->degree()==2) v.push_back(*i);
        assert(v.size()==9); // v[0:2], v[3:5], v[6:8]が融合した3つの三角形の頂点に対応
        // 三角形の頂点同士を縮約して三角形を連結
        contract(g, v[0], v[3]);
        contract(g, v[1], v[6]);
        contract(g, v[4], v[7]);

        if (message)
          *message = (boost::wformat(L"%s: Sierpinski Gasket [D=%d, N=%d, E=%d]")
                      %sociarium_project_message::MAKING_GRAPH_SNAPSHOT
                      %depth%g->nsize()%g->esize()).str();
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////
      // プロパティの設定
      node_property.resize(1, vector<sociarium_project_user_defined_property::NodeProperty>(g->nsize()));
      int const digit = int(log10(double(g->nsize())))+1;
      for (size_t i=0, nsz=g->nsize(); i<nsz; ++i) {
        // ----------------------------------------------------------------------------------------------------
        if (parent->cancel_check()) { graph.clear(); return; }
        // ----------------------------------------------------------------------------------------------------
        sociarium_project_user_defined_property::NodeProperty& np = node_property[0][i];
        if (digit==1) np.name = (boost::wformat(L"n%d")%i).str();
        else if (digit==2) np.name = (boost::wformat(L"n%02d")%i).str();
        else if (digit==3) np.name = (boost::wformat(L"n%03d")%i).str();
        else if (digit==4) np.name = (boost::wformat(L"n%04d")%i).str();
        else if (digit==5) np.name = (boost::wformat(L"n%05d")%i).str();
        else np.name = (boost::wformat(L"n%d")%i).str();
        np.text = L"";
        np.weight = 1.0f;
      }
      edge_property.resize(1, vector<sociarium_project_user_defined_property::EdgeProperty>(g->esize()));
      for (size_t i=0, esz=g->esize(); i<esz; ++i) {
        // ----------------------------------------------------------------------------------------------------
        if (parent->cancel_check()) { graph.clear(); return; }
        // ----------------------------------------------------------------------------------------------------
        sociarium_project_user_defined_property::EdgeProperty& ep = edge_property[0][i];
        ep.text = L"";
        ep.weight = 1.0f;
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////
      // 時刻ラベルの設定
      time_label.push_back(title);
    }

} // The end of the namespace "hashimoto_ut"

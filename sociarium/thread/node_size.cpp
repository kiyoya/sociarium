// s.o.c.i.a.r.i.u.m
// HASHIMOTO, Yasuhiro (E-mail: hy @ sys.t.u-tokyo.ac.jp)
// update: 2009/04/02

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

#include <string>
#ifdef _MSC_VER
#include <unordered_map>
#else
#include <tr1/unordered_map>
#endif
#include <boost/format.hpp>
#include "node_size.h"
#include "../graph_extractor.h"
#include "../common.h"
#include "../thread.h"
#include "../message.h"
#include "../view.h"
#include "../draw.h"
#include "../graph_utility.h"
#include "../sociarium_graph_time_series.h"
#include "../../shared/msgbox.h"
#include "../../graph/util/traverser.h"

namespace hashimoto_ut {

  using std::vector;
  using std::wstring;
  using std::pair;
  using std::tr1::unordered_map;
  using std::tr1::shared_ptr;

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  NodeSizeThread::NodeSizeThread(shared_ptr<SociariumGraphTimeSeries> const& time_series)
       : time_series_(time_series) {}

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  void NodeSizeThread::terminate(void) const {
    time_series_->read_unlock();
    sociarium_project_thread::set_current_node_size_thread(shared_ptr<Thread>());
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  void NodeSizeThread::operator()(void) {

    time_series_->read_lock();

    vector<vector<double> > node_size;

    for (size_t layer=0; layer<time_series_->number_of_layers(); ++layer) {

      shared_ptr<SociariumGraph> const& g = time_series_->get_graph(0, layer);
      vector<double> sz(g->nsize(), sociarium_project_draw::get_default_node_size());

      // ----------------------------------------------------------------------------------------------------
      // 表示された要素のみからなるグラフを構築
      unordered_map<Node*, Node const*> node2node; // g_targetのノードからgのノードを取得
      unordered_map<Edge*, Edge const*> edge2edge; // g_targetのエッジからgのエッジを取得
      shared_ptr<Graph> g_target = sociarium_project_graph_extractor::get(this, 0, g, node2node, edge2edge, ElementFlag::VISIBLE);
      if (g_target==0 || g_target->nsize()==0) {
        node_size.push_back(sz);
        continue;
      }

      // ----------------------------------------------------------------------------------------------------
      // サイズを計算
      int const size_factor = sociarium_project_view::get_node_size_factor();

      if (size_factor==sociarium_project_view::NodeView::SizeFactor::CLOSENESS_CENTRALITY) {

      }

      else if (size_factor==sociarium_project_view::NodeView::SizeFactor::BETWEENNESS_CENTRALITY) {
        shared_ptr<BFSRecordingTraverser> t =
          g_target->is_directed()?BFSRecordingTraverser::create<downward_tag>(g_target)
            :BFSRecordingTraverser::create<bidirectional_tag>(g_target);
				pair<bool, pair<vector<double>, vector<double> > > bc = sociarium_project_graph_utility::betweenness_centrality(this, 0, t);
        if (bc.first) {
          for (node_iterator i=g_target->nbegin(); i!=g_target->nend(); ++i)
            sz[node2node[*i]->index()] = bc.second.first[(*i)->index()];
          node_size.push_back(sz);
        }
        else return terminate();
      }

      else if (size_factor==sociarium_project_view::NodeView::SizeFactor::PAGERANK) {

      }

      else assert(0 && "never reach");
    }

    if (node_size.size()==time_series_->number_of_layers()) time_series_->update_node_size(node_size);
    terminate();
  }

} // The endof the namespace "hashimoto_ut"

// s.o.c.i.a.r.i.u.m
// HASHIMOTO, Yasuhiro (E-mail: hy @ sys.t.u-tokyo.ac.jp)
// update: 2009/03/31

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
#include "layout.h"
#include "../module/layout.h"
#include "../graph_extractor.h"
#include "../common.h"
#include "../thread.h"
#include "../message.h"
#include "../draw.h"
#include "../algorithm_selector.h"
#include "../sociarium_graph.h"
#include "../sociarium_graph_time_series.h"
#include "../../shared/msgbox.h"

namespace hashimoto_ut {

  using std::vector;
  using std::wstring;
  using std::pair;
  using std::tr1::unordered_map;
  using std::tr1::shared_ptr;

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  LayoutThread::LayoutThread(shared_ptr<SociariumGraphTimeSeries> const& time_series)
       : time_series_(time_series) {}

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  void LayoutThread::terminate(void) const {
    sociarium_project_message::get()->first = L"";
    sociarium_project_message::get()->second = L"";
    time_series_->read_unlock();
    sociarium_project_thread::set_current_graph_layout_thread(shared_ptr<Thread>());
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  void LayoutThread::operator()(void) {

    time_series_->read_lock();

    using namespace sociarium_project_algorithm_selector;

    // ----------------------------------------------------------------------------------------------------
    // 選択された要素のみからなるグラフを構築
    size_t const index = time_series_->index_of_current_layer();
    shared_ptr<SociariumGraph> const& g = time_series_->get_graph(0, index);
    unordered_map<Node*, Node const*> node2node; // g_targetのノードからgのノードを取得
    unordered_map<Edge*, Edge const*> edge2edge; // g_targetのエッジからgのエッジを取得
    shared_ptr<Graph> g_target = sociarium_project_graph_extractor::get(this, &sociarium_project_message::get()->first, g, node2node, edge2edge, ElementFlag::MARKED);
    if (g_target==0) return terminate();

    // ----------------------------------------------------------------------------------------------------
    // 選択された要素の位置を取得
    size_t const nsz = g->nsize();

    vector<double> input_values; // レイアウトで用いる構造以外の情報
    input_values.reserve(nsz);

    vector<Vector2<double> > position(nsz); // すべてのノードの位置
    for (SociariumGraph::node_property_iterator i=g->node_property_begin(), end=g->node_property_end(); i!=end; ++i) {
      position[i->first->index()] = i->second->get_static_property()->get_position();
    }

    vector<Vector2<double> > position_target; // レイアウト対象のノードの位置
    position_target.reserve(nsz);
    for (node_iterator i=g_target->nbegin(), end=g_target->nend(); i!=end; ++i) {
      shared_ptr<DynamicNodeProperty> const& dnp = g->property(node2node[*i]);
      position_target.push_back(dnp->get_static_property()->get_position());
    }

    if (get_graph_layout_algorithm()==LayoutAlgorithm::CIRCLE_IN_SIZE_ORDER ||
        get_graph_layout_algorithm()==LayoutAlgorithm::ARRAY) {
      for (node_iterator i=g_target->nbegin(), end=g_target->nend(); i!=end; ++i) {
        shared_ptr<DynamicNodeProperty> const& dnp = g->property(node2node[*i]);
        input_values.push_back(dnp->get_size());
      }
    } else if (get_graph_layout_algorithm()==LayoutAlgorithm::KAMADA_KAWAI_METHOD) {
      input_values.push_back(double(sociarium_project_draw::get_layout_frame_size()));
    }

    assert(g_target->nsize()==position_target.size());

    // ----------------------------------------------------------------------------------------------------
    // グラフレイアウトモジュールをロード
    sociarium_project_message::get()->first = (boost::wformat(L"%s")%sociarium_project_message::LOADING_MODULE).str();
    FuncLayoutGraph layout_graph = sociarium_project_module_layout::get(get_graph_layout_algorithm());
    if (layout_graph==0) return terminate();

    // ----------------------------------------------------------------------------------------------------
    // グラフレイアウトモジュールを実行
    sociarium_project_message::get()->first = (boost::wformat(L"%s")%sociarium_project_message::MODULE_IS_RUNNING).str();
    layout_graph(this, sociarium_project_message::get(), g_target, input_values, position_target);

    // ---------------------------------------------------------------------------------------------------
    // レイアウトフレーム内に収める
    Vector2<double> pos_max(-1e+10, -1e+10);
    Vector2<double> pos_min( 1e+10,  1e+10);

    size_t count = 0;
    size_t const count_max = 2*g_target->nsize();
    for (node_iterator i=g_target->nbegin(), end=g_target->nend(); i!=end; ++i) {

      sociarium_project_message::get()->second = (boost::wformat(L"%s: %d%%")
                                                  %sociarium_project_message::LAYOUT_ADJUSTING
                                                  %int(100.0*(0.5*(++count+1.0)/count_max))).str();

      Node const* n = node2node[*i];
      assert(n!=0);
      Vector2<double> const& pos = position[n->index()] = position_target[(*i)->index()];
      if (pos_max.x<pos.x) pos_max.x = pos.x;
      if (pos_min.x>pos.x) pos_min.x = pos.x;
      if (pos_max.y<pos.y) pos_max.y = pos.y;
      if (pos_min.y>pos.y) pos_min.y = pos.y;
    }

    Vector2<double> const pos_center(0.5*(pos_max+pos_min));
    Vector2<double> const size(0.5*(pos_max-pos_min));
    double const scale = size.x>size.y?sociarium_project_draw::get_layout_frame_size()/size.x
			:(size.y>0.0?sociarium_project_draw::get_layout_frame_size()/size.y:0.0);

    for (node_iterator i=g_target->nbegin(), end=g_target->nend(); i!=end; ++i) {

      sociarium_project_message::get()->second = (boost::wformat(L"%s: %d%%")
                                                  %sociarium_project_message::LAYOUT_ADJUSTING
                                                  %int(100.0*0.5*(++count+1.0)/count_max)).str();

      Node const* n = node2node[*i];
      assert(n!=0);
      Vector2<double>& pos = position[n->index()];
      pos = scale*(pos-pos_center)+sociarium_project_draw::get_layout_frame_position();
    }

    vector<Vector2<double> > position_prev(nsz);
    for (SociariumGraph::node_property_iterator i=g->node_property_begin(), end=g->node_property_end(); i!=end; ++i) {
      position_prev[i->first->index()] = i->second->get_static_property()->get_position();
    }

#if 0
    // 単純置換
    time_series_->update_node_position(index, position);
#else
    // 漸近置換
    for (int iteration=0; iteration<20; ++iteration) {
#ifdef _MSC_VER
      Sleep(10);
#else
      usleep(10);
#endif
      double const j = double(20-iteration);
      for (size_t i=0; i<nsz; ++i) position_prev[i] = (position[i]+(j-1.0)*position_prev[i])/j;
      time_series_->update_node_position(index, position_prev);
    }
#endif

    terminate();
  }

} // The endof the namespace "hashimoto_ut"

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

#include <vector>
#include <set>
#include <string>
#include <numeric>
#include <fstream>
#include <boost/bind.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#ifdef _MSC_VER
#include <functional>
#else
#include <tr1/functional>
#endif
#include "message.h"
#include "common.h"
#include "thread.h"
#include "mouse_and_selection.h"
#include "view.h"
#include "draw.h"
#include "texture.h"
#include "force_direction.h"
#include "sociarium_graph_time_series.h"
#include "../shared/msgbox.h"
#include "../shared/predefined_color.h"
#include "../graph/util/statistics.h"

namespace hashimoto_ut {

  using std::vector;
  using std::wstring;
  using std::accumulate;
  using std::pair;
  using std::make_pair;
  using std::ofstream;
  using std::endl;
  using std::tr1::bind;
  using std::tr1::shared_ptr;
  using std::tr1::unordered_map;

  namespace {
    ///////////////////////////////////////////////////////////////////////////////////////////////////
    struct SetNodeSizeUniform {
      void operator()(SociariumGraph::node_property_iterator::value_type p, float size) const { p.second->set_size(size); }
    };

    struct SetNodeSizeUsingDegreeCentrality {
      void operator()(SociariumGraph::node_property_iterator::value_type p, float size) const { p.second->set_size(0.5f*size*float(sqrt(p.first->degree()+1.0))); }
    };

    struct CreateSociariumGraph {
      void operator()(shared_ptr<SociariumGraph>& g) const { g = SociariumGraph::create(); } //[TODO]
    };

  } // The end of the anonymous namespace


  ///////////////////////////////////////////////////////////////////////////////////////////////////
  class SociariumGraphTimeSeriesImpl : public SociariumGraphTimeSeries {
  private:
    mutable boost::mutex mutex_;
    mutable boost::condition condition_;
    volatile int number_of_reading_blocks_;
    volatile int number_of_writing_blocks_;

    size_t index_of_current_layer_;
    vector<wstring> layer_label_;

    vector<shared_ptr<SociariumGraph> > graph_series_[2];
    StaticNodePropertyContainer static_node_property_[2];
    StaticEdgePropertyContainer static_edge_property_[2];

  public:

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    SociariumGraphTimeSeriesImpl(void) :
    index_of_current_layer_(0), number_of_reading_blocks_(0), number_of_writing_blocks_(0) {
      graph_series_[0].push_back(SociariumGraph::create());
      graph_series_[1].push_back(SociariumGraph::create());
      layer_label_.push_back(sociarium_project_message::WELCOME);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    ~SociariumGraphTimeSeriesImpl() {
      for (size_t i=0; i<number_of_layers(); ++i) {
        assert(graph_series_[0][i].unique());
        assert(graph_series_[1][i].unique());
      }
      graph_series_[0].clear();
      graph_series_[1].clear();
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    void read_lock(void) {
      boost::mutex::scoped_lock lock(mutex_);
      while (number_of_writing_blocks_>0) condition_.wait(lock);
      ++number_of_reading_blocks_;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    void write_lock(void) {
      boost::mutex::scoped_lock lock(mutex_);
      while (number_of_reading_blocks_>0 || number_of_writing_blocks_>0) condition_.wait(lock);
      ++number_of_writing_blocks_;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    void read_to_write_lock(void) {
      boost::mutex::scoped_lock lock(mutex_);
      while (number_of_writing_blocks_>0) condition_.wait(lock);
      ++number_of_writing_blocks_;
      while (number_of_reading_blocks_>1) condition_.wait(lock);
      --number_of_reading_blocks_;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    void write_to_read_lock(void) {
      boost::mutex::scoped_lock lock(mutex_);
      ++number_of_reading_blocks_;
      --number_of_writing_blocks_;
      condition_.notify_all();
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    void read_unlock(void) {
      boost::mutex::scoped_lock lock(mutex_);
      --number_of_reading_blocks_;
      condition_.notify_all();
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    void write_unlock(void) {
      boost::mutex::scoped_lock lock(mutex_);
      --number_of_writing_blocks_;
      condition_.notify_all();
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    size_t number_of_layers(void) const {
      return layer_label_.size();
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    size_t index_of_current_layer(void) const {
      return index_of_current_layer_;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    void move_layer(int index_of_layer) {
      int const current = index_of_current_layer_;
      if (index_of_layer<0) index_of_current_layer_ = 0;
      else if (size_t(index_of_layer)>=number_of_layers()) index_of_current_layer_ = number_of_layers()-1;
      else index_of_current_layer_ = size_t(index_of_layer);
      if (current!=index_of_current_layer_) sociarium_project_force_direction::fd_element_should_be_updated(true);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    shared_ptr<SociariumGraph> const& get_graph(size_t select, size_t index_of_layer) const {
      assert(index_of_layer>=0 && index_of_layer<number_of_layers());
      return graph_series_[select][index_of_layer];
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    wstring const& get_layer_label(size_t index_of_layer) const {
      return layer_label_[index_of_layer];
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    StaticNodePropertyContainer::const_iterator static_node_property_begin(size_t select) const {
      return static_node_property_[select].begin();
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    StaticNodePropertyContainer::const_iterator static_node_property_end(size_t select) const {
      return static_node_property_[select].end();
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    StaticEdgePropertyContainer::const_iterator static_edge_property_begin(size_t select) const {
      return static_edge_property_[select].begin();
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    StaticEdgePropertyContainer::const_iterator static_edge_property_end(size_t select) const {
      return static_edge_property_[select].end();
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    void remove_node(shared_ptr<StaticNodeProperty> const& snp) {
      StaticNodePropertyContainer::iterator pos = static_node_property_[0].find(snp);
      assert(pos!=static_node_property_[0].end());
      for (unordered_map<DynamicNodeProperty*, size_t>::const_iterator i, j=snp->dynamic_property_begin(); j!=snp->dynamic_property_end();) {
        i = j++;
        shared_ptr<SociariumGraph> const& g = graph_series_[0][i->second];
        Node* n = i->first->get_graph_element();
        StaticEdgePropertyContainer removed_edges; // std::vector will fail in the case a loop edge exists.
        for (adjacency_list_iterator k=n->begin(), kend=n->end(); k!=kend; ++k)
          removed_edges.insert(g->property(*k)->get_static_property());
        for (StaticEdgePropertyContainer::const_iterator k=removed_edges.begin(), kend=removed_edges.end(); k!=kend; ++k)
          remove_edge(*k);
        g->remove_node(n);
      }
      static_node_property_[0].erase(pos);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    void remove_edge(shared_ptr<StaticEdgeProperty> const& sep) {
      StaticEdgePropertyContainer::iterator pos = static_edge_property_[0].find(sep);
      assert(pos!=static_edge_property_[0].end());
      for (unordered_map<DynamicEdgeProperty*, size_t>::const_iterator i, j=sep->dynamic_property_begin(); j!=sep->dynamic_property_end();) {
        i = j++;
        graph_series_[0][i->second]->remove_edge(i->first->get_graph_element());
      }
      static_edge_property_[0].erase(pos);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    void remove_empty_static_property(size_t select) {
      StaticNodePropertyContainer& nodes = static_node_property_[select];
      StaticEdgePropertyContainer& edges = static_edge_property_[select];
      for (StaticNodePropertyContainer::iterator i, j=nodes.begin(), end=nodes.end(); j!=end;) {
        i = j++; if ((*i)->number_of_dynamic_properties()==0) nodes.erase(i);
      }
      for (StaticEdgePropertyContainer::iterator i, j=edges.begin(), end=edges.end(); j!=end;) {
        i = j++; if ((*i)->number_of_dynamic_properties()==0) edges.erase(i);
      }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    StaticNodeProperty* find_node(size_t select, wstring const& name) const {
      StaticNodePropertyContainer::const_iterator i
        = find_if(static_node_property_[select].begin(), static_node_property_[select].end(),
                  bind(std::equal_to<wstring>(), bind(&StaticNodeProperty::get_name, std::tr1::placeholders::_1), name));
      if (i==static_node_property_[select].end()) return 0;
      return i->get();
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    StaticEdgeProperty* find_edge(size_t select, wstring const& name) const {
      StaticEdgePropertyContainer::const_iterator i
        = find_if(static_edge_property_[select].begin(), static_edge_property_[select].end(),
                  bind(std::equal_to<wstring>(), bind(&StaticEdgeProperty::get_name, std::tr1::placeholders::_1), name));
      if (i==static_edge_property_[select].end()) return 0;
      return i->get();
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    void update_node_size(int size_factor) {

      // 一様サイズ
      if (size_factor==sociarium_project_view::NodeView::SizeFactor::UNIFORM) {

        for (size_t i=0; i<number_of_layers(); ++i) {
          shared_ptr<SociariumGraph> const& g = get_graph(0, i);
          size_t const nsz = g->nsize();
          if (nsz==0) continue;
          for_each(g->node_property_begin(), g->node_property_end(),
                   boost::bind<void>(SetNodeSizeUniform(), _1, sociarium_project_draw::get_default_node_size()));
        }
      }

      // 点
      else if (size_factor==sociarium_project_view::NodeView::SizeFactor::NONE){
        sociarium_project_view::set_node_size_factor(size_factor);
        for (size_t i=0; i<number_of_layers(); ++i) {
          shared_ptr<SociariumGraph> const& g = get_graph(0, i);
          size_t const nsz = g->nsize();
          if (nsz==0) continue;
          for_each(g->node_property_begin(), g->node_property_end(),
                   boost::bind<void>(SetNodeSizeUniform(), _1, 0.0f));
        }
      }

      // 次数中心性
      else if (size_factor==sociarium_project_view::NodeView::SizeFactor::DEGREE_CENTRALITY) {
        sociarium_project_view::set_node_size_factor(size_factor);
        for (size_t i=0; i<number_of_layers(); ++i) {
          shared_ptr<SociariumGraph> const& g = get_graph(0, i);
          size_t const nsz = g->nsize();
          if (nsz==0) continue;
          for_each(g->node_property_begin(), g->node_property_end(),
                   boost::bind<void>(SetNodeSizeUsingDegreeCentrality(), _1, sociarium_project_draw::get_default_node_size()));
        }
      }

      else assert(0 && "never reach");
    }
    //
    //     // 媒介中心性
    //     else if (size_factor==sociarium_project_view::NodeView::SizeFactor::BETWEENNESS_CENTRALITY) {
    //       if (sociarium_project_thread::get_current_graph_creation_thread()) {
    //         MsgBox(sociarium_project_common::get_window_handle(), sociarium_project_common::APPLICATION_TITLE, L"%s",
    //                sociarium_project_message::GRAPH_TIME_SERIES_IS_LOCKED);
    //         return;
    //       }
    //       shared_ptr<Thread> th(new NodeSizeThread(this);
    //       sociarium_project_thread::set_current_graph_statistics_thread(th);
    //       boost::thread(boost::ref(*th));
    //     }


    //     else if (// 距離中心性
    //              size_factor==sociarium_project_view::NodeView::SizeFactor::CLOSENESS_CENTRALITY ||
    //              // 媒介中心性
    //              size_factor==sociarium_project_view::NodeView::SizeFactor::BETWEENNESS_CENTRALITY ||
    //              // ページランク（固有ベクトル中心性）
    //              size_factor==sociarium_project_view::NodeView::SizeFactor::PAGERANK) {
    //       vector<double> size;
    //       graph_statistics_thread.reset(new CalcNodeSizeThread(sociarium_project_message::get(), time_series_, size));
    //       sociarium_project_thread::set_current_thread(graph_statistics_thread);
    //       boost::thread(boost::ref(*graph_statistics_thread));
    //       double const sum = accumulate(size.begin(), size.end(), 0.0)/nsz;
    //         if (sum<=0.0) continue;
    //     //         // サイズの規格化
    //       boost::mutex::scoped_lock lock(mutex_);
    //         for (size_t i=0; i<nsz; ++i)
    //           g->property(g->node(i))->set_size(sociarium_project_draw::get_default_node_size()*float(sqrt((size[i]+1.0)/sum)));
    //       }
    //     }

    //     else {
    //       vector<double> size;
    //       if (size_factor==sociarium_project_view::NodeView::SizeFactor::CLOSENESS_CENTRALITY) {
    //         // 距離中心性
    //         for (size_t i=0; i<number_of_layers(); ++i) {
    //           shared_ptr<SociariumGraph> const& g = get_graph(0, i);
    //           size_t const nsz = g->nsize();
    //           if (nsz==0) continue;
    //           size = closeness_centrality(BFSTraverser::create<downward_tag>(g));
    //           double const mean = accumulate(size.begin(), size.end(), 0.0)/nsz;
    //           if (mean<=0.0) continue;
    //           // サイズの規格化
    //           for (size_t i=0; i<nsz; ++i)
    //             g->property(g->node(i))->set_size(sociarium_project_draw::get_default_node_size()*float(sqrt((size[i]/mean+1.0))));
    //         }
    //       }
    //
    //       else if (size_factor==sociarium_project_view::NodeView::SizeFactor::PAGERANK) {
    //         // ページランク（固有ベクトル中心性）
    //         for (size_t i=0; i<number_of_layers(); ++i) {
    //           shared_ptr<SociariumGraph> const& g = get_graph(0, i);
    //           size_t const nsz = g->nsize();
    //           if (nsz==0) continue;
    //           size = pagerank(g);
    //           double const mean = accumulate(size.begin(), size.end(), 0.0)/nsz;
    //           if (mean<=0.0) continue;
    //           // サイズの規格化
    //           for (size_t i=0; i<nsz; ++i)
    //             g->property(g->node(i))->set_size(sociarium_project_draw::get_default_node_size()*float(sqrt((size[i]/mean+1.0))));
    //         }
    //       } else assert(0 && "never reach");
    //
    //   }

    void update_node_size(vector<vector<double> > const& node_size) {
      assert(node_size.size()==number_of_layers());
      for (size_t layer=0; layer<number_of_layers(); ++layer) {
        shared_ptr<SociariumGraph> const& g = get_graph(0, layer);
        size_t const nsz = g->nsize();
        if (nsz==0) continue;
        assert(node_size[layer].size()==nsz);
        double const mean = accumulate(node_size[layer].begin(), node_size[layer].end(), 0.0)/nsz;
        for (size_t i=0; i<nsz; ++i)
          g->property(g->node(i))->set_size(sociarium_project_draw::get_default_node_size()*float(sqrt((node_size[layer][i]/mean+1.0))));
      }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    void update_node_position(size_t index_of_layer, vector<Vector2<double> > const& position) {
      shared_ptr<SociariumGraph> const& g0 = graph_series_[0][index_of_layer];
      shared_ptr<SociariumGraph> const& g1 = graph_series_[1][index_of_layer];
      size_t const nsz = g0->nsize();
      assert(nsz==position.size());
      for (SociariumGraph::node_property_iterator i=g0->node_property_begin(), end=g0->node_property_end(); i!=end; ++i)
        i->second->get_static_property()->set_position(position[i->first->index()]);
      for (SociariumGraph::node_property_iterator i=g1->node_property_begin(), end=g1->node_property_end(); i!=end; ++i)
        i->second->move_to_center_of_lower_nodes_position();
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    void clear_community(void) {

      // ----------------------------------------------------------------------------------------------------
      write_lock();
      // ----------------------------------------------------------------------------------------------------

      vector<shared_ptr<SociariumGraph> >(number_of_layers()).swap(graph_series_[1]);
      for (size_t layer=0; layer<number_of_layers(); ++layer) {
        graph_series_[1][layer] = SociariumGraph::create(); // [TODO]
        shared_ptr<SociariumGraph> const& g0 = graph_series_[0][layer];
        for (SociariumGraph::node_property_iterator i=g0->node_property_begin(), end=g0->node_property_end(); i!=end; ++i)
          i->second->set_color_id(PredefinedColor::WHITE);
        for (SociariumGraph::edge_property_iterator i=g0->edge_property_begin(), end=g0->edge_property_end(); i!=end; ++i)
          i->second->set_color_id(PredefinedColor::WHITE);
      }

      static_node_property_[1].clear();
      static_edge_property_[1].clear();

      // ----------------------------------------------------------------------------------------------------
      write_unlock();
      // ----------------------------------------------------------------------------------------------------
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    void update(
      vector<shared_ptr<SociariumGraph> >& graph_series,
      StaticNodePropertyContainer& static_node_property,
      StaticEdgePropertyContainer& static_edge_property,
      vector<wstring>& layer_label) {

      assert(graph_series.size()==layer_label.size());

      // ----------------------------------------------------------------------------------------------------
      read_to_write_lock();
      // ----------------------------------------------------------------------------------------------------

      sociarium_project_mouse_and_selection::initialize_selection();

      index_of_current_layer_ = 0;
      graph_series.swap(graph_series_[0]);
      static_node_property.swap(static_node_property_[0]);
      static_edge_property.swap(static_edge_property_[0]);
      layer_label.swap(layer_label_);

      if (graph_series_[0].empty()) {
        for (int i=0; i<2; ++i) {
          graph_series_[i].push_back(SociariumGraph::create());
          static_node_property_[i].clear();
          static_edge_property_[i].clear();
        }
        layer_label_.push_back(sociarium_project_message::ERROR_FAILED_TO_READ_DATA);
      }

      vector<shared_ptr<SociariumGraph> > community_series(number_of_layers());
      for_each(community_series.begin(), community_series.end(), CreateSociariumGraph());
      community_series.swap(graph_series_[1]);

      // ----------------------------------------------------------------------------------------------------
      write_to_read_lock();
      // ----------------------------------------------------------------------------------------------------

      sociarium_project_force_direction::fd_element_should_be_updated(true);

      using namespace sociarium_project_view;
      int size_factor = get_node_size_factor();
      if (size_factor==NodeView::SizeFactor::UNIFORM ||
          size_factor==NodeView::SizeFactor::NONE ||
          size_factor==NodeView::SizeFactor::DEGREE_CENTRALITY) {
        update_node_size(size_factor);
      } else {
        set_node_size_factor(NodeView::SizeFactor::DEGREE_CENTRALITY);
        update_node_size(get_node_size_factor());
      }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    void update_community(
      vector<shared_ptr<SociariumGraph> >& community_series,
      StaticNodePropertyContainer& static_community_property,
      StaticEdgePropertyContainer& static_community_edge_property) {

      assert(graph_series_[0].size()==community_series.size());

      // ----------------------------------------------------------------------------------------------------
      read_to_write_lock();
      // ----------------------------------------------------------------------------------------------------

      sociarium_project_mouse_and_selection::initialize_selection();

      community_series.swap(graph_series_[1]);
      static_community_property.swap(static_node_property_[1]);
      static_community_edge_property.swap(static_edge_property_[1]);

      for (size_t layer=0; layer<number_of_layers(); ++layer) {
        shared_ptr<SociariumGraph> const& g0 = graph_series_[0][layer];
        shared_ptr<SociariumGraph> const& g1 = graph_series_[1][layer];
        for (SociariumGraph::node_property_iterator i=g0->node_property_begin(), end=g0->node_property_end(); i!=end; ++i)
          i->second->clear_upper_nodes();
        for (SociariumGraph::edge_property_iterator i=g0->edge_property_begin(), end=g0->edge_property_end(); i!=end; ++i)
          i->second->clear_upper_nodes();
        for (SociariumGraph::node_property_iterator i=g1->node_property_begin(), end=g1->node_property_end(); i!=end; ++i) {
          DynamicNodeProperty* dnp = i->second.get();
          for_each(dnp->lower_nbegin(), dnp->lower_nend(), bind(&DynamicNodeProperty::register_upper_element, std::tr1::placeholders::_1, dnp));
          for_each(dnp->lower_ebegin(), dnp->lower_eend(), bind(&DynamicEdgeProperty::register_upper_element, std::tr1::placeholders::_1, dnp));
        }

        for (SociariumGraph::node_property_iterator i=g0->node_property_begin(), end=g0->node_property_end(); i!=end; ++i)
          if (i->second->number_of_upper_nodes()==0) i->second->set_color_id(PredefinedColor::LIGHT_GRAY);
          else if (i->second->number_of_upper_nodes()==1) i->second->set_color_id((*i->second->upper_nbegin())->get_color_id());
          else i->second->set_color_id(PredefinedColor::WHITE);
        for (SociariumGraph::edge_property_iterator i=g0->edge_property_begin(), end=g0->edge_property_end(); i!=end; ++i)
          if (i->second->number_of_upper_nodes()==0) i->second->set_color_id(PredefinedColor::LIGHT_GRAY);
          else if (i->second->number_of_upper_nodes()==1) i->second->set_color_id((*i->second->upper_nbegin())->get_color_id());
          else i->second->set_color_id(PredefinedColor::WHITE);
      };

      // ----------------------------------------------------------------------------------------------------
      write_to_read_lock();
      // ----------------------------------------------------------------------------------------------------

      for (size_t layer=0; layer<number_of_layers(); ++layer) {
        shared_ptr<SociariumGraph> const& g = graph_series_[1][layer];
        for (SociariumGraph::node_property_iterator i=g->node_property_begin(); i!=g->node_property_end(); ++i)
          i->second->move_to_center_of_lower_nodes_position();
      }
    }
  };

  shared_ptr<SociariumGraphTimeSeries> SociariumGraphTimeSeries::create(void) {
    return shared_ptr<SociariumGraphTimeSeries>(new SociariumGraphTimeSeriesImpl());
  }
} // The end of the namespace "hashimoto_ut"

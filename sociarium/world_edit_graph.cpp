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

#include <vector>
#include <algorithm>
#include <boost/bind.hpp>
#ifdef _MSC_VER
#include <windows.h>
#endif
#include "resource.h"
#include "world.h"
#include "common.h"
#include "thread.h"
#include "flag_operation.h"
#include "sociarium_graph_time_series.h"
#include "message.h"
#include "draw.h"
#include "force_direction.h"
#include "../shared/msgbox.h"
#include "../shared/predefined_color.h"

namespace hashimoto_ut {

  using std::vector;
  using std::pair;
  using std::for_each;
  using std::transform;
  using std::back_inserter;
  using std::tr1::shared_ptr;
  using std::tr1::unordered_set;
  using std::tr1::unordered_map;

  namespace {

    // 不可視にする際は必ず選択フラグをオフ
    template <typename T>
    void hide(T const& dp) { dp->set_flag(dp->get_flag()&~(ElementFlag::VISIBLE|ElementFlag::MARKED|ElementFlag::HIGHLIGHT)); }

    // 可視にする際に選択フラグをオン
    template <typename T>
    void show(T const& dp) { dp->set_flag(dp->get_flag()|(ElementFlag::VISIBLE|ElementFlag::MARKED)); }

    struct HideMarkedElement {
      void operator()(pair<Node const* const, shared_ptr<DynamicNodeProperty> > const& p) const { if (is_marked(p.second)) hide(p.second); }
      void operator()(pair<Edge const* const, shared_ptr<DynamicEdgeProperty> > const& p) const { if (is_marked(p.second)) hide(p.second); }
      void operator()(pair<DynamicNodeProperty* const, size_t> const& p) const { if (is_marked(p.first)) hide(p.first); }
      void operator()(pair<DynamicEdgeProperty* const, size_t> const& p) const { if (is_marked(p.first)) hide(p.first); }
    };

    struct HideEdge {
      void operator()(Edge const* e, shared_ptr<SociariumGraph const> const& g) const { hide(g->property(e)); }
    };

    struct HideExtractMarkedElement {
      template <typename T>
      void operator()(T const& p, vector<shared_ptr<typename T::second_type::element_type> >& container) const {
        if (is_marked(p.second)) { hide(p.second); container.push_back(p.second); }
      }
      template <typename T>
      void operator()(T const& p, vector<shared_ptr<typename T::second_type::element_type::StaticProperty> >& container) const {
        if (is_marked(p.second)) { hide(p.second); container.push_back(p.second->get_static_property()); }
      }
      template <typename T>
      void operator()(T const& p, unordered_set<shared_ptr<typename T::second_type::element_type>, shared_ptr_hash<typename T::second_type::element_type> >& container) const {
        if (is_marked(p.second)) { hide(p.second); container.insert(p.second); }
      }
      template <typename T>
      void operator()(T const& p, unordered_set<shared_ptr<typename T::second_type::element_type::StaticProperty>, shared_ptr_hash<typename T::second_type::element_type::StaticProperty> >& container) const {
        if (is_marked(p.second)) { hide(p.second); container.insert(p.second->get_static_property()); }
      }
    };

    struct ShowElement {
      template <typename T>
      void operator()(T const& p) { if (is_hidden(p.second)) show(p.second); }
    };

    struct ExtractMarkedElement {
      template <typename T>
      void operator()(T const& p, vector<shared_ptr<typename T::second_type::element_type> >& container) const {
        if (is_marked(p.second)) container.push_back(p.second);
      }
      template <typename T>
      void operator()(T const& p, vector<shared_ptr<typename T::second_type::element_type::StaticProperty> >& container) const {
        if (is_marked(p.second)) container.push_back(p.second->get_static_property());
      }
      template <typename T>
      void operator()(T const& p, unordered_set<shared_ptr<typename T::second_type::element_type>, shared_ptr_hash<typename T::second_type::element_type> >& container) const {
        if (is_marked(p.second)) container.insert(p.second);
      }
      template <typename T>
      void operator()(T const& p, unordered_set<shared_ptr<typename T::second_type::element_type::StaticProperty>, shared_ptr_hash<typename T::second_type::element_type::StaticProperty> >& container) const {
        if (is_marked(p.second)) container.insert(p.second->get_static_property());
      }
    };

    bool another_thread_is_running(void) {
      if (sociarium_project_thread::get_current_graph_creation_thread()) {
        MsgBox(sociarium_project_common::get_window_handle(),
               sociarium_project_common::APPLICATION_TITLE, L"%s",
               sociarium_project_message::GRAPH_TIME_SERIES_IS_LOCKED);
        return true;
      } else {
        if (sociarium_project_thread::get_current_graph_layout_thread()) {
          MsgBox(sociarium_project_common::get_window_handle(),
                 sociarium_project_common::APPLICATION_TITLE, L"%s",
                 sociarium_project_message::ANOTHER_THREAD_IS_RUNNING);
          return true;
        }
        if (sociarium_project_thread::get_current_graph_statistics_thread()) {
          MsgBox(sociarium_project_common::get_window_handle(),
                 sociarium_project_common::APPLICATION_TITLE, L"%s",
                 sociarium_project_message::ANOTHER_THREAD_IS_RUNNING);
          return true;
        }
        if (sociarium_project_thread::get_current_community_detection_thread()) {
          MsgBox(sociarium_project_common::get_window_handle(),
                 sociarium_project_common::APPLICATION_TITLE, L"%s",
                 sociarium_project_message::ANOTHER_THREAD_IS_RUNNING);
          return true;
        }
      }
      return false;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    vector<Node const*> inner_community_nodes(shared_ptr<SociariumGraph const> const& g) {
      vector<Node const*> retval;
      for (SociariumGraph::node_property_iterator i=g->node_property_begin(); i!=g->node_property_end(); ++i)
        if (i->second->number_of_upper_nodes()>0) retval.push_back(i->first);
      return retval;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    vector<Node const*> outer_community_nodes(shared_ptr<SociariumGraph const> const& g) {
      vector<Node const*> retval;
      for (SociariumGraph::node_property_iterator i=g->node_property_begin(); i!=g->node_property_end(); ++i)
        if (i->second->number_of_upper_nodes()==0) retval.push_back(i->first);
      return retval;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    vector<Edge const*> inner_community_edges(shared_ptr<SociariumGraph const> const& g) {
      vector<Edge const*> retval;
      for (SociariumGraph::edge_property_iterator i=g->edge_property_begin(); i!=g->edge_property_end(); ++i)
        if (i->second->number_of_upper_nodes()>0) retval.push_back(i->first);
      return retval;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    vector<Edge const*> outer_community_edges(shared_ptr<SociariumGraph const> const& g) {
      vector<Edge const*> retval;
      for (SociariumGraph::edge_property_iterator i=g->edge_property_begin(); i!=g->edge_property_end(); ++i)
        if (i->second->number_of_upper_nodes()==0) retval.push_back(i->first);
      return retval;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // 選択されたノードとそれに接続するエッジを隠す
    void hide_marked_nodes(shared_ptr<SociariumGraph const> const& g) {
      vector<shared_ptr<DynamicNodeProperty> > hidden_nodes;
      for_each(g->node_property_begin(), g->node_property_end(), boost::bind<void>(HideExtractMarkedElement(), _1, boost::ref(hidden_nodes)));
      for (vector<shared_ptr<DynamicNodeProperty> >::const_iterator i=hidden_nodes.begin(); i!=hidden_nodes.end(); ++i)
        for_each((*i)->get_graph_element()->begin(), (*i)->get_graph_element()->end(), boost::bind<void>(HideEdge(), _1, g));
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // 選択されたノードの全レイヤー要素とそれに接続するエッジを隠す
    void hide_marked_nodes_all(shared_ptr<SociariumGraph const> const& g, shared_ptr<SociariumGraphTimeSeries> const& time_series) {
      vector<shared_ptr<StaticNodeProperty> > hidden_nodes;
      for_each(g->node_property_begin(), g->node_property_end(), boost::bind<void>(HideExtractMarkedElement(), _1, boost::ref(hidden_nodes)));
      for (vector<shared_ptr<StaticNodeProperty> >::const_iterator i=hidden_nodes.begin(); i!=hidden_nodes.end(); ++i)
        for (unordered_map<DynamicNodeProperty*, size_t>::const_iterator j=(*i)->dynamic_property_begin(); j!=(*i)->dynamic_property_end(); ++j) {
          DynamicNodeProperty* dnp = j->first;
          hide(dnp);
          for_each(dnp->get_graph_element()->begin(), dnp->get_graph_element()->end(), boost::bind<void>(HideEdge(), _1, time_series->get_graph(0, j->second)));
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // 選択されたノードの全レイヤー要素とそれに接続するエッジを隠す
    void hide_marked_nodes_all(shared_ptr<SociariumGraphTimeSeries> const& time_series) {
      typedef unordered_set<shared_ptr<StaticNodeProperty>, shared_ptr_hash<StaticNodeProperty> > HiddenNodes;
      HiddenNodes hidden_nodes;
      for (size_t layer=0; layer<time_series->number_of_layers(); ++layer) {
        shared_ptr<SociariumGraph const> const& g = time_series->get_graph(0, layer);
        for_each(g->node_property_begin(), g->node_property_end(), boost::bind<void>(HideExtractMarkedElement(), _1, boost::ref(hidden_nodes)));
      }
      for (HiddenNodes::const_iterator i=hidden_nodes.begin(); i!=hidden_nodes.end(); ++i)
        for (unordered_map<DynamicNodeProperty*, size_t>::const_iterator j=(*i)->dynamic_property_begin(); j!=(*i)->dynamic_property_end(); ++j) {
          DynamicNodeProperty* dnp = j->first;
          hide(dnp);
          for_each(dnp->get_graph_element()->begin(), dnp->get_graph_element()->end(), boost::bind<void>(HideEdge(), _1, time_series->get_graph(0, j->second)));
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // 選択されたエッジの全レイヤー要素を隠す
    void hide_marked_edges_all(shared_ptr<SociariumGraph const> const& g) {
      vector<shared_ptr<StaticEdgeProperty> > hidden_edges;
      for_each(g->edge_property_begin(), g->edge_property_end(), boost::bind<void>(HideExtractMarkedElement(), _1, boost::ref(hidden_edges)));
      for (vector<shared_ptr<StaticEdgeProperty> >::const_iterator i=hidden_edges.begin(); i!=hidden_edges.end(); ++i)
        for_each((*i)->dynamic_property_begin(), (*i)->dynamic_property_end(), HideMarkedElement());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // 選択されたエッジの全レイヤー要素を隠す
    void hide_marked_edges_all(shared_ptr<SociariumGraphTimeSeries> const& time_series) {
      typedef unordered_set<shared_ptr<StaticEdgeProperty>, shared_ptr_hash<StaticEdgeProperty> > HiddenEdges;
      HiddenEdges hidden_edges;
      for (size_t layer=0; layer<time_series->number_of_layers(); ++layer) {
        shared_ptr<SociariumGraph const> const& g = time_series->get_graph(0, layer);
        for_each(g->edge_property_begin(), g->edge_property_end(), boost::bind<void>(HideExtractMarkedElement(), _1, boost::ref(hidden_edges)));
        for (HiddenEdges::const_iterator i=hidden_edges.begin(); i!=hidden_edges.end(); ++i)
          for_each((*i)->dynamic_property_begin(), (*i)->dynamic_property_end(), HideMarkedElement());
      }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // 選択されたノードとそれに接続するエッジを削除
    void remove_marked_nodes(shared_ptr<SociariumGraph> const& g) {
      vector<shared_ptr<DynamicNodeProperty> > removed_nodes;
      for_each(g->node_property_begin(), g->node_property_end(), boost::bind<void>(ExtractMarkedElement(), _1, boost::ref(removed_nodes)));
      for (vector<shared_ptr<DynamicNodeProperty> >::const_iterator i=removed_nodes.begin(); i!=removed_nodes.end(); ++i)
        g->remove_node((*i)->get_graph_element());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // 選択されたノードの全レイヤー要素とそれに接続するエッジを削除
    void remove_marked_nodes_all(shared_ptr<SociariumGraph const> const& g_current, shared_ptr<SociariumGraphTimeSeries> const& time_series) {
      vector<shared_ptr<StaticNodeProperty> > removed_static_nodes;
      for_each(g_current->node_property_begin(), g_current->node_property_end(), boost::bind<void>(ExtractMarkedElement(), _1, boost::ref(removed_static_nodes)));
      for (vector<shared_ptr<StaticNodeProperty> >::const_iterator i=removed_static_nodes.begin(); i!=removed_static_nodes.end(); ++i)
        time_series->remove_node(*i);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // 選択されたノードの全レイヤー要素とそれに接続するエッジを削除
    void remove_marked_nodes_all(shared_ptr<SociariumGraphTimeSeries> const& time_series) {
      typedef unordered_set<shared_ptr<StaticNodeProperty>, shared_ptr_hash<StaticNodeProperty> > RemovedNodes;
      RemovedNodes removed_nodes;
      for (size_t layer=0; layer<time_series->number_of_layers(); ++layer) {
        shared_ptr<SociariumGraph const> const& g = time_series->get_graph(0, layer);
        for_each(g->node_property_begin(), g->node_property_end(), boost::bind<void>(ExtractMarkedElement(), _1, boost::ref(removed_nodes)));
      }
      for (RemovedNodes::const_iterator i=removed_nodes.begin(); i!=removed_nodes.end(); ++i)
        time_series->remove_node(*i);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // 選択されたエッジを削除
    void remove_marked_edges(shared_ptr<SociariumGraph> const& g) {
      vector<shared_ptr<DynamicEdgeProperty> > removed_edges;
      for_each(g->edge_property_begin(), g->edge_property_end(), boost::bind<void>(ExtractMarkedElement(), _1, boost::ref(removed_edges)));
      for (vector<shared_ptr<DynamicEdgeProperty> >::const_iterator i=removed_edges.begin(); i!=removed_edges.end(); ++i)
        g->remove_edge((*i)->get_graph_element());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // 選択されたエッジの全レイヤー要素を削除
    void remove_marked_edges_all(shared_ptr<SociariumGraph const> const& g_current, shared_ptr<SociariumGraphTimeSeries> const& time_series) {
      vector<shared_ptr<StaticEdgeProperty> > removed_edges;
      for_each(g_current->edge_property_begin(), g_current->edge_property_end(), boost::bind<void>(ExtractMarkedElement(), _1, boost::ref(removed_edges)));
      for (vector<shared_ptr<StaticEdgeProperty> >::const_iterator i=removed_edges.begin(); i!=removed_edges.end(); ++i)
        time_series->remove_edge(*i);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // 選択されたエッジの全レイヤー要素を削除
    void remove_marked_edges_all(shared_ptr<SociariumGraphTimeSeries> const& time_series) {
      typedef unordered_set<shared_ptr<StaticEdgeProperty>, shared_ptr_hash<StaticEdgeProperty> > RemovedEdges;
      RemovedEdges removed_edges;
      for (size_t layer=0; layer<time_series->number_of_layers(); ++layer) {
        shared_ptr<SociariumGraph const> const& g = time_series->get_graph(0, layer);
        for_each(g->edge_property_begin(), g->edge_property_end(), boost::bind<void>(ExtractMarkedElement(), _1, boost::ref(removed_edges)));
      }
      for (RemovedEdges::const_iterator i=removed_edges.begin(); i!=removed_edges.end(); ++i)
        time_series->remove_edge(*i);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // メンバーのいなくなったコミュニティを削除＋メンバーの削除に伴うサイズ再設定
    void adjust_community(shared_ptr<SociariumGraph> const& g) {
      vector<shared_ptr<DynamicNodeProperty> > removed_communities;
      for (SociariumGraph::node_property_iterator i=g->node_property_begin(); i!=g->node_property_end(); ++i) {
        size_t const number_of_nodes = i->second->number_of_lower_nodes();
        if (number_of_nodes==0) removed_communities.push_back(i->second);
        i->second->set_size(sociarium_project_draw::get_default_node_size()*(float)sqrt(double(number_of_nodes)));
      }

      vector<DynamicNodeProperty*> member_nodes;
      vector<DynamicEdgeProperty*> member_edges;
      for (vector<shared_ptr<DynamicNodeProperty> >::const_iterator i=removed_communities.begin(); i!=removed_communities.end(); ++i) {
        member_nodes.insert(member_nodes.end(), (*i)->lower_nbegin(), (*i)->lower_nend());
        member_edges.insert(member_edges.end(), (*i)->lower_ebegin(), (*i)->lower_eend());
        g->remove_node((*i)->get_graph_element());
      }

      for (vector<DynamicNodeProperty*>::const_iterator i=member_nodes.begin(); i!=member_nodes.end(); ++i) {
        if ((*i)->number_of_upper_nodes()==0) (*i)->set_color_id(PredefinedColor::LIGHT_GRAY);
        else if ((*i)->number_of_upper_nodes()==1) (*i)->set_color_id((*(*i)->upper_nbegin())->get_color_id());
        else (*i)->set_color_id(PredefinedColor::WHITE);
      }

      for (vector<DynamicEdgeProperty*>::const_iterator i=member_edges.begin(); i!=member_edges.end(); ++i) {
        if ((*i)->number_of_upper_nodes()==0) (*i)->set_color_id(PredefinedColor::LIGHT_GRAY);
        else if ((*i)->number_of_upper_nodes()==1) (*i)->set_color_id((*(*i)->upper_nbegin())->get_color_id());
        else (*i)->set_color_id(PredefinedColor::WHITE);
      }
    }

  } // The end of the anonymous namespace


  ////////////////////////////////////////////////////////////////////////////////////////////////////
  void World::mark_elements(int menu) const {

    // --------------------------------------------------------------------------------
    time_series_->read_lock();
    // --------------------------------------------------------------------------------

    shared_ptr<SociariumGraph const> const& g0 = time_series_->get_graph(0, time_series_->index_of_current_layer());
    shared_ptr<SociariumGraph const> const& g1 = time_series_->get_graph(1, time_series_->index_of_current_layer());

    if (menu==IDM_EDIT_FOR_CURRENT_SELECT_NODES)
      for_each(g0->node_property_begin(), g0->node_property_end(), boost::bind<void>(ActivateFlag(), _1, ElementFlag::MARKED));
    else if (menu==IDM_EDIT_FOR_CURRENT_SELECT_EDGES)
      for_each(g0->edge_property_begin(), g0->edge_property_end(), boost::bind<void>(ActivateFlag(), _1, ElementFlag::MARKED));
    else if (menu==IDM_EDIT_FOR_CURRENT_SELECT_COMMUNITIES)
      for_each(g1->node_property_begin(), g1->node_property_end(), boost::bind<void>(ActivateFlag(), _1, ElementFlag::MARKED));
    else if (menu==IDM_EDIT_FOR_CURRENT_SELECT_COMMUNITY_EDGES)
      for_each(g1->edge_property_begin(), g1->edge_property_end(), boost::bind<void>(ActivateFlag(), _1, ElementFlag::MARKED));
    else if (menu==IDM_EDIT_FOR_CURRENT_SELECT_ALL) {
      for_each(g0->node_property_begin(), g0->node_property_end(), boost::bind<void>(ActivateFlag(), _1, ElementFlag::MARKED));
      for_each(g0->edge_property_begin(), g0->edge_property_end(), boost::bind<void>(ActivateFlag(), _1, ElementFlag::MARKED));
      for_each(g1->node_property_begin(), g1->node_property_end(), boost::bind<void>(ActivateFlag(), _1, ElementFlag::MARKED));
      for_each(g1->edge_property_begin(), g1->edge_property_end(), boost::bind<void>(ActivateFlag(), _1, ElementFlag::MARKED));
    }

    else if (menu==IDM_EDIT_FOR_CURRENT_SELECT_INNER_COMMUNITY_ELEMENTS) {
      vector<Node const*> nodes = inner_community_nodes(g0);
      vector<Edge const*> edges = inner_community_edges(g0);
      for_each(nodes.begin(), nodes.end(), boost::bind<void>(ActivateFlag(), _1, ElementFlag::MARKED, g0));
      for_each(edges.begin(), edges.end(), boost::bind<void>(ActivateFlag(), _1, ElementFlag::MARKED, g0));
    }
    else if (menu==IDM_EDIT_FOR_CURRENT_SELECT_INNER_COMMUNITY_NODES) {
      vector<Node const*> nodes = inner_community_nodes(g0);
      for_each(nodes.begin(), nodes.end(), boost::bind<void>(ActivateFlag(), _1, ElementFlag::MARKED, g0));
    }
    else if (menu==IDM_EDIT_FOR_CURRENT_SELECT_INNER_COMMUNITY_EDGES) {
      vector<Edge const*> edges = inner_community_edges(g0);
      for_each(edges.begin(), edges.end(), boost::bind<void>(ActivateFlag(), _1, ElementFlag::MARKED, g0));
    }
    else if (menu==IDM_EDIT_FOR_CURRENT_SELECT_OUTER_COMMUNITY_ELEMENTS) {
      vector<Node const*> nodes = outer_community_nodes(g0);
      vector<Edge const*> edges = outer_community_edges(g0);
      for_each(nodes.begin(), nodes.end(), boost::bind<void>(ActivateFlag(), _1, ElementFlag::MARKED, g0));
      for_each(edges.begin(), edges.end(), boost::bind<void>(ActivateFlag(), _1, ElementFlag::MARKED, g0));
    }
    else if (menu==IDM_EDIT_FOR_CURRENT_SELECT_OUTER_COMMUNITY_NODES) {
      vector<Node const*> nodes = outer_community_nodes(g0);
      for_each(nodes.begin(), nodes.end(), boost::bind<void>(ActivateFlag(), _1, ElementFlag::MARKED, g0));
    }
    else if (menu==IDM_EDIT_FOR_CURRENT_SELECT_OUTER_COMMUNITY_EDGES) {
      vector<Edge const*> edges = outer_community_edges(g0);
      for_each(edges.begin(), edges.end(), boost::bind<void>(ActivateFlag(), _1, ElementFlag::MARKED, g0));
    }

    else if (menu==IDM_EDIT_FOR_ALL_SELECT_NODES)
      for (size_t layer=0; layer<time_series_->number_of_layers(); ++layer) {
        shared_ptr<SociariumGraph const> const& g = time_series_->get_graph(0, layer);
        for_each(g->node_property_begin(), g->node_property_end(), boost::bind<void>(ActivateFlag(), _1, ElementFlag::MARKED));
      }
    else if (menu==IDM_EDIT_FOR_ALL_SELECT_EDGES)
      for (size_t layer=0; layer<time_series_->number_of_layers(); ++layer) {
        shared_ptr<SociariumGraph const> const& g = time_series_->get_graph(0, layer);
        for_each(g->edge_property_begin(), g->edge_property_end(), boost::bind<void>(ActivateFlag(), _1, ElementFlag::MARKED));
      }
    else if (menu==IDM_EDIT_FOR_ALL_SELECT_COMMUNITIES)
      for (size_t layer=0; layer<time_series_->number_of_layers(); ++layer) {
        shared_ptr<SociariumGraph const> const& g = time_series_->get_graph(1, layer);
        for_each(g->node_property_begin(), g->node_property_end(), boost::bind<void>(ActivateFlag(), _1, ElementFlag::MARKED));
      }
    else if (menu==IDM_EDIT_FOR_ALL_SELECT_COMMUNITY_EDGES)
      for (size_t layer=0; layer<time_series_->number_of_layers(); ++layer) {
        shared_ptr<SociariumGraph const> const& g = time_series_->get_graph(1, layer);
        for_each(g->edge_property_begin(), g->edge_property_end(), boost::bind<void>(ActivateFlag(), _1, ElementFlag::MARKED));
      }
    else if (menu==IDM_EDIT_FOR_ALL_SELECT_ALL)
      for (size_t layer=0; layer<time_series_->number_of_layers(); ++layer) {
        shared_ptr<SociariumGraph const> const& g0 = time_series_->get_graph(0, layer);
        shared_ptr<SociariumGraph const> const& g1 = time_series_->get_graph(1, layer);
        for_each(g0->node_property_begin(), g0->node_property_end(), boost::bind<void>(ActivateFlag(), _1, ElementFlag::MARKED));
        for_each(g0->edge_property_begin(), g0->edge_property_end(), boost::bind<void>(ActivateFlag(), _1, ElementFlag::MARKED));
        for_each(g1->node_property_begin(), g1->node_property_end(), boost::bind<void>(ActivateFlag(), _1, ElementFlag::MARKED));
        for_each(g1->edge_property_begin(), g1->edge_property_end(), boost::bind<void>(ActivateFlag(), _1, ElementFlag::MARKED));
      }

    else if (menu==IDM_EDIT_FOR_ALL_SELECT_INNER_COMMUNITY_ELEMENTS) {
      for (size_t layer=0; layer<time_series_->number_of_layers(); ++layer) {
        shared_ptr<SociariumGraph const> const& g = time_series_->get_graph(0, layer);
        vector<Node const*> nodes = inner_community_nodes(g);
        vector<Edge const*> edges = inner_community_edges(g);
        for_each(nodes.begin(), nodes.end(), boost::bind<void>(ActivateFlag(), _1, ElementFlag::MARKED, g));
        for_each(edges.begin(), edges.end(), boost::bind<void>(ActivateFlag(), _1, ElementFlag::MARKED, g));
      }
    }
    else if (menu==IDM_EDIT_FOR_ALL_SELECT_INNER_COMMUNITY_NODES)
      for (size_t layer=0; layer<time_series_->number_of_layers(); ++layer) {
        shared_ptr<SociariumGraph const> const& g = time_series_->get_graph(0, layer);
        vector<Node const*> nodes = inner_community_nodes(g);
        for_each(nodes.begin(), nodes.end(), boost::bind<void>(ActivateFlag(), _1, ElementFlag::MARKED, g));
      }
    else if (menu==IDM_EDIT_FOR_ALL_SELECT_INNER_COMMUNITY_EDGES)
      for (size_t layer=0; layer<time_series_->number_of_layers(); ++layer) {
        shared_ptr<SociariumGraph const> const& g = time_series_->get_graph(0, layer);
        vector<Edge const*> edges = inner_community_edges(g);
        for_each(edges.begin(), edges.end(), boost::bind<void>(ActivateFlag(), _1, ElementFlag::MARKED, g));
      }
    else if (menu==IDM_EDIT_FOR_ALL_SELECT_OUTER_COMMUNITY_ELEMENTS) {
      for (size_t layer=0; layer<time_series_->number_of_layers(); ++layer) {
        shared_ptr<SociariumGraph const> const& g = time_series_->get_graph(0, layer);
        vector<Node const*> nodes = outer_community_nodes(g);
        vector<Edge const*> edges = outer_community_edges(g);
        for_each(nodes.begin(), nodes.end(), boost::bind<void>(ActivateFlag(), _1, ElementFlag::MARKED, g));
        for_each(edges.begin(), edges.end(), boost::bind<void>(ActivateFlag(), _1, ElementFlag::MARKED, g));
      }
    }
    else if (menu==IDM_EDIT_FOR_ALL_SELECT_OUTER_COMMUNITY_NODES)
      for (size_t layer=0; layer<time_series_->number_of_layers(); ++layer) {
        shared_ptr<SociariumGraph const> const& g = time_series_->get_graph(0, layer);
        vector<Node const*> nodes = outer_community_nodes(g);
        for_each(nodes.begin(), nodes.end(), boost::bind<void>(ActivateFlag(), _1, ElementFlag::MARKED, g));
      }
    else if (menu==IDM_EDIT_FOR_ALL_SELECT_OUTER_COMMUNITY_EDGES)
      for (size_t layer=0; layer<time_series_->number_of_layers(); ++layer) {
        shared_ptr<SociariumGraph const> const& g = time_series_->get_graph(0, layer);
        vector<Edge const*> edges = outer_community_edges(g);
        for_each(edges.begin(), edges.end(), boost::bind<void>(ActivateFlag(), _1, ElementFlag::MARKED, g));
      }
    else assert(0 && "never reach");

    // --------------------------------------------------------------------------------
    time_series_->read_unlock();
    // --------------------------------------------------------------------------------
  }


  ////////////////////////////////////////////////////////////////////////////////////////////////////
  void World::inverse_mark(int menu) const {

    // --------------------------------------------------------------------------------
    time_series_->read_lock();
    // --------------------------------------------------------------------------------

    if (menu==IDM_EDIT_FOR_CURRENT_SELECT_INVERT) {
      shared_ptr<SociariumGraph const> const& g0 = time_series_->get_graph(0, time_series_->index_of_current_layer());
      shared_ptr<SociariumGraph const> const& g1 = time_series_->get_graph(1, time_series_->index_of_current_layer());
      for_each(g0->node_property_begin(), g0->node_property_end(), boost::bind<void>(ToggleFlag(), _1, ElementFlag::MARKED));
      for_each(g0->edge_property_begin(), g0->edge_property_end(), boost::bind<void>(ToggleFlag(), _1, ElementFlag::MARKED));
      for_each(g1->node_property_begin(), g1->node_property_end(), boost::bind<void>(ToggleFlag(), _1, ElementFlag::MARKED));
      for_each(g1->edge_property_begin(), g1->edge_property_end(), boost::bind<void>(ToggleFlag(), _1, ElementFlag::MARKED));
    }
    else if (menu==IDM_EDIT_FOR_ALL_SELECT_INVERT) {
      for (size_t layer=0; layer<time_series_->number_of_layers(); ++layer) {
        shared_ptr<SociariumGraph const> const& g0 = time_series_->get_graph(0, layer);
        shared_ptr<SociariumGraph const> const& g1 = time_series_->get_graph(1, layer);
        for_each(g0->node_property_begin(), g0->node_property_end(), boost::bind<void>(ToggleFlag(), _1, ElementFlag::MARKED));
        for_each(g0->edge_property_begin(), g0->edge_property_end(), boost::bind<void>(ToggleFlag(), _1, ElementFlag::MARKED));
        for_each(g1->node_property_begin(), g1->node_property_end(), boost::bind<void>(ToggleFlag(), _1, ElementFlag::MARKED));
        for_each(g1->edge_property_begin(), g1->edge_property_end(), boost::bind<void>(ToggleFlag(), _1, ElementFlag::MARKED));
      }
    }
    else assert(0 && "never reach");

    // --------------------------------------------------------------------------------
    time_series_->read_unlock();
    // --------------------------------------------------------------------------------
  }


  ////////////////////////////////////////////////////////////////////////////////////////////////////
  void World::hide_marked_element(int menu) const {

    // --------------------------------------------------------------------------------
    time_series_->read_lock();
    // --------------------------------------------------------------------------------

    if (menu==IDM_EDIT_FOR_CURRENT_HIDE_MARKED_NODES) {
      shared_ptr<SociariumGraph const> const& g = time_series_->get_graph(0, time_series_->index_of_current_layer());
      hide_marked_nodes(g);
    }
    else if (menu==IDM_EDIT_FOR_CURRENT_HIDE_MARKED_EDGES) {
      shared_ptr<SociariumGraph const> const& g = time_series_->get_graph(0, time_series_->index_of_current_layer());
      for_each(g->edge_property_begin(), g->edge_property_end(), HideMarkedElement());
    }
    else if (menu==IDM_EDIT_FOR_CURRENT_HIDE_MARKED_ELEMENTS) {
      shared_ptr<SociariumGraph const> const& g = time_series_->get_graph(0, time_series_->index_of_current_layer());
      hide_marked_nodes(g);
      for_each(g->edge_property_begin(), g->edge_property_end(), HideMarkedElement());
    }
    else if (menu==IDM_EDIT_FOR_ALL_HIDE_MARKED_NODES)
      for (size_t layer=0; layer<time_series_->number_of_layers(); ++layer) {
        shared_ptr<SociariumGraph const> const& g = time_series_->get_graph(0, layer);
        hide_marked_nodes(g);
      }
    else if (menu==IDM_EDIT_FOR_ALL_HIDE_MARKED_EDGES)
      for (size_t layer=0; layer<time_series_->number_of_layers(); ++layer) {
        shared_ptr<SociariumGraph const> const& g = time_series_->get_graph(0, layer);
        for_each(g->edge_property_begin(), g->edge_property_end(), HideMarkedElement());
      }
    else if (menu==IDM_EDIT_FOR_ALL_HIDE_MARKED_ELEMENTS)
      for (size_t layer=0; layer<time_series_->number_of_layers(); ++layer) {
        shared_ptr<SociariumGraph const> const& g = time_series_->get_graph(0, layer);
        hide_marked_nodes(g);
        for_each(g->edge_property_begin(), g->edge_property_end(), HideMarkedElement());
      }
    else if (menu==IDM_EDIT_FOR_CURRENT_HIDE_MARKED_NODES_ALL) {
      shared_ptr<SociariumGraph const> const& g = time_series_->get_graph(0, time_series_->index_of_current_layer());
      hide_marked_nodes_all(g, time_series_);
    }
    else if (menu==IDM_EDIT_FOR_CURRENT_HIDE_MARKED_EDGES_ALL) {
      shared_ptr<SociariumGraph const> const& g = time_series_->get_graph(0, time_series_->index_of_current_layer());
      hide_marked_edges_all(g);
    }
    else if (menu==IDM_EDIT_FOR_CURRENT_HIDE_MARKED_ELEMENTS_ALL) {
      shared_ptr<SociariumGraph const> const& g = time_series_->get_graph(0, time_series_->index_of_current_layer());
      hide_marked_nodes_all(g, time_series_);
      hide_marked_edges_all(g);
    }
    else if (menu==IDM_EDIT_FOR_ALL_HIDE_MARKED_NODES_ALL) {
      hide_marked_nodes_all(time_series_);
    }
    else if (menu==IDM_EDIT_FOR_ALL_HIDE_MARKED_EDGES_ALL) {
      hide_marked_edges_all(time_series_);
    }
    else if (menu==IDM_EDIT_FOR_ALL_HIDE_MARKED_ELEMENTS_ALL) {
      hide_marked_nodes_all(time_series_);
      hide_marked_edges_all(time_series_);
    }
    else assert(0 && "never reach");

    sociarium_project_force_direction::fd_element_should_be_updated(true);

    // --------------------------------------------------------------------------------
    time_series_->read_unlock();
    // --------------------------------------------------------------------------------
  }


  ////////////////////////////////////////////////////////////////////////////////////////////////////
  void World::show_elements(int menu) const {

    // --------------------------------------------------------------------------------
    time_series_->read_lock();
    // --------------------------------------------------------------------------------

    shared_ptr<SociariumGraph const> const& g = time_series_->get_graph(0, time_series_->index_of_current_layer());

    if (menu==IDM_EDIT_FOR_CURRENT_SHOW_NODES)
      for_each(g->node_property_begin(), g->node_property_end(), ShowElement());
    else if (menu==IDM_EDIT_FOR_CURRENT_SHOW_EDGES)
      for_each(g->edge_property_begin(), g->edge_property_end(), ShowElement());
    else if (menu==IDM_EDIT_FOR_CURRENT_SHOW_ELEMENTS) {
      for_each(g->node_property_begin(), g->node_property_end(), ShowElement());
      for_each(g->edge_property_begin(), g->edge_property_end(), ShowElement());
    }
    else if (menu==IDM_EDIT_FOR_ALL_SHOW_NODES)
      for (size_t layer=0; layer<time_series_->number_of_layers(); ++layer) {
        shared_ptr<SociariumGraph const> const& g = time_series_->get_graph(0, layer);
        for_each(g->node_property_begin(), g->node_property_end(), ShowElement());
      }
    else if (menu==IDM_EDIT_FOR_ALL_SHOW_EDGES)
      for (size_t layer=0; layer<time_series_->number_of_layers(); ++layer) {
        shared_ptr<SociariumGraph const> const& g = time_series_->get_graph(0, layer);
        for_each(g->edge_property_begin(), g->edge_property_end(), ShowElement());
      }
    else if (menu==IDM_EDIT_FOR_ALL_SHOW_ELEMENTS)
      for (size_t layer=0; layer<time_series_->number_of_layers(); ++layer) {
        shared_ptr<SociariumGraph const> const& g = time_series_->get_graph(0, layer);
        for_each(g->node_property_begin(), g->node_property_end(), ShowElement());
        for_each(g->edge_property_begin(), g->edge_property_end(), ShowElement());
      }
    else assert(0 && "never reach");

    sociarium_project_force_direction::fd_element_should_be_updated(true);

    // --------------------------------------------------------------------------------
    time_series_->read_unlock();
    // --------------------------------------------------------------------------------
  }


  ////////////////////////////////////////////////////////////////////////////////////////////////////
  void World::remove_marked_elements(int menu) const {
    if (another_thread_is_running()
#ifdef __APPLE__
#elif _MSC_VER
        || MessageBox(NULL, sociarium_project_message::DO_YOU_REALLY_REMOVE_ELEMENTS,
                      sociarium_project_common::APPLICATION_TITLE, MB_OKCANCEL|MB_ICONQUESTION|MB_DEFBUTTON1)==IDCANCEL
#endif
      )
      return;

    // --------------------------------------------------------------------------------
    time_series_->write_lock();
    // --------------------------------------------------------------------------------

    if (menu==IDM_EDIT_FOR_CURRENT_REMOVE_MARKED_NODES) {
      shared_ptr<SociariumGraph> const& g0 = time_series_->get_graph(0, time_series_->index_of_current_layer());
      shared_ptr<SociariumGraph> const& g1 = time_series_->get_graph(1, time_series_->index_of_current_layer());
      remove_marked_nodes(g0);
      adjust_community(g1);
    }
    else if (menu==IDM_EDIT_FOR_CURRENT_REMOVE_MARKED_EDGES) {
      shared_ptr<SociariumGraph> const& g0 = time_series_->get_graph(0, time_series_->index_of_current_layer());
      remove_marked_edges(g0);
    }
    else if (menu==IDM_EDIT_FOR_CURRENT_REMOVE_MARKED_ELEMENTS) {
      shared_ptr<SociariumGraph> const& g0 = time_series_->get_graph(0, time_series_->index_of_current_layer());
      shared_ptr<SociariumGraph> const& g1 = time_series_->get_graph(1, time_series_->index_of_current_layer());
      remove_marked_nodes(g0);
      remove_marked_edges(g0);
      adjust_community(g1);
    }
    else if (menu==IDM_EDIT_FOR_ALL_REMOVE_MARKED_NODES)
      for (size_t layer=0; layer<time_series_->number_of_layers(); ++layer) {
        shared_ptr<SociariumGraph> const& g0 = time_series_->get_graph(0, layer);
        shared_ptr<SociariumGraph> const& g1 = time_series_->get_graph(1, layer);
        remove_marked_nodes(g0);
        adjust_community(g1);
      }
    else if (menu==IDM_EDIT_FOR_ALL_REMOVE_MARKED_EDGES)
      for (size_t layer=0; layer<time_series_->number_of_layers(); ++layer) {
        shared_ptr<SociariumGraph> const& g0 = time_series_->get_graph(0, layer);
        remove_marked_edges(g0);
      }
    else if (menu==IDM_EDIT_FOR_ALL_REMOVE_MARKED_ELEMENTS)
      for (size_t layer=0; layer<time_series_->number_of_layers(); ++layer) {
        shared_ptr<SociariumGraph> const& g0 = time_series_->get_graph(0, layer);
        shared_ptr<SociariumGraph> const& g1 = time_series_->get_graph(1, layer);
        remove_marked_nodes(g0);
        remove_marked_edges(g0);
        adjust_community(g1);
      }
    else if (menu==IDM_EDIT_FOR_CURRENT_REMOVE_MARKED_NODES_ALL) {
      shared_ptr<SociariumGraph const> const& g0 = time_series_->get_graph(0, time_series_->index_of_current_layer());
      shared_ptr<SociariumGraph> const& g1 = time_series_->get_graph(1, time_series_->index_of_current_layer());
      remove_marked_nodes_all(g0, time_series_);
      adjust_community(g1);
    }
    else if (menu==IDM_EDIT_FOR_CURRENT_REMOVE_MARKED_EDGES_ALL) {
      shared_ptr<SociariumGraph const> const& g = time_series_->get_graph(0, time_series_->index_of_current_layer());
      remove_marked_edges_all(g, time_series_);
    }
    else if (menu==IDM_EDIT_FOR_CURRENT_REMOVE_MARKED_ELEMENTS_ALL) {
      shared_ptr<SociariumGraph const> const& g0 = time_series_->get_graph(0, time_series_->index_of_current_layer());
      shared_ptr<SociariumGraph> const& g1 = time_series_->get_graph(1, time_series_->index_of_current_layer());
      remove_marked_nodes_all(g0, time_series_);
      remove_marked_edges_all(g0, time_series_);
      adjust_community(g1);
    }
    else if (menu==IDM_EDIT_FOR_ALL_REMOVE_MARKED_NODES_ALL) {
      remove_marked_nodes_all(time_series_);
      for (size_t layer=0; layer<time_series_->number_of_layers(); ++layer) {
        shared_ptr<SociariumGraph> const& g1 = time_series_->get_graph(1, layer);
        adjust_community(g1);
      }
    }
    else if (menu==IDM_EDIT_FOR_ALL_REMOVE_MARKED_EDGES_ALL)
      remove_marked_edges_all(time_series_);
    else if (menu==IDM_EDIT_FOR_ALL_REMOVE_MARKED_ELEMENTS_ALL) {
      remove_marked_nodes_all(time_series_);
      remove_marked_edges_all(time_series_);
      for (size_t layer=0; layer<time_series_->number_of_layers(); ++layer) {
        shared_ptr<SociariumGraph> const& g1 = time_series_->get_graph(1, layer);
        adjust_community(g1);
      }
    }
    else assert(0 && "never reach");

    time_series_->remove_empty_static_property(0);
    time_series_->remove_empty_static_property(1);

    sociarium_project_force_direction::fd_element_should_be_updated(true);

    // --------------------------------------------------------------------------------
    time_series_->write_unlock();
    // --------------------------------------------------------------------------------
  }

} // The end of the namespace "hashimoto_ut"

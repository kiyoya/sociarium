// s.o.c.i.a.r.i.u.m: world_impl_edit_graph.cpp
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

#include <vector>
#include <algorithm>
#include <boost/bind.hpp>
#include <windows.h>
#include "resource.h"
#include "color.h"
#include "common.h"
#include "community_transition_diagram.h"
#include "flag_operation.h"
#include "menu_and_message.h"
#include "sociarium_graph_time_series.h"
#include "thread.h"
#include "world_impl.h"
#include "thread/force_direction.h"
#include "../shared/msgbox.h"

namespace hashimoto_ut {

  using std::vector;
  using std::pair;
  using std::for_each;
  using std::tr1::shared_ptr;
  using std::tr1::unordered_set;
  using std::tr1::unordered_map;

  using namespace sociarium_project_common;
  using namespace sociarium_project_color;
  using namespace sociarium_project_thread;
  using namespace sociarium_project_menu_and_message;

  namespace {

    typedef SociariumGraph::node_property_iterator node_property_iterator;
    typedef SociariumGraph::edge_property_iterator edge_property_iterator;

    typedef StaticNodeProperty::DynamicPropertyMap DynamicNodePropertyMap;
    typedef StaticEdgeProperty::DynamicPropertyMap DynamicEdgePropertyMap;


    ////////////////////////////////////////////////////////////////////////////////
    unsigned int const hide_mask
      = ~(ElementFlag::ACTIVE|ElementFlag::MARKED|ElementFlag::HIGHLIGHT);

    unsigned int const show_mask
      = ElementFlag::ACTIVE|ElementFlag::MARKED;


    ////////////////////////////////////////////////////////////////////////////////
    // Never mark invisible elements.
    template <typename T>
    void hide(T& p) {
      p.set_flag(p.get_flag()&hide_mask);
    }

    template <typename T>
    void show(T& p) {
      p.set_flag(p.get_flag()|show_mask);
    }


    ////////////////////////////////////////////////////////////////////////////////
    struct HideMarkedElement {
      template <typename T, typename U>
      void operator()(pair<T, U>& pp) const {
        if (is_marked(pp.second))
          hide(pp.second);
      }

      template <typename T>
      void operator()(pair<T, size_t> const& pp) const {
        if (is_marked(*pp.first))
          hide(*pp.first);
      }
    };


    ////////////////////////////////////////////////////////////////////////////////
    struct HideEdge {
      void operator()(Edge const* e, shared_ptr<SociariumGraph const> g) const {
        hide(g->property(e));
      }
    };


    ////////////////////////////////////////////////////////////////////////////////
    struct ShowElement {
      template <typename T>
      void operator()(T& pp) {
        if (is_inactive(pp.second))
          show(pp.second);
      }
    };


    ////////////////////////////////////////////////////////////////////////////////
    struct ExtractMarkedElement {

      template <typename T>
      void operator()(
        T const& pp,
        vector<typename T::second_type*>& container) const {
          if (is_marked(pp.second))
            container.push_back(const_cast<typename T::second_type*>(&pp.second));
        }

      template <typename T>
      void operator()(
        T const& pp,
        vector<typename T::second_type::StaticProperty*>& container) const {
          if (is_marked(pp.second))
            container.push_back(pp.second.get_static_property());
        }

      template <typename T>
      void operator()(
        T const& pp,
        unordered_set<typename T::second_type::StaticProperty*>& container) const {
          if (is_marked(pp.second))
            container.insert(pp.second.get_static_property());
        }

    };


    ////////////////////////////////////////////////////////////////////////////////
    void update_community_size(DynamicNodeProperty& dnp) {
      dnp.set_size(sqrtf(float(dnp.number_of_lower_nodes())));
    }


    ////////////////////////////////////////////////////////////////////////////////
    vector<DynamicNodeProperty*>
      get_nodes_inside_communities(shared_ptr<SociariumGraph const> g0) {

        vector<DynamicNodeProperty*> retval;
        node_property_iterator first = g0->node_property_begin();
        node_property_iterator last  = g0->node_property_end();

        for (; first!=last; ++first)
          if (first->second.number_of_upper_nodes()>0)
            retval.push_back(&first->second);

        return retval;
      }


    ////////////////////////////////////////////////////////////////////////////////
    vector<DynamicEdgeProperty*>
      get_edges_inside_communities(shared_ptr<SociariumGraph const> g0) {

        vector<DynamicEdgeProperty*> retval;
        edge_property_iterator first = g0->edge_property_begin();
        edge_property_iterator last  = g0->edge_property_end();

        for (; first!=last; ++first)
          if (first->second.number_of_upper_nodes()>0)
            retval.push_back(&first->second);

        return retval;
      }


    ////////////////////////////////////////////////////////////////////////////////
    vector<DynamicNodeProperty*>
      get_nodes_outside_communities(shared_ptr<SociariumGraph const> g0) {

        vector<DynamicNodeProperty*> retval;
        node_property_iterator first = g0->node_property_begin();
        node_property_iterator last  = g0->node_property_end();

        for (; first!=last; ++first)
          if (first->second.number_of_upper_nodes()==0)
            retval.push_back(&first->second);

        return retval;
      }


    ////////////////////////////////////////////////////////////////////////////////
    vector<DynamicEdgeProperty*>
      get_edges_outside_communities(shared_ptr<SociariumGraph const> g0) {

        vector<DynamicEdgeProperty*> retval;
        edge_property_iterator first = g0->edge_property_begin();
        edge_property_iterator last  = g0->edge_property_end();

        for (; first!=last; ++first)
          if (first->second.number_of_upper_nodes()==0)
            retval.push_back(&first->second);

        return retval;
      }


    ////////////////////////////////////////////////////////////////////////////////
    vector<DynamicNodeProperty*>
      get_nodes_inside_marked_communities(shared_ptr<SociariumGraph const> g1) {

        vector<DynamicNodeProperty*> retval;
        node_property_iterator first = g1->node_property_begin();
        node_property_iterator last  = g1->node_property_end();

        for (; first!=last; ++first) {

          DynamicNodeProperty const& dnp = first->second;

          if (!is_marked(dnp)) continue;

          vector<DynamicNodeProperty*>::const_iterator i = dnp.lower_nbegin();
          vector<DynamicNodeProperty*>::const_iterator end = dnp.lower_nend();

          for (; i!=end; ++i)
            if (is_active(**i))
              retval.push_back(*i);
        }

        return retval;
      }


    ////////////////////////////////////////////////////////////////////////////////
    vector<DynamicEdgeProperty*>
      get_edges_inside_marked_communities(shared_ptr<SociariumGraph const> g1) {

        vector<DynamicEdgeProperty*> retval;
        node_property_iterator first = g1->node_property_begin();
        node_property_iterator last  = g1->node_property_end();

        for (; first!=last; ++first) {

          DynamicNodeProperty const& dnp = first->second;

          if (!is_marked(dnp)) continue;

          vector<DynamicEdgeProperty*>::const_iterator i = dnp.lower_ebegin();
          vector<DynamicEdgeProperty*>::const_iterator end = dnp.lower_eend();

          for (; i!=end; ++i)
            if (is_active(**i))
              retval.push_back(*i);
        }

        return retval;
      }


    ////////////////////////////////////////////////////////////////////////////////
    vector<DynamicNodeProperty*>
      get_nodes_in_marked_community_continuums(shared_ptr<SociariumGraph const> g1) {

        vector<DynamicNodeProperty*> retval;
        node_property_iterator first = g1->node_property_begin();
        node_property_iterator last  = g1->node_property_end();

        for (; first!=last; ++first) {

          DynamicNodeProperty const& dnp = first->second;

          if (!is_marked(dnp)) continue;

          StaticNodeProperty const* snp = dnp.get_static_property();

          StaticNodeProperty::DynamicPropertyMap::const_iterator i
            = snp->dynamic_property_begin();
          StaticNodeProperty::DynamicPropertyMap::const_iterator iend
            = snp->dynamic_property_end();

          for (; i!=iend; ++i) {

            vector<DynamicNodeProperty*>::const_iterator j
              = i->first->lower_nbegin();
            vector<DynamicNodeProperty*>::const_iterator jend
              = i->first->lower_nend();

            for (; j!=jend; ++j)
              if (is_active(**j))
                retval.push_back(*j);
          }
        }

        return retval;
      }


    ////////////////////////////////////////////////////////////////////////////////
    vector<DynamicEdgeProperty*>
      get_edges_in_marked_community_continuums(shared_ptr<SociariumGraph const> g1) {

        vector<DynamicEdgeProperty*> retval;
        node_property_iterator first = g1->node_property_begin();
        node_property_iterator last  = g1->node_property_end();

        for (; first!=last; ++first) {

          if (!is_marked(first->second)) continue;

          StaticNodeProperty const* snp = first->second.get_static_property();

          StaticNodeProperty::DynamicPropertyMap::const_iterator i
            = snp->dynamic_property_begin();
          StaticNodeProperty::DynamicPropertyMap::const_iterator iend
            = snp->dynamic_property_end();

          for (; i!=iend; ++i) {

            vector<DynamicEdgeProperty*>::const_iterator j
              = i->first->lower_ebegin();
            vector<DynamicEdgeProperty*>::const_iterator jend
              = i->first->lower_eend();

            for (; j!=jend; ++j)
              if (is_active(**j))
                retval.push_back(*j);
          }
        }

        return retval;
      }


    ////////////////////////////////////////////////////////////////////////////////
    void hide_marked_nodes(shared_ptr<SociariumGraph const> g) {

      typedef vector<DynamicNodeProperty*> HiddenNodes;
      HiddenNodes hidden_nodes;

      // Extract marked nodes in @g.
      for_each(g->node_property_begin(),
               g->node_property_end(),
               boost::bind<void>(ExtractMarkedElement(), _1,
                                 boost::ref(hidden_nodes)));

      // Hide marked nodes extracted above.
      for_each(g->node_property_begin(),
               g->node_property_end(),
               HideMarkedElement());

      // Hide connecting edges with hidden nodes above.
      HiddenNodes::const_iterator first = hidden_nodes.begin();
      HiddenNodes::const_iterator last  = hidden_nodes.end();

      for (; first!=last; ++first) {
        for_each((*first)->get_graph_element()->begin(),
                 (*first)->get_graph_element()->end(),
                 boost::bind<void>(HideEdge(), _1, g));
      }
    }


    ////////////////////////////////////////////////////////////////////////////////
    void hide_marked_nodes_all(shared_ptr<SociariumGraph const> g) {

      typedef vector<StaticNodeProperty*> HiddenNodes;
      HiddenNodes hidden_nodes;

      // Extract marked nodes in @g.
      for_each(g->node_property_begin(),
               g->node_property_end(),
               boost::bind<void>(ExtractMarkedElement(), _1,
                                 boost::ref(hidden_nodes)));

      // Hide nodes associated with ones extracted above and their connecting edges
      // from all layers.
      HiddenNodes::const_iterator first = hidden_nodes.begin();
      HiddenNodes::const_iterator last  = hidden_nodes.end();

      shared_ptr<SociariumGraphTimeSeries> ts
        = sociarium_project_graph_time_series::get();

      for (; first!=last; ++first) {
        DynamicNodePropertyMap::const_iterator first2 = (*first)->dynamic_property_begin();
        DynamicNodePropertyMap::const_iterator last2  = (*first)->dynamic_property_end();
        for (; first2!=last2; ++first2) {
          DynamicNodeProperty* dnp = first2->first;
          size_t const layer = first2->second;
          hide(*dnp);
          for_each(dnp->get_graph_element()->begin(),
                   dnp->get_graph_element()->end(),
                   boost::bind<void>(HideEdge(), _1,
                                     ts->get_graph(0, layer)));
        }
      }
    }


    ////////////////////////////////////////////////////////////////////////////////
    void hide_marked_nodes_all(void) {

      shared_ptr<SociariumGraphTimeSeries> ts
        = sociarium_project_graph_time_series::get();

      typedef unordered_set<StaticNodeProperty*> HiddenNodes;
      HiddenNodes hidden_nodes;

      size_t const number_of_layers = ts->number_of_layers();

      // Extract marked nodes on all layers.
      for (size_t layer=0; layer<number_of_layers; ++layer) {
        shared_ptr<SociariumGraph const> g = ts->get_graph(0, layer);
        for_each(g->node_property_begin(),
                 g->node_property_end(),
                 boost::bind<void>(ExtractMarkedElement(), _1,
                                   boost::ref(hidden_nodes)));
      }

      // Hide nodes associated with extracted above and their connecting edges
      // from all layers.
      HiddenNodes::const_iterator first = hidden_nodes.begin();
      HiddenNodes::const_iterator last  = hidden_nodes.end();

      for (; first!=last; ++first) {
        DynamicNodePropertyMap::const_iterator first2 = (*first)->dynamic_property_begin();
        DynamicNodePropertyMap::const_iterator last2  = (*first)->dynamic_property_end();

        for (; first2!=last2; ++first2) {
          DynamicNodeProperty* dnp = first2->first;
          size_t const layer = first2->second;
          hide(*dnp);
          for_each(dnp->get_graph_element()->begin(),
                   dnp->get_graph_element()->end(),
                   boost::bind<void>(HideEdge(), _1,
                                     ts->get_graph(0, layer)));
        }
      }
    }


    ////////////////////////////////////////////////////////////////////////////////
    void hide_marked_edges_all(shared_ptr<SociariumGraph const> g) {

      typedef vector<StaticEdgeProperty*> HiddenEdges;
      HiddenEdges hidden_edges;

      // Extract marked edges in @g.
      for_each(g->edge_property_begin(),
               g->edge_property_end(),
               boost::bind<void>(ExtractMarkedElement(), _1,
                                 boost::ref(hidden_edges)));

      // Hide edges associated with ones extracted above from all layers.
      HiddenEdges::const_iterator first = hidden_edges.begin();
      HiddenEdges::const_iterator last  = hidden_edges.end();

      for (; first!=last; ++first)
        for_each((*first)->dynamic_property_begin(),
                 (*first)->dynamic_property_end(),
                 HideMarkedElement());
    }


    ////////////////////////////////////////////////////////////////////////////////
    void hide_marked_edges_all(void) {

      shared_ptr<SociariumGraphTimeSeries> ts
        = sociarium_project_graph_time_series::get();

      typedef unordered_set<StaticEdgeProperty*> HiddenEdges;
      HiddenEdges hidden_edges;

      size_t const number_of_layers = ts->number_of_layers();

      // Extract marked edges on all layers.
      for (size_t layer=0; layer<number_of_layers; ++layer) {
        shared_ptr<SociariumGraph const> g = ts->get_graph(0, layer);
        for_each(g->edge_property_begin(),
                 g->edge_property_end(),
                 boost::bind<void>(ExtractMarkedElement(), _1,
                                   boost::ref(hidden_edges)));
      }

      // Hide edges associated with ones extracted above from all layers.
      HiddenEdges::const_iterator first = hidden_edges.begin();
      HiddenEdges::const_iterator last  = hidden_edges.end();

      for (; first!=last; ++first)
        for_each((*first)->dynamic_property_begin(),
                 (*first)->dynamic_property_end(),
                 HideMarkedElement());
    }


    ////////////////////////////////////////////////////////////////////////////////
    void remove_marked_nodes(shared_ptr<SociariumGraph> g) {

      typedef vector<DynamicNodeProperty*> RemovedNodes;
      RemovedNodes removed_nodes;

      // Extract marked nodes in @g.
      for_each(g->node_property_begin(),
               g->node_property_end(),
               boost::bind<void>(ExtractMarkedElement(), _1,
                                 boost::ref(removed_nodes)));

      // Remove marked nodes extracted above.
      RemovedNodes::const_iterator first = removed_nodes.begin();
      RemovedNodes::const_iterator last  = removed_nodes.end();

      for (; first!=last; ++first)
        g->remove_node((*first)->get_graph_element());
    }


    ////////////////////////////////////////////////////////////////////////////////
    void remove_marked_nodes_all(shared_ptr<SociariumGraph const> g) {

      typedef vector<StaticNodeProperty*> RemovedNodes;
      RemovedNodes removed_nodes;

      // Extract marked nodes in @g.
      for_each(g->node_property_begin(),
               g->node_property_end(),
               boost::bind<void>(ExtractMarkedElement(), _1,
                                 boost::ref(removed_nodes)));

      // Remove nodes associated with nodes extracted above and their connecting
      // edges from all layers.
      RemovedNodes::const_iterator first = removed_nodes.begin();
      RemovedNodes::const_iterator last  = removed_nodes.end();

      shared_ptr<SociariumGraphTimeSeries> ts
        = sociarium_project_graph_time_series::get();

      for (; first!=last; ++first)
        ts->remove_static_node(**first);
    }


    ////////////////////////////////////////////////////////////////////////////////
    void remove_marked_nodes_all(void) {

      shared_ptr<SociariumGraphTimeSeries> ts
        = sociarium_project_graph_time_series::get();

      typedef unordered_set<StaticNodeProperty*> RemovedNodes;
      RemovedNodes removed_nodes;

      size_t const number_of_layers = ts->number_of_layers();

      // Extract marked nodes on all layers.
      for (size_t layer=0; layer<number_of_layers; ++layer) {
        shared_ptr<SociariumGraph const> g = ts->get_graph(0, layer);
        for_each(g->node_property_begin(),
                 g->node_property_end(),
                 boost::bind<void>(ExtractMarkedElement(), _1,
                                   boost::ref(removed_nodes)));
      }

      // Remove all nodes extracted above and their connecting edges from all layers.
      RemovedNodes::const_iterator first = removed_nodes.begin();
      RemovedNodes::const_iterator last  = removed_nodes.end();

      for (; first!=last; ++first)
        ts->remove_static_node(**first);
    }


    ////////////////////////////////////////////////////////////////////////////////
    void remove_marked_edges(shared_ptr<SociariumGraph> g) {

      typedef vector<DynamicEdgeProperty*> RemovedEdges;
      RemovedEdges removed_edges;

      // Extract marked edges in @g.
      for_each(g->edge_property_begin(), g->edge_property_end(),
               boost::bind<void>(ExtractMarkedElement(), _1,
                                 boost::ref(removed_edges)));

      // Remove marked edges extracted above.
      RemovedEdges::const_iterator first = removed_edges.begin();
      RemovedEdges::const_iterator last  = removed_edges.end();

      for (; first!=last; ++first)
        g->remove_edge((*first)->get_graph_element());
    }


    ////////////////////////////////////////////////////////////////////////////////
    void remove_marked_edges_all(shared_ptr<SociariumGraph const> g) {

      typedef vector<StaticEdgeProperty*> RemovedEdges;
      RemovedEdges removed_edges;

      // Extract marked edges in @g.
      for_each(g->edge_property_begin(),
               g->edge_property_end(),
               boost::bind<void>(ExtractMarkedElement(), _1,
                                 boost::ref(removed_edges)));

      // Remove edges associated with ones extracted above from all layers.
      RemovedEdges::const_iterator first = removed_edges.begin();
      RemovedEdges::const_iterator last  = removed_edges.end();

      shared_ptr<SociariumGraphTimeSeries> ts
        = sociarium_project_graph_time_series::get();

      for (; first!=last; ++first)
        ts->remove_static_edge(**first);
    }


    ////////////////////////////////////////////////////////////////////////////////
    void remove_marked_edges_all(void) {

      shared_ptr<SociariumGraphTimeSeries> ts
        = sociarium_project_graph_time_series::get();

      typedef unordered_set<StaticEdgeProperty*> RemovedEdges;
      RemovedEdges removed_edges;

      size_t const number_of_layers = ts->number_of_layers();

      // Extract marked edges on all layers.
      for (size_t layer=0; layer<number_of_layers; ++layer) {
        shared_ptr<SociariumGraph const> g = ts->get_graph(0, layer);
        for_each(g->edge_property_begin(),
                 g->edge_property_end(),
                 boost::bind<void>(ExtractMarkedElement(), _1,
                                   boost::ref(removed_edges)));
      }

      // Remove all edges extracted above from all layers.
      RemovedEdges::const_iterator first = removed_edges.begin();
      RemovedEdges::const_iterator last  = removed_edges.end();

      for (; first!=last; ++first)
        ts->remove_static_edge(**first);
    }


    ////////////////////////////////////////////////////////////////////////////////
    void adjust_community_size(shared_ptr<SociariumGraph> g) {

      vector<DynamicNodeProperty*> recolored_nodes;
      vector<DynamicEdgeProperty*> recolored_edges;

      {
        // Communities where no member exists anymore are removed,
        // and communities where the number of members has changed are resized.
        node_property_iterator first = g->node_property_begin();
        node_property_iterator last  = g->node_property_end();

        while (first!=last) {
          node_property_iterator i = first++;
          DynamicNodeProperty& dnp = i->second;

          if (dnp.number_of_lower_nodes()==0)
            g->remove_node(dnp.get_graph_element());
          else {
            // Resize the community.
            update_community_size(dnp);
            // Recolor member nodes.
            recolored_nodes.insert(recolored_nodes.end(),
                                   dnp.lower_nbegin(),
                                   dnp.lower_nend());
            // Recolor members edges.
            recolored_edges.insert(recolored_edges.end(),
                                   dnp.lower_ebegin(),
                                   dnp.lower_eend());
          }
        }
      }

      {
        // Set color of member nodes.
        vector<DynamicNodeProperty*>::const_iterator first = recolored_nodes.begin();
        vector<DynamicNodeProperty*>::const_iterator last  = recolored_nodes.end();

        for (; first!=last; ++first)
          sociarium_project_color::update_color_under_community_information(**first);
      }

      {
        // Set color of member edges.
        vector<DynamicEdgeProperty*>::const_iterator first = recolored_edges.begin();
        vector<DynamicEdgeProperty*>::const_iterator last  = recolored_edges.begin();

        for (; first!=last; ++first)
          sociarium_project_color::update_color_under_community_information(**first);
      }
    }

  } // The end of the anonymous namespace


  ////////////////////////////////////////////////////////////////////////////////
  void WorldImpl::mark_elements(int menu) const {

    shared_ptr<SociariumGraphTimeSeries> ts
      = sociarium_project_graph_time_series::get();

    TimeSeriesLock lock(ts, TimeSeriesLock::Read);

    // --------------------------------------------------------------------------------
    shared_ptr<SociariumGraph const> g0_current
      = ts->get_graph(0, ts->index_of_current_layer());

    shared_ptr<SociariumGraph const> g1_current
      = ts->get_graph(1, ts->index_of_current_layer());

    size_t const number_of_layers = ts->number_of_layers();

    // --------------------------------------------------------------------------------
    if (menu==IDM_EDIT_MARK_ALL_NODES_ON_CURRENT_LAYER)
      // Mark all nodes on the current layer.
      for_each(g0_current->node_property_begin(), g0_current->node_property_end(),
               boost::bind<void>(SetFlag(), _1, ElementFlag::MARKED));

    else if (menu==IDM_EDIT_MARK_ALL_EDGES_ON_CURRENT_LAYER)
      // Mark all edges on the current layer.
      for_each(g0_current->edge_property_begin(), g0_current->edge_property_end(),
               boost::bind<void>(SetFlag(), _1, ElementFlag::MARKED));

    else if (menu==IDM_EDIT_MARK_ALL_ELEMENTS_ON_CURRENT_LAYER) {
      // Mark all nodes on the current layer.
      for_each(g0_current->node_property_begin(), g0_current->node_property_end(),
               boost::bind<void>(SetFlag(), _1, ElementFlag::MARKED));
      // Mark all edges on the current layer.
      for_each(g0_current->edge_property_begin(), g0_current->edge_property_end(),
               boost::bind<void>(SetFlag(), _1, ElementFlag::MARKED));
    }

    // --------------------------------------------------------------------------------
    else if (menu==IDM_EDIT_MARK_NODES_INSIDE_COMMUNITY_ON_CURRENT_LAYER) {
      vector<DynamicNodeProperty*> const nodes = get_nodes_inside_communities(g0_current);
      // Mark all inner community nodes on the current layer.
      for_each(nodes.begin(), nodes.end(),
               boost::bind<void>(SetFlag(), _1, ElementFlag::MARKED));
    }

    else if (menu==IDM_EDIT_MARK_EDGES_INSIDE_COMMUNITY_ON_CURRENT_LAYER) {
      vector<DynamicEdgeProperty*> const edges = get_edges_inside_communities(g0_current);
      // Mark all inner community edges on the current layer.
      for_each(edges.begin(), edges.end(),
               boost::bind<void>(SetFlag(), _1, ElementFlag::MARKED));
    }

    else if (menu==IDM_EDIT_MARK_ELEMENTS_INSIDE_COMMUNITY_ON_CURRENT_LAYER) {
      vector<DynamicNodeProperty*> const nodes = get_nodes_inside_communities(g0_current);
      vector<DynamicEdgeProperty*> const edges = get_edges_inside_communities(g0_current);
      // Mark all inner community nodes and edges on the current layer.
      for_each(nodes.begin(), nodes.end(),
               boost::bind<void>(SetFlag(), _1, ElementFlag::MARKED));
      for_each(edges.begin(), edges.end(),
               boost::bind<void>(SetFlag(), _1, ElementFlag::MARKED));
    }

    // --------------------------------------------------------------------------------
    else if (menu==IDM_EDIT_MARK_NODES_OUTSIDE_COMMUNITY_ON_CURRENT_LAYER) {
      vector<DynamicNodeProperty*> const nodes = get_nodes_outside_communities(g0_current);
      // Mark all outer community nodes on the current layer.
      for_each(nodes.begin(), nodes.end(),
               boost::bind<void>(SetFlag(), _1, ElementFlag::MARKED));
    }

    else if (menu==IDM_EDIT_MARK_EDGES_OUTSIDE_COMMUNITY_ON_CURRENT_LAYER) {
      vector<DynamicEdgeProperty*> const edges = get_edges_outside_communities(g0_current);
      // Mark all outer community edges on the current layer.
      for_each(edges.begin(), edges.end(),
               boost::bind<void>(SetFlag(), _1, ElementFlag::MARKED));
    }

    else if (menu==IDM_EDIT_MARK_ELEMENTS_OUTSIDE_COMMUNITY_ON_CURRENT_LAYER) {
      vector<DynamicNodeProperty*> const nodes = get_nodes_outside_communities(g0_current);
      vector<DynamicEdgeProperty*> const edges = get_edges_outside_communities(g0_current);
      // Mark all outer community nodes and edges on the current layer.
      for_each(nodes.begin(), nodes.end(),
               boost::bind<void>(SetFlag(), _1, ElementFlag::MARKED));
      for_each(edges.begin(), edges.end(),
               boost::bind<void>(SetFlag(), _1, ElementFlag::MARKED));
    }

    // --------------------------------------------------------------------------------
    else if (menu==IDM_EDIT_MARK_NODES_IN_SELECTED_COMMUNITY_CONTINUUMS_ON_CURRENT_LAYER) {
      vector<DynamicNodeProperty*> const nodes
         = get_nodes_inside_marked_communities(g1_current);
      // Mark nodes in selected communities on the current layer.
      for_each(nodes.begin(), nodes.end(),
               boost::bind<void>(SetFlag(), _1, ElementFlag::MARKED));
    }

    else if (menu==IDM_EDIT_MARK_EDGES_IN_SELECTED_COMMUNITY_CONTINUUMS_ON_CURRENT_LAYER) {
      vector<DynamicEdgeProperty*> const edges
        = get_edges_inside_marked_communities(g1_current);
      // Mark edges in selected communities on the current layer.
      for_each(edges.begin(), edges.end(),
               boost::bind<void>(SetFlag(), _1, ElementFlag::MARKED));
    }

    else if (menu==IDM_EDIT_MARK_ELEMENTS_IN_SELECTED_COMMUNITY_CONTINUUMS_ON_CURRENT_LAYER) {
      vector<DynamicNodeProperty*> const nodes
        = get_nodes_inside_marked_communities(g1_current);
      vector<DynamicEdgeProperty*> const edges
        = get_edges_inside_marked_communities(g1_current);
      // Mark elements in selected communities on the current layer.
      for_each(nodes.begin(), nodes.end(),
               boost::bind<void>(SetFlag(), _1, ElementFlag::MARKED));
      for_each(edges.begin(), edges.end(),
               boost::bind<void>(SetFlag(), _1, ElementFlag::MARKED));
    }

    // --------------------------------------------------------------------------------
    else if (menu==IDM_EDIT_MARK_ALL_NODES_ON_EACH_LAYER)
      for (size_t layer=0; layer<number_of_layers; ++layer) {
        shared_ptr<SociariumGraph const> g0 = ts->get_graph(0, layer);
        // Mark all nodes on all layers.
        for_each(g0->node_property_begin(), g0->node_property_end(),
                 boost::bind<void>(SetFlag(), _1, ElementFlag::MARKED));
      }

    else if (menu==IDM_EDIT_MARK_ALL_EDGES_ON_EACH_LAYER) {
      for (size_t layer=0; layer<number_of_layers; ++layer) {
        shared_ptr<SociariumGraph const> g0 = ts->get_graph(0, layer);
        // Mark all edges on all layers.
        for_each(g0->edge_property_begin(), g0->edge_property_end(),
                 boost::bind<void>(SetFlag(), _1, ElementFlag::MARKED));
      }
    }

    else if (menu==IDM_EDIT_MARK_ALL_ELEMENTS_ON_EACH_LAYER) {
      for (size_t layer=0; layer<number_of_layers; ++layer) {
        shared_ptr<SociariumGraph const> g0 = ts->get_graph(0, layer);
        shared_ptr<SociariumGraph const> g1 = ts->get_graph(1, layer);
        // Mark all nodes on all layers.
        for_each(g0->node_property_begin(), g0->node_property_end(),
                 boost::bind<void>(SetFlag(), _1, ElementFlag::MARKED));
        // Mark all edges on all layers.
        for_each(g0->edge_property_begin(), g0->edge_property_end(),
                 boost::bind<void>(SetFlag(), _1, ElementFlag::MARKED));
      }
    }

    // --------------------------------------------------------------------------------
    else if (menu==IDM_EDIT_MARK_NODES_INSIDE_COMMUNITY_ON_EACH_LAYER)
      for (size_t layer=0; layer<number_of_layers; ++layer) {
        shared_ptr<SociariumGraph const> g0 = ts->get_graph(0, layer);
        vector<DynamicNodeProperty*> nodes = get_nodes_inside_communities(g0);
        // Mark all inner community nodes on all layers.
        for_each(nodes.begin(), nodes.end(),
                 boost::bind<void>(SetFlag(), _1, ElementFlag::MARKED));
      }

    else if (menu==IDM_EDIT_MARK_EDGES_INSIDE_COMMUNITY_ON_EACH_LAYER) {
      for (size_t layer=0; layer<number_of_layers; ++layer) {
        shared_ptr<SociariumGraph const> g0 = ts->get_graph(0, layer);
        vector<DynamicEdgeProperty*> edges = get_edges_inside_communities(g0);
        // Mark all inner community edges on all layers.
        for_each(edges.begin(), edges.end(),
                 boost::bind<void>(SetFlag(), _1, ElementFlag::MARKED));
      }
    }

    else if (menu==IDM_EDIT_MARK_ELEMENTS_INSIDE_COMMUNITY_ON_EACH_LAYER) {
      for (size_t layer=0; layer<number_of_layers; ++layer) {
        shared_ptr<SociariumGraph const> g0 = ts->get_graph(0, layer);
        vector<DynamicNodeProperty*> nodes = get_nodes_inside_communities(g0);
        vector<DynamicEdgeProperty*> edges = get_edges_inside_communities(g0);
        // Mark all inner community nodes and edges on all layers.
        for_each(nodes.begin(), nodes.end(),
                 boost::bind<void>(SetFlag(), _1, ElementFlag::MARKED));
        for_each(edges.begin(), edges.end(),
                 boost::bind<void>(SetFlag(), _1, ElementFlag::MARKED));
      }
    }

    // --------------------------------------------------------------------------------
    else if (menu==IDM_EDIT_MARK_NODES_OUTSIDE_COMMUNITY_ON_EACH_LAYER) {
      for (size_t layer=0; layer<number_of_layers; ++layer) {
        shared_ptr<SociariumGraph const> g0 = ts->get_graph(0, layer);
        vector<DynamicNodeProperty*> nodes = get_nodes_outside_communities(g0);
        // Mark all outer community nodes on all layers.
        for_each(nodes.begin(), nodes.end(),
                 boost::bind<void>(SetFlag(), _1, ElementFlag::MARKED));
      }
    }

    else if (menu==IDM_EDIT_MARK_EDGES_OUTSIDE_COMMUNITY_ON_EACH_LAYER) {
      for (size_t layer=0; layer<number_of_layers; ++layer) {
        shared_ptr<SociariumGraph const> g0 = ts->get_graph(0, layer);
        vector<DynamicEdgeProperty*> edges = get_edges_outside_communities(g0);
        // Mark all outer community edges on all layers.
        for_each(edges.begin(), edges.end(),
                 boost::bind<void>(SetFlag(), _1, ElementFlag::MARKED));
      }
    }

    else if (menu==IDM_EDIT_MARK_ELEMENTS_OUTSIDE_COMMUNITY_ON_EACH_LAYER) {
      for (size_t layer=0; layer<number_of_layers; ++layer) {
        shared_ptr<SociariumGraph const> g0 = ts->get_graph(0, layer);
        vector<DynamicNodeProperty*> nodes = get_nodes_outside_communities(g0);
        vector<DynamicEdgeProperty*> edges = get_edges_outside_communities(g0);
        // Mark all outer community nodes and edges on all layers.
        for_each(nodes.begin(), nodes.end(),
                 boost::bind<void>(SetFlag(), _1, ElementFlag::MARKED));
        for_each(edges.begin(), edges.end(),
                 boost::bind<void>(SetFlag(), _1, ElementFlag::MARKED));
      }
    }

    // --------------------------------------------------------------------------------
    else if (menu==IDM_EDIT_MARK_NODES_IN_SELECTED_COMMUNITY_CONTINUUMS_ON_EACH_LAYER) {
      vector<DynamicNodeProperty*> const nodes
        = get_nodes_in_marked_community_continuums(g1_current);
      // Mark nodes, on each layer, which are in selected community-continuums.
      for_each(nodes.begin(), nodes.end(),
               boost::bind<void>(SetFlag(), _1, ElementFlag::MARKED));
    }

    else if (menu==IDM_EDIT_MARK_EDGES_IN_SELECTED_COMMUNITY_CONTINUUMS_ON_EACH_LAYER) {
      vector<DynamicEdgeProperty*> const edges
        = get_edges_in_marked_community_continuums(g1_current);
      // Mark edges, on each layer, which are in selected community-continuums.
      for_each(edges.begin(), edges.end(),
               boost::bind<void>(SetFlag(), _1, ElementFlag::MARKED));
    }

    else if (menu==IDM_EDIT_MARK_ELEMENTS_IN_SELECTED_COMMUNITY_CONTINUUMS_ON_EACH_LAYER) {
      vector<DynamicNodeProperty*> const nodes
        = get_nodes_in_marked_community_continuums(g1_current);
      vector<DynamicEdgeProperty*> const edges
        = get_edges_in_marked_community_continuums(g1_current);
      // Mark elements, on each layer, which are in selected community-continuums.
      for_each(nodes.begin(), nodes.end(),
               boost::bind<void>(SetFlag(), _1, ElementFlag::MARKED));
      for_each(edges.begin(), edges.end(),
               boost::bind<void>(SetFlag(), _1, ElementFlag::MARKED));
    }

    // --------------------------------------------------------------------------------
    else assert(0 && "never reach");
  }


  ////////////////////////////////////////////////////////////////////////////////
  void WorldImpl::inverse_mark(int menu) const {

    shared_ptr<SociariumGraphTimeSeries> ts
      = sociarium_project_graph_time_series::get();

    TimeSeriesLock lock(ts, TimeSeriesLock::Read);

    // --------------------------------------------------------------------------------
    shared_ptr<SociariumGraph const> g0_current
      = ts->get_graph(0, ts->index_of_current_layer());

    shared_ptr<SociariumGraph const> g1_current
      = ts->get_graph(1, ts->index_of_current_layer());

    size_t const number_of_layers = ts->number_of_layers();

    // --------------------------------------------------------------------------------
    if (menu==IDM_EDIT_INVERT_MARK_ON_CURRENT_LAYER) {
      // Inverse marking of all nodes on the current layer.
      for_each(g0_current->node_property_begin(), g0_current->node_property_end(),
               boost::bind<void>(ToggleFlag(), _1, ElementFlag::MARKED));
      // Inverse marking of all edges on the current layer.
      for_each(g0_current->edge_property_begin(), g0_current->edge_property_end(),
               boost::bind<void>(ToggleFlag(), _1, ElementFlag::MARKED));
      // Inverse marking of all communities on the current layer.
      for_each(g1_current->node_property_begin(), g1_current->node_property_end(),
               boost::bind<void>(ToggleFlag(), _1, ElementFlag::MARKED));
      // Inverse marking of all community edges on the current layer.
      for_each(g1_current->edge_property_begin(), g1_current->edge_property_end(),
               boost::bind<void>(ToggleFlag(), _1, ElementFlag::MARKED));
    }

    // --------------------------------------------------------------------------------
    else if (menu==IDM_EDIT_INVERT_MARK_ON_EACH_LAYER) {
      for (size_t layer=0; layer<number_of_layers; ++layer) {
        shared_ptr<SociariumGraph const> g0 = ts->get_graph(0, layer);
        shared_ptr<SociariumGraph const> g1 = ts->get_graph(1, layer);
        // Inverse marking of all nodes on all layers.
        for_each(g0->node_property_begin(), g0->node_property_end(),
                 boost::bind<void>(ToggleFlag(), _1, ElementFlag::MARKED));
        // Inverse marking of all edges on all layers.
        for_each(g0->edge_property_begin(), g0->edge_property_end(),
                 boost::bind<void>(ToggleFlag(), _1, ElementFlag::MARKED));
        // Inverse marking of all communities on all layers.
        for_each(g1->node_property_begin(), g1->node_property_end(),
                 boost::bind<void>(ToggleFlag(), _1, ElementFlag::MARKED));
        // Inverse marking of all community edges on all layers.
        for_each(g1->edge_property_begin(), g1->edge_property_end(),
                 boost::bind<void>(ToggleFlag(), _1, ElementFlag::MARKED));
      }
    }

    // --------------------------------------------------------------------------------
    else assert(0 && "never reach");
  }


  ////////////////////////////////////////////////////////////////////////////////
  void WorldImpl::hide_marked_element(int menu) const {

    shared_ptr<SociariumGraphTimeSeries> ts
      = sociarium_project_graph_time_series::get();

    TimeSeriesLock lock(ts, TimeSeriesLock::Read);

    // --------------------------------------------------------------------------------
    shared_ptr<SociariumGraph const> g0_current
      = ts->get_graph(0, ts->index_of_current_layer());

    size_t const number_of_layers = ts->number_of_layers();

    // --------------------------------------------------------------------------------
    if (menu==IDM_EDIT_HIDE_MARKED_NODES_ON_CURRENT_LAYER)
      // Hide marked nodes and their connecting edges on the current layer.
      hide_marked_nodes(g0_current);

    else if (menu==IDM_EDIT_HIDE_MARKED_EDGES_ON_CURRENT_LAYER)
      // Hide marked edges on the current layer.
      for_each(g0_current->edge_property_begin(), g0_current->edge_property_end(),
               HideMarkedElement());

    else if (menu==IDM_EDIT_HIDE_MARKED_ELEMENTS_ON_CURRENT_LAYER) {
      shared_ptr<SociariumGraph const> g =
        ts->get_graph(0, ts->index_of_current_layer());
      // Hide marked nodes and their connecting edges and marked edges on the current layer.
      hide_marked_nodes(g);
      for_each(g0_current->edge_property_begin(), g0_current->edge_property_end(),
               HideMarkedElement());
    }

    // --------------------------------------------------------------------------------
    else if (menu==IDM_EDIT_HIDE_MARKED_NODES_ON_EACH_LAYER)
      for (size_t layer=0; layer<number_of_layers; ++layer) {
        shared_ptr<SociariumGraph const> g0 = ts->get_graph(0, layer);
        // Hide marked nodes and their connecting edges on all layers.
        hide_marked_nodes(g0);
      }

    else if (menu==IDM_EDIT_HIDE_MARKED_EDGES_ON_EACH_LAYER)
      for (size_t layer=0; layer<number_of_layers; ++layer) {
        shared_ptr<SociariumGraph const> g0 = ts->get_graph(0, layer);
        // Hide marked edges on all layers.
        for_each(g0->edge_property_begin(), g0->edge_property_end(),
                 HideMarkedElement());
      }

    else if (menu==IDM_EDIT_HIDE_MARKED_ELEMENTS_ON_EACH_LAYER)
      for (size_t layer=0; layer<number_of_layers; ++layer) {
        shared_ptr<SociariumGraph const> g0 = ts->get_graph(0, layer);
        // Hide marked nodes and their connecting edges and marked edges on all layers.
        hide_marked_nodes(g0);
        for_each(g0->edge_property_begin(), g0->edge_property_end(),
                 HideMarkedElement());
      }

    // --------------------------------------------------------------------------------
    else if (menu==IDM_EDIT_HIDE_MARKED_NODES_ON_CURRENT_LAYER_ALL)
      // Hide marked nodes and their connecting edges on the current layer,
      // and hide all elements on all layers that correspond to the hidden elements
      // on the current layer.
      hide_marked_nodes_all(g0_current);

    else if (menu==IDM_EDIT_HIDE_MARKED_EDGES_ON_CURRENT_LAYER_ALL)
      // Hide marked edges on the current layer, and hide all edges on all layers
      // that correspond to the hidden edges on the current layer.
      hide_marked_edges_all(g0_current);

    else if (menu==IDM_EDIT_HIDE_MARKED_ELEMENTS_ON_CURRENT_LAYER_ALL) {
      // Hide marked nodes and their connecting edges and marked edges
      // on the current layer, and hide all elements on all layers that correspond to
      // the hidden elements on the current layer.
      hide_marked_nodes_all(g0_current);
      hide_marked_edges_all(g0_current);
    }

    else if (menu==IDM_EDIT_HIDE_MARKED_NODES_ON_EACH_LAYER_ALL)
      // Hide marked nodes and their connecting edges on all layers,
      // and hide all elements on all layers that correspond to the hidden elements
      // on any layer.
      hide_marked_nodes_all();

    else if (menu==IDM_EDIT_HIDE_MARKED_EDGES_ON_EACH_LAYER_ALL)
      // Hide marked edges on all layers, and hide all elements on all layers
      // that correspond to the hidden elements on any layer.
      hide_marked_edges_all();

    else if (menu==IDM_EDIT_HIDE_MARKED_ELEMENTS_ON_EACH_LAYER_ALL) {
      // Hide marked nodes and their connecting edges and marked edges on all layers,
      // and hide all elements on all layers that correspond to the hidden elements
      // on any layer.
      hide_marked_nodes_all();
      hide_marked_edges_all();
    }

    // --------------------------------------------------------------------------------
    else assert(0 && "never reach");

    sociarium_project_force_direction::should_be_updated();
  }


  ////////////////////////////////////////////////////////////////////////////////
  void WorldImpl::show_elements(int menu) const {

    shared_ptr<SociariumGraphTimeSeries> ts
      = sociarium_project_graph_time_series::get();

    TimeSeriesLock lock(ts, TimeSeriesLock::Read);

    // --------------------------------------------------------------------------------
    shared_ptr<SociariumGraph const> g0_current
      = ts->get_graph(0, ts->index_of_current_layer());

    size_t const number_of_layers = ts->number_of_layers();

    // --------------------------------------------------------------------------------
    if (menu==IDM_EDIT_SHOW_HIDDEN_NODES_ON_CURRENT_LAYER)
      // Show hidden nodes on the current layer.
      for_each(g0_current->node_property_begin(), g0_current->node_property_end(),
               ShowElement());

    else if (menu==IDM_EDIT_SHOW_HIDDEN_EDGES_ON_CURRENT_LAYER)
      // Show hidden edges on the current layer.
      for_each(g0_current->edge_property_begin(), g0_current->edge_property_end(),
               ShowElement());

    else if (menu==IDM_EDIT_SHOW_HIDDEN_ELEMENTS_ON_CURRENT_LAYER) {
      // Show hidden nodes and edges on the current layer.
      for_each(g0_current->node_property_begin(), g0_current->node_property_end(),
               ShowElement());
      for_each(g0_current->edge_property_begin(), g0_current->edge_property_end(),
               ShowElement());
    }

    // --------------------------------------------------------------------------------
    else if (menu==IDM_EDIT_SHOW_HIDDEN_NODES_ON_EACH_LAYER)
      for (size_t layer=0; layer<number_of_layers; ++layer) {
        shared_ptr<SociariumGraph const> g0 = ts->get_graph(0, layer);
        // Show hidden nodes on all layers.
        for_each(g0->node_property_begin(), g0->node_property_end(),
                 ShowElement());
      }

    else if (menu==IDM_EDIT_SHOW_HIDDEN_EDGES_ON_EACH_LAYER)
      for (size_t layer=0; layer<number_of_layers; ++layer) {
        shared_ptr<SociariumGraph const> g0 = ts->get_graph(0, layer);
        // Show hidden edges on all layers.
        for_each(g0->edge_property_begin(), g0->edge_property_end(),
                 ShowElement());
      }

    else if (menu==IDM_EDIT_SHOW_HIDDEN_ELEMENTS_ON_EACH_LAYER)
      for (size_t layer=0; layer<number_of_layers; ++layer) {
        shared_ptr<SociariumGraph const> g0 = ts->get_graph(0, layer);
        // Show hidden nodes and edges on all layers.
        for_each(g0->node_property_begin(), g0->node_property_end(),
                 ShowElement());
        for_each(g0->edge_property_begin(), g0->edge_property_end(),
                 ShowElement());
      }

    // --------------------------------------------------------------------------------
    else assert(0 && "never reach");

    sociarium_project_force_direction::should_be_updated();
  }


  ////////////////////////////////////////////////////////////////////////////////
  void WorldImpl::remove_marked_elements(int menu) const {

    // --------------------------------------------------------------------------------
    // Check another thread is running.

    bool another_thread_is_running = false;

    for (int i=0; i<NUMBER_OF_THREAD_CATEGORIES; ++i) {
      if (i!=FORCE_DIRECTION && joinable(i)) {
        another_thread_is_running = true;
        break;
      }
    }

    if (another_thread_is_running) {
      message_box(hwnd_, mb_notice, APPLICATION_TITLE,
                  L"%s", get_message(Message::ANOTHER_THREAD_IS_RUNNING));
      return;
    }

    if (message_box(hwnd_, mb_ok_cancel, APPLICATION_TITLE,
                    L"%s", get_message(Message::REMOVE_ELEMENTS))==IDCANCEL)
      return;

    // --------------------------------------------------------------------------------
    shared_ptr<SociariumGraphTimeSeries> ts
      = sociarium_project_graph_time_series::get();

    ts->read_unlock();

    shared_ptr<CommunityTransitionDiagram> diagram
      = sociarium_project_community_transition_diagram::get();

    if (diagram && !diagram->empty())
      diagram->write_lock();

    /*
     * Don't forget to call write_unlock().
     */

    // --------------------------------------------------------------------------------
    shared_ptr<SociariumGraph> g0_current
      = ts->get_graph(0, ts->index_of_current_layer());

    shared_ptr<SociariumGraph> g1_current
      = ts->get_graph(1, ts->index_of_current_layer());

    size_t const number_of_layers = ts->number_of_layers();

    // --------------------------------------------------------------------------------
    if (menu==IDM_EDIT_REMOVE_MARKED_NODES_ON_CURRENT_LAYER) {
      // Remove marked nodes on the current layer.
      remove_marked_nodes(g0_current);
      adjust_community_size(g1_current);
    }

    else if (menu==IDM_EDIT_REMOVE_MARKED_EDGES_ON_CURRENT_LAYER) {
      // Remove marked edges on the current layer.
      remove_marked_edges(g0_current);
    }

    else if (menu==IDM_EDIT_REMOVE_MARKED_ELEMENTS_ON_CURRENT_LAYER) {
      // Remove marked nodes and edges on the current layer.
      remove_marked_nodes(g0_current);
      remove_marked_edges(g0_current);
      adjust_community_size(g1_current);
    }

    // --------------------------------------------------------------------------------
    else if (menu==IDM_EDIT_REMOVE_MARKED_NODES_ON_EACH_LAYER) {
      for (size_t layer=0; layer<number_of_layers; ++layer) {
        shared_ptr<SociariumGraph> g0 = ts->get_graph(0, layer);
        shared_ptr<SociariumGraph> g1 = ts->get_graph(1, layer);
        // Remove marked nodes on all layers.
        remove_marked_nodes(g0);
        adjust_community_size(g1);
      }
    }

    else if (menu==IDM_EDIT_REMOVE_MARKED_EDGES_ON_EACH_LAYER) {
      for (size_t layer=0; layer<number_of_layers; ++layer) {
        shared_ptr<SociariumGraph> g0 = ts->get_graph(0, layer);
        // Remove marked edges on all layers.
        remove_marked_edges(g0);
      }
    }

    else if (menu==IDM_EDIT_REMOVE_MARKED_ELEMENTS_ON_EACH_LAYER) {
      for (size_t layer=0; layer<number_of_layers; ++layer) {
        shared_ptr<SociariumGraph> g0 = ts->get_graph(0, layer);
        shared_ptr<SociariumGraph> g1 = ts->get_graph(1, layer);
        // Remove marked nodes and edges on all layers.
        remove_marked_nodes(g0);
        remove_marked_edges(g0);
        adjust_community_size(g1);
      }
    }

    // --------------------------------------------------------------------------------
    else if (menu==IDM_EDIT_REMOVE_MARKED_NODES_ON_CURRENT_LAYER_ALL) {
      // Remove marked nodes and their connecting edges on the current layer,
      // and remove all elements on all layers that correspond to the removed elements
      // on the current layer.
      remove_marked_nodes_all(g0_current);

      for (size_t layer=0; layer<number_of_layers; ++layer) {
        shared_ptr<SociariumGraph> g1 = ts->get_graph(1, layer);
        adjust_community_size(g1);
      }
    }

    else if (menu==IDM_EDIT_REMOVE_MARKED_EDGES_ON_CURRENT_LAYER_ALL)
      // Remove marked edges on the current layer, and remove all edges on all layers
      // that correspond to the removed edges on the current layer.
      remove_marked_edges_all(g0_current);

    else if (menu==IDM_EDIT_REMOVE_MARKED_ELEMENTS_ON_CURRENT_LAYER_ALL) {
      // Remove marked nodes and their connecting edges and marked edges on the current layer,
      // and remove all elements on all layers that correspond to the removed elements
      // on the current layer.
      remove_marked_nodes_all(g0_current);
      remove_marked_edges_all(g0_current);

      for (size_t layer=0; layer<number_of_layers; ++layer) {
        shared_ptr<SociariumGraph> g1 = ts->get_graph(1, layer);
        adjust_community_size(g1);
      }
    }

    // --------------------------------------------------------------------------------
    else if (menu==IDM_EDIT_REMOVE_MARKED_NODES_ON_EACH_LAYER_ALL) {
      // Remove marked nodes and their connecting edges on all layers, and remove all
      // elements on all layers that correspond to the removed elements on the current layer.
      remove_marked_nodes_all();

      for (size_t layer=0; layer<number_of_layers; ++layer) {
        shared_ptr<SociariumGraph> g1 = ts->get_graph(1, layer);
        adjust_community_size(g1);
      }
    }

    else if (menu==IDM_EDIT_REMOVE_MARKED_EDGES_ON_EACH_LAYER_ALL)
      // Remove marked edges on all layers, and remove all elements on all layers that
      // correspond to the removed edges on the current layer.
      remove_marked_edges_all();

    else if (menu==IDM_EDIT_REMOVE_MARKED_ELEMENTS_ON_EACH_LAYER_ALL) {
      // Remove marked nodes and their connecting edges and marked edges on all layers,
      // and remove all elements on all layers that correspond to the removed elements
      // on the current layer.
      remove_marked_nodes_all();
      remove_marked_edges_all();

      for (size_t layer=0; layer<number_of_layers; ++layer) {
        shared_ptr<SociariumGraph> g1 = ts->get_graph(1, layer);
        adjust_community_size(g1);
      }
    }

    // --------------------------------------------------------------------------------
    else assert(0 && "never reach");

    ts->remove_empty_static_nodes(0);
    ts->remove_empty_static_edges(0);
    ts->remove_empty_static_nodes(1);
    ts->remove_empty_static_edges(1);

    sociarium_project_force_direction::should_be_updated();

    ts->write_to_read_lock();

    if (diagram && !diagram->empty()) {
      using namespace sociarium_project_community_transition_diagram;
      diagram->update(get_diagram_mode());
      diagram->write_unlock();
    }

    ts->read_unlock();
  }

} // The end of the namespace "hashimoto_ut"

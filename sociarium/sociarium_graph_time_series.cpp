// s.o.c.i.a.r.i.u.m: sociarium_graph_time_series.cpp
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
#include <string>
#include <numeric>
#include <fstream>
#include <functional>
#include <array>
#include <unordered_set>
#include <unordered_map>
#include "algorithm_selector.h"
#include "color.h"
#include "community_transition_diagram.h"
#include "flag_operation.h"
#include "layout.h"
#include "selection.h"
#include "sociarium_graph_time_series.h"
#include "thread/force_direction.h"
#include "../shared/mutex.h"
#include "../shared/predefined_color.h"

namespace hashimoto_ut {

  using std::vector;
  using std::wstring;
  using std::accumulate;
  using std::pair;
  using std::tr1::bind;
  using std::tr1::placeholders::_1;
  using std::tr1::shared_ptr;
  using std::tr1::array;
  using std::tr1::unordered_set;
  using std::tr1::unordered_map;

  namespace {

    wchar_t const* LAYER_TITLE = L"s.o.c.i.a.r.i.u.m";

    struct CreateSociariumGraph {
      void operator()(shared_ptr<SociariumGraph>& g) const {
        g = SociariumGraph::create();
      }
    };

    size_t const number_of_levels = 2;
    /*
     * There are 2 levels of networks: The one is the base network,
     * and another is the upper level network which consists of communities
     * (groups of nodes) and community-community relations.
     */

  } // The end of the anonymous namespace


  class SociariumGraphTimeSeriesImpl :
    public SociariumGraphTimeSeries,
    public Mutex {

  public:

    typedef SociariumGraph::node_property_iterator node_property_iterator;
    typedef SociariumGraph::edge_property_iterator edge_property_iterator;


    ////////////////////////////////////////////////////////////////////////////////
    SociariumGraphTimeSeriesImpl(void) :
    index_of_current_layer_(0) {
      assert(graph_series_.size()==number_of_levels);
      graph_series_[0].push_back(SociariumGraph::create());
      graph_series_[1].push_back(SociariumGraph::create());
      layer_name_.push_back(LAYER_TITLE);
    }

    ~SociariumGraphTimeSeriesImpl() {

      // Check if no object refers to any network.
      for (size_t i=0; i<number_of_layers(); ++i) {
        assert(graph_series_[0][i].unique());
        assert(graph_series_[1][i].unique());
      }

      graph_series_[0].clear();
      graph_series_[1].clear();
    }


    ////////////////////////////////////////////////////////////////////////////////
    void read_lock(void) const {
      Mutex::read_lock();
    }

    void write_lock(void) const {
      Mutex::write_lock();
    }

    void read_to_write_lock(void) const {
      Mutex::read_to_write_lock();
    }

    void write_to_read_lock(void) const {
      Mutex::write_to_read_lock();
    }

    void read_unlock(void) const {
      Mutex::read_unlock();
    }

    void write_unlock(void) const {
      Mutex::write_unlock();
    }


    ////////////////////////////////////////////////////////////////////////////////
    size_t number_of_layers(void) const {
      return layer_name_.size();
    }

    size_t index_of_current_layer(void) const {
      return index_of_current_layer_;
    }

    void move_layer(int index_of_layer) {
      int const current = index_of_current_layer_;

      if (index_of_layer<0)
        index_of_current_layer_ = 0;
      else if (size_t(index_of_layer)>=number_of_layers())
        index_of_current_layer_ = number_of_layers()-1;
      else
        index_of_current_layer_ = size_t(index_of_layer);

      shared_ptr<CommunityTransitionDiagram> diagram
        = sociarium_project_community_transition_diagram::get();
      if (diagram && !diagram->empty())
        diagram->set_current_layer(index_of_current_layer_);

      if (current!=index_of_current_layer_)
        sociarium_project_force_direction::should_be_updated();
    }


    ////////////////////////////////////////////////////////////////////////////////
    shared_ptr<SociariumGraph>
      get_graph(size_t level, size_t index_of_layer) const {
        assert(0<=index_of_layer && index_of_layer<number_of_layers());
        return graph_series_[level][index_of_layer];
      }


    ////////////////////////////////////////////////////////////////////////////////
    wstring const& get_layer_name(size_t index_of_layer) const {
      assert(0<=index_of_layer && index_of_layer<number_of_layers());
      return layer_name_[index_of_layer];
    }


    ////////////////////////////////////////////////////////////////////////////////
    size_t number_of_static_nodes(size_t level) const  {
      assert(0<=level && level<number_of_levels);
      return static_node_property_[level].size();
    }

    size_t number_of_static_edges(size_t level) const {
      assert(0<=level && level<number_of_levels);
      return static_edge_property_[level].size();
    }


    ////////////////////////////////////////////////////////////////////////////////
    StaticNodePropertySet::const_iterator
      static_node_property_begin(size_t level) const {
        assert(0<=level && level<number_of_levels);
        return static_node_property_[level].begin();
      }

    StaticNodePropertySet::const_iterator
      static_node_property_end(size_t level) const {
        assert(0<=level && level<number_of_levels);
        return static_node_property_[level].end();
      }

    StaticEdgePropertySet::const_iterator
      static_edge_property_begin(size_t level) const {
        assert(0<=level && level<number_of_levels);
        return static_edge_property_[level].begin();
      }

    StaticEdgePropertySet::const_iterator
      static_edge_property_end(size_t level) const {
        assert(0<=level && level<number_of_levels);
        return static_edge_property_[level].end();
      }


    ////////////////////////////////////////////////////////////////////////////////
    void remove_static_node(StaticNodeProperty& snp) {

      StaticNodePropertySet::iterator i = static_node_property_[0].find(snp);

      if (i!=static_node_property_[0].end()) {

        DynamicNodePropertyMap::const_iterator j = snp.dynamic_property_begin();

        // Remove all associated dynamic properties from all layers.
        while (j!=snp.dynamic_property_end()) {

          DynamicNodePropertyMap::const_iterator k = j++;
          DynamicNodeProperty& dnp = *k->first;
          size_t const layer = k->second;

          shared_ptr<SociariumGraph> g = graph_series_[0][layer];
          Node* n = dnp.get_graph_element();

          // Remove all connecting edges in @layer.
          for (adjacency_list_iterator m=n->begin(); m!=n->end();) {
            DynamicEdgeProperty& dep = g->property(*m);
            StaticEdgeProperty& sep = *dep.get_static_property();

            g->remove_edge(*m);

            // If there is no associated property in other layers,
            // delete a static property.
            if (sep.number_of_dynamic_properties()==0)
              static_edge_property_[0].erase(sep);

            m = n->begin();
          }

          g->remove_node(n);
        }

        static_node_property_[0].erase(i);
      }
    }


    ////////////////////////////////////////////////////////////////////////////////
    void remove_static_edge(StaticEdgeProperty& sep) {

      StaticEdgePropertySet::iterator i = static_edge_property_[0].find(sep);

      if (i!=static_edge_property_[0].end()) {

        DynamicEdgePropertyMap::const_iterator j = sep.dynamic_property_begin();

        // Remove all associated dynamic properties from all layers.
        while (j!=sep.dynamic_property_end()) {
          DynamicEdgePropertyMap::const_iterator k = j++;
          graph_series_[0][k->second]->remove_edge(k->first->get_graph_element());
        }

        static_edge_property_[0].erase(i);
      }
    }


    ////////////////////////////////////////////////////////////////////////////////
    void remove_empty_static_nodes(size_t level) {
      assert(0<=level && level<number_of_levels);

      StaticNodePropertySet& nodes = static_node_property_[level];
      for (StaticNodePropertySet::iterator i=nodes.begin(); i!=nodes.end();) {
        StaticNodePropertySet::iterator j = i++;
        if (j->number_of_dynamic_properties()==0)
          nodes.erase(j);
      }
    }

    void remove_empty_static_edges(size_t level) {
      assert(0<=level && level<number_of_levels);

      StaticEdgePropertySet& edges = static_edge_property_[level];
      for (StaticEdgePropertySet::iterator i=edges.begin(); i!=edges.end();) {
        StaticEdgePropertySet::iterator j = i++;
        if (j->number_of_dynamic_properties()==0)
          edges.erase(j);
      }
    }


    ////////////////////////////////////////////////////////////////////////////////
    StaticNodePropertySet::iterator
      find_static_node(size_t level, wstring const& name) {
        assert(0<=level && level<number_of_levels);
        return find_if(static_node_property_[level].begin(),
                       static_node_property_[level].end(),
                       bind(std::equal_to<wstring>(),
                            bind(&StaticNodeProperty::get_name, _1), name));
      }

    StaticEdgePropertySet::iterator
      find_static_edge(size_t level, wstring const& name) {
        assert(0<=level && level<number_of_levels);
        return find_if(static_edge_property_[level].begin(),
                       static_edge_property_[level].end(),
                       bind(std::equal_to<wstring>(),
                            bind(&StaticEdgeProperty::get_name, _1), name));
      }

    StaticNodePropertySet::const_iterator
      find_static_node(size_t level, wstring const& name) const {
        assert(0<=level && level<number_of_levels);
        return find_if(static_node_property_[level].begin(),
                       static_node_property_[level].end(),
                       bind(std::equal_to<wstring>(),
                            bind(&StaticNodeProperty::get_name, _1), name));
      }

    StaticEdgePropertySet::const_iterator
      find_static_edge(size_t level, wstring const& name) const {
        assert(0<=level && level<number_of_levels);
        return find_if(static_edge_property_[level].begin(),
                       static_edge_property_[level].end(),
                       bind(std::equal_to<wstring>(),
                            bind(&StaticEdgeProperty::get_name, _1), name));
      }


    ////////////////////////////////////////////////////////////////////////////////
    StaticNodePropertySet::iterator
      find_static_node(size_t level, size_t id) {
        assert(0<=level && level<number_of_levels);
        return find_if(static_node_property_[level].begin(),
                       static_node_property_[level].end(),
                       bind(std::equal_to<size_t>(),
                            bind(&StaticNodeProperty::get_id, _1), id));
      }

    StaticEdgePropertySet::iterator
      find_static_edge(size_t level, size_t id) {
        assert(0<=level && level<number_of_levels);
        return find_if(static_edge_property_[level].begin(),
                       static_edge_property_[level].end(),
                       bind(std::equal_to<size_t>(),
                            bind(&StaticEdgeProperty::get_id, _1), id));
      }

    StaticNodePropertySet::const_iterator
      find_static_node(size_t level, size_t id) const {
        assert(0<=level && level<number_of_levels);
        return find_if(static_node_property_[level].begin(),
                       static_node_property_[level].end(),
                       bind(std::equal_to<size_t>(),
                            bind(&StaticNodeProperty::get_id, _1), id));
      }

    StaticEdgePropertySet::const_iterator
      find_static_edge(size_t level, size_t id) const {
        assert(0<=level && level<number_of_levels);
        return find_if(static_edge_property_[level].begin(),
                       static_edge_property_[level].end(),
                       bind(std::equal_to<size_t>(),
                            bind(&StaticEdgeProperty::get_id, _1), id));
      }


    ////////////////////////////////////////////////////////////////////////////////
    void update_node_size(vector<vector<double> > const& node_size) {
      assert(node_size.size()==number_of_layers());

      for (size_t layer=0; layer<number_of_layers(); ++layer) {

        shared_ptr<SociariumGraph> g = get_graph(0, layer);
        size_t const nsz = g->nsize();

        if (nsz==0) continue;

        assert(node_size[layer].size()==nsz);

        for (size_t i=0; i<nsz; ++i)
          g->property(g->node(i)).set_size(float(node_size[layer][i]));
      }
    }


    ////////////////////////////////////////////////////////////////////////////////
    void update_edge_width(vector<vector<double> > const& edge_width) {
      assert(edge_width.size()==number_of_layers());

      for (size_t layer=0; layer<number_of_layers(); ++layer) {

        shared_ptr<SociariumGraph> g = get_graph(0, layer);
        size_t const esz = g->esize();

        if (esz==0) continue;

        assert(edge_width[layer].size()==esz);

        for (size_t i=0; i<esz; ++i)
          g->property(g->edge(i)).set_width(float(edge_width[layer][i]));
      }
    }


    ////////////////////////////////////////////////////////////////////////////////
    void update_node_position(size_t index_of_layer,
                              vector<Vector2<double> > const& position) {

      assert(0<=index_of_layer && index_of_layer<number_of_layers());

      shared_ptr<SociariumGraph> g0 = graph_series_[0][index_of_layer];
      shared_ptr<SociariumGraph> g1 = graph_series_[1][index_of_layer];

      assert(g0->nsize()==position.size());

      {
        node_property_iterator i   = g0->node_property_begin();
        node_property_iterator end = g0->node_property_end();

        for (; i!=end; ++i)
          i->second.get_static_property()->set_position(position[i->first->index()]);
      }{
        node_property_iterator i   = g1->node_property_begin();
        node_property_iterator end = g1->node_property_end();

        for (; i!=end; ++i)
          sociarium_project_layout::reset_position(i->second);
      }

      sociarium_project_force_direction::should_be_updated();
    }


    ////////////////////////////////////////////////////////////////////////////////
    void clear_community(void) {

      write_lock();
      /*
       * Don't forget to call write_unlock().
       */

      shared_ptr<CommunityTransitionDiagram> diagram
        = sociarium_project_community_transition_diagram::get();

      if (diagram && !diagram->empty()) {
        diagram->write_lock();
        diagram->clear();
        diagram->write_unlock();
      }

      // Delete a community level.
      vector<shared_ptr<SociariumGraph> >(number_of_layers()).swap(graph_series_[1]);

      static_node_property_[1].clear();
      static_edge_property_[1].clear();

      // Rebuild an empty community level.
      for (size_t layer=0; layer<number_of_layers(); ++layer) {
        graph_series_[1][layer] = SociariumGraph::create();

        shared_ptr<SociariumGraph> g0 = graph_series_[0][layer];

        using namespace sociarium_project_color;

        {
          // Reset color of nodes.
          node_property_iterator i   = g0->node_property_begin();
          node_property_iterator end = g0->node_property_end();

          for (; i!=end; ++i)
            if (is_active(*i->second.get_static_property(),
                          ElementFlag::TEXTURE_IS_SPECIFIED))
              i->second.set_color_id(PredefinedColor::WHITE);
            else
              i->second.set_color_id(get_default_node_color_id());
        }{
          // Reset color of edges.
          edge_property_iterator i   = g0->edge_property_begin();
          edge_property_iterator end = g0->edge_property_end();

          for (; i!=end; ++i)
            i->second.set_color_id(get_default_edge_color_id());
        }
      }

      using namespace sociarium_project_algorithm_selector;

      if (get_force_direction_algorithm()
          ==RealTimeForceDirectionAlgorithm::KAMADA_KAWAI_METHOD_WITH_COMMUNITY_SEPARATION
          || get_force_direction_algorithm()
          ==RealTimeForceDirectionAlgorithm::COMMUNITY_ORIENTED)
        sociarium_project_force_direction::should_be_updated();

      write_unlock();
    }


    ////////////////////////////////////////////////////////////////////////////////
    void update(vector<shared_ptr<SociariumGraph> >& graph_series,
                StaticNodePropertySet& static_node_property,
                StaticEdgePropertySet& static_edge_property,
                vector<wstring>& layer_name) {

      assert(graph_series.size()==layer_name.size());

      shared_ptr<CommunityTransitionDiagram> diagram
        = sociarium_project_community_transition_diagram::get();

      if (diagram && !diagram->empty()) {
        diagram->write_lock();
        diagram->clear();
        diagram->write_unlock();
      }

      read_to_write_lock();
      /*
       * Don't forget to call write_to_read_lock().
       */

      sociarium_project_selection::clear();

      // Build the base level.
      index_of_current_layer_ = 0;
      graph_series.swap(graph_series_[0]);
      static_node_property.swap(static_node_property_[0]);
      static_edge_property.swap(static_edge_property_[0]);
      layer_name.swap(layer_name_);

      if (graph_series_[0].empty()) {
        assert(static_node_property_[0].empty());
        assert(static_edge_property_[0].empty());
        graph_series_[0].push_back(SociariumGraph::create());
        layer_name_.push_back(L"Empty");
      }

      // Build the community level.
      vector<shared_ptr<SociariumGraph> > community_series(number_of_layers());

      for_each(community_series.begin(), community_series.end(),
               CreateSociariumGraph());

      community_series.swap(graph_series_[1]);

      sociarium_project_force_direction::should_be_updated();

      write_to_read_lock();
    }


    ////////////////////////////////////////////////////////////////////////////////
    void update_community(
      vector<shared_ptr<SociariumGraph> >& community_series,
      StaticNodePropertySet& static_community_property,
      StaticEdgePropertySet& static_community_edge_property) {

      assert(graph_series_[0].size()==community_series.size());

      read_to_write_lock();
      /*
       * Don't forget to call write_to_read_lock().
       */

      sociarium_project_selection::clear();

      shared_ptr<CommunityTransitionDiagram> diagram
        = sociarium_project_community_transition_diagram::get();

      if (diagram)
        diagram->write_lock();

      // Build the community level.
      community_series.swap(graph_series_[1]);
      static_community_property.swap(static_node_property_[1]);
      static_community_edge_property.swap(static_edge_property_[1]);

      // Associate the base level with the community level.
      for (size_t layer=0; layer<number_of_layers(); ++layer) {

        shared_ptr<SociariumGraph> g0 = graph_series_[0][layer];
        shared_ptr<SociariumGraph> g1 = graph_series_[1][layer];

        // Delete old community information from the base level.
        {
          node_property_iterator i   = g0->node_property_begin();
          node_property_iterator end = g0->node_property_end();

          for (; i!=end; ++i)
            i->second.clear_upper_nodes();
        }{
          edge_property_iterator i   = g0->edge_property_begin();
          edge_property_iterator end = g0->edge_property_end();

          for (; i!=end; ++i)
            i->second.clear_upper_nodes();
        }

        // Give new community information to the base level.
        {
          node_property_iterator i   = g1->node_property_begin();
          node_property_iterator end = g1->node_property_end();

          for (; i!=end; ++i) {

            DynamicNodeProperty& dnp = i->second;

            for_each(dnp.lower_nbegin(), dnp.lower_nend(),
                     bind(&DynamicNodeProperty::register_upper_node, _1, &dnp));

            for_each(dnp.lower_ebegin(), dnp.lower_eend(),
                     bind(&DynamicEdgeProperty::register_upper_node, _1, &dnp));
          }
        }

        {
          // Set color of nodes.
          node_property_iterator i   = g0->node_property_begin();
          node_property_iterator end = g0->node_property_end();

          for (; i!=end; ++i)
            sociarium_project_color::update_color_under_community_information(i->second);
        }{
          // Set color of edges.
          edge_property_iterator i   = g0->edge_property_begin();
          edge_property_iterator end = g0->edge_property_end();

          for (; i!=end; ++i)
            sociarium_project_color::update_color_under_community_information(i->second);
        }
      }

      sociarium_project_force_direction::should_be_updated();

      write_to_read_lock();

      // Locate communities on the center of the member nodes position.
      for (size_t layer=0; layer<number_of_layers(); ++layer) {

        shared_ptr<SociariumGraph> g = graph_series_[1][layer];

        node_property_iterator i   = g->node_property_begin();
        node_property_iterator end = g->node_property_end();

        for (; i!=end; ++i)
          sociarium_project_layout::reset_position(i->second);
      }

      if (diagram) {
        using namespace sociarium_project_community_transition_diagram;
        if (number_of_layers()>1)
          diagram->update(get_diagram_mode());
        diagram->write_unlock();
      }
    }


//     ////////////////////////////////////////////////////////////////////////////////
//     void append(unordered_set<wstring> const& node_list, vector<pair<pair<wstring, wstring>, double> > const& edge_list) {
//
//       if (node_list.empty()) return;
//
//       shared_ptr<SociariumGraph> g = graph_series_[0][index_of_current_layer_];
//       unordered_map<wstring, Node*> name2node;
//       unordered_map<wstring, Edge*> name2edge;
//
//       for (node_iterator i=g->nbegin(); i!=g->nend(); ++i)
//         name2node[g->property(*i)->get_static_property()->get_name()] = *i;
//       for (edge_iterator i=g->ebegin(); i!=g->eend(); ++i)
//         name2edge[g->property(*i)->get_static_property()->get_name()] = *i;
//
//       // ----------------------------------------------------------------------------------------------------
//       read_to_write_lock();
//       // ----------------------------------------------------------------------------------------------------
//
//       float const deg = float(M_2PI)/node_list.size();
//       float const rad = sociarium_project_layout::get_layout_frame_size();
//       Vector2<float> const& center = sociarium_project_layout::get_layout_frame_position();
//
//       size_t count = 0;
//       for (unordered_set<wstring>::const_iterator i=node_list.begin(); i!=node_list.end(); ++i) {
//         StaticNodeProperty* snp = find_node(0, *i);
//
//         if (snp==0) {
//           snp.reset(new StaticNodeProperty(static_node_property_[0].size()));
//           static_node_property_[0].insert(snp);
//           snp->set_name(*i);
//           snp->set_position(Vector2<float>(center.x+rad*cosf(count*deg), center.y+rad*sinf(count*deg))); ++count;
//           snp->set_texture(sociarium_project_texture::get_node_texture(*i));
//         } else {
//           unordered_map<wstring, Node*>::const_iterator j = name2node.find(*i);
//           if (j!=name2node.end()) continue;
//         }
//
//         Node* n = g->add_node();
//         name2node[*i] = n;
//         DynamicNodeProperty* dnp = g->property(n);
//         dnp->set_graph_element(n);
//         dnp->set_static_property(snp);
//         dnp->set_flag(ElementFlag::VISIBLE);
//         dnp->set_color_id(sociarium_project_color::get_default_node_color_id());
//         dnp->set_weight(1.0f);
//         dnp->set_size(sociarium_project_draw::get_node_size());
//         // 静的属性に動的属性を登録
//         snp->register_dynamic_property(dnp.get(), index_of_current_layer_);
//         assert(dnp->is_valid(snp));
//       }
//
//       for (vector<pair<pair<wstring, wstring>, double> >::const_iterator i=edge_list.begin(); i!=edge_list.end(); ++i) {
//         wstring const& name0 = g->is_directed()?i->first.first:(i->first.first<i->first.second?i->first.first:i->first.second);
//         wstring const& name1 = g->is_directed()?i->first.second:(i->first.first<i->first.second?i->first.second:i->first.first);
//         double const weight = i->second;
//         wstring const name = name0+L"~"+name1;
//         StaticEdgeProperty* sep = find_edge(0, name);
//
//         if (sep==0) {
//           sep.reset(new StaticEdgeProperty(static_edge_property_[0].size()));
//           static_edge_property_[0].insert(sep);
//           sep->set_name(name);
//           sep->set_texture(sociarium_project_texture::get_default_edge_texture());
//         } else {
//           unordered_map<wstring, Edge*>::const_iterator j = name2edge.find(name);
//           if (j!=name2edge.end()) {
//             g->property(j->second)->set_weight(float(weight));
//             continue;
//           }
//         }
//
//         Node* n0 = name2node[name0];
//         Node* n1 = name2node[name1];
//         assert(n0!=0);
//         assert(n1!=0);
//         Edge* e = g->add_edge(n0, n1);
//         DynamicEdgeProperty* dep = g->property(e);
//         dep->set_graph_element(e);
//         dep->set_static_property(sep);
//         dep->set_flag(ElementFlag::VISIBLE);
//         dep->set_color_id(sociarium_project_color::get_default_edge_color_id());
//         dep->set_weight(float(weight));
//         // 静的属性に動的属性を登録
//         sep->register_dynamic_property(dep.get(), index_of_current_layer_);
//         assert(dep->is_valid(sep));
//       }
//
//       sociarium_project_force_direction::should_be_updated();
//
//       // ----------------------------------------------------------------------------------------------------
//       write_to_read_lock();
//       // ----------------------------------------------------------------------------------------------------
//     }


  private:
    size_t index_of_current_layer_;
    vector<wstring> layer_name_;

    array<vector<shared_ptr<SociariumGraph> >, number_of_levels> graph_series_;
    array<StaticNodePropertySet, number_of_levels> static_node_property_;
    array<StaticEdgePropertySet, number_of_levels> static_edge_property_;
  };


  ////////////////////////////////////////////////////////////////////////////////
  namespace sociarium_project_graph_time_series {

    namespace {
      shared_ptr<SociariumGraphTimeSeries> time_series;
    }

    void initialize(void) {
      time_series.reset(new SociariumGraphTimeSeriesImpl());
    }

    void finalize(void) {
      time_series.reset();
    }

    shared_ptr<SociariumGraphTimeSeries> get(void) {
      return time_series;
    }

  } // The end of the namespace "sociarium_project_graph_time_series"

} // The end of the namespace "hashimoto_ut"

// s.o.c.i.a.r.i.u.m: sociarium_graph_time_series.h
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

#ifndef INCLUDE_GUARD_SOCIARIUM_PROJECT_SOCIARIUM_GRAPH_TIME_SERIES_H
#define INCLUDE_GUARD_SOCIARIUM_PROJECT_SOCIARIUM_GRAPH_TIME_SERIES_H

#include <vector>
#include <string>
#include <memory>
#include <unordered_set>
#include <windows.h>
#include "sociarium_graph.h"

namespace hashimoto_ut {

  class SociariumGraphTimeSeries {
  public:
    typedef std::tr1::unordered_set<
      StaticNodeProperty,
      static_property_hash<StaticNodeProperty>,
      static_property_compare<StaticNodeProperty> >
        StaticNodePropertySet;

    typedef std::tr1::unordered_set<
      StaticEdgeProperty,
      static_property_hash<StaticEdgeProperty>,
      static_property_compare<StaticEdgeProperty> >
        StaticEdgePropertySet;

    typedef StaticNodeProperty::DynamicPropertyMap DynamicNodePropertyMap;
    typedef StaticEdgeProperty::DynamicPropertyMap DynamicEdgePropertyMap;

    ////////////////////////////////////////////////////////////////////////////////
    virtual ~SociariumGraphTimeSeries() {}

    ////////////////////////////////////////////////////////////////////////////////
    // For exclusive control of access to the members.
    virtual void read_lock(void) const = 0;
    virtual void write_lock(void) const = 0;
    virtual void read_unlock(void) const = 0;
    virtual void write_unlock(void) const = 0;
    virtual void read_to_write_lock(void) const = 0;
    virtual void write_to_read_lock(void) const = 0;

    ////////////////////////////////////////////////////////////////////////////////
    virtual size_t number_of_layers(void) const = 0;
    virtual size_t index_of_current_layer(void) const = 0;
    virtual void move_layer(int index_of_layer) = 0;

    ////////////////////////////////////////////////////////////////////////////////
    virtual std::tr1::shared_ptr<SociariumGraph>
      get_graph(size_t level, size_t index_of_layer) const = 0;
    virtual std::wstring const& get_layer_name(size_t index_of_layer) const = 0;

    ////////////////////////////////////////////////////////////////////////////////
    virtual size_t number_of_static_nodes(size_t level) const = 0;
    virtual size_t number_of_static_edges(size_t level) const = 0;

    ////////////////////////////////////////////////////////////////////////////////
    virtual StaticNodePropertySet::const_iterator
      static_node_property_begin(size_t level) const = 0;
    virtual StaticNodePropertySet::const_iterator
      static_node_property_end(size_t level) const = 0;
    virtual StaticEdgePropertySet::const_iterator
      static_edge_property_begin(size_t level) const = 0;
    virtual StaticEdgePropertySet::const_iterator
      static_edge_property_end(size_t level) const = 0;

    ////////////////////////////////////////////////////////////////////////////////
    virtual void remove_static_node(StaticNodeProperty& snp) = 0;
    virtual void remove_static_edge(StaticEdgeProperty& sep) = 0;

    ////////////////////////////////////////////////////////////////////////////////
    virtual void remove_empty_static_nodes(size_t level) = 0;
    virtual void remove_empty_static_edges(size_t level) = 0;

    ////////////////////////////////////////////////////////////////////////////////
    virtual StaticNodePropertySet::iterator
      find_static_node(size_t level, std::wstring const& name) = 0;
    virtual StaticEdgePropertySet::iterator
      find_static_edge(size_t level, std::wstring const& name) = 0;
    virtual StaticNodePropertySet::const_iterator
      find_static_node(size_t level, std::wstring const& name) const = 0;
    virtual StaticEdgePropertySet::const_iterator
      find_static_edge(size_t level, std::wstring const& name) const = 0;

    virtual StaticNodePropertySet::iterator
      find_static_node(size_t level, size_t id) = 0;
    virtual StaticEdgePropertySet::iterator
      find_static_edge(size_t level, size_t id) = 0;
    virtual StaticNodePropertySet::const_iterator
      find_static_node(size_t level, size_t id) const = 0;
    virtual StaticEdgePropertySet::const_iterator
      find_static_edge(size_t level, size_t id) const = 0;

    ////////////////////////////////////////////////////////////////////////////////
    virtual void clear_community(void) = 0;

    ////////////////////////////////////////////////////////////////////////////////
    // The following functions are called after each thread completion.

    // GraphCreationThread
    virtual void update(
      std::vector<std::tr1::shared_ptr<SociariumGraph> >& graph_series,
      StaticNodePropertySet& static_node_property,
      StaticEdgePropertySet& static_edge_property,
      std::vector<std::wstring>& layer_name) = 0;

    // CommunityDetectionThread
    virtual void update_community(
      std::vector<std::tr1::shared_ptr<SociariumGraph> >& community_series,
      StaticNodePropertySet& static_node_property,
      StaticEdgePropertySet& static_edge_property) = 0;

    // LayoutThread
    virtual void update_node_position(
      size_t index_of_layer,
      std::vector<Vector2<double> > const& position) = 0;

    // NodeSizeUpdateThread
    virtual void update_node_size(
      std::vector<std::vector<double> > const& node_size) = 0;

    // EdgeWidthUpdateThread
    virtual void update_edge_width(
      std::vector<std::vector<double> > const& edge_width) = 0;

    //virtual void tamabi_update(std::vector<std::wstring> const& layer_name) = 0;
  };


  ////////////////////////////////////////////////////////////////////////////////
  struct TimeSeriesLock {
    enum { Read, Write };
    std::tr1::shared_ptr<SociariumGraphTimeSeries> ts_;
    int op_;

    TimeSeriesLock(std::tr1::shared_ptr<SociariumGraphTimeSeries> ts, int op) : ts_(ts), op_(op) {
      if (op_==Read) ts_->read_lock();
      else if (op_==Write) ts_->write_lock();
      else assert(0);
    }

    ~TimeSeriesLock() {
      if (op_==Read) ts_->read_unlock();
      else if (op_==Write) ts_->write_unlock();
      else assert(0);
    }
  };


  namespace sociarium_project_graph_time_series {

    ////////////////////////////////////////////////////////////////////////////////
    // Create a graph time-series.
    // This should be called when the world is created.
    void initialize(void);

    ////////////////////////////////////////////////////////////////////////////////
    // Delete a graph time-series.
    // This should be called when the world is destructed.
    void finalize(void);

    ////////////////////////////////////////////////////////////////////////////////
    std::tr1::shared_ptr<SociariumGraphTimeSeries> get(void);

  } // The end of the namespace "sociarium_project_graph_time_series"

} // The end of the namespace "hashimoto_ut"

#endif // INCLUDE_GUARD_SOCIARIUM_PROJECT_SOCIARIUM_GRAPH_TIME_SERIES_H

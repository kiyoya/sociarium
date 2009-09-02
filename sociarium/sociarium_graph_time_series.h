// s.o.c.i.a.r.i.u.m
// HASHIMOTO, Yasuhiro (E-mail: hy @ sys.t.u-tokyo.ac.jp)
// update: 2009/04/22

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
    typedef std::tr1::unordered_set<std::tr1::shared_ptr<StaticNodeProperty>, shared_ptr_hash<StaticNodeProperty> > StaticNodePropertyContainer;
    typedef std::tr1::unordered_set<std::tr1::shared_ptr<StaticEdgeProperty>, shared_ptr_hash<StaticEdgeProperty> > StaticEdgePropertyContainer;

    virtual ~SociariumGraphTimeSeries() {}

    virtual void read_lock(void) = 0;
    virtual void write_lock(void) = 0;
    virtual void read_unlock(void) = 0;
    virtual void write_unlock(void) = 0;
    virtual void read_to_write_lock(void) = 0;
    virtual void write_to_read_lock(void) = 0;

    virtual size_t number_of_layers(void) const = 0;
    virtual size_t index_of_current_layer(void) const = 0;
    virtual void move_layer(int index_of_layer) = 0;
    virtual std::tr1::shared_ptr<SociariumGraph> const& get_graph(size_t select, size_t index_of_layer) const = 0;
    virtual std::wstring const& get_layer_label(size_t index_of_layer) const = 0;

    virtual StaticNodePropertyContainer::const_iterator static_node_property_begin(size_t select) const = 0;
    virtual StaticNodePropertyContainer::const_iterator static_node_property_end(size_t select) const = 0;
    virtual StaticEdgePropertyContainer::const_iterator static_edge_property_begin(size_t select) const = 0;
    virtual StaticEdgePropertyContainer::const_iterator static_edge_property_end(size_t select) const = 0;

    virtual void remove_node(std::tr1::shared_ptr<StaticNodeProperty> const& snp) = 0;
    virtual void remove_edge(std::tr1::shared_ptr<StaticEdgeProperty> const& sep) = 0;
    virtual void remove_empty_static_property(size_t select) = 0;

    virtual StaticNodeProperty* find_node(size_t select, std::wstring const& name) const = 0;
    virtual StaticEdgeProperty* find_edge(size_t select, std::wstring const& name) const = 0;

    virtual void update_node_size(std::vector<std::vector<double> > const& size) = 0;
    virtual void update_node_size(int size_factor) = 0;
    virtual void update_node_position(size_t index_of_layer, std::vector<Vector2<double> > const& position) = 0;

    virtual void clear_community(void) = 0;

    virtual void update(
      std::vector<std::tr1::shared_ptr<SociariumGraph> >& graph_series,
      StaticNodePropertyContainer& static_node_property,
      StaticEdgePropertyContainer& static_edge_property,
      std::vector<std::wstring>& layer_label) = 0;

    virtual void update_community(
      std::vector<std::tr1::shared_ptr<SociariumGraph> >& community_series,
      StaticNodePropertyContainer& static_node_property,
      StaticEdgePropertyContainer& static_edge_property) = 0;

    static std::tr1::shared_ptr<SociariumGraphTimeSeries> create(void);

  protected:
    SociariumGraphTimeSeries(void) {}
  };

} // The end of the namespace "hashimoto_ut"

#endif // INCLUDE_GUARD_SOCIARIUM_PROJECT_SOCIARIUM_GRAPH_TIME_SERIES_H

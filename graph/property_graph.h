// C++ GRAPH LIBRARY
// HASHIMOTO, Yasuhiro (E-mail: hy @ sys.t.u-tokyo.ac.jp)
// update: 2009/03/25

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


#ifndef INCLUDE_GUARD_PROPERTY_GRAPH_H
#define INCLUDE_GUARD_PROPERTY_GRAPH_H

#include <cassert>
#include <algorithm>

#ifdef _MSC_VER
#include <unordered_map>
#else
#include <tr1/unordered_map>
#endif

#include "graph.h"

namespace hashimoto_ut {

  template <typename N, typename E>
  class PropertyGraph : public Graph {
  public:
    typedef N node_property;
    typedef E edge_property;
    typedef typename std::tr1::unordered_map<Node const*, std::tr1::shared_ptr<node_property> >::iterator node_property_iterator;
    typedef typename std::tr1::unordered_map<Edge const*, std::tr1::shared_ptr<edge_property> >::iterator edge_property_iterator;

  public:
    ~PropertyGraph() {}

    bool is_directed(void) const { return impl_->is_directed(); }
    void set_directed(bool directed) { impl_->set_directed(directed); }

    Node* add_node(void) {
      Node* n = impl_->add_node();
      node_properties_.insert(std::make_pair(n, std::tr1::shared_ptr<node_property>(new node_property())));
      return n;
    }

    Node* add_node(node_property const& property) {
      Node* n = impl_->add_node();
      node_properties_.insert(std::make_pair(n, std::tr1::shared_ptr<node_property>(new node_property(property))));
      return n;
    }

    Edge* add_edge(Node* source, Node* target) {
      Edge* e = impl_->add_edge(source, target);
      edge_properties_.insert(std::make_pair(e, std::tr1::shared_ptr<edge_property>(new edge_property())));
      return e;
    }

    Edge* add_edge(Node* source, Node* target, edge_property const& property) {
      Edge* e = impl_->add_edge(source, target);
      edge_properties_.insert(std::make_pair(e, std::tr1::shared_ptr<edge_property>(new edge_property(property))));
      return e;
    }

    void remove_node(Node* n) {
      for (adjacency_list_iterator i=n->obegin(), end=n->oend(); i!=end; ++i)
        edge_properties_.erase(*i);
      for (adjacency_list_iterator i=n->ibegin(), end=n->iend(); i!=end; ++i)
        if ((*i)->source()!=n) edge_properties_.erase(*i);
      node_properties_.erase(n);
      impl_->remove_node(n);
    }

    void remove_edge(Edge* e) {
      edge_properties_.erase(e);
      impl_->remove_edge(e);
    }

    void remove_nodes(node_iterator first, node_iterator last) {
      for (node_iterator i=first; i!=last; ++i) {
        for (adjacency_list_iterator j=(*first)->obegin(), end=(*first)->oend(); j!=end; ++j)
          edge_properties_.erase(*j);
        for (adjacency_list_iterator j=(*first)->ibegin(), end=(*first)->iend(); j!=end; ++j)
          if ((*j)->source()!=*first) edge_properties_.erase(*j);
        node_properties_.erase(*i);
      }
      impl_->remove_nodes(first, last);
    }

    void remove_edges(edge_iterator first, edge_iterator last) {
      for (edge_iterator i=first; i!=last; ++i) edge_properties_.erase(*i);
      impl_->remove_edges(first, last);
    }

    size_t nsize(void) const { return impl_->nsize(); }
    size_t esize(void) const { return impl_->esize(); }

    bool empty(void) const { return impl_->empty(); }

    void clear(void) {
      node_properties_.clear();
      edge_properties_.clear();
      impl_->clear();
    }

    node_iterator nbegin(void) const { return impl_->nbegin(); }
    node_iterator nend(void) const { return impl_->nend(); }
    edge_iterator ebegin(void) const { return impl_->ebegin(); }
    edge_iterator eend(void) const { return impl_->eend(); }

    node_reverse_iterator nrbegin(void) const { return impl_->nrbegin(); }
    node_reverse_iterator nrend(void) const { return impl_->nrend(); }
    edge_reverse_iterator erbegin(void) const { return impl_->erbegin(); }
    edge_reverse_iterator erend(void) const { return impl_->erend(); }

    Node* node(size_t index) const { return impl_->node(index); }
    Edge* edge(size_t index) const { return impl_->edge(index); }

    void change_source(Edge* e, Node* n) { impl_->change_source(e, n); }
    void change_target(Edge* e, Node* n) { impl_->change_target(e, n); }

    std::tr1::shared_ptr<Graph> copy_induced_subgraph(node_iterator first, node_iterator last) const {
      std::tr1::shared_ptr<Graph> g = impl_->copy_induced_subgraph(first, last);
      std::tr1::shared_ptr<PropertyGraph> retval(new PropertyGraph(g));
      for (node_iterator i=retval->nbegin(), end=retval->nend(); i!=end; ++i)
        retval->node_properties_.insert(std::make_pair(*i, std::tr1::shared_ptr<node_property>(new node_property)));
      for (edge_iterator i=retval->ebegin(), end=retval->eend(); i!=end; ++i)
        retval->edge_properties_.insert(std::make_pair(*i, std::tr1::shared_ptr<edge_property>(new edge_property)));
      return retval;
    }

    std::tr1::shared_ptr<Graph> split_induced_subgraph(node_iterator first, node_iterator last) {
      std::tr1::shared_ptr<Graph> g = impl_->split_induced_subgraph(first, last);
      std::tr1::shared_ptr<PropertyGraph> retval(new PropertyGraph(g));
      // Move node properties.
      for (node_iterator i=retval->nbegin(), end=retval->nend(); i!=end; ++i) {
        retval->node_properties_.insert(std::make_pair(*i, node_properties_[*i]));
        node_properties_.erase(*i);
      }
      // Move edge properties.
      std::tr1::unordered_map<Edge const*, std::tr1::shared_ptr<edge_property> > ep;
      for (edge_iterator i=ebegin(), end=eend(); i!=end; ++i)
        ep.insert(std::make_pair(*i, edge_properties_[*i]));
      for (edge_iterator i=retval->ebegin(), end=retval->eend(); i!=end; ++i)
        retval->edge_properties_.insert(std::make_pair(*i, edge_properties_[*i]));
      ep.swap(edge_properties_);
      return retval;
    }

    void merge(std::tr1::shared_ptr<Graph> const& g) {
      PropertyGraph* pg = dynamic_cast<PropertyGraph*>(g.get());
      assert(pg!=0);
      impl_->merge(g);
      node_properties_.insert(pg->node_properties_.begin(), pg->node_properties_.end());
      edge_properties_.insert(pg->edge_properties_.begin(), pg->edge_properties_.end());
      pg->node_properties_.clear();
      pg->edge_properties_.clear();
    }

    std::tr1::shared_ptr<Graph> clone(void) const {
      std::tr1::shared_ptr<PropertyGraph> retval(new PropertyGraph(is_directed()));
      return retval;
    }

    std::tr1::shared_ptr<node_property> const& property(Node const* n) const { return node_properties_[n]; }
    std::tr1::shared_ptr<edge_property> const& property(Edge const* e) const { return edge_properties_[e]; }

    std::tr1::shared_ptr<Graph> get_raw_graph(void) { return impl_; }
    std::tr1::shared_ptr<Graph const> get_raw_graph(void) const { return impl_; }

    void import_raw_graph(std::tr1::shared_ptr<Graph> const& g) {
      impl_ = g;
      node_properties_.clear();
      edge_properties_.clear();
      for (node_iterator i=nbegin(), end=nend(); i!=end; ++i)
        node_properties_.insert(std::make_pair(*i, std::tr1::shared_ptr<node_property>(new node_property())));
      for (edge_iterator i=ebegin(), end=eend(); i!=end; ++i)
        edge_properties_.insert(std::make_pair(*i, std::tr1::shared_ptr<edge_property>(new edge_property())));
    }

    node_property_iterator node_property_begin(void) const { return node_properties_.begin(); }
    node_property_iterator node_property_end(void) const { return node_properties_.end(); }

    edge_property_iterator edge_property_begin(void) const { return edge_properties_.begin(); }
    edge_property_iterator edge_property_end(void) const { return edge_properties_.end(); }

    static std::tr1::shared_ptr<PropertyGraph> create(bool directed=false) {
      return std::tr1::shared_ptr<PropertyGraph>(new PropertyGraph(directed));
    }

  private:
    PropertyGraph(bool directed) : impl_(Graph::create(directed)) {}
    PropertyGraph(std::tr1::shared_ptr<Graph> const& g) : impl_(g) {}

  private:
    std::tr1::shared_ptr<Graph> impl_;
    mutable std::tr1::unordered_map<Node const*, std::tr1::shared_ptr<node_property> > node_properties_;
    mutable std::tr1::unordered_map<Edge const*, std::tr1::shared_ptr<edge_property> > edge_properties_;
  };

} // The end of the namespace "hashimoto_ut"

#endif // INCLUDE_GUARD_PROPERTY_GRAPH_H

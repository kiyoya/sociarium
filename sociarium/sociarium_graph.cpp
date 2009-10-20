// s.o.c.i.a.r.i.u.m: sociarium_graph.cpp
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

#include <cassert>
#include "sociarium_graph.h"
#include "../shared/vector3.h"

namespace hashimoto_ut {

  using std::vector;
  using std::make_pair;
  using std::wstring;
  using std::tr1::unordered_map;


  ////////////////////////////////////////////////////////////////////////////////
  // DynamicPropertyBase

  template <typename GraphElement, typename StaticProperty>
  DynamicPropertyBase<GraphElement, StaticProperty>::~DynamicPropertyBase() {
    if (static_property_)
      static_property_->unregister_dynamic_property(
        static_cast<typename StaticProperty::DynamicProperty*>(this));
  }

  template <typename GraphElement, typename StaticProperty>
  GraphElement*
    DynamicPropertyBase<GraphElement, StaticProperty>::get_graph_element(void) const {
    assert(graph_element_!=0);
    return graph_element_;
  }

  template <typename GraphElement, typename StaticProperty>
  void DynamicPropertyBase<GraphElement, StaticProperty>::set_graph_element(
    GraphElement* graph_element) {
    graph_element_ = graph_element;
  }

  template <typename GraphElement, typename StaticProperty>
  StaticProperty*
    DynamicPropertyBase<GraphElement, StaticProperty>::get_static_property(void) const {
    assert(static_property_!=0);
    return static_property_;
  }

  template <typename GraphElement, typename StaticProperty>
  void DynamicPropertyBase<GraphElement, StaticProperty>::set_static_property(
    StaticProperty* static_property) {
    static_property_ = static_property;
  }

  template <typename GraphElement, typename StaticProperty>
  unsigned int DynamicPropertyBase<GraphElement, StaticProperty>::get_flag(void) const {
    return flag_;
  }

  template <typename GraphElement, typename StaticProperty>
  void DynamicPropertyBase<GraphElement, StaticProperty>::set_flag(unsigned int flag) {
    flag_ = flag;
  }

  template <typename GraphElement, typename StaticProperty>
  int DynamicPropertyBase<GraphElement, StaticProperty>::get_color_id(void) const {
    return color_id_;
  }

  template <typename GraphElement, typename StaticProperty>
  void DynamicPropertyBase<GraphElement, StaticProperty>::set_color_id(int color_id) {
    color_id_ = color_id;
  }

  template <typename GraphElement, typename StaticProperty>
  float DynamicPropertyBase<GraphElement, StaticProperty>::get_weight(void) const {
    return weight_;
  }

  template <typename GraphElement, typename StaticProperty>
  void DynamicPropertyBase<GraphElement, StaticProperty>::set_weight(float weight) {
    weight_ = weight;
  }

  template <typename GraphElement, typename StaticProperty>
  vector<DynamicNodeProperty*>::const_iterator
    DynamicPropertyBase<GraphElement, StaticProperty>::upper_nbegin(void) const {
      return upper_nodes_.begin();
    }

  template <typename GraphElement, typename StaticProperty>
  vector<DynamicNodeProperty*>::const_iterator
    DynamicPropertyBase<GraphElement, StaticProperty>::upper_nend(void) const {
      return upper_nodes_.end();
    }

  template <typename GraphElement, typename StaticProperty>
  size_t DynamicPropertyBase<GraphElement, StaticProperty>::number_of_upper_nodes(void) const {
    return upper_nodes_.size();
  }

  template <typename GraphElement, typename StaticProperty>
  void DynamicPropertyBase<GraphElement, StaticProperty>::register_upper_node(
    DynamicNodeProperty* dnp) {
    upper_nodes_.push_back(dnp);
  }

  template <typename GraphElement, typename StaticProperty>
  void DynamicPropertyBase<GraphElement, StaticProperty>::unregister_upper_node(
    DynamicNodeProperty* dnp) {
    for (size_t i=0; i<upper_nodes_.size(); ) {
      DynamicNodeProperty*& p_ref = upper_nodes_[i];
      if (p_ref==dnp) {
        p_ref = upper_nodes_.back();
        upper_nodes_.pop_back();
      } else ++i;
    }
  }

  template <typename GraphElement, typename StaticProperty>
  void DynamicPropertyBase<GraphElement, StaticProperty>::clear_upper_nodes(void) {
    // Remove this element from upper nodes.
    vector<DynamicNodeProperty*>::iterator i   = upper_nodes_.begin();
    vector<DynamicNodeProperty*>::iterator end = upper_nodes_.end();

    for (; i!=end; ++i)
      (*i)->unregister_lower_element(
        static_cast<typename StaticProperty::DynamicProperty*>(this));

    upper_nodes_.clear();
  }


  ////////////////////////////////////////////////////////////////////////////////
  // DynamicNodeProperty

  DynamicNodeProperty::~DynamicNodeProperty() {
    clear_lower_elements();
    clear_upper_nodes();
  }

  float DynamicNodeProperty::get_size(void) const {
    return size_;
  }

  void DynamicNodeProperty::set_size(float size) {
    size_ = size;
  }

  vector<DynamicNodeProperty*>::const_iterator
    DynamicNodeProperty::lower_nbegin(void) const {
    return lower_nodes_.begin();
  }

  vector<DynamicNodeProperty*>::const_iterator
    DynamicNodeProperty::lower_nend(void) const {
    return lower_nodes_.end();
  }

  vector<DynamicEdgeProperty*>::const_iterator
    DynamicNodeProperty::lower_ebegin(void) const {
    return lower_edges_.begin();
  }

  vector<DynamicEdgeProperty*>::const_iterator
    DynamicNodeProperty::lower_eend(void) const {
    return lower_edges_.end();
  }

  size_t DynamicNodeProperty::number_of_lower_nodes(void) const {
    return lower_nodes_.size();
  }

  size_t DynamicNodeProperty::number_of_lower_edges(void) const {
    return lower_edges_.size();
  }
  
  void DynamicNodeProperty::register_lower_element(DynamicNodeProperty* dnp) {
    lower_nodes_.push_back(dnp);
  }

  void DynamicNodeProperty::register_lower_element(DynamicEdgeProperty* dep) {
    lower_edges_.push_back(dep);
  }

  void DynamicNodeProperty::unregister_lower_element(DynamicNodeProperty* dnp) {
    for (size_t i=0; i<lower_nodes_.size(); ) {
      if (lower_nodes_[i]==dnp) {
        lower_nodes_[i] = lower_nodes_.back();
        lower_nodes_.pop_back();
      } else ++i;
    }
  }

  void DynamicNodeProperty::unregister_lower_element(DynamicEdgeProperty* dep) {
    for (size_t i=0; i<lower_edges_.size(); ) {
      if (lower_edges_[i]==dep) {
        lower_edges_[i] = lower_edges_.back();
        lower_edges_.pop_back();
      } else ++i;
    }
  }

  void DynamicNodeProperty::clear_lower_elements(void) {
    {
      // Remove this element from lower nodes.
      vector<DynamicNodeProperty*>::iterator i   = lower_nodes_.begin();
      vector<DynamicNodeProperty*>::iterator end = lower_nodes_.end();
      for (; i!=end; ++i) (*i)->unregister_upper_node(this);
    }{
      // Remove this element from lower edges.
      vector<DynamicEdgeProperty*>::iterator i   = lower_edges_.begin();
      vector<DynamicEdgeProperty*>::iterator end = lower_edges_.end();
      for (; i!=end; ++i) (*i)->unregister_upper_node(this);
    }

    lower_nodes_.clear();
    lower_edges_.clear();
  }


  ////////////////////////////////////////////////////////////////////////////////
  // DynamicEdgeProperty

  DynamicEdgeProperty::~DynamicEdgeProperty() {
    clear_upper_nodes();
  }

  float DynamicEdgeProperty::get_width(void) const {
    return width_;
  }

  void DynamicEdgeProperty::set_width(float width) {
    width_ = width;
  }


  ////////////////////////////////////////////////////////////////////////////////
  // StaticPropertyBase

  template <typename DynamicProperty>
  StaticPropertyBase<DynamicProperty>::~StaticPropertyBase() {
    typename DynamicPropertyMap::iterator i   = dynamic_property_.begin();
    typename DynamicPropertyMap::iterator end = dynamic_property_.end();
    for (; i!=end; ++i)
      i->first->set_static_property(0);
  }

  template <typename DynamicProperty>
  size_t StaticPropertyBase<DynamicProperty>::get_id(void) const {
    return id_;
  }

  template <typename DynamicProperty>
  wstring const& StaticPropertyBase<DynamicProperty>::get_identifier(void) const {
    return identifier_;
  }

  template <typename DynamicProperty>
  void StaticPropertyBase<DynamicProperty>::set_identifier(wstring const& identifier) {
    identifier_ = identifier;
  }

  template <typename DynamicProperty>
  wstring const& StaticPropertyBase<DynamicProperty>::get_name(void) const {
    return name_;
  }

  template <typename DynamicProperty>
  void StaticPropertyBase<DynamicProperty>::set_name(wstring const& name) {
    name_ = name;
  }

  template <typename DynamicProperty>
  unsigned int StaticPropertyBase<DynamicProperty>::get_flag(void) const {
    return flag_;
  }

  template <typename DynamicProperty>
  void StaticPropertyBase<DynamicProperty>::set_flag(unsigned int flag) {
    flag_ = flag;
  }

  template <typename DynamicProperty>
  Texture const* StaticPropertyBase<DynamicProperty>::get_texture(void) const {
    return texture_;
  }

  template <typename DynamicProperty>
  void StaticPropertyBase<DynamicProperty>::set_texture(Texture const* texture) {
    texture_ = texture;
  }

  template <typename DynamicProperty>
  typename unordered_map<DynamicProperty*, size_t>::const_iterator
    StaticPropertyBase<DynamicProperty>::dynamic_property_begin(void) const {
      return dynamic_property_.begin();
    }

  template <typename DynamicProperty>
  typename unordered_map<DynamicProperty*, size_t>::const_iterator
    StaticPropertyBase<DynamicProperty>::dynamic_property_end(void) const {
      return dynamic_property_.end();
    }

  template <typename DynamicProperty>
  size_t StaticPropertyBase<DynamicProperty>::number_of_dynamic_properties(void) const {
    return dynamic_property_.size();
  }

  template <typename DynamicProperty>
  void StaticPropertyBase<DynamicProperty>::register_dynamic_property(
    DynamicProperty* dp, size_t layer) {
    dynamic_property_.insert(make_pair(dp, layer));
  }

  template <typename DynamicProperty>
  void StaticPropertyBase<DynamicProperty>::unregister_dynamic_property(
    DynamicProperty* dp) {
    typename DynamicPropertyMap::iterator i = dynamic_property_.find(dp);
    assert(i!=dynamic_property_.end());
    dynamic_property_.erase(i);
  }


  ////////////////////////////////////////////////////////////////////////////////
  // StaticNodeProperty

  Vector2<float> const& StaticNodeProperty::get_position(void) const {
    return position_;
  }

  void StaticNodeProperty::set_position(Vector2<float> const& position) {
    position_ = position;
  }


  ////////////////////////////////////////////////////////////////////////////////
  // StaticEdgeProperty

  /* no implementation.
   */


  // Explicit instantiation of template specialization.
  template class DynamicPropertyBase<Node, StaticNodeProperty>;
  template class DynamicPropertyBase<Edge, StaticEdgeProperty>;
  template class StaticPropertyBase<DynamicNodeProperty>;
  template class StaticPropertyBase<DynamicEdgeProperty>;

} // The end of the namespace "hashimoto_ut"

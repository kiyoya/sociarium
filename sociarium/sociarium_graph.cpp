// s.o.c.i.a.r.i.u.m
// HASHIMOTO, Yasuhiro (E-mail: hy @ sys.t.u-tokyo.ac.jp)
// update: 2009/05/10

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

#include <cassert>
#include "sociarium_graph.h"
#include "../shared/vector3.h"

namespace hashimoto_ut {

  using std::vector;
  using std::make_pair;
  using std::wstring;
  using std::tr1::shared_ptr;
  using std::tr1::unordered_map;

  // テンプレートクラスの明示的なインスタンス化
  template class DynamicPropertyBase<Node, StaticNodeProperty>;
  template class DynamicPropertyBase<Edge, StaticEdgeProperty>;
  template class StaticPropertyBase<DynamicNodeProperty>;
  template class StaticPropertyBase<DynamicEdgeProperty>;

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // 動的属性：ノードとエッジ共通

  template <typename GraphElement, typename StaticProperty>
  bool DynamicPropertyBase<GraphElement, StaticProperty>::is_valid(shared_ptr<StaticProperty> const& static_property) const {
    return graph_element_!=0 && static_property_==static_property;
  }

  template <typename GraphElement, typename StaticProperty>
  GraphElement* DynamicPropertyBase<GraphElement, StaticProperty>::get_graph_element(void) const {
    return graph_element_;
  }

  template <typename GraphElement, typename StaticProperty>
  void DynamicPropertyBase<GraphElement, StaticProperty>::set_graph_element(GraphElement* graph_element) {
    graph_element_ = graph_element;
  }

  template <typename GraphElement, typename StaticProperty>
  shared_ptr<StaticProperty> const& DynamicPropertyBase<GraphElement, StaticProperty>::get_static_property(void) const {
    assert(static_property_!=0);
    return static_property_;
  }

  template <typename GraphElement, typename StaticProperty>
  void DynamicPropertyBase<GraphElement, StaticProperty>::set_static_property(shared_ptr<StaticProperty> const& static_property) {
    static_property_ = static_property;
  }

  template <typename GraphElement, typename StaticProperty>
  unsigned int DynamicPropertyBase<GraphElement, StaticProperty>::get_flag(void) const {
    return flag_;
  }

  template <typename GraphElement, typename StaticProperty>
  void DynamicPropertyBase<GraphElement, StaticProperty>::set_flag(unsigned int value) {
    flag_ = value;
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
  wstring const& DynamicPropertyBase<GraphElement, StaticProperty>::get_text(void) const {
    return text_;
  }

  template <typename GraphElement, typename StaticProperty>
  void DynamicPropertyBase<GraphElement, StaticProperty>::set_text(wstring const& text) {
    text_ = text;
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
  std::vector<DynamicNodeProperty*>::const_iterator DynamicPropertyBase<GraphElement, StaticProperty>::upper_nbegin(void) const {
    return upper_nodes_.begin();
  }

  template <typename GraphElement, typename StaticProperty>
  std::vector<DynamicNodeProperty*>::const_iterator DynamicPropertyBase<GraphElement, StaticProperty>::upper_nend(void) const {
    return upper_nodes_.end();
  }

  template <typename GraphElement, typename StaticProperty>
  size_t DynamicPropertyBase<GraphElement, StaticProperty>::number_of_upper_nodes(void) const {
    return upper_nodes_.size();
  }

  template <typename GraphElement, typename StaticProperty>
  void DynamicPropertyBase<GraphElement, StaticProperty>::register_upper_element(DynamicNodeProperty* dnp) {
    upper_nodes_.push_back(dnp);
  }

  template <typename GraphElement, typename StaticProperty>
  void DynamicPropertyBase<GraphElement, StaticProperty>::clear_upper_nodes(void) {
    upper_nodes_.clear();
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // 動的属性：ノード

  DynamicNodeProperty::~DynamicNodeProperty() {
    clear_lower_elements();
    clear_upper_nodes();
    if (static_property_) {
      unordered_map<DynamicNodeProperty*, size_t>& m = static_property_->dynamic_property_;
      unordered_map<DynamicNodeProperty*, size_t>::iterator pos = m.find(this);
      assert(pos!=m.end());
      m.erase(pos);
    }
  }

  float DynamicNodeProperty::get_size(void) const {
    return size_;
  }

  void DynamicNodeProperty::set_size(float size) {
    size_ = size;
  }

  void DynamicNodeProperty::register_lower_element(DynamicNodeProperty* dnp) {
    lower_nodes_.push_back(dnp);
  }

  void DynamicNodeProperty::register_lower_element(DynamicEdgeProperty* dep) {
    lower_edges_.push_back(dep);
  }

  vector<DynamicNodeProperty*>::const_iterator DynamicNodeProperty::lower_nbegin(void) const {
    return lower_nodes_.begin();
  }

  vector<DynamicNodeProperty*>::const_iterator DynamicNodeProperty::lower_nend(void) const {
    return lower_nodes_.end();
  }

  vector<DynamicEdgeProperty*>::const_iterator DynamicNodeProperty::lower_ebegin(void) const {
    return lower_edges_.begin();
  }

  vector<DynamicEdgeProperty*>::const_iterator DynamicNodeProperty::lower_eend(void) const {
    return lower_edges_.end();
  }

  size_t DynamicNodeProperty::number_of_lower_nodes(void) const {
    return lower_nodes_.size();
  }

  size_t DynamicNodeProperty::number_of_lower_edges(void) const {
    return lower_edges_.size();
  }
  
  void DynamicNodeProperty::clear_lower_elements(void) {
    for (vector<DynamicNodeProperty*>::iterator i=lower_nodes_.begin(), end=lower_nodes_.end(); i!=end; ++i) {
      for (size_t j=0; j<(*i)->upper_nodes_.size(); ) {
        if ((*i)->upper_nodes_[j]==this) {
          (*i)->upper_nodes_[j] = (*i)->upper_nodes_.back();
          (*i)->upper_nodes_.pop_back();
        } else ++j;
      }
    }
    for (vector<DynamicEdgeProperty*>::iterator i=lower_edges_.begin(), end=lower_edges_.end(); i!=end; ++i) {
      for (size_t j=0; j<(*i)->upper_nodes_.size(); ) {
        if ((*i)->upper_nodes_[j]==this) {
          (*i)->upper_nodes_[j] = (*i)->upper_nodes_.back();
          (*i)->upper_nodes_.pop_back();
        } else ++j;
      }
    }
    lower_nodes_.clear();
    lower_edges_.clear();
  }

  void DynamicNodeProperty::clear_upper_nodes(void) {
    for (vector<DynamicNodeProperty*>::iterator i=upper_nodes_.begin(), end=upper_nodes_.end(); i!=end; ++i) {
      for (size_t j=0; j<(*i)->lower_nodes_.size(); ) {
        if ((*i)->lower_nodes_[j]==this) {
          (*i)->lower_nodes_[j] = (*i)->lower_nodes_.back();
          (*i)->lower_nodes_.pop_back();
        } else ++j;
      }
    }
    upper_nodes_.clear();
  }

  void DynamicNodeProperty::move_to_center_of_lower_nodes_position(void) {
    Vector2<float> pos;
    int number_of_visible_members = 0;
    for (vector<DynamicNodeProperty*>::const_iterator i=lower_nbegin(); i!=lower_nend(); ++i) {
      if (is_visible(*i)) {
        pos += (*i)->get_static_property()->get_position();
        ++number_of_visible_members;
      }
    }
    if (number_of_visible_members>0) {
      pos /= float(number_of_visible_members);
      static_property_->set_position(pos);
    }
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // 動的属性：エッジ

  DynamicEdgeProperty::~DynamicEdgeProperty() {
    clear_upper_nodes();
    if (static_property_) {
      unordered_map<DynamicEdgeProperty*, size_t>& m = static_property_->dynamic_property_;
      unordered_map<DynamicEdgeProperty*, size_t>::iterator pos = m.find(this);
      assert(pos!=m.end());
      m.erase(pos);
    }
  }

  float DynamicEdgeProperty::get_length(void) const {
    return length_;
  }

  void DynamicEdgeProperty::set_length(float length) {
    length_ = length;
  }

  void DynamicEdgeProperty::clear_upper_nodes(void) {
    for (vector<DynamicNodeProperty*>::iterator i=upper_nodes_.begin(), end=upper_nodes_.end(); i!=end; ++i) {
      for (size_t j=0; j<(*i)->lower_edges_.size(); ) {
        if ((*i)->lower_edges_[j]==this) {
          (*i)->lower_edges_[j] = (*i)->lower_edges_.back();
          (*i)->lower_edges_.pop_back();
        } else ++j;
      }
    }
    upper_nodes_.clear();
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // 静的属性：ノードとエッジ共通

  template <typename DynamicProperty>
  size_t StaticPropertyBase<DynamicProperty>::get_id(void) const {
    return id_;
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
  GLTexture const* StaticPropertyBase<DynamicProperty>::get_texture(void) const {
    return texture_;
  }

  template <typename DynamicProperty>
  void StaticPropertyBase<DynamicProperty>::set_texture(GLTexture const* texture) {
    texture_ = texture;
  }

  template <typename DynamicProperty>
  void StaticPropertyBase<DynamicProperty>::add_dynamic_property(DynamicProperty* dp, size_t index) {
    dynamic_property_.insert(make_pair(dp, index));
  }

  template <typename DynamicProperty>
  size_t StaticPropertyBase<DynamicProperty>::number_of_dynamic_properties(void) const {
    return dynamic_property_.size();
  }

  template <typename DynamicProperty>
  typename unordered_map<DynamicProperty*, size_t>::const_iterator StaticPropertyBase<DynamicProperty>::dynamic_property_begin(void) const {
    return dynamic_property_.begin();
  }

  template <typename DynamicProperty>
  typename unordered_map<DynamicProperty*, size_t>::const_iterator StaticPropertyBase<DynamicProperty>::dynamic_property_end(void) const {
    return dynamic_property_.end();
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // 静的属性：ノード

  Vector2<float> const& StaticNodeProperty::get_position(void) const {
    return position_;
  }

  void StaticNodeProperty::set_position(Vector2<float> const& position) {
    position_ = position;
  }

} // The end of the namespace "hashimoto_ut"

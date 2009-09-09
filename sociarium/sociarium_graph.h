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

#ifndef INCLUDE_GUARD_SOCIARIUM_PROJECT_SOCIARIUM_GRAPH_H
#define INCLUDE_GUARD_SOCIARIUM_PROJECT_SOCIARIUM_GRAPH_H

#include <vector>
#include <string>
#include <memory>
#ifdef _MSC_VER
#include <unordered_map>
#else
#include <tr1/unordered_map>
#endif
#include "../graph/property_graph.h"
#include "../graph/util/traverser.h"
#include "../shared/vector2.h"

namespace hashimoto_ut {

  class GLTexture;
  class DynamicNodeProperty;
  class DynamicEdgeProperty;
  class StaticNodeProperty;
  class StaticEdgeProperty;

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // 動的属性：ノードとエッジ共通
  template <typename GraphElement, typename StaticProperty>
  class DynamicPropertyBase {
  public:
    ~DynamicPropertyBase() {}
    bool is_valid(const std::tr1::shared_ptr<StaticProperty>& static_property) const;
    GraphElement* get_graph_element(void) const;
    void set_graph_element(GraphElement* graph_element);
    std::tr1::shared_ptr<StaticProperty> const& get_static_property(void) const;
    void set_static_property(const std::tr1::shared_ptr<StaticProperty>& static_property);
    unsigned int get_flag(void) const;
    void set_flag(unsigned int value);
    int get_color_id(void) const;
    void set_color_id(int color_id);
    std::wstring const& get_text(void) const;
    void set_text(std::wstring const& text);
    float get_weight(void) const;
    void set_weight(float weight);
    std::vector<DynamicNodeProperty*>::const_iterator upper_nbegin(void) const;
    std::vector<DynamicNodeProperty*>::const_iterator upper_nend(void) const;
    size_t number_of_upper_nodes(void) const;
    void register_upper_element(DynamicNodeProperty* dnp);
    void clear_upper_nodes(void);

  protected:
    DynamicPropertyBase(void) {}

  private:
    GraphElement* graph_element_;
    std::tr1::shared_ptr<StaticProperty> static_property_;
    unsigned int flag_;
    int color_id_;
    std::wstring text_;
    float weight_;

    friend class DynamicNodeProperty;
    friend class DynamicEdgeProperty;
    std::vector<DynamicNodeProperty*> upper_nodes_;
  };

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // 動的属性：ノード
  class DynamicNodeProperty : public DynamicPropertyBase<Node, StaticNodeProperty> {
  public:
    typedef DynamicPropertyBase<Node, StaticNodeProperty> Base;
    typedef Node GraphElement;
    typedef node_iterator GraphElementIterator;
    typedef StaticNodeProperty StaticProperty;

    DynamicNodeProperty(void) {}
    ~DynamicNodeProperty();
    float get_size(void) const;
    void set_size(float size);

    void register_lower_element(DynamicNodeProperty* dnp);
    void register_lower_element(DynamicEdgeProperty* dep);
    std::vector<DynamicNodeProperty*>::const_iterator lower_nbegin(void) const;
    std::vector<DynamicNodeProperty*>::const_iterator lower_nend(void) const;
    std::vector<DynamicEdgeProperty*>::const_iterator lower_ebegin(void) const;
    std::vector<DynamicEdgeProperty*>::const_iterator lower_eend(void) const;
    size_t number_of_lower_nodes(void) const;
    size_t number_of_lower_edges(void) const;
    void clear_lower_elements(void);
    void clear_upper_nodes(void);
    void move_to_center_of_lower_nodes_position(void);

  private:
    float size_;

    friend class DynamicEdgeProperty;
    std::vector<DynamicNodeProperty*> lower_nodes_;
    std::vector<DynamicEdgeProperty*> lower_edges_;
  };

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // 動的属性：エッジ
  class DynamicEdgeProperty : public DynamicPropertyBase<Edge, StaticEdgeProperty> {
  public:
    typedef DynamicPropertyBase<Edge, StaticEdgeProperty> Base;
    typedef Edge GraphElement;
    typedef edge_iterator GraphElementIterator;
    typedef StaticEdgeProperty StaticProperty;

    DynamicEdgeProperty(void) {}
    ~DynamicEdgeProperty();
    float get_length(void) const;
    void set_length(float length);
    void clear_upper_nodes(void);

  private:
    float length_;
  };

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // 静的属性：ノードとエッジ共通
  template <typename DynamicProperty>
  class StaticPropertyBase {
  public:
    ~StaticPropertyBase() {}
    size_t get_id(void) const;
    std::wstring const& get_name(void) const;
    void set_name(std::wstring const& name);
    GLTexture const* get_texture(void) const;
    void set_texture(GLTexture const* texture);
    void add_dynamic_property(DynamicProperty* dp, size_t index);
    size_t number_of_dynamic_properties(void) const;
    typename std::tr1::unordered_map<DynamicProperty*, size_t>::const_iterator dynamic_property_begin(void) const;
    typename std::tr1::unordered_map<DynamicProperty*, size_t>::const_iterator dynamic_property_end(void) const;

  protected:
    StaticPropertyBase(size_t id) : id_(id) {}

  private:
    size_t const id_;
    std::wstring name_;
    GLTexture const* texture_;

    friend class DynamicNodeProperty;
    friend class DynamicEdgeProperty;
    std::tr1::unordered_map<DynamicProperty*, size_t> dynamic_property_;
  };

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // 静的属性：ノード
  class StaticNodeProperty : public StaticPropertyBase<DynamicNodeProperty> {
  public:
    typedef StaticPropertyBase<DynamicNodeProperty> Base;
    typedef DynamicNodeProperty DynamicProperty;

    StaticNodeProperty(size_t id) : Base(id) {}
    ~StaticNodeProperty() {}
    Vector2<float> const& get_position(void) const;
    void set_position(Vector2<float> const& position);

  private:
    Vector2<float> position_;
  };

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // 静的属性：エッジ
  class StaticEdgeProperty : public StaticPropertyBase<DynamicEdgeProperty> {
  public:
    typedef StaticPropertyBase<DynamicEdgeProperty> Base;
    typedef DynamicEdgeProperty DynamicProperty;

    StaticEdgeProperty(size_t id) : Base(id) {}
    ~StaticEdgeProperty() {}
  };

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // sociariumグラフの定義
  typedef PropertyGraph<DynamicNodeProperty, DynamicEdgeProperty> SociariumGraph;

  template <typename T> struct DynamicProperty;
  template <> struct DynamicProperty<Node> { typedef DynamicNodeProperty type; };
  template <> struct DynamicProperty<Edge> { typedef DynamicEdgeProperty type; };

  template <typename T> struct StaticProperty;
  template <> struct StaticProperty<Node> { typedef StaticNodeProperty type; };
  template <> struct StaticProperty<Edge> { typedef StaticEdgeProperty type; };

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // フラグ
  struct ElementFlag {
    enum _ {
      VISIBLE            = 0x01, // 可視
      CAPTURED           = 0x02, // マウスによるキャプチャ
      MARKED             = 0x04, // 選択
      TEMPORARY_MARKED   = 0x08, // マウスドラッグエリア内
      TEMPORARY_UNMARKED = 0x10, // マウスドラッグエリア内
      HIGHLIGHT          = 0x20, // 多用途
      MASK               = 0x3f  // 全て
    };
  };

  template <typename T> bool is_visible(T const& dp) { return dp->get_flag()&ElementFlag::VISIBLE; }
  template <typename T> bool is_hidden(T const& dp) { return (dp->get_flag()&ElementFlag::VISIBLE)==0; }
  template <typename T> bool is_marked(T const& dp) { return (dp->get_flag()&ElementFlag::MARKED)!=0; }
  template <typename T> bool is_temporary_marked(T const& dp) { return (dp->get_flag()&ElementFlag::TEMPORARY_MARKED)!=0; }
  template <typename T> bool is_temporary_unmarked(T const& dp) { return (dp->get_flag()&ElementFlag::TEMPORARY_UNMARKED)!=0; }

  struct AvoidHiddenElements : ConditionalPass {
    AvoidHiddenElements(std::tr1::shared_ptr<SociariumGraph const> const& g) : nflag_(g->nsize(), 0), eflag_(g->esize(), 0) {
      for (SociariumGraph::node_property_iterator i=g->node_property_begin(), end=g->node_property_end(); i!=end; ++i)
        if (is_hidden(i->second)) nflag_[i->first->index()] = 1;
      for (SociariumGraph::edge_property_iterator i=g->edge_property_begin(), end=g->edge_property_end(); i!=end; ++i)
        if (is_hidden(i->second)) eflag_[i->first->index()] = 1;
    }
    int operator()(Edge const* e, Node const* n) const {
      return (nflag_[n->index()]==0 && eflag_[e->index()]==0)?PASS:CLOSED;
    }
    std::vector<int> nflag_;
    std::vector<int> eflag_;
  };

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // unordered_set<shared_ptr<T> >用のハッシュ関数
  template <typename T>
  struct shared_ptr_hash : std::unary_function<std::tr1::shared_ptr<T>, size_t> {
    size_t operator()(std::tr1::shared_ptr<T> const& value) const {
      std::pair<long, long> q = std::make_pair(long(size_t(value.get())), 127773L);
      q.second = 16807*q.second-2836*q.first;
      if (q.second<0) q.second += 2147483647;
      return size_t(q.second);
    }
  };
} // The end of the namespace "hashimoto_ut"

#endif // INCLUDE_GUARD_SOCIARIUM_PROJECT_SOCIARIUM_GRAPH_H

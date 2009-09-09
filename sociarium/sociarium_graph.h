// s.o.c.i.a.r.i.u.m: sociarium_graph.h
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

#ifndef INCLUDE_GUARD_SOCIARIUM_PROJECT_SOCIARIUM_GRAPH_H
#define INCLUDE_GUARD_SOCIARIUM_PROJECT_SOCIARIUM_GRAPH_H

#include <cassert>
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


  ////////////////////////////////////////////////////////////////////////////////
  // DynamicPropertyBase
  template <typename GraphElement, typename StaticProperty>
  class DynamicPropertyBase {
  public:
    ~DynamicPropertyBase();

    GraphElement* get_graph_element(void) const;
    void set_graph_element(GraphElement* graph_element);

    StaticProperty* get_static_property(void) const;
    void set_static_property(StaticProperty* static_property);

    unsigned int get_flag(void) const;
    void set_flag(unsigned int flag);

    int get_color_id(void) const;
    void set_color_id(int color_id);

    float get_weight(void) const;
    void set_weight(float weight);

    std::vector<DynamicNodeProperty*>::const_iterator upper_nbegin(void) const;
    std::vector<DynamicNodeProperty*>::const_iterator upper_nend(void) const;

    size_t number_of_upper_nodes(void) const;

    void register_upper_node(DynamicNodeProperty* dnp);
    void unregister_upper_node(DynamicNodeProperty* dnp);
    void clear_upper_nodes(void);

  protected:
    DynamicPropertyBase(void)
         : graph_element_(0), static_property_(0),
           flag_(0), color_id_(0), weight_(0) {}

  private:
    GraphElement* graph_element_;
    StaticProperty* static_property_;
    unsigned int flag_;
    int color_id_;
    float weight_;
    std::vector<DynamicNodeProperty*> upper_nodes_;
  };


  ////////////////////////////////////////////////////////////////////////////////
  // DynamicNodeProperty
  class DynamicNodeProperty :
    public DynamicPropertyBase<Node, StaticNodeProperty> {

  public:
    typedef DynamicPropertyBase<Node, StaticNodeProperty> Base;
    typedef Node GraphElement;
    typedef node_iterator GraphElementIterator;
    typedef StaticNodeProperty StaticProperty;

    DynamicNodeProperty(void) {}
    ~DynamicNodeProperty();

    float get_size(void) const;
    void set_size(float size);

    std::vector<DynamicNodeProperty*>::const_iterator lower_nbegin(void) const;
    std::vector<DynamicNodeProperty*>::const_iterator lower_nend(void) const;
    std::vector<DynamicEdgeProperty*>::const_iterator lower_ebegin(void) const;
    std::vector<DynamicEdgeProperty*>::const_iterator lower_eend(void) const;

    size_t number_of_lower_nodes(void) const;
    size_t number_of_lower_edges(void) const;

    void register_lower_element(DynamicNodeProperty* dnp);
    void register_lower_element(DynamicEdgeProperty* dep);
    void unregister_lower_element(DynamicNodeProperty* dnp);
    void unregister_lower_element(DynamicEdgeProperty* dep);
    void clear_lower_elements(void);

    mutable int ivar;
    mutable float fvar;

  private:
    float size_;
    std::vector<DynamicNodeProperty*> lower_nodes_;
    std::vector<DynamicEdgeProperty*> lower_edges_;
  };


  ////////////////////////////////////////////////////////////////////////////////
  // DynamicEdgeProperty
  class DynamicEdgeProperty :
    public DynamicPropertyBase<Edge, StaticEdgeProperty> {

  public:
    typedef DynamicPropertyBase<Edge, StaticEdgeProperty> Base;
    typedef Edge GraphElement;
    typedef edge_iterator GraphElementIterator;
    typedef StaticEdgeProperty StaticProperty;

    DynamicEdgeProperty(void) {}
    ~DynamicEdgeProperty();

    float get_width(void) const;
    void set_width(float width);

  private:
    float width_;
  };


  ////////////////////////////////////////////////////////////////////////////////
  // StaticPropertyBase
  template <typename DynamicProperty>
  class StaticPropertyBase {
  public:
    typedef typename std::tr1::unordered_map<DynamicProperty*, size_t>
      DynamicPropertyMap;

    ~StaticPropertyBase();

    size_t get_id(void) const;

    std::wstring const& get_identifier(void) const;
    void set_identifier(std::wstring const& identifier);

    std::wstring const& get_name(void) const;
    void set_name(std::wstring const& name);

    unsigned int get_flag(void) const;
    void set_flag(unsigned int flag);

    GLTexture const* get_texture(void) const;
    void set_texture(GLTexture const* texture);

    typename DynamicPropertyMap::const_iterator dynamic_property_begin(void) const;
    typename DynamicPropertyMap::const_iterator dynamic_property_end(void) const;

    size_t number_of_dynamic_properties(void) const;

    void register_dynamic_property(DynamicProperty* dp, size_t layer);
    void unregister_dynamic_property(DynamicProperty* dp);

  protected:
    StaticPropertyBase(size_t id) : id_(id), flag_(0), texture_(0) {}

  private:
    size_t const id_;
    std::wstring identifier_;
    std::wstring name_;
    unsigned flag_;
    GLTexture const* texture_;
    DynamicPropertyMap dynamic_property_;
  };


  ////////////////////////////////////////////////////////////////////////////////
  // StaticNodeProperty
  class StaticNodeProperty :
    public StaticPropertyBase<DynamicNodeProperty> {

  public:
    typedef StaticPropertyBase<DynamicNodeProperty> Base;
    typedef DynamicNodeProperty DynamicProperty;
    typedef Base::DynamicPropertyMap DynamicPropertyMap;

    StaticNodeProperty(size_t id) : Base(id) {}
    ~StaticNodeProperty() {}

    Vector2<float> const& get_position(void) const;
    void set_position(Vector2<float> const& position);

  private:
    Vector2<float> position_;
  };


  ////////////////////////////////////////////////////////////////////////////////
  // StaticEdgeProperty
  class StaticEdgeProperty
       : public StaticPropertyBase<DynamicEdgeProperty> {

  public:
    typedef StaticPropertyBase<DynamicEdgeProperty> Base;
    typedef DynamicEdgeProperty DynamicProperty;
    typedef Base::DynamicPropertyMap DynamicPropertyMap;

    StaticEdgeProperty(size_t id) : Base(id) {}
    ~StaticEdgeProperty() {}
  };


  ////////////////////////////////////////////////////////////////////////////////
  // SociariumGraph
  typedef PropertyGraph<DynamicNodeProperty, DynamicEdgeProperty> SociariumGraph;


  ////////////////////////////////////////////////////////////////////////////////
  template <typename T> struct DynamicProperty;
  template <> struct DynamicProperty<Node> { typedef DynamicNodeProperty type; };
  template <> struct DynamicProperty<Edge> { typedef DynamicEdgeProperty type; };

  template <typename T> struct StaticProperty;
  template <> struct StaticProperty<Node> { typedef StaticNodeProperty type; };
  template <> struct StaticProperty<Edge> { typedef StaticEdgeProperty type; };


  ////////////////////////////////////////////////////////////////////////////////
  template <typename DynamicProperty>
  DynamicProperty& link_dynamic_property_and_graph_element(
    std::tr1::shared_ptr<SociariumGraph> g,
    typename DynamicProperty::GraphElement* em) {
    DynamicProperty& retval = g->property(em);
    retval.set_graph_element(em);
    return retval;
  }

  template <typename DynamicProperty>
  void link_dynamic_and_static_properties(
    size_t layer, DynamicProperty* dp,
    typename DynamicProperty::StaticProperty* sp) {
    dp->set_static_property(sp);
    sp->register_dynamic_property(dp, layer);
  }


  ////////////////////////////////////////////////////////////////////////////////
  struct ElementFlag {
    enum _ {
      VISIBLE            = 0x01,
      CAPTURED           = 0x02,
      MARKED             = 0x04,
      TEMPORARY_MARKED   = 0x08,
      TEMPORARY_UNMARKED = 0x10,
      HIGHLIGHT          = 0x20,
      MASK               = 0x3f,
      // Other.
      TEXTURE_IS_SPECIFIED = 0x40
    };
  };

  template <typename T>
  bool is_active(T const& p, int flag) {
    return (p.get_flag()&flag)!=0;
  }

  template <typename T>
  bool is_visible(T const& p) {
    return (p.get_flag()&ElementFlag::VISIBLE)!=0;
  }

  template <typename T>
  bool is_hidden(T const& p) {
    return (p.get_flag()&ElementFlag::VISIBLE)==0;
  }

  template <typename T>
  bool is_marked(T const& p) {
    return (p.get_flag()&ElementFlag::MARKED)!=0;
  }

  template <typename T>
  bool is_temporary_marked(T const& p) {
    return (p.get_flag()&ElementFlag::TEMPORARY_MARKED)!=0;
  }

  template <typename T>
  bool is_temporary_unmarked(T const& p) {
    return (p.get_flag()&ElementFlag::TEMPORARY_UNMARKED)!=0;
  }


  ////////////////////////////////////////////////////////////////////////////////
  // Traversing condition.
  struct CircumventHiddenElements : ConditionalPass {
    typedef SociariumGraph::node_property_iterator node_property_iterator;
    typedef SociariumGraph::edge_property_iterator edge_property_iterator;

    CircumventHiddenElements(std::tr1::shared_ptr<SociariumGraph const> g) :
    nflag_(g->nsize(), PASS), eflag_(g->esize(), PASS) {

      {
        node_property_iterator i   = g->node_property_begin();
        node_property_iterator end = g->node_property_end();

        for (; i!=end; ++i)
          if (is_hidden(i->second))
            nflag_[i->first->index()] = CLOSED;
      }{
        edge_property_iterator i   = g->edge_property_begin();
        edge_property_iterator end = g->edge_property_end();

        for (; i!=end; ++i)
          if (is_hidden(i->second))
            eflag_[i->first->index()] = CLOSED;
      }
    }

    int operator()(Edge const* e, Node const* n) const {
      return (nflag_[n->index()]==PASS && eflag_[e->index()]==PASS)?PASS:CLOSED;
    }

    std::vector<int> nflag_;
    std::vector<int> eflag_;
  };


  ////////////////////////////////////////////////////////////////////////////////
  // The hash function for 'unordered_set<StaticProperty>'.
  template <typename StaticProperty>
  struct static_property_hash :
    std::unary_function<StaticProperty, size_t> {
    size_t operator()(StaticProperty const& sp) const {
      return sp.get_id();
    }
  };

  // The compare function for 'unordered_set<StaticProperty>'.
  template <typename StaticProperty>
  struct static_property_compare :
    std::binary_function<StaticProperty, StaticProperty, bool> {
    bool operator()(StaticProperty const& lhs, StaticProperty const& rhs) const {
      return lhs.get_id()==rhs.get_id();
    }
  };

} // The end of the namespace "hashimoto_ut"

#endif // INCLUDE_GUARD_SOCIARIUM_PROJECT_SOCIARIUM_GRAPH_H

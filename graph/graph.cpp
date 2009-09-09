// C++ GRAPH LIBRARY: graph.cpp
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
#include <algorithm>

#ifdef _MSC_VER
#include <functional>
#else
#include <tr1/functional>
#endif

#include "graph.h"

namespace hashimoto_ut {

  using std::vector;
  using std::deque;
  using std::find_if;
  using std::equal_to;
  using std::tr1::bind;
  using std::tr1::shared_ptr;
  using std::tr1::placeholders::_1;

  namespace {

    ////////////////////////////////////////////////////////////////////////////////
    class NodeImpl : public Node {
    public:
      NodeImpl(size_t index) : index_(index), idegree_(0) {}
      ~NodeImpl() {}

      size_t index(void) const {
        return index_;
      }

      size_t degree(void) const {
        return adjacency_list_.size();
      }

      size_t odegree(void) const {
        return degree()-idegree_;
      }

      size_t idegree(void) const {
        return idegree_;
      }

      bool empty(void) const {
        return adjacency_list_.empty();
      }

      Edge* edge(size_t index) const {
        assert(index<degree());
        return adjacency_list_[index];
      }

      Edge* oedge(size_t index) const {
        assert(index<odegree());
        return adjacency_list_[idegree_+index];
      }

      Edge* iedge(size_t index) const {
        assert(index<idegree());
        return adjacency_list_[idegree_-index-1];
      }

      adjacency_list_iterator begin(void) const {
        return adjacency_list_.begin();
      }

      adjacency_list_iterator end(void) const {
        return adjacency_list_.end();
      }

      adjacency_list_iterator obegin(void) const {
        return begin()+idegree_;
      }

      adjacency_list_iterator oend(void) const {
        return end();
      }

      adjacency_list_iterator ibegin(void) const {
        return begin();
      }

      adjacency_list_iterator iend(void) const {
        return obegin();
      }

      deque<Edge*>::iterator begin(void) {
        return adjacency_list_.begin();
      }

      deque<Edge*>::iterator end(void) {
        return adjacency_list_.end();
      }

      deque<Edge*>::iterator obegin(void) {
        return begin()+idegree_;
      }

      deque<Edge*>::iterator oend(void) {
        return end();
      }

      deque<Edge*>::iterator ibegin(void) {
        return begin();
      }

      deque<Edge*>::iterator iend(void) {
        return obegin();
      }

      adjacency_list_iterator find(
        adjacency_list_iterator first,
        adjacency_list_iterator last,
        Node const* n) const {

          assert(begin()<=first && first<=end());
          assert(first<=last && last<=end());

          if (last<=iend())
            return find_if(first, last,
                           bind(equal_to<Node const*>(),
                                bind(&Edge::source, _1), n));
          else if (first>=obegin())
            return find_if(first, last,
                           bind(equal_to<Node const*>(),
                                bind(&Edge::target, _1), n));
          else {
            adjacency_list_iterator i
              = find_if(first, iend(),
                        bind(equal_to<Node const*>(),
                             bind(&Edge::source, _1), n));
            if (i!=iend()) return i;
            return find_if(obegin(), last,
                           bind(equal_to<Node const*>(),
                                bind(&Edge::target, _1), n));
          }
        }

    private:
      friend class GraphImpl;
      size_t index_;
      deque<Edge*> adjacency_list_;
      size_t idegree_;
    };


    ////////////////////////////////////////////////////////////////////////////////
    class EdgeImpl : public Edge {
    public:
      EdgeImpl(size_t index) : index_(index) {}

      ~EdgeImpl() {}

      size_t index(void) const {
        return index_;
      }

      Node* const& source(void) const {
        return source_;
      }

      Node* const& target(void) const {
        return target_;
      }

    private:
      friend class GraphImpl;
      size_t index_;
      Node* source_;
      Node* target_;
    };


    ////////////////////////////////////////////////////////////////////////////////
    NodeImpl* get_impl(Node* n) {
      return static_cast<NodeImpl*>(n);
    }

    EdgeImpl* get_impl(Edge* e) {
      return static_cast<EdgeImpl*>(e);
    }


    ////////////////////////////////////////////////////////////////////////////////
    class GraphImpl : public Graph {
    public:
      GraphImpl(bool directed) : is_directed_(directed) {}

      ~GraphImpl() {}

      bool is_directed(void) const {
        return is_directed_;
      }

      void set_directed(bool directed) {
        is_directed_ = directed;
      }

      Node* add_node(void) {
        NodeImpl* n(new NodeImpl(nsize()));
        nodes_.push_back(n);
        return nodes_.back();
      }

      Edge* add_edge(Node* source, Node* target) {
        assert(source==node(source->index()));
        assert(target==node(target->index()));
        EdgeImpl* e(new EdgeImpl(esize()));
        e->source_ = source;
        e->target_ = target;
        get_impl(source)->adjacency_list_.push_back(e);
        get_impl(target)->adjacency_list_.push_front(e);
        ++get_impl(target)->idegree_;
        edges_.push_back(e);
        return edges_.back();
      }

      void remove_node(Node* n) {
        assert(n==node(n->index()));
        while (!n->empty()) remove_edge(*n->begin());
        get_impl(nodes_.back())->index_ = n->index();
        nodes_[n->index()] = nodes_.back();
        nodes_.pop_back();
        delete get_impl(n);
      }

      void remove_edge(Edge* e) {
        assert(e==edge(e->index()));
        remove_connection(e);
        get_impl(edges_.back())->index_ = e->index();
        edges_[e->index()] = edges_.back();
        edges_.pop_back();
        delete get_impl(e);
      }

      void remove_nodes(vector<Node*>::const_iterator first,
                        vector<Node*>::const_iterator last) {

        for (; first!=last; ++first) {
          NodeImpl* n = get_impl(*first);
          assert(n==node(n->index()));
          while (!n->empty()) {
            EdgeImpl* e = get_impl(*n->begin());
            remove_connection(e);
            edges_[e->index()] = 0;
            delete e;
          }
          nodes_[n->index()] = 0;
          delete n;
        }

        nodes_.erase(remove(nodes_.begin(), nodes_.end(), (Node*)0), nodes_.end());
        edges_.erase(remove(edges_.begin(), edges_.end(), (Edge*)0), edges_.end());
        for_each(nodes_.begin(), nodes_.end(), SetConsecutiveIndex(0));
        for_each(edges_.begin(), edges_.end(), SetConsecutiveIndex(0));
      }

      void remove_edges(vector<Edge*>::const_iterator first,
                        vector<Edge*>::const_iterator last) {

        for (; first!=last; ++first) {
          EdgeImpl* e = get_impl(*first);
          assert(e==edge(e->index()));
          remove_connection(e);
          edges_[e->index()] = 0;
          delete e;
        }

        edges_.erase(remove(edges_.begin(), edges_.end(), (Edge*)0), edges_.end());
        for_each(edges_.begin(), edges_.end(), SetConsecutiveIndex(0));
      }

      void remove_connection(Edge* e) {
        NodeImpl* source = get_impl(e->source());
        NodeImpl* target = get_impl(e->target());
        deque<Edge*>::iterator i
          = find(source->obegin(), source->oend(), e); assert(i!=source->oend());
        deque<Edge*>::iterator j
          = find(target->ibegin(), target->iend(), e); assert(j!=target->iend());
        *i = source->adjacency_list_.back();
        *j = target->adjacency_list_.front();
        source->adjacency_list_.pop_back();
        target->adjacency_list_.pop_front();
        --target->idegree_;
      }

      size_t nsize(void) const {
        return nodes_.size();
      }

      size_t esize(void) const {
        return edges_.size();
      }

      bool empty(void) const {
        return nodes_.empty() && edges_.empty();
      }

      void clear(void) {
        nodes_.clear();
        edges_.clear();
      }

      node_iterator nbegin(void) const {
        return nodes_.begin();
      }

      node_iterator nend(void) const {
        return nodes_.end();
      }

      node_reverse_iterator nrbegin(void) const {
        return nodes_.rbegin();
      }

      node_reverse_iterator nrend(void) const {
        return nodes_.rend();
      }

      edge_iterator ebegin(void) const {
        return edges_.begin();
      }

      edge_iterator eend(void) const {
        return edges_.end();
      }

      edge_reverse_iterator erbegin(void) const {
        return edges_.rbegin();
      }

      edge_reverse_iterator erend(void) const {
        return edges_.rend();
      }

      Node* node(size_t index) const {
        assert(index<nsize());
        return nodes_[index];
      }

      Edge* edge(size_t index) const {
        assert(index<esize());
        return edges_[index];
      }

      void change_source(Edge* e, Node* n) {
        assert(e==edge(e->index()));
        assert(n==node(n->index()));
        NodeImpl* source = get_impl(e->source());
        deque<Edge*>::iterator i
          = find(source->obegin(), source->oend(), e);
        assert(i!=source->oend());
        *i = source->adjacency_list_.back();
        source->adjacency_list_.pop_back();
        get_impl(n)->adjacency_list_.push_back(e);
        get_impl(e)->source_ = n;
      }

      void change_target(Edge* e, Node* n) {
        assert(e==edge(e->index()));
        assert(n==node(n->index()));
        NodeImpl* target = get_impl(e->target());
        deque<Edge*>::iterator i
          = find(target->ibegin(), target->iend(), e);
        assert(i!=target->iend());
        *i = target->adjacency_list_.front();
        target->adjacency_list_.pop_front();
        get_impl(n)->adjacency_list_.push_front(e);
        --target->idegree_;
        ++get_impl(n)->idegree_;
        get_impl(e)->target_ = n;
      }

      shared_ptr<Graph> copy_induced_subgraph(
        vector<Node*>::const_iterator first,
        vector<Node*>::const_iterator last) const {

          GraphImpl* g(new GraphImpl(is_directed()));
          vector<size_t> index_of_copied_node(nsize(), size_t(-1));

          for (; first!=last; ++first) {
            Node* n = *first;
            assert(n==node(n->index()));
            index_of_copied_node[n->index()] = g->add_node()->index();
          }

          for (edge_iterator i=ebegin(), end=eend(); i!=end; ++i) {
            size_t const s = index_of_copied_node[(*i)->source()->index()];
            size_t const t = index_of_copied_node[(*i)->target()->index()];
            if (s!=size_t(-1) && t!=size_t(-1))
              g->add_edge(g->node(s), g->node(t));
          }

          return shared_ptr<Graph>(g);
        }

      shared_ptr<Graph> split_induced_subgraph(
        vector<Node*>::const_iterator first,
        vector<Node*>::const_iterator last) {

        GraphImpl* g(new GraphImpl(is_directed()));

        for (; first!=last; ++first) {
          Node* n = *first;
          assert(n==node(n->index()));
          g->nodes_.push_back(n);
          nodes_[n->index()] = 0;
        }

        for (vector<Edge*>::iterator i=edges_.begin(), end=edges_.end(); i!=end; ++i) {
          const bool s = nodes_[(*i)->source()->index()]==0;
          const bool t = nodes_[(*i)->target()->index()]==0;
          if (s && t) {
            g->edges_.push_back(*i);
            *i = 0;
          } else if (s || t) {
            remove_connection(*i);
            *i = 0;
          }
        }

        nodes_.erase(remove(nodes_.begin(), nodes_.end(), (Node*)0), nodes_.end());
        edges_.erase(remove(edges_.begin(), edges_.end(), (Edge*)0), edges_.end());
        for_each(nodes_.begin(), nodes_.end(), SetConsecutiveIndex(0));
        for_each(edges_.begin(), edges_.end(), SetConsecutiveIndex(0));
        for_each(g->nodes_.begin(), g->nodes_.end(), SetConsecutiveIndex(0));
        for_each(g->edges_.begin(), g->edges_.end(), SetConsecutiveIndex(0));
        return shared_ptr<Graph>(g);
      }

      void merge(shared_ptr<Graph> g) {
        size_t const nsz_old = nsize();
        size_t const esz_old = esize();
        GraphImpl* gimpl = static_cast<GraphImpl*>(g.get());
        nodes_.insert(nodes_.end(), gimpl->nodes_.begin(), gimpl->nodes_.end());
        edges_.insert(edges_.end(), gimpl->edges_.begin(), gimpl->edges_.end());
        gimpl->nodes_.clear();
        gimpl->edges_.clear();
        for_each(nodes_.begin()+nsz_old, nodes_.end(), SetConsecutiveIndex(nsz_old));
        for_each(edges_.begin()+esz_old, edges_.end(), SetConsecutiveIndex(esz_old));
      }

      shared_ptr<Graph> clone(void) const {
        shared_ptr<Graph> retval(new GraphImpl(is_directed()));
        return retval;
      }

      struct SetConsecutiveIndex {
        SetConsecutiveIndex(size_t offset) : offset_(offset) {}

        void operator()(Node* n) {
          get_impl(n)->index_ = offset_++;
        }

        void operator()(Edge* e) {
          get_impl(e)->index_ = offset_++;
        }

        size_t offset_;
      };

    private:
      bool is_directed_;
      vector<Node*> nodes_;
      vector<Edge*> edges_;
    };

  } // The end of the anonymous namespace

  shared_ptr<Graph> Graph::create(bool directed) {
    return shared_ptr<Graph>(new GraphImpl(directed));
  }

  Node* get_pair(Node const* n, adjacency_list_iterator c) {
    return c<n->iend()?(*c)->source():(*c)->target();
  }

} // The end of the namespace "hashimoto_ut"

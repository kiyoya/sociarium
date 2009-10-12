// C++ GRAPH LIBRARY: util/traverser.cpp
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
#include <vector>
#include <deque>
#include <map>
#include <algorithm>

#ifdef _MSC_VER
#pragma warning(disable:4250)
#include <functional>
#else
#include <tr1/functional>
#endif

#include "traverser.h"

namespace hashimoto_ut {

  using std::vector;
  using std::deque;
  using std::multimap;
  using std::pair;
  using std::make_pair;
  using std::for_each;
  using std::tr1::shared_ptr;
  using std::tr1::const_pointer_cast;
  using std::tr1::bind;
  using std::tr1::placeholders::_1;

  namespace {

    ////////////////////////////////////////////////////////////////////////////////
    class DirectionalSearchImpl {
    public:
      DirectionalSearchImpl(downward_tag)
           : advance_(&DirectionalSearchImpl::advance_downward) {}
      DirectionalSearchImpl(upward_tag)
           : advance_(&DirectionalSearchImpl::advance_upward) {}
      DirectionalSearchImpl(bidirectional_tag)
           : advance_(&DirectionalSearchImpl::advance_bidirectional) {}

      void advance(Node const* n) {
        (this->*advance_)(n);
      }

    private:
      void advance_downward(Node const* n) {
        for_each(n->obegin(), n->oend(),
                 bind(&DirectionalSearchImpl::search_neighbor, this, _1,
                      bind(&Edge::target, _1)));
      }

      void advance_upward(Node const* n) {
        for_each(n->ibegin(), n->iend(),
                 bind(&DirectionalSearchImpl::search_neighbor, this, _1,
                      bind(&Edge::source, _1)));
      }

      void advance_bidirectional(Node const* n) {
        for_each(n->obegin(), n->oend(),
                 bind(&DirectionalSearchImpl::search_neighbor, this, _1,
                      bind(&Edge::target, _1)));

        for_each(n->ibegin(), n->iend(),
                 bind(&DirectionalSearchImpl::search_neighbor, this, _1,
                      bind(&Edge::source, _1)));
      }

      virtual void search_neighbor(Edge const* e, Node const* n) = 0;

      void (DirectionalSearchImpl::*advance_)(Node const*);
    };


    ////////////////////////////////////////////////////////////////////////////////
    class BFSTraverserImpl :
      public virtual BFSTraverser,
      private DirectionalSearchImpl {

    public:
      enum { UNVISITED=-1 };

      BFSTraverserImpl(downward_tag) : DirectionalSearchImpl(downward_tag()) {}
      BFSTraverserImpl(upward_tag) : DirectionalSearchImpl(upward_tag()) {}
      BFSTraverserImpl(bidirectional_tag) : DirectionalSearchImpl(bidirectional_tag()) {}

      BFSTraverserImpl(downward_tag, shared_ptr<Graph const> g)
           : DirectionalSearchImpl(downward_tag()) { set_graph(g); }
      BFSTraverserImpl(upward_tag, shared_ptr<Graph const> g)
           : DirectionalSearchImpl(upward_tag()) { set_graph(g); }
      BFSTraverserImpl(bidirectional_tag, shared_ptr<Graph const> g)
           : DirectionalSearchImpl(bidirectional_tag()) { set_graph(g); }

      ~BFSTraverserImpl() {}

      void set_graph(shared_ptr<Graph const> g) {
        graph_ = g;
        deque<Node const*>().swap(que_);
        vector<double>(graph_->nsize(), UNVISITED).swap(distance_);
        conditional_pass_.reset(new ConditionalPass);
      }

      void set_condition(shared_ptr<ConditionalPass> conditional_pass) {
        conditional_pass_ = conditional_pass;
      }

      void reset(void) {
        deque<Node const*>().swap(que_);
        vector<double>(graph_->nsize(), UNVISITED).swap(distance_);
      }

      void start(Node const* n, double distance_offset) {
        assert(graph_!=0);
        assert(n==graph_->node(n->index()));
        assert(graph_->nsize()==distance_.size());
        if (distance_[n->index()]==UNVISITED) {
          que_.push_back(n);
          distance_[n->index()] = distance_offset;
        } else {
          que_.clear();
        }
      }

      void start(Node const* n) {
        start(n, 0);
      }

      void advance(void) {
        DirectionalSearchImpl::advance(node());
        que_.pop_front();
      }

      void search_neighbor(Edge const* e, Node const* n) {
        if ((*conditional_pass_)(e, n)==ConditionalPass::CLOSED) return;
        size_t const index = n->index();
        if (distance_[index]==UNVISITED) {
          que_.push_back(n);
          distance_[index] = distance()+1;
        }
      }

      bool end(void) const {
        return que_.empty();
      }

      shared_ptr<Graph> graph(void) const {
        return const_pointer_cast<Graph>(graph_);
      }

      Node* node(void) const {
        return const_cast<Node*>(que_.front());
      }

      double distance(void) const {
        return distance_[node()->index()];
      }

      double distance(Node const* n) const {
        assert(n==graph_->node(n->index()));
        return distance_[n->index()];
      }

    protected:
      shared_ptr<Graph const> graph_;
      deque<Node const*> que_;
      vector<double> distance_;
      shared_ptr<ConditionalPass> conditional_pass_;
    };


    ////////////////////////////////////////////////////////////////////////////////
    class BFSRecordingTraverserImpl :
      public BFSRecordingTraverser,
      private BFSTraverserImpl {

    public:
      typedef BFSTraverserImpl Base;

      using Base::UNVISITED;
      using Base::graph_;
      using Base::que_;
      using Base::distance_;
      using Base::conditional_pass_;
      using Base::node;
      using Base::distance;

      BFSRecordingTraverserImpl(downward_tag)
           : BFSTraverserImpl(downward_tag()) {}
      BFSRecordingTraverserImpl(upward_tag)
           : BFSTraverserImpl(upward_tag()) {}
      BFSRecordingTraverserImpl(bidirectional_tag)
           : BFSTraverserImpl(bidirectional_tag()) {}

      BFSRecordingTraverserImpl(downward_tag, shared_ptr<Graph const> g)
           : BFSTraverserImpl(downward_tag()) { set_graph(g); }
      BFSRecordingTraverserImpl(upward_tag, shared_ptr<Graph const> g)
           : BFSTraverserImpl(upward_tag()) { set_graph(g); }
      BFSRecordingTraverserImpl(bidirectional_tag, shared_ptr<Graph const> g)
           : BFSTraverserImpl(bidirectional_tag()) { set_graph(g); }

      ~BFSRecordingTraverserImpl() {}

      void set_graph(shared_ptr<Graph const> g) {
        Base::set_graph(g);
        vector<vector<pair<Edge*, Node*> > >(graph_->nsize()).swap(predecessors_);
      }

      void reset(void) {
        Base::reset();
        vector<vector<pair<Edge*, Node*> > >(graph_->nsize()).swap(predecessors_);
      }

      void search_neighbor(Edge const* e, Node const* n) {
        if ((*conditional_pass_)(e, n)==ConditionalPass::CLOSED) return;
        size_t const index = n->index();
        if (distance_[index]==UNVISITED) {
          que_.push_back(n);
          distance_[index] = distance()+1;
          predecessors_[index].push_back(make_pair(const_cast<Edge*>(e), node()));
        } else if (distance_[index]==distance()+1) {
          predecessors_[index].push_back(make_pair(const_cast<Edge*>(e), node()));
        }
      }

      vector<pair<Edge*, Node*> >::const_iterator pbegin(void) const {
        assert(node()->index()<predecessors_.size());
        return predecessors_[node()->index()].begin();
      }

      vector<pair<Edge*, Node*> >::const_iterator pend(void) const {
        assert(node()->index()<predecessors_.size());
        return predecessors_[node()->index()].end();
      }

      vector<pair<Edge*, Node*> >::const_iterator pbegin(Node const* n) const {
        assert(n==graph_->node(n->index()));
        assert(n->index()<predecessors_.size());
        return predecessors_[n->index()].begin();
      }

      vector<pair<Edge*, Node*> >::const_iterator pend(Node const* n) const {
        assert(n==graph_->node(n->index()));
        assert(n->index()<predecessors_.size());
        return predecessors_[n->index()].end();
      }

    private:
      vector<vector<pair<Edge*, Node*> > > predecessors_;
    };


    ////////////////////////////////////////////////////////////////////////////////
    class DijkstraTraverserImpl :
      public virtual DijkstraTraverser,
      private DirectionalSearchImpl {

    public:
      enum { UNVISITED=-1 };
      typedef multimap<double, Node const*> PriorityQueue;

      DijkstraTraverserImpl(downward_tag)
           : DirectionalSearchImpl(downward_tag()) {}
      DijkstraTraverserImpl(upward_tag)
           : DirectionalSearchImpl(upward_tag()) {}
      DijkstraTraverserImpl(bidirectional_tag)
           : DirectionalSearchImpl(bidirectional_tag()) {}

      DijkstraTraverserImpl(
        downward_tag, shared_ptr<Graph const> g,
        vector<double> const& edge_weight)
           : DirectionalSearchImpl(downward_tag()) { set_graph(g, edge_weight); }

      DijkstraTraverserImpl(
        upward_tag, shared_ptr<Graph const> g,
        vector<double> const& edge_weight)
           : DirectionalSearchImpl(upward_tag()) { set_graph(g, edge_weight); }

      DijkstraTraverserImpl(
        bidirectional_tag, shared_ptr<Graph const> g,
        vector<double> const& edge_weight)
           : DirectionalSearchImpl(bidirectional_tag()) { set_graph(g, edge_weight); }

      ~DijkstraTraverserImpl() {}

      void set_graph(shared_ptr<Graph const> g, vector<double> const& edge_weight) {
        assert(g!=0);
        assert(g->esize()==edge_weight.size());
        graph_ = g;
        edge_weight_ = edge_weight;
        PriorityQueue().swap(que_);
        vector<double>(graph_->nsize(), UNVISITED).swap(distance_);
        conditional_pass_.reset(new ConditionalPass);
      }

      void set_condition(shared_ptr<ConditionalPass> conditional_pass) {
        conditional_pass_ = conditional_pass;
      }

      void reset(void) {
        PriorityQueue().swap(que_);
        vector<double>(graph_->nsize(), UNVISITED).swap(distance_);
      }

      void start(Node const* n, double distance_offset) {
        assert(graph_!=0);
        assert(n==graph_->node(n->index()));
        assert(graph_->nsize()==distance_.size());
        if (distance_[n->index()]==UNVISITED) {
          que_.insert(make_pair(0.0, n));
          distance_[n->index()] = distance_offset;
          vector<PriorityQueue::iterator>(graph_->nsize()).swap(qpos_);
        } else {
          que_.clear();
        }
      }

      void start(Node const* n) {
        start(n, 0.0);
      }

      void advance(void) {
        DirectionalSearchImpl::advance(node());
        que_.erase(que_.begin());
      }

      void search_neighbor(Edge const* e, Node const* n) {
        if ((*conditional_pass_)(e, n)==ConditionalPass::CLOSED) return;
        size_t const index = n->index();
        double const dst_next = que_.begin()->first+edge_weight_[e->index()];
        if (distance_[index]==UNVISITED) {
          qpos_[index] = que_.insert(make_pair(dst_next, n));
          distance_[index] = dst_next;
        } else if (dst_next<distance_[index]) {
          que_.erase(qpos_[index]);
          qpos_[index] = que_.insert(make_pair(dst_next, n));
          distance_[index] = dst_next;
        }
      }

      bool end(void) const {
        return que_.empty();
      }

      shared_ptr<Graph> graph(void) const {
        return const_pointer_cast<Graph>(graph_);
      }

      Node* node(void) const {
        return const_cast<Node*>(que_.begin()->second);
      }

      double distance(void) const {
        return distance_[node()->index()];
      }

      double distance(Node const* n) const {
        assert(n==graph_->node(n->index()));
        return distance_[n->index()];
      }

    protected:
      shared_ptr<Graph const> graph_;
      PriorityQueue que_;
      vector<double> distance_;
      vector<PriorityQueue::iterator> qpos_;
      vector<double> edge_weight_;
      shared_ptr<ConditionalPass> conditional_pass_;

    private:
      void set_graph(shared_ptr<Graph const> g) {}
    };


    ////////////////////////////////////////////////////////////////////////////////
    class DijkstraRecordingTraverserImpl :
      public DijkstraRecordingTraverser,
      public DijkstraTraverserImpl {

    public:
      typedef DijkstraTraverserImpl Base;

      using Base::UNVISITED;
      using Base::graph_;
      using Base::que_;
      using Base::distance_;
      using Base::qpos_;
      using Base::edge_weight_;
      using Base::conditional_pass_;
      using Base::node;
      using Base::distance;

      DijkstraRecordingTraverserImpl(downward_tag)
           : DijkstraTraverserImpl(downward_tag()) {}
      DijkstraRecordingTraverserImpl(upward_tag)
           : DijkstraTraverserImpl(upward_tag()) {}
      DijkstraRecordingTraverserImpl(bidirectional_tag)
           : DijkstraTraverserImpl(bidirectional_tag()) {}

      DijkstraRecordingTraverserImpl(
        downward_tag, shared_ptr<Graph const> g,
        vector<double> const& edge_weight)
           : DijkstraTraverserImpl(downward_tag()) { set_graph(g, edge_weight); }

      DijkstraRecordingTraverserImpl(
        upward_tag, shared_ptr<Graph const> g,
        vector<double> const& edge_weight)
           : DijkstraTraverserImpl(upward_tag()) { set_graph(g, edge_weight); }

      DijkstraRecordingTraverserImpl(
        bidirectional_tag, shared_ptr<Graph const> g,
        vector<double> const& edge_weight)
           : DijkstraTraverserImpl(bidirectional_tag()) { set_graph(g, edge_weight); }

      ~DijkstraRecordingTraverserImpl() {}

      void set_graph(shared_ptr<Graph const> g, vector<double> const& edge_weight) {
        Base::set_graph(g, edge_weight);
        vector<vector<pair<Edge*, Node*> > >(graph_->nsize()).swap(predecessors_);
      }

      void reset(void) {
        Base::reset();
        vector<vector<pair<Edge*, Node*> > >(graph_->nsize()).swap(predecessors_);
      }

      void search_neighbor(Edge const* e, Node const* n) {
        if ((*conditional_pass_)(e, n)==ConditionalPass::CLOSED) return;
        size_t const index = n->index();
        double const dst_next = que_.begin()->first+edge_weight_[e->index()];
        if (distance_[index]<0) {
          qpos_[index] = que_.insert(make_pair(dst_next, n));
          distance_[index] = dst_next;
          predecessors_[index].push_back(make_pair(const_cast<Edge*>(e), node()));
        } else if (dst_next<distance_[index]) {
          que_.erase(qpos_[index]);
          qpos_[index] = que_.insert(make_pair(dst_next, n));
          distance_[index] = dst_next;
          vector<pair<Edge*, Node*> >(1, make_pair(const_cast<Edge*>(e), node())).swap(predecessors_[index]);
        } else if (dst_next==distance_[index]) {
          predecessors_[index].push_back(make_pair(const_cast<Edge*>(e), node()));
        }
      }

      vector<pair<Edge*, Node*> >::const_iterator pbegin(void) const {
        assert(node()->index()<predecessors_.size());
        return predecessors_[node()->index()].begin();
      }

      vector<pair<Edge*, Node*> >::const_iterator pend(void) const {
        assert(node()->index()<predecessors_.size());
        return predecessors_[node()->index()].end();
      }

      vector<pair<Edge*, Node*> >::const_iterator pbegin(Node const* n) const {
        assert(n==graph_->node(n->index()));
        assert(n->index()<predecessors_.size());
        return predecessors_[n->index()].begin();
      }

      vector<pair<Edge*, Node*> >::const_iterator pend(Node const* n) const {
        assert(n==graph_->node(n->index()));
        assert(n->index()<predecessors_.size());
        return predecessors_[n->index()].end();
      }

    private:
      vector<vector<pair<Edge*, Node*> > > predecessors_;
    };


    ////////////////////////////////////////////////////////////////////////////////
    class DFSTraverserImpl :
      public virtual DFSTraverser {

    public:
      enum { UNVISITED=-1 };

      DFSTraverserImpl(downward_tag)
           : neighbor_(&DFSTraverserImpl::neighbor_downward),
             adjacency_list_begin_(&DFSTraverserImpl::adjacency_list_begin_downward),
             adjacency_list_end_(&DFSTraverserImpl::adjacency_list_end_downward) {}
      DFSTraverserImpl(upward_tag)
           : neighbor_(&DFSTraverserImpl::neighbor_upward),
             adjacency_list_begin_(&DFSTraverserImpl::adjacency_list_begin_upward),
             adjacency_list_end_(&DFSTraverserImpl::adjacency_list_end_upward) {}
      DFSTraverserImpl(bidirectional_tag)
           : neighbor_(&DFSTraverserImpl::neighbor_bidirectional),
             adjacency_list_begin_(&DFSTraverserImpl::adjacency_list_begin_bidirectional),
             adjacency_list_end_(&DFSTraverserImpl::adjacency_list_end_bidirectional) {}

      DFSTraverserImpl(downward_tag, shared_ptr<Graph const> g)
           : neighbor_(&DFSTraverserImpl::neighbor_downward),
             adjacency_list_begin_(&DFSTraverserImpl::adjacency_list_begin_downward),
             adjacency_list_end_(&DFSTraverserImpl::adjacency_list_end_downward) {
               set_graph(g);
             }

      DFSTraverserImpl(upward_tag, shared_ptr<Graph const> g)
           : neighbor_(&DFSTraverserImpl::neighbor_upward),
             adjacency_list_begin_(&DFSTraverserImpl::adjacency_list_begin_upward),
             adjacency_list_end_(&DFSTraverserImpl::adjacency_list_end_upward) {
               set_graph(g);
             }

      DFSTraverserImpl(bidirectional_tag, shared_ptr<Graph const> g)
           : neighbor_(&DFSTraverserImpl::neighbor_bidirectional),
             adjacency_list_begin_(&DFSTraverserImpl::adjacency_list_begin_bidirectional),
             adjacency_list_end_(&DFSTraverserImpl::adjacency_list_end_bidirectional) {
               set_graph(g);
             }

      void set_graph(shared_ptr<Graph const> g) {
        graph_ = g;
        vector<Node const*>().swap(stack_);
        vector<int>(graph_->nsize(), UNVISITED).swap(order_);
        vector<int>(graph_->nsize(), UNVISITED).swap(backtrack_order_);
        order_counter_ = 0;
        backtrack_order_counter_ = 0;
        conditional_pass_.reset(new ConditionalPass);
      }

      void set_condition(shared_ptr<ConditionalPass> conditional_pass) {
        conditional_pass_ = conditional_pass;
      }

      void reset(void) {
        vector<Node const*>().swap(stack_);
        vector<adjacency_list_iterator>().swap(astack_);
        vector<int>(graph_->nsize(), UNVISITED).swap(order_);
        vector<int>(graph_->nsize(), UNVISITED).swap(backtrack_order_);
        order_counter_ = 0;
        backtrack_order_counter_ = 0;
      }

      void start(Node const* n, int order_counter_offset) {
        assert(graph_!=0);
        assert(n==graph_->node(n->index()));
        assert(graph_->nsize()==order_.size());
        if (order_[n->index()]==UNVISITED) {
          order_counter_ = order_counter_offset;
          stack_.push_back(n);
          astack_.push_back((this->*adjacency_list_begin_)(n));
          order_[n->index()] = order_counter_++;
        } else {
          stack_.clear();
          astack_.clear();
        }
      }

      void start(Node const* n) {
        start(n, order_counter_);
      }

      void advance(void) {
        Node const* n = node();
        for (adjacency_list_iterator i=astack_.back(),
             iend=(this->*adjacency_list_end_)(n); i!=iend; ++i) {
          Node const* next = (this->*neighbor_)(i);
          if (order_[next->index()]==UNVISITED
              && (*conditional_pass_)(*i, next)==ConditionalPass::PASS) {
            stack_.push_back(next);
            astack_.back() = ++i;
            astack_.push_back((this->*adjacency_list_begin_)(next));
            order_[next->index()] = order_counter_++;
            break;
          }
        }

        if (n==stack_.back()) backtrack();
      }

      bool end(void) const {
        return stack_.empty();
      }

      shared_ptr<Graph> graph(void) const {
        return const_pointer_cast<Graph>(graph_);
      }

      Node* node(void) const {
        return const_cast<Node*>(stack_.back());
      }

      int order(void) const {
        assert(graph_->nsize()==order_.size());
        return order_[node()->index()];
      }

      int order(Node const* n) const {
        assert(graph_->nsize()==order_.size());
        assert(n==graph_->node(n->index()));
        return order_[n->index()];
      }

      int backtrack_order(Node const* n) const {
        assert(graph_->nsize()==backtrack_order_.size());
        assert(n==graph_->node(n->index()));
        return backtrack_order_[n->index()];
      }

      void backtrack(void) {
        backtrack_order_[stack_.back()->index()] = backtrack_order_counter_++;
        stack_.pop_back();
        astack_.pop_back();
        if (stack_.empty()) return;
        advance();
      }

      Node const* neighbor_downward(adjacency_list_iterator i) const {
        return (*i)->target();
      }

      Node const* neighbor_upward(adjacency_list_iterator i) const {
        return (*i)->source();
      }

      Node const* neighbor_bidirectional(adjacency_list_iterator i) const {
        return i<node()->iend()?(*i)->source():(*i)->target();
      }

      adjacency_list_iterator
        adjacency_list_begin_downward(Node const* n) const {
          return n->obegin();
        }

      adjacency_list_iterator
        adjacency_list_end_downward(Node const* n) const {
          return n->oend();
        }

      adjacency_list_iterator
        adjacency_list_begin_upward(Node const* n) const {
          return n->ibegin();
        }

      adjacency_list_iterator
        adjacency_list_end_upward(Node const* n) const {
          return n->iend();
        }

      adjacency_list_iterator
        adjacency_list_begin_bidirectional(Node const* n) const {
          return n->begin();
        }

      adjacency_list_iterator
        adjacency_list_end_bidirectional(Node const* n) const {
          return n->end();
        }

    protected:
      shared_ptr<Graph const> graph_;
      vector<Node const*> stack_;
      vector<adjacency_list_iterator> astack_;
      vector<int> order_;
      vector<int> backtrack_order_;
      int order_counter_;
      int backtrack_order_counter_;
      shared_ptr<ConditionalPass> conditional_pass_;
      Node const* (DFSTraverserImpl::*neighbor_)(adjacency_list_iterator) const;
      adjacency_list_iterator (DFSTraverserImpl::*adjacency_list_begin_)(Node const*) const;
      adjacency_list_iterator (DFSTraverserImpl::*adjacency_list_end_)(Node const*) const;
    };


    ////////////////////////////////////////////////////////////////////////////////
    class DFSRecordingTraverserImpl :
      public DFSRecordingTraverser,
      public DFSTraverserImpl {

    public:
      typedef DFSTraverserImpl Base;

      using Base::UNVISITED;
      using Base::graph_;
      using Base::stack_;
      using Base::order_;
      using Base::backtrack_order_;
      using Base::order_counter_;
      using Base::backtrack_order_counter_;
      using Base::conditional_pass_;
      using Base::neighbor_;
      using Base::adjacency_list_begin_;
      using Base::adjacency_list_end_;

      DFSRecordingTraverserImpl(downward_tag)
           : DFSTraverserImpl(downward_tag()) {}
      DFSRecordingTraverserImpl(upward_tag)
           : DFSTraverserImpl(upward_tag()) {}
      DFSRecordingTraverserImpl(bidirectional_tag)
           : DFSTraverserImpl(bidirectional_tag()) {}

      DFSRecordingTraverserImpl(downward_tag, shared_ptr<Graph const> g)
           : DFSTraverserImpl(downward_tag()) { set_graph(g); }
      DFSRecordingTraverserImpl(upward_tag, shared_ptr<Graph const> g)
           : DFSTraverserImpl(upward_tag()) { set_graph(g); }
      DFSRecordingTraverserImpl(bidirectional_tag, shared_ptr<Graph const> g)
           : DFSTraverserImpl(bidirectional_tag()) { set_graph(g); }

      ~DFSRecordingTraverserImpl() {}

      void set_graph(shared_ptr<Graph const> g) {
        Base::set_graph(g);
        vector<pair<Edge*, Node*> >(graph_->nsize()).swap(predecessor_);
      }

      void reset(void) {
        Base::reset();
        vector<pair<Edge*, Node*> >(graph_->nsize()).swap(predecessor_);
      }

      void advance(void) {
        Node const* n = node();
        for (adjacency_list_iterator i=astack_.back(),
             iend=(this->*adjacency_list_end_)(n); i!=iend; ++i) {
          Node const* next = (this->*neighbor_)(i);
          if (order_[next->index()]==UNVISITED
              && (*conditional_pass_)(*i, next)==ConditionalPass::PASS) {
            stack_.push_back(next);
            predecessor_[next->index()]
              = make_pair(const_cast<Edge*>(*i), const_cast<Node*>(n));
            astack_.back() = ++i;
            astack_.push_back((this->*adjacency_list_begin_)(next));
            order_[next->index()] = order_counter_++;
            break;
          }
        }
        if (n==stack_.back()) backtrack();
      }

      pair<Edge*, Node*> const& predecessor(void) const {
        assert(graph_->nsize()==predecessor_.size());
        return predecessor_[node()->index()];
      }

      pair<Edge*, Node*> const& predecessor(Node const* n) const {
        assert(graph_->nsize()==predecessor_.size());
        assert(n==graph_->node(n->index()));
        return predecessor_[n->index()];
      }

    private:
      vector<pair<Edge*, Node*> > predecessor_;
    };

  } // The end of the anonymous namespace


  ////////////////////////////////////////////////////////////////////////////////
  template <>
  shared_ptr<BFSTraverser>
    BFSTraverser::create<downward_tag>(shared_ptr<Graph const> g) {
      return shared_ptr<BFSTraverser>(new BFSTraverserImpl(downward_tag(), g));
    }

  template <>
  shared_ptr<BFSTraverser>
    BFSTraverser::create<upward_tag>(shared_ptr<Graph const> g) {
      return shared_ptr<BFSTraverser>(new BFSTraverserImpl(upward_tag(), g));
    }

  template <>
  shared_ptr<BFSTraverser>
    BFSTraverser::create<bidirectional_tag>(shared_ptr<Graph const> g) {
      return shared_ptr<BFSTraverser>(new BFSTraverserImpl(bidirectional_tag(), g));
    }

  template <>
  shared_ptr<BFSRecordingTraverser>
    BFSRecordingTraverser::create<downward_tag>(shared_ptr<Graph const> g) {
      return shared_ptr<BFSRecordingTraverser>(
        new BFSRecordingTraverserImpl(downward_tag(), g));
    }

  template <>
  shared_ptr<BFSRecordingTraverser>
    BFSRecordingTraverser::create<upward_tag>(shared_ptr<Graph const> g) {
      return shared_ptr<BFSRecordingTraverser>(
        new BFSRecordingTraverserImpl(upward_tag(), g));
    }

  template <>
  shared_ptr<BFSRecordingTraverser>
    BFSRecordingTraverser::create<bidirectional_tag>(shared_ptr<Graph const> g) {
      return shared_ptr<BFSRecordingTraverser>(
        new BFSRecordingTraverserImpl(bidirectional_tag(), g));
    }

  template <>
  shared_ptr<DijkstraTraverser>
    DijkstraTraverser::create<downward_tag>(
      shared_ptr<Graph const> g, vector<double> const& edge_weight) {
      return shared_ptr<DijkstraTraverser>(
        new DijkstraTraverserImpl(downward_tag(), g, edge_weight));
    }

  template <>
  shared_ptr<DijkstraTraverser>
    DijkstraTraverser::create<upward_tag>(
      shared_ptr<Graph const> g, vector<double> const& edge_weight) {
      return shared_ptr<DijkstraTraverser>(
        new DijkstraTraverserImpl(upward_tag(), g, edge_weight));
    }

  template <>
  shared_ptr<DijkstraTraverser>
    DijkstraTraverser::create<bidirectional_tag>(
      shared_ptr<Graph const> g, vector<double> const& edge_weight) {
      return shared_ptr<DijkstraTraverser>(
        new DijkstraTraverserImpl(bidirectional_tag(), g, edge_weight));
    }

  template <>
  shared_ptr<DijkstraRecordingTraverser>
    DijkstraRecordingTraverser::create<downward_tag>(
      shared_ptr<Graph const> g, vector<double> const& edge_weight) {
      return shared_ptr<DijkstraRecordingTraverser>(
        new DijkstraRecordingTraverserImpl(downward_tag(), g, edge_weight));
    }

  template <>
  shared_ptr<DijkstraRecordingTraverser>
    DijkstraRecordingTraverser::create<upward_tag>(
      shared_ptr<Graph const> g, vector<double> const& edge_weight) {
      return shared_ptr<DijkstraRecordingTraverser>(
        new DijkstraRecordingTraverserImpl(upward_tag(), g, edge_weight));
    }

  template <>
  shared_ptr<DijkstraRecordingTraverser>
    DijkstraRecordingTraverser::create<bidirectional_tag>(
      shared_ptr<Graph const> g, vector<double> const& edge_weight) {
      return shared_ptr<DijkstraRecordingTraverser>(
        new DijkstraRecordingTraverserImpl(bidirectional_tag(), g, edge_weight));
    }

  template <>
  shared_ptr<DFSTraverser>
    DFSTraverser::create<downward_tag>(shared_ptr<Graph const> g) {
      return shared_ptr<DFSTraverser>(new DFSTraverserImpl(downward_tag(), g));
    }

  template <>
  shared_ptr<DFSTraverser>
    DFSTraverser::create<upward_tag>(shared_ptr<Graph const> g) {
      return shared_ptr<DFSTraverser>(new DFSTraverserImpl(upward_tag(), g));
    }

  template <>
  shared_ptr<DFSTraverser>
    DFSTraverser::create<bidirectional_tag>(shared_ptr<Graph const> g) {
      return shared_ptr<DFSTraverser>(new DFSTraverserImpl(bidirectional_tag(), g));
    }

  template <>
  shared_ptr<DFSRecordingTraverser>
    DFSRecordingTraverser::create<downward_tag>(shared_ptr<Graph const> g) {
      return shared_ptr<DFSRecordingTraverser>(
        new DFSRecordingTraverserImpl(downward_tag(), g));
    }

  template <>
  shared_ptr<DFSRecordingTraverser>
    DFSRecordingTraverser::create<upward_tag>(shared_ptr<Graph const> g) {
      return shared_ptr<DFSRecordingTraverser>(
        new DFSRecordingTraverserImpl(upward_tag(), g));
    }

  template <>
  shared_ptr<DFSRecordingTraverser>
    DFSRecordingTraverser::create<bidirectional_tag>(shared_ptr<Graph const> g) {
      return shared_ptr<DFSRecordingTraverser>(
        new DFSRecordingTraverserImpl(bidirectional_tag(), g));
    }

  ////////////////////////////////////////////////////////////////////////////////
  template <>
  shared_ptr<BFSTraverser>
    BFSTraverser::create<downward_tag>(void) {
      return shared_ptr<BFSTraverser>(new BFSTraverserImpl(downward_tag()));
    }

  template <>
  shared_ptr<BFSTraverser>
    BFSTraverser::create<upward_tag>(void) {
      return shared_ptr<BFSTraverser>(new BFSTraverserImpl(upward_tag()));
    }

  template <>
  shared_ptr<BFSTraverser>
    BFSTraverser::create<bidirectional_tag>(void) {
      return shared_ptr<BFSTraverser>(new BFSTraverserImpl(bidirectional_tag()));
    }

  template <>
  shared_ptr<BFSRecordingTraverser>
    BFSRecordingTraverser::create<downward_tag>(void) {
      return shared_ptr<BFSRecordingTraverser>(
        new BFSRecordingTraverserImpl(downward_tag()));
    }

  template <>
  shared_ptr<BFSRecordingTraverser>
    BFSRecordingTraverser::create<upward_tag>(void) {
      return shared_ptr<BFSRecordingTraverser>(
        new BFSRecordingTraverserImpl(upward_tag()));
    }

  template <>
  shared_ptr<BFSRecordingTraverser>
    BFSRecordingTraverser::create<bidirectional_tag>(void) {
      return shared_ptr<BFSRecordingTraverser>(
        new BFSRecordingTraverserImpl(bidirectional_tag()));
    }

  template <>
  shared_ptr<DijkstraTraverser>
    DijkstraTraverser::create<downward_tag>(void) {
      return shared_ptr<DijkstraTraverser>(new DijkstraTraverserImpl(downward_tag()));
    }

  template <>
  shared_ptr<DijkstraTraverser>
    DijkstraTraverser::create<upward_tag>(void) {
      return shared_ptr<DijkstraTraverser>(new DijkstraTraverserImpl(upward_tag()));
    }

  template <>
  shared_ptr<DijkstraTraverser>
    DijkstraTraverser::create<bidirectional_tag>(void) {
      return shared_ptr<DijkstraTraverser>(new DijkstraTraverserImpl(bidirectional_tag()));
    }

  template <>
  shared_ptr<DijkstraRecordingTraverser>
    DijkstraRecordingTraverser::create<downward_tag>(void) {
      return shared_ptr<DijkstraRecordingTraverser>(
        new DijkstraRecordingTraverserImpl(downward_tag()));
    }

  template <>
  shared_ptr<DijkstraRecordingTraverser>
    DijkstraRecordingTraverser::create<upward_tag>(void) {
      return shared_ptr<DijkstraRecordingTraverser>(
        new DijkstraRecordingTraverserImpl(upward_tag()));
    }

  template <>
  shared_ptr<DijkstraRecordingTraverser>
    DijkstraRecordingTraverser::create<bidirectional_tag>(void) {
      return shared_ptr<DijkstraRecordingTraverser>(
        new DijkstraRecordingTraverserImpl(bidirectional_tag()));
    }

  template <>
  shared_ptr<DFSTraverser>
    DFSTraverser::create<downward_tag>(void) {
      return shared_ptr<DFSTraverser>(new DFSTraverserImpl(downward_tag()));
    }

  template <>
  shared_ptr<DFSTraverser>
    DFSTraverser::create<upward_tag>(void) {
      return shared_ptr<DFSTraverser>(new DFSTraverserImpl(upward_tag()));
    }

  template <>
  shared_ptr<DFSTraverser>
    DFSTraverser::create<bidirectional_tag>(void) {
      return shared_ptr<DFSTraverser>(new DFSTraverserImpl(bidirectional_tag()));
    }

  template <>
  shared_ptr<DFSRecordingTraverser>
    DFSRecordingTraverser::create<downward_tag>(void) {
      return shared_ptr<DFSRecordingTraverser>(
        new DFSRecordingTraverserImpl(downward_tag()));
    }

  template <>
  shared_ptr<DFSRecordingTraverser>
    DFSRecordingTraverser::create<upward_tag>(void) {
      return shared_ptr<DFSRecordingTraverser>(
        new DFSRecordingTraverserImpl(upward_tag()));
    }

  template <>
  shared_ptr<DFSRecordingTraverser>
    DFSRecordingTraverser::create<bidirectional_tag>(void) {
      return shared_ptr<DFSRecordingTraverser>(
        new DFSRecordingTraverserImpl(bidirectional_tag()));
    }

} // The end of the namespace "hashimoto_ut"

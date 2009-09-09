// C++ GRAPH LIBRARY: graph/traverser.h
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

#ifndef INCLUDE_GUARD_GRAPH_UTIL_TRAVERSER_H
#define INCLUDE_GUARD_GRAPH_UTIL_TRAVERSER_H

#include <cassert>
#include <vector>
#include <utility>
#include "../graph.h"

namespace hashimoto_ut {

  ////////////////////////////////////////////////////////////////////////////////
  // Search direction.
  struct downward_tag {};
  struct upward_tag {};
  struct bidirectional_tag {};


  ////////////////////////////////////////////////////////////////////////////////
  // Search condition: whether the traverser can pass the node/edge or not.
  struct ConditionalPass {
    enum { PASS, CLOSED };
    virtual int operator()(Edge const* e, Node const* n) const {
      return PASS;
    }
  };


  ////////////////////////////////////////////////////////////////////////////////
  // Common interfaces between each traverser.
  class Traverser {
  public:
    virtual void set_graph(std::tr1::shared_ptr<Graph const> g) = 0;
    virtual void set_condition(std::tr1::shared_ptr<ConditionalPass> conditional_pass) = 0;
    virtual void reset(void) = 0;
    virtual void start(Node const* n) = 0;
    virtual void advance(void) = 0;
    virtual bool end(void) const = 0;
    virtual std::tr1::shared_ptr<Graph> graph(void) const = 0;
    virtual Node* node(void) const = 0;

  protected:
    virtual ~Traverser() {}
  };


  ////////////////////////////////////////////////////////////////////////////////
  // Breadth-first search.
  class BFSTraverser : public Traverser {
  public:
    using Traverser::start;
    virtual void start(Node const* n, double distance_offset) = 0;
    virtual double distance(void) const = 0;
    virtual double distance(Node const* n) const = 0;

    template <typename D>
    static std::tr1::shared_ptr<BFSTraverser> create(void);

    template <typename D>
    static std::tr1::shared_ptr<BFSTraverser>
      create(std::tr1::shared_ptr<Graph const> g);

  protected:
    virtual ~BFSTraverser() {}
  };

  ////////////////////////////////////////////////////////////////////////////////
  // Breadth-first search with path recording.
  class BFSRecordingTraverser : public virtual BFSTraverser {
  public:
    virtual std::vector<std::pair<Edge*, Node*> >::const_iterator
      pbegin(void) const = 0;
    virtual std::vector<std::pair<Edge*, Node*> >::const_iterator
      pend(void) const = 0;
    virtual std::vector<std::pair<Edge*, Node*> >::const_iterator
      pbegin(Node const* n) const = 0;
    virtual std::vector<std::pair<Edge*, Node*> >::const_iterator
      pend(Node const* n) const = 0;

    template <typename D>
    static std::tr1::shared_ptr<BFSRecordingTraverser> create(void);

    template <typename D>
    static std::tr1::shared_ptr<BFSRecordingTraverser>
      create(std::tr1::shared_ptr<Graph const> g);

  protected:
    virtual ~BFSRecordingTraverser() {}
  };


  ////////////////////////////////////////////////////////////////////////////////
  // Breadth-first search [Dijkstra's algorithm].
  class DijkstraTraverser : public virtual BFSTraverser {
  public:
    virtual void set_graph(std::tr1::shared_ptr<Graph const> g,
                           std::vector<double> const& edge_weight) = 0;

    template <typename D>
    static std::tr1::shared_ptr<DijkstraTraverser> create(void);

    template <typename D>
    static std::tr1::shared_ptr<DijkstraTraverser>
      create(std::tr1::shared_ptr<Graph const> g,
             std::vector<double> const& edge_weight);

  protected:
    virtual ~DijkstraTraverser() {}
  };

  ////////////////////////////////////////////////////////////////////////////////
  // Breadth-first search [Dijkstra's algorithm] with path recording.
  class DijkstraRecordingTraverser :
    public DijkstraTraverser,
    public BFSRecordingTraverser {
  public:
    template <typename D>
    static std::tr1::shared_ptr<DijkstraRecordingTraverser> create(void);

    template <typename D>
    static std::tr1::shared_ptr<DijkstraRecordingTraverser>
      create(std::tr1::shared_ptr<Graph const> g,
             std::vector<double> const& edge_weight);

  protected:
    virtual ~DijkstraRecordingTraverser() {}
  };


  ////////////////////////////////////////////////////////////////////////////////
  // Depth-first search.
  class DFSTraverser : public Traverser {
  public:
    using Traverser::start;
    virtual void start(Node const* n, int order_counter_offset) = 0;
    virtual int order(void) const = 0;
    virtual int order(Node const* n) const = 0;
    virtual int backtrack_order(Node const* n) const = 0;

    template <typename D>
    static std::tr1::shared_ptr<DFSTraverser> create(void);

    template <typename D>
    static std::tr1::shared_ptr<DFSTraverser>
      create(std::tr1::shared_ptr<Graph const> g);

  protected:
    virtual ~DFSTraverser() {}
  };

  ////////////////////////////////////////////////////////////////////////////////
  // Depth-first search with path recording.
  class DFSRecordingTraverser : public virtual DFSTraverser {
  public:
    virtual std::pair<Edge*, Node*> const& predecessor(void) const = 0;
    virtual std::pair<Edge*, Node*> const& predecessor(Node const* n) const = 0;

    template <typename D>
    static std::tr1::shared_ptr<DFSRecordingTraverser> create(void);

    template <typename D>
    static std::tr1::shared_ptr<DFSRecordingTraverser>
      create(std::tr1::shared_ptr<Graph const> g);

  protected:
    virtual ~DFSRecordingTraverser() {}
  };


  ////////////////////////////////////////////////////////////////////////////////
  // Examples of search condition.

  // CircumventSpecifiedNodes
  struct CircumventSpecifiedNodes : ConditionalPass {
    CircumventSpecifiedNodes(
      std::tr1::shared_ptr<Graph const> g,
      std::vector<Node*>::const_iterator first,
      std::vector<Node*>::const_iterator last) :

    flag_(g->nsize(), PASS) {

      for (; first!=last; ++first) {
        assert(*first==g->node((*first)->index()));
        flag_[(*first)->index()] = CLOSED;
      }
    }

    int operator()(Edge const* e, Node const* n) const {
      return flag_[n->index()];
    }

    std::vector<int> flag_;
  };

  // CircumventSpecifiedEdges
  struct CircumventSpecifiedEdges : ConditionalPass {
    CircumventSpecifiedEdges(
      std::tr1::shared_ptr<Graph const> g,
      std::vector<Edge*>::const_iterator first,
      std::vector<Edge*>::const_iterator last) :

    flag_(g->esize(), PASS) {

      for (; first!=last; ++first) {
        assert(*first==g->edge((*first)->index()));
        flag_[(*first)->index()] = CLOSED;
      }
    }

    int operator()(Edge const* e, Node const* n) const {
      return flag_[e->index()];
    }

    std::vector<int> flag_;
  };

} // The end of the namespace "hashimoto_ut"

#endif // INCLUDE_GUARD_GRAPH_UTIL_TRAVERSER_H

// C++ GRAPH LIBRARY: graph.h
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

#ifndef INCLUDE_GUARD_GRAPH_H
#define INCLUDE_GUARD_GRAPH_H

#include <vector>
#include <deque>

#ifdef _MSC_VER
#include <memory>
#else
#include <tr1/memory>
#endif

namespace hashimoto_ut {

  class Node;
  class Edge;
  /* "Adjacency List" and "Edge List" are used to store graph elements:
   * "Adjacency List" is the list of nodes that have the list of edges
   * connecting with neighboring nodes, and "Edge List" is the list of edges
   * that have two end nodes that the edge connects.
   */

  typedef std::vector<Node*>::const_iterator node_iterator;
  typedef std::vector<Node*>::const_reverse_iterator node_reverse_iterator;

  typedef std::vector<Edge*>::const_iterator edge_iterator;
  typedef std::vector<Edge*>::const_reverse_iterator edge_reverse_iterator;

  typedef std::deque<Edge*>::const_iterator adjacency_list_iterator;
  typedef std::deque<Edge*>::const_reverse_iterator adjacency_list_reverse_iterator;
  /* In "Adjacency List", each edge is stored in "std::deque" as in the following
   * order [3][2][1][0](0)(1)(2) where [*] represent incoming edges and (*) represent
   * outgoing edges, and the digit denotes the order of establishing the connection
   * (unless the deletion of any edge occurs).
   */


  ////////////////////////////////////////////////////////////////////////////////
  class Node {
  public:
    virtual ~Node() {}

    virtual size_t index(void) const = 0;
    virtual size_t degree(void) const = 0;
    virtual size_t odegree(void) const = 0;
    virtual size_t idegree(void) const = 0;
    virtual bool empty(void) const = 0;

    virtual Edge* edge(size_t index) const = 0;
    virtual Edge* oedge(size_t index) const = 0;
    virtual Edge* iedge(size_t index) const = 0;

    virtual adjacency_list_iterator begin(void) const = 0;
    virtual adjacency_list_iterator end(void) const = 0;
    virtual adjacency_list_iterator obegin(void) const = 0;
    virtual adjacency_list_iterator oend(void) const = 0;
    virtual adjacency_list_iterator ibegin(void) const = 0;
    virtual adjacency_list_iterator iend(void) const = 0;

    virtual adjacency_list_iterator find(
      adjacency_list_iterator first,
      adjacency_list_iterator last, Node const* n) const = 0;
  };


  ////////////////////////////////////////////////////////////////////////////////
  class Edge {
  public:
    virtual ~Edge() {}

    virtual size_t index(void) const = 0;
    virtual Node* const& source(void) const = 0;
    virtual Node* const& target(void) const = 0;
  };


  ////////////////////////////////////////////////////////////////////////////////
  class Graph {
  public:
    virtual ~Graph() {}

    virtual bool is_directed(void) const = 0;
    virtual void set_directed(bool directed) = 0;

    virtual Node* add_node(void) = 0;
    virtual Edge* add_edge(Node* source, Node* target) = 0;
    /* The new element is appended to the end of the container.
     */

    virtual void remove_node(Node* n) = 0;
    /* The last node in the container moves to the deleted position and changes
     * its index. The order (indices) of other nodes never change.
     * The order of edges might change.
     */

    virtual void remove_edge(Edge* e) = 0;
    /* The last edge in the container moves to the deleted position and changes
     * its index. The order (indices) of other edges never change.
     */

    virtual void remove_nodes(
      std::vector<Node*>::const_iterator first,
      std::vector<Node*>::const_iterator last) = 0;

    virtual void remove_edges(
      std::vector<Edge*>::const_iterator first,
      std::vector<Edge*>::const_iterator last) = 0;
    /* The relative order of remaining elements is the same as their relative order
     * before removing the elements. Consequently, the indices of all elements
     * posterior to the head of removed elements might change.
     */

    virtual size_t nsize(void) const = 0;
    virtual size_t esize(void) const = 0;

    virtual bool empty(void) const = 0;
    virtual void clear(void) = 0;

    virtual node_iterator nbegin(void) const = 0;
    virtual node_iterator nend(void) const = 0;
    virtual node_reverse_iterator nrbegin(void) const = 0;
    virtual node_reverse_iterator nrend(void) const = 0;

    virtual edge_iterator ebegin(void) const = 0;
    virtual edge_iterator eend(void) const = 0;
    virtual edge_reverse_iterator erbegin(void) const = 0;
    virtual edge_reverse_iterator erend(void) const = 0;

    virtual Node* node(size_t index) const = 0;
    virtual Edge* edge(size_t index) const = 0;

    virtual void change_source(Edge* e, Node* n) = 0;
    virtual void change_target(Edge* e, Node* n) = 0;

    virtual std::tr1::shared_ptr<Graph>
      copy_induced_subgraph(
        std::vector<Node*>::const_iterator first,
        std::vector<Node*>::const_iterator last) const = 0;
    /* The relative order of the elelemtns in the returned value is the same as
     * the relative order of corresponding elements in @this.
     */

    virtual std::tr1::shared_ptr<Graph>
      split_induced_subgraph(
        std::vector<Node*>::const_iterator first,
        std::vector<Node*>::const_iterator last) = 0;
    /* The relative order of nodes in the returned value is the same as the order
     * in [@first:@last) and the order of edges is the same as their relative order
     * in @this. The relative order of remaining elements in @this is the same as
     *  their relative order before removing the elements.
     */

    virtual void merge(std::tr1::shared_ptr<Graph> g) = 0;
    /* All elements in @g move to @this.
     */

    virtual std::tr1::shared_ptr<Graph> clone(void) const = 0;

    static std::tr1::shared_ptr<Graph> create(bool directed=false);
  };

  Node* get_pair(Node const* n, adjacency_list_iterator c);

} // The end of the namespace "hashimoto_ut"

#endif // INCLUDE_GUARD_GRAPH_H

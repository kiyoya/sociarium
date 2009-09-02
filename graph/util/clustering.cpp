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

#define NOMINMAX
#include <cassert>
#include <map>
#include <set>
#include <numeric>
#include "clustering.h"

namespace hashimoto_ut {

  using std::vector;
  using std::multimap;
  using std::tr1::shared_ptr;
  using std::pair;
  using std::make_pair;

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  vector<Node*> connected_component(shared_ptr<Traverser> const& t, Node const* n) {
    assert(t!=0);
    shared_ptr<Graph const> const& g = t->graph();
    assert(g!=0);
    assert(n==g->node(n->index()));
    vector<Node*> retval;
    for (t->start(n); !t->end(); t->advance()) retval.push_back(const_cast<Node*>(t->node()));
    return retval;
  }

  vector<vector<Node*> > connected_component(shared_ptr<Traverser> const& t) {
    assert(t!=0);
    shared_ptr<Graph const> const& g = t->graph();
    assert(g!=0);
    t->set_graph(g); // necessary to initialize traverser's state.
    vector<vector<Node*> > retval;
    for (node_iterator i=g->nbegin(), end=g->nend(); i!=end; ++i) {
      vector<Node*> c;
      for (t->reset(), t->start(*i); !t->end(); t->advance()) c.push_back(const_cast<Node*>(t->node()));
      if (!c.empty()) retval.push_back(c);
    }
    return retval;
  }


  ////////////////////////////////////////////////////////////////////////////////////////////////////
  vector<vector<Node*> > strongly_connected_component(shared_ptr<Graph> const& g) {
    assert(g!=0);
    typedef multimap<int, Node const*, std::greater<int> > BacktrackOrder;
    BacktrackOrder backtrack_order;
    { // Sort nodes in descending DFS-backtrack-order.
      shared_ptr<DFSTraverser> t = DFSTraverser::create<downward_tag>(g);
      for (node_iterator i=g->nbegin(), end=g->nend(); i!=end; ++i) {
        t->reset();
        for (t->start(*i, 0); !t->end(); t->advance()); // traverse just to determine the backtrack order.
      }
      for (node_iterator i=g->nbegin(), end=g->nend(); i!=end; ++i)
        backtrack_order.insert(make_pair(t->backtrack_order(*i), *i));
    }
    // The nodes that reverse-search can reach compose SCC.
    shared_ptr<BFSTraverser> t = BFSTraverser::create<upward_tag>(g);
    vector<vector<Node*> > retval;
    for (BacktrackOrder::const_iterator i=backtrack_order.begin(), end=backtrack_order.end(); i!=end; ++i) {
      vector<Node*> c;
      t->reset();
      for (t->start(i->second, 0.0); !t->end(); t->advance()) c.push_back(const_cast<Node*>(t->node()));
      if (!c.empty()) retval.push_back(c);
    }
    return retval;
  }

} // The end of the namespace "hashimoto_ut"

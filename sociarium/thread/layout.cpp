// s.o.c.i.a.r.i.u.m: thread/layout.cpp
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

#include <string>
#include <unordered_map>
#include <boost/format.hpp>
#include "layout.h"
#include "../module/layout.h"
#include "../graph_extractor.h"
#include "../thread.h"
#include "../layout.h"
#include "../language.h"
#include "../algorithm_selector.h"
#include "../sociarium_graph.h"
#include "../sociarium_graph_time_series.h"

namespace hashimoto_ut {

  using std::vector;
  using std::deque;
  using std::wstring;
  using std::pair;
  using std::tr1::unordered_map;
  using std::tr1::shared_ptr;
  using std::tr1::weak_ptr;

  using namespace sociarium_project_thread;
  using namespace sociarium_project_module_layout;
  using namespace sociarium_project_algorithm_selector;
  using namespace sociarium_project_layout;
  using namespace sociarium_project_language;

  typedef SociariumGraph::node_property_iterator node_property_iterator;
  typedef SociariumGraph::edge_property_iterator edge_property_iterator;

  class LayoutThreadImpl : public LayoutThread {
  public:
    ////////////////////////////////////////////////////////////////////////////////
    LayoutThreadImpl(void) {}


    ////////////////////////////////////////////////////////////////////////////////
    ~LayoutThreadImpl() {}


    ////////////////////////////////////////////////////////////////////////////////
    void terminate(void) {

      // Clear the progress message.
      deque<wstring>& status = get_status(LAYOUT);
      deque<wstring>(status.size()).swap(status);

      detach(LAYOUT);
    }


    ////////////////////////////////////////////////////////////////////////////////
    void operator()(void) {

      shared_ptr<SociariumGraphTimeSeries> ts
        = sociarium_project_graph_time_series::get();

      ts->read_lock();
      /*
       * Don't forget to call read_unlock().
       */

      deque<wstring>& status = get_status(LAYOUT);

      // --------------------------------------------------------------------------------
      // Extract marked elements.

      size_t const index_of_current_layer = ts->index_of_current_layer();
      shared_ptr<SociariumGraph> g = ts->get_graph(0, index_of_current_layer);

      unordered_map<Node*, Node const*> node2node; // Map nodes in @g_target to nodes in @g.
      unordered_map<Edge*, Edge const*> edge2edge; // not used.

      pair<bool, shared_ptr<Graph const> > const ext
        = sociarium_project_graph_extractor::get(
          this, &status[0], g, node2node, edge2edge, ElementFlag::MARKED);

      if (ext.first==false) {
        ts->read_unlock();
        return terminate();
      }

      shared_ptr<Graph const> g_target = ext.second; // Extracted graph.

      // --------------------------------------------------------------------------------
      // Memory the position of extracted nodes.

      size_t const nsz = g->nsize();

      vector<double> hint; // for versatile use.
      hint.reserve(nsz);

      vector<Vector2<double> > position(nsz); // The position before.

      {
        node_property_iterator i   = g->node_property_begin();
        node_property_iterator end = g->node_property_end();

        for (; i!=end; ++i)
          position[i->first->index()]
            = i->second.get_static_property()->get_position();
      }

      vector<Vector2<double> > position_target; // The position after.
      position_target.reserve(nsz);

      {
        node_iterator i   = g_target->nbegin();
        node_iterator end = g_target->nend();

        for (; i!=end; ++i) {
          DynamicNodeProperty& dnp = g->property(node2node[*i]);
          position_target.push_back(dnp.get_static_property()->get_position());
        }
      }

      if (get_layout_algorithm()==LayoutAlgorithm::CIRCLE) {
        hint.resize(nsz);

        node_iterator i   = g_target->nbegin();
        node_iterator end = g_target->nend();

        for (; i!=end; ++i)
          hint[(*i)->index()] = node2node[*i]->index();

      } else if (get_layout_algorithm()==LayoutAlgorithm::CIRCLE_IN_SIZE_ORDER
                 || get_layout_algorithm()==LayoutAlgorithm::LATTICE) {

        node_iterator i   = g_target->nbegin();
        node_iterator end = g_target->nend();

        for (; i!=end; ++i) {
          DynamicNodeProperty& dnp = g->property(node2node[*i]);
          hint.push_back(dnp.get_size());
        }

      } else if (get_layout_algorithm()==LayoutAlgorithm::KAMADA_KAWAI_METHOD) {
        hint.push_back(double(get_layout_frame_size()));

      } else if (get_layout_algorithm()
                 ==LayoutAlgorithm::HIGH_DIMENSIONAL_EMBEDDING_1_2) {
        hint.push_back(1);

      } else if (get_layout_algorithm()
                 ==LayoutAlgorithm::HIGH_DIMENSIONAL_EMBEDDING_1_3) {
        hint.push_back(2);

      } else if (get_layout_algorithm()
                 ==LayoutAlgorithm::HIGH_DIMENSIONAL_EMBEDDING_2_3) {
        hint.push_back(3);
      }

      assert(g_target->nsize()==position_target.size());

      // --------------------------------------------------------------------------------
      // Load a layout module.

      FuncLayout layout = sociarium_project_module_layout::get(get_layout_algorithm());

      if (layout==0) {
        ts->read_unlock();
        return terminate();
      }

      // --------------------------------------------------------------------------------
      // Execute the module.

      layout(this,
             status,
             get_message_object(),
             position_target,
             g_target,
             hint);

      // --------------------------------------------------------------------------------
      // Adjust positions inside the layout frame.

      Vector2<double> pos_max(-1e+10, -1e+10);
      Vector2<double> pos_min( 1e+10,  1e+10);

      size_t count = 0;

      {
        node_iterator i   = g_target->nbegin();
        node_iterator end = g_target->nend();

        for (; i!=end; ++i) {
          Node const* n = node2node[*i];
          assert(n!=0);

          Vector2<double> const& pos = position[n->index()]
            = position_target[(*i)->index()];

          if (pos_max.x<pos.x) pos_max.x = pos.x;
          if (pos_min.x>pos.x) pos_min.x = pos.x;
          if (pos_max.y<pos.y) pos_max.y = pos.y;
          if (pos_min.y>pos.y) pos_min.y = pos.y;
        }
      }

      Vector2<double> const pos_center(0.5*(pos_max+pos_min));
      Vector2<double> const size(0.5*(pos_max-pos_min));
      double const scale = size.x>size.y?get_layout_frame_size()/size.x
        :(size.y>0.0?get_layout_frame_size()/size.y:0.0);

      {
        node_iterator i   = g_target->nbegin();
        node_iterator end = g_target->nend();

        for (; i!=end; ++i) {
          Node const* n = node2node[*i];
          assert(n!=0);
          Vector2<double>& pos = position[n->index()];
          pos = scale*(pos-pos_center)+get_layout_frame_position();
        }
      }

      vector<Vector2<double> > position_prev(nsz);

      {
        node_property_iterator i   = g->node_property_begin();
        node_property_iterator end = g->node_property_end();

        for (; i!=end; ++i)
          position_prev[i->first->index()]
            = i->second.get_static_property()->get_position();
      }

#if 0
      ts->update_node_position(index, position);
#else
      // Asymptotically locating.
      for (int iteration=0; iteration<20; ++iteration) {
        Sleep(10);
        double const j = double(20-iteration);
        for (size_t i=0; i<nsz; ++i)
          position_prev[i] = (position[i]+(j-1.0)*position_prev[i])/j;
        ts->update_node_position(index_of_current_layer, position_prev);
      }
#endif

      ts->read_unlock();
      terminate();
    }

  };


  ////////////////////////////////////////////////////////////////////////////////
  // Functory function of LayoutThread.
  shared_ptr<LayoutThread> LayoutThread::create(void) {
    return shared_ptr<LayoutThread>(new LayoutThreadImpl);
  }

} // The end of the namespace "hashimoto_ut"

// s.o.c.i.a.r.i.u.m: thread/edge_width_update.cpp
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
#include <numeric>
#ifdef _MSC_VER
#include <unordered_map>
#else
#include <tr1/unordered_map>
#endif
#include <boost/format.hpp>
#include "edge_width_update.h"
#include "../graph_extractor.h"
#include "../thread.h"
#include "../algorithm_selector.h"
#include "../flag_operation.h"
#include "../graph_utility.h"
#include "../menu_and_message.h"
#include "../sociarium_graph_time_series.h"
#include "../../graph/util/traverser.h"

namespace hashimoto_ut {

  using std::vector;
  using std::deque;
  using std::wstring;
  using std::pair;
  using std::tr1::shared_ptr;
  using std::tr1::weak_ptr;
  using std::tr1::unordered_map;

  using namespace sociarium_project_thread;
  using namespace sociarium_project_algorithm_selector;
  using namespace sociarium_project_menu_and_message;

  typedef SociariumGraph::node_property_iterator node_property_iterator;
  typedef SociariumGraph::edge_property_iterator edge_property_iterator;

  class EdgeWidthUpdateThreadImpl : public EdgeWidthUpdateThread {
  public:
    ////////////////////////////////////////////////////////////////////////////////
    EdgeWidthUpdateThreadImpl(void) {}


    ////////////////////////////////////////////////////////////////////////////////
    ~EdgeWidthUpdateThreadImpl() {}


    ////////////////////////////////////////////////////////////////////////////////
    void terminate(void) {
      // Clear the progress message.
      deque<wstring>& status = get_status(EDGE_WIDTH_UPDATE);
      deque<wstring>(status.size()).swap(status);

      detach(EDGE_WIDTH_UPDATE);
    }


    ////////////////////////////////////////////////////////////////////////////////
    void operator()(void) {

      shared_ptr<SociariumGraphTimeSeries> ts
        = sociarium_project_graph_time_series::get();

      ts->read_lock();
      /*
       * Don't forget to call read_unlock().
       */

      deque<wstring>& status = get_status(EDGE_WIDTH_UPDATE);

      size_t const number_of_layers = ts->number_of_layers();

      vector<vector<double> > edge_width(number_of_layers);

      double mean = 0.0;
      double denom = 0.0;

      // --------------------------------------------------------------------------------
      // Uniform
      if (get_edge_width_update_algorithm()==EdgeWidthUpdateAlgorithm::UNIFORM) {
        for (size_t layer=0; layer<number_of_layers; ++layer) {

          // **********  Catch a termination signal  **********
          if (cancel_check()) {
            ts->read_unlock();
            return terminate();
          }

          status[0]
            = number_of_layers<2
              ?get_message(Message::UPDATING_EDGE_WIDTH)
                :(boost::wformat(L"%s: %d%%")
                  %get_message(Message::UPDATING_EDGE_WIDTH)
                  %int(100.0*(layer+1.0)/number_of_layers)).str();

          shared_ptr<SociariumGraph> g = ts->get_graph(0, layer);
          edge_property_iterator i   = g->edge_property_begin();
          edge_property_iterator end = g->edge_property_end();

          for (; i!=end; ++i) {
            DynamicEdgeProperty& dep = i->second;
            dep.set_width(1.0f);
          }
        }
      }

      // --------------------------------------------------------------------------------
      // Use weight
      else if (get_edge_width_update_algorithm()==EdgeWidthUpdateAlgorithm::WEIGHT) {
        for (size_t layer=0; layer<number_of_layers; ++layer) {

          // **********  Catch a termination signal  **********
          if (cancel_check()) {
            ts->read_unlock();
            return terminate();
          }

          status[0]
            = number_of_layers<2
              ?get_message(Message::UPDATING_EDGE_WIDTH)
                :(boost::wformat(L"%s: %d%%")
                  %get_message(Message::UPDATING_EDGE_WIDTH)
                  %int(100.0*(layer+1.0)/number_of_layers)).str();

          shared_ptr<SociariumGraph> g = ts->get_graph(0, layer);
          edge_width[layer].resize(g->esize());
          edge_property_iterator i   = g->edge_property_begin();
          edge_property_iterator end = g->edge_property_end();

          for (; i!=end; ++i) {
            DynamicEdgeProperty& dep = i->second;
            mean += edge_width[layer][i->first->index()] = sqrt(double(dep.get_weight()));
          }

          denom += g->esize();
        }

        if (denom>0.0) mean /= denom;
        if (mean==0.0) {
          ts->read_unlock();
          return terminate();
        }

        for (size_t layer=0; layer<number_of_layers; ++layer) {
          shared_ptr<SociariumGraph> g = ts->get_graph(0, layer);
          edge_property_iterator i   = g->edge_property_begin();
          edge_property_iterator end = g->edge_property_end();

          for (; i!=end; ++i) {
            DynamicEdgeProperty& dep = i->second;
            dep.set_width(float(edge_width[layer][i->first->index()]/mean));
          }
        }
      }

      else {

        for (size_t layer=0; layer<number_of_layers; ++layer) {

          // **********  Catch a termination signal  **********
          if (cancel_check()) {
            ts->read_unlock();
            return terminate();
          }

          status[0]
            = number_of_layers<2
              ?get_message(Message::UPDATING_EDGE_WIDTH)
                :(boost::wformat(L"%s: %d%%")
                  %get_message(Message::UPDATING_EDGE_WIDTH)
                  %int(100.0*(layer+1.0)/number_of_layers)).str();

          // --------------------------------------------------------------------------------
          // Extract visible elements.

          shared_ptr<SociariumGraph> g = ts->get_graph(0, layer);

          unordered_map<Node*, Node const*> node2node; // not used.
          unordered_map<Edge*, Edge const*> edge2edge; // Map edges in @g_target to edges in @g.

          pair<bool, shared_ptr<Graph const> > const ext
            = sociarium_project_graph_extractor::get(
              this, 0, g, node2node, edge2edge, ElementFlag::VISIBLE);

          if (ext.first==false) {
            ts->read_unlock();
            return terminate();
          }

          shared_ptr<Graph const> g_target = ext.second; // Extracted graph.

          size_t const nsz = g->nsize();
          size_t const esz = g->esize();
          size_t const tnsz = g_target->nsize();
          size_t const tesz = g_target->esize();

          if (tesz==0) {
            edge_width.push_back(vector<double>(esz, 1.0));
            continue;
          }

          // --------------------------------------------------------------------------------
          // Use Betweenness Centrality
          if (get_edge_width_update_algorithm()
              ==EdgeWidthUpdateAlgorithm::BETWEENNESS_CENTRALITY) {

            vector<double> width(esz, 1.0);

            shared_ptr<BFSRecordingTraverser> t =
              g_target->is_directed()?BFSRecordingTraverser::create<downward_tag>(g_target)
                :BFSRecordingTraverser::create<bidirectional_tag>(g_target);

            pair<bool, pair<vector<double>, vector<double> > > bc =
              sociarium_project_graph_utility::betweenness_centrality(
                this, &status[1], &get_message_object(), t);

            if (bc.first) {
              assert(bc.second.second.size()==tesz);

              for (edge_iterator i=g_target->ebegin(); i!=g_target->eend(); ++i)
                mean += width[edge2edge[*i]->index()] = bc.second.second[(*i)->index()];

              denom += tesz;
              width.swap(edge_width[layer]);
            }

            else {
              ts->read_unlock();
              return terminate();
            }
          }
        }

        if (denom>0.0) mean /= denom;
        if (mean==0.0) {
          ts->read_unlock();
          return terminate();
        }

        // --------------------------------------------------------------------------------
        // Fine adjustment

        if (get_edge_width_update_algorithm()
            ==EdgeWidthUpdateAlgorithm::BETWEENNESS_CENTRALITY) {
          for (size_t i=0; i<edge_width.size(); ++i) {
            for (size_t j=0; j<edge_width[i].size(); ++j) {
              edge_width[i][j] = log(edge_width[i][j]/mean+1.0);
            }
          }
        }

        ts->update_edge_width(edge_width);
      }

      ts->read_unlock();
      terminate();
    }

  };


  ////////////////////////////////////////////////////////////////////////////////
  // Factory function of EdgeWidthUpdateThread.
  shared_ptr<EdgeWidthUpdateThread> EdgeWidthUpdateThread::create(void) {
    return shared_ptr<EdgeWidthUpdateThread>(new EdgeWidthUpdateThreadImpl);
  }

} // The end of the namespace "hashimoto_ut"

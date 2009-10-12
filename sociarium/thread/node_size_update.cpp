// s.o.c.i.a.r.i.u.m: thread/node_size_update.cpp
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
#include <unordered_map>
#include <boost/format.hpp>
#include "node_size_update.h"
#include "../algorithm_selector.h"
#include "../flag_operation.h"
#include "../graph_extractor.h"
#include "../graph_utility.h"
#include "../menu_and_message.h"
#include "../sociarium_graph_time_series.h"
#include "../thread.h"
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

  class NodeSizeUpdateThreadImpl : public NodeSizeUpdateThread {
  public:
    ////////////////////////////////////////////////////////////////////////////////
    NodeSizeUpdateThreadImpl(void) {}


    ////////////////////////////////////////////////////////////////////////////////
    ~NodeSizeUpdateThreadImpl() {}


    ////////////////////////////////////////////////////////////////////////////////
    void terminate(void) {
      // Clear the progress message.
      deque<wstring>& status = get_status(NODE_SIZE_UPDATE);
      deque<wstring>(status.size()).swap(status);

      detach(NODE_SIZE_UPDATE);
    }


    ////////////////////////////////////////////////////////////////////////////////
    void operator()(void) {

      shared_ptr<SociariumGraphTimeSeries> ts
        = sociarium_project_graph_time_series::get();

      ts->read_lock();
      /*
       * Don't forget to call read_unlock().
       */

      deque<wstring>& status = get_status(NODE_SIZE_UPDATE);

      size_t const number_of_layers = ts->number_of_layers();

      vector<vector<double> > node_size(number_of_layers);

      double mean = 0.0;
      double denom = 0.0;

      // --------------------------------------------------------------------------------
      // Point
      if (get_node_size_update_algorithm()==NodeSizeUpdateAlgorithm::POINT) {
        for (size_t layer=0; layer<number_of_layers; ++layer) {

          // **********  Catch a termination signal  **********
          if (cancel_check()) {
            ts->read_unlock();
            return terminate();
          }

          status[0] = number_of_layers<2
            ?get_message(Message::UPDATING_NODE_SIZE)
              :(boost::wformat(L"%s: %d%%")
                %get_message(Message::UPDATING_NODE_SIZE)
                %int(100.0*(layer+1.0)/number_of_layers)).str();

          shared_ptr<SociariumGraph> g = ts->get_graph(0, layer);

          node_property_iterator i   = g->node_property_begin();
          node_property_iterator end = g->node_property_end();

          for (; i!=end; ++i) {
            DynamicNodeProperty& dnp = i->second;
            dnp.set_size(0.0f);
          }
        }
      }

      // --------------------------------------------------------------------------------
      // Uniform
      else if (get_node_size_update_algorithm()==NodeSizeUpdateAlgorithm::UNIFORM) {
        for (size_t layer=0; layer<number_of_layers; ++layer) {

          // **********  Catch a termination signal  **********
          if (cancel_check()) {
            ts->read_unlock();
            return terminate();
          }

          status[0] = number_of_layers<2
            ?get_message(Message::UPDATING_NODE_SIZE)
              :(boost::wformat(L"%s: %d%%")
                %get_message(Message::UPDATING_NODE_SIZE)
                %int(100.0*(layer+1.0)/number_of_layers)).str();

          shared_ptr<SociariumGraph> g = ts->get_graph(0, layer);

          node_property_iterator i   = g->node_property_begin();
          node_property_iterator end = g->node_property_end();

          for (; i!=end; ++i) {
            DynamicNodeProperty& dnp = i->second;
            dnp.set_size(1.0f);
          }
        }
      }

      // --------------------------------------------------------------------------------
      // Use weight
      else if (get_node_size_update_algorithm()==NodeSizeUpdateAlgorithm::WEIGHT) {
        for (size_t layer=0; layer<number_of_layers; ++layer) {

          // **********  Catch a termination signal  **********
          if (cancel_check()) {
            ts->read_unlock();
            return terminate();
          }

          status[0]
            = number_of_layers<2
              ?get_message(Message::UPDATING_NODE_SIZE)
                :(boost::wformat(L"%s: %d%%")
                  %get_message(Message::UPDATING_NODE_SIZE)
                  %int(100.0*(layer+1.0)/number_of_layers)).str();

          shared_ptr<SociariumGraph> g = ts->get_graph(0, layer);
          node_size[layer].resize(g->nsize());

          node_property_iterator i   = g->node_property_begin();
          node_property_iterator end = g->node_property_end();

          for (; i!=end; ++i) {
            DynamicNodeProperty& dnp = i->second;
            mean += node_size[layer][i->first->index()] = sqrt(double(dnp.get_weight()));
          }

          denom += g->nsize();
        }

        if (denom>0.0)
          mean /= denom;

        if (mean==0.0) {
          ts->read_unlock();
          return terminate();
        }

        for (size_t layer=0; layer<number_of_layers; ++layer) {
          shared_ptr<SociariumGraph> g = ts->get_graph(0, layer);

          node_property_iterator i   = g->node_property_begin();
          node_property_iterator end = g->node_property_end();

          for (; i!=end; ++i) {
            DynamicNodeProperty& dnp = i->second;
            dnp.set_size(float(node_size[layer][i->first->index()]/mean));
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
              ?get_message(Message::UPDATING_NODE_SIZE)
                :(boost::wformat(L"%s: %d%%")
                  %get_message(Message::UPDATING_NODE_SIZE)
                  %int(100.0*(layer+1.0)/number_of_layers)).str();

          // --------------------------------------------------------------------------------
          // Extract visible elements.

          shared_ptr<SociariumGraph> g = ts->get_graph(0, layer);

          unordered_map<Node*, Node const*> node2node; // Map nodes in @g_target to nodes in @g.
          unordered_map<Edge*, Edge const*> edge2edge; // not used.

          pair<bool, shared_ptr<Graph const> > const ext
            = sociarium_project_graph_extractor::get(
              this, 0, g, node2node, edge2edge, ElementFlag::VISIBLE);

          if (ext.first==false) {
            ts->read_unlock();
            return terminate();
          }

          shared_ptr<Graph const> g_target = ext.second; // Extracted graph.

          size_t const nsz = g->nsize();
          size_t const tnsz = g_target->nsize();

          if (tnsz==0) {
            node_size[layer].resize(nsz, 1.0);
            continue;
          }

          // --------------------------------------------------------------------------------
          // Use Degree Centrality
          if (get_node_size_update_algorithm()
              ==NodeSizeUpdateAlgorithm::DEGREE_CENTRALITY) {

            vector<double> sz(nsz, 1.0);

            for (size_t i=0; i<tnsz; ++i) {

              // **********  Catch a termination signal  **********
              if (cancel_check()) {
                ts->read_unlock();
                return terminate();
              }

              status[1]
                = (boost::wformat(L"%s: %d%%")
                   %get_message(Message::CALCULATING_DEGREE_CENTRALITY)
                   %int(100.0*(i+1.0)/tnsz)).str();

              Node* n = g_target->node(i);
              double degree = 0;

              for (adjacency_list_iterator k=n->obegin(); k!=n->oend(); ++k)
                if (is_visible(g->property(node2node[(*k)->target()]))) ++degree;

              for (adjacency_list_iterator k=n->ibegin(); k!=n->iend(); ++k)
                if (is_visible(g->property(node2node[(*k)->source()])))
                  ++degree;

              mean += sz[node2node[n]->index()] = double(degree);
            }

            denom += tnsz;
            sz.swap(node_size[layer]);
          }

          // --------------------------------------------------------------------------------
          // Use Closeness Centrality
          else if (get_node_size_update_algorithm()
                   ==NodeSizeUpdateAlgorithm::CLOSENESS_CENTRALITY) {

            vector<double> sz(nsz, 1.0);

            shared_ptr<BFSTraverser> t
              = g_target->is_directed()?BFSTraverser::create<downward_tag>(g_target)
                :BFSTraverser::create<bidirectional_tag>(g_target);

            pair<bool, vector<double> > cc
              = sociarium_project_graph_utility::closeness_centrality(
                this, &status[1], &get_message_object(), t);

            if (cc.first) {
              assert(cc.second.size()==tnsz);

              for (node_iterator i=g_target->nbegin(); i!=g_target->nend(); ++i)
                mean += sz[node2node[*i]->index()] = cc.second[(*i)->index()];

              denom += tnsz;
              sz.swap(node_size[layer]);
            }

            else {
              ts->read_unlock();
              return terminate();
            }
          }

          // --------------------------------------------------------------------------------
          // Use Betweenness Centrality
          else if (get_node_size_update_algorithm()
                   ==NodeSizeUpdateAlgorithm::BETWEENNESS_CENTRALITY) {

            vector<double> sz(nsz, 1.0);

            shared_ptr<BFSRecordingTraverser> t
              = g_target->is_directed()
                ?BFSRecordingTraverser::create<downward_tag>(g_target)
                  :BFSRecordingTraverser::create<bidirectional_tag>(g_target);

            pair<bool, pair<vector<double>, vector<double> > > bc
              = sociarium_project_graph_utility::betweenness_centrality(
                this, &status[1], &get_message_object(), t);

            if (bc.first) {
              assert(bc.second.first.size()==tnsz);

              for (node_iterator i=g_target->nbegin(); i!=g_target->nend(); ++i)
                mean += sz[node2node[*i]->index()] = bc.second.first[(*i)->index()];

              denom += tnsz;
              sz.swap(node_size[layer]);
            }

            else {
              ts->read_unlock();
              return terminate();
            }
          }

          // --------------------------------------------------------------------------------
          // Use PageRank
          else if (get_node_size_update_algorithm()
                   ==NodeSizeUpdateAlgorithm::PAGERANK) {

            vector<double> sz(nsz, 1.0);

            pair<bool, vector<double> > pr
              = sociarium_project_graph_utility::pagerank(
                this, &status[1], &get_message_object(), g_target, 0.85, 1e-2);

            if (pr.first) {
              assert(pr.second.size()==tnsz);

              for (node_iterator i=g_target->nbegin(); i!=g_target->nend(); ++i)
                mean += sz[node2node[*i]->index()] = tnsz*pr.second[(*i)->index()];

              denom += tnsz;
              sz.swap(node_size[layer]);
            }

            else {
              ts->read_unlock();
              return terminate();
            }
          }
        }

        if (denom>0.0)
          mean /= denom;

        if (mean==0.0) {
          ts->read_unlock();
          return terminate();
        }

        // --------------------------------------------------------------------------------
        // Fine adjustment

        if (get_node_size_update_algorithm()
            ==NodeSizeUpdateAlgorithm::DEGREE_CENTRALITY) {

          for (size_t i=0; i<node_size.size(); ++i) {
            for (size_t j=0; j<node_size[i].size(); ++j) {
              node_size[i][j] = sqrt((node_size[i][j]+1.0)/mean);
            }
          }
        }

        else if (get_node_size_update_algorithm()
                 ==NodeSizeUpdateAlgorithm::CLOSENESS_CENTRALITY) {

          for (size_t i=0; i<node_size.size(); ++i) {
            for (size_t j=0; j<node_size[i].size(); ++j) {
              node_size[i][j] = sqrt(node_size[i][j]/mean+1.0);
            }
          }
        }

        else if (get_node_size_update_algorithm()
                 ==NodeSizeUpdateAlgorithm::BETWEENNESS_CENTRALITY) {

          for (size_t i=0; i<node_size.size(); ++i) {
            for (size_t j=0; j<node_size[i].size(); ++j) {
              node_size[i][j] = log(node_size[i][j]/mean+1.0);
            }
          }
        }

        else if (get_node_size_update_algorithm()
                 ==NodeSizeUpdateAlgorithm::PAGERANK) {

          for (size_t i=0; i<node_size.size(); ++i) {
            for (size_t j=0; j<node_size[i].size(); ++j) {
              node_size[i][j] = sqrt(node_size[i][j]/mean);
            }
          }
        }

        ts->update_node_size(node_size);
      }

      ts->read_unlock();
      terminate();
    }

  };


  ////////////////////////////////////////////////////////////////////////////////
  // Factory function of NodeSizeUpdateThread.
  shared_ptr<NodeSizeUpdateThread> NodeSizeUpdateThread::create(void) {
    return shared_ptr<NodeSizeUpdateThread>(new NodeSizeUpdateThreadImpl);
  }

} // The end of the namespace "hashimoto_ut"

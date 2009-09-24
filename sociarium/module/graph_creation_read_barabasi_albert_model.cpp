// s.o.c.i.a.r.i.u.m: module/graph_creation_read_barabasi_albert_model.cpp
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
#include <boost/random.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <windows.h>
#include "graph_creation.h"
#include "../common.h"
#include "../menu_and_message.h"
#include "../../shared/msgbox.h"
#include "../../shared/thread.h"
#include "../../shared/util.h"
#include "../../graph/graph.h"

BOOL WINAPI DllMain (HINSTANCE hinstDll, DWORD fdwReason, LPVOID lpvReserved) {
  return TRUE;
}

namespace hashimoto_ut {

  using std::vector;
  using std::deque;
  using std::string;
  using std::wstring;
  using std::pair;
  using std::tr1::shared_ptr;
  using std::tr1::unordered_map;

  using namespace sociarium_project_common;
  using namespace sociarium_project_module_graph_creation;
  using namespace sociarium_project_menu_and_message;

  extern "C" __declspec(dllexport)
    void __cdecl create_graph_time_series(

      Thread& parent,
      deque<wstring>& status,
      Message const& message,
      vector<shared_ptr<Graph> >& graph,
      vector<vector<NodeProperty> >& node_property,
      vector<vector<EdgeProperty> >& edge_property,
      vector<wstring>& layer_name,
      unordered_map<wstring, pair<wstring, int> > const& params,
      vector<pair<wstring, int> > const& data,
      wstring const& filename) {

      assert(graph.empty());
      assert(node_property.empty());
      assert(edge_property.empty());
      assert(layer_name.empty());
      assert(status.size()==2);

      time_t t;
      boost::mt19937 generator((unsigned long)time(&t));
      boost::uniform_real<> distribution(0.0, 1.0);
      boost::variate_generator<boost::mt19937, boost::uniform_real<> >
        rand(generator, distribution);


      ////////////////////////////////////////////////////////////////////////////////
      // Read parameters.

      bool directed = false;
      wstring title;
      size_t nsz = 0;
      double ksz = 0.0;

      unordered_map<wstring, pair<wstring, int> >::const_iterator pos;

      if ((pos=params.find(L"directed"))!=params.end())
        directed = true;

      if ((pos=params.find(L"title"))!=params.end() && !pos->second.first.empty())
        title = pos->second.first;

      if ((pos=params.find(L"N"))!=params.end()) {
        try {
          nsz = boost::lexical_cast<size_t>(pos->second.first);
        } catch (...) {
          message_box(get_window_handle(), mb_error, APPLICATION_TITLE,
                      L"bad data: %s [line=%d]",
                      filename.c_str(), pos->second.second);
        }
      }

      if ((pos=params.find(L"K"))!=params.end()) {
        try {
          ksz = boost::lexical_cast<double>(pos->second.first);
        } catch (...) {
          message_box(get_window_handle(), mb_error, APPLICATION_TITLE,
                      L"bad data: %s [line=%d]",
                      filename.c_str(), pos->second.second);
        }
      }


      ////////////////////////////////////////////////////////////////////////////////
      // Make a graph.

      vector<shared_ptr<Graph> >(1, Graph::create(directed)).swap(graph);
      shared_ptr<Graph>& g = graph[0];

      status[0]
        = (boost::wformat(L"%s")
           %message.get(Message::MAKING_GRAPH_SNAPSHOT)).str();

      status[1]
        = (boost::wformat(L"Barabasi-Albert Model [N=%d, E=%d]")
           %g->nsize()%g->esize()).str();

      double const m = 0.5*ksz;
      size_t const m_base = size_t(m);
      double const m_diff = m-double(m_base);
      size_t const seed = (m<=1.0)?2:(m_diff>0.0)?m_base+1:m_base;

      // Create a seed graph.
      for (size_t i=0; i<seed; ++i) {
        // **********  Catch a termination signal  **********
        if (parent.cancel_check()) {
          graph.clear();
          return;
        }

        g->add_node();
      }

      for (size_t i=0; i<seed; ++i) {
        // **********  Catch a termination signal  **********
        if (parent.cancel_check()) {
          graph.clear();
          return;
        }

        for (size_t j=i+1; j<seed; ++j)
          g->add_edge(g->node(i), g->node(j));
      }

      // Grow a graph.
      for (size_t i=seed; i<nsz; ++i) {

        // **********  Catch a termination signal  **********
        if (parent.cancel_check()) {
          graph.clear();
          return;
        }

        Node* ni = g->add_node();
        int const mm = (rand()<m_diff)?m_base+1:m_base;
        int const esz = int(g->esize());

        for (int j=0; j<mm;) {
          Edge* e = g->edge(int(rand()*esz));
          Node* nj = (rand()<0.5)?e->source():e->target();
          if (ni->find(ni->obegin(), ni->oend(), nj)==ni->oend()) {
            g->add_edge(ni, nj);
            ++j;
          }
        }

        status[0]
          = (boost::wformat(L"%s")
             %message.get(Message::MAKING_GRAPH_SNAPSHOT)).str();

        status[1]
          = (boost::wformat(L"Barabasi-Albert Model [N=%d, E=%d]")
             %g->nsize()%g->esize()).str();
      }


      ////////////////////////////////////////////////////////////////////////////////
      // Set properties.

      node_property.resize(1, vector<NodeProperty>(g->nsize()));
      edge_property.resize(1, vector<EdgeProperty>(g->esize()));

      int const digit = int(log10(double(g->nsize())))+1;

      for (size_t i=0, nsz=g->nsize(); i<nsz; ++i) {

        // **********  Catch a termination signal  **********
        if (parent.cancel_check()) {
          graph.clear();
          return;
        }

        NodeProperty& np = node_property[0][i];
        if (digit==1) np.identifier = (boost::wformat(L"n%d")%i).str();
        else if (digit==2) np.identifier = (boost::wformat(L"n%02d")%i).str();
        else if (digit==3) np.identifier = (boost::wformat(L"n%03d")%i).str();
        else if (digit==4) np.identifier = (boost::wformat(L"n%04d")%i).str();
        else if (digit==5) np.identifier = (boost::wformat(L"n%05d")%i).str();
        else np.identifier = (boost::wformat(L"n%d")%i).str();
        np.name = np.identifier;
        np.weight = float(g->node(i)->degree());
      }

      for (size_t i=0, esz=g->esize(); i<esz; ++i) {

        // **********  Catch a termination signal  **********
        if (parent.cancel_check()) {
          graph.clear();
          return;
        }

        Edge* e = g->edge(i);
        wstring const& source = node_property[0][e->source()->index()].name;
        wstring const& target = node_property[0][e->target()->index()].name;
        EdgeProperty& ep = edge_property[0][i];
        ep.identifier = (directed||source<target)?(source+L'~'+target):(target+L'~'+source);
        ep.name = ep.identifier;
        ep.weight = 1.0f;
      }


      ////////////////////////////////////////////////////////////////////////////////
      // Set a layer name.

      layer_name.push_back(title);
    }

} // The end of the namespace "hashimoto_ut"

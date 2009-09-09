// s.o.c.i.a.r.i.u.m: thread/community_detection.cpp
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

#include <vector>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <boost/format.hpp>
#include "community_detection.h"
#include "../module/community_detection.h"
#include "../graph_extractor.h"
#include "../thread_manager.h"
#include "../algorithm_selector.h"
#include "../texture.h"
#include "../color.h"
#include "../language.h"
#include "../sociarium_graph_time_series.h"
#include "../../graph/graphex.h"
#include "../../shared/predefined_color.h"

namespace hashimoto_ut {

  using std::vector;
  using std::deque;
  using std::map;
  using std::wstring;
  using std::pair;
  using std::make_pair;
  using std::tr1::shared_ptr;
  using std::tr1::weak_ptr;
  using std::tr1::unordered_set;
  using std::tr1::unordered_map;
  using std::tr1::unordered_multimap;

  using namespace sociarium_project_module_community_detection;
  using namespace sociarium_project_algorithm_selector;
  using namespace sociarium_project_language;

  typedef SociariumGraphTimeSeries::StaticNodePropertySet StaticNodePropertySet;
  typedef SociariumGraphTimeSeries::StaticEdgePropertySet StaticEdgePropertySet;

  typedef SociariumGraph::node_property_iterator node_property_iterator;
  typedef SociariumGraph::edge_property_iterator edge_property_iterator;

  typedef std::multimap<double,
  pair<DynamicNodeProperty*, DynamicNodeProperty*>, std::greater<double> >
    CommunityMatchingList;

  namespace {
    ////////////////////////////////////////////////////////////////////////////////
    // The threshold value which two communities are considered as the same community
    // if the similarity of their members is greater than.
    double const similarity_threshold = 0.3;


    ////////////////////////////////////////////////////////////////////////////////
    unordered_set<StaticNodeProperty*> shared_elements(
      vector<DynamicNodeProperty*>::const_iterator first0,
      vector<DynamicNodeProperty*>::const_iterator last0,
      vector<DynamicNodeProperty*>::const_iterator first1,
      vector<DynamicNodeProperty*>::const_iterator last1) {

      unordered_set<StaticNodeProperty*> retval;

      for (; first0!=last0; ++first0) {
        StaticNodeProperty* snp = (*first0)->get_static_property();
        for (vector<DynamicNodeProperty*>::const_iterator i=first1; i!=last1; ++i) {
          if (snp==(*i)->get_static_property()) {
            retval.insert(snp);
            break;
          }
        }
      }

      return retval;
    }

    ////////////////////////////////////////////////////////////////////////////////
    unordered_set<StaticNodeProperty*> unique_elements(
      vector<DynamicNodeProperty*>::const_iterator first0,
      vector<DynamicNodeProperty*>::const_iterator last0,
      vector<DynamicNodeProperty*>::const_iterator first1,
      vector<DynamicNodeProperty*>::const_iterator last1) {

      unordered_set<StaticNodeProperty*> retval;

      for (; first0!=last0; ++first0)
        retval.insert((*first0)->get_static_property());

      for (; first1!=last1; ++first1)
        retval.insert((*first1)->get_static_property());

      return retval;
    }

    ////////////////////////////////////////////////////////////////////////////////
    vector<StaticNodeProperty*> diff_elements(
      vector<DynamicNodeProperty*>::const_iterator first,
      vector<DynamicNodeProperty*>::const_iterator last,
      unordered_set<StaticNodeProperty*> const& s) {

      vector<StaticNodeProperty*> retval;

      for (; first!=last; ++first) {
        StaticNodeProperty* snp = (*first)->get_static_property();
        if (s.find(snp)==s.end()) retval.push_back(snp);
      }

      return retval;
    }

    ////////////////////////////////////////////////////////////////////////////////
    CommunityMatchingList similarity(
      node_property_iterator first0,
      node_property_iterator last0,
      node_property_iterator first1,
      node_property_iterator last1) {

      CommunityMatchingList retval;

      for (; first0!=last0; ++first0) {
        DynamicNodeProperty& dnp0 = first0->second;
        for (node_property_iterator i=first1; i!=last1; ++i) {
          DynamicNodeProperty& dnp1 = i->second;

          double const num0 =
            shared_elements(
              dnp0.lower_nbegin(), dnp0.lower_nend(),
              dnp1.lower_nbegin(), dnp1.lower_nend()).size();

          double const num1 =
            unique_elements(
              dnp0.lower_nbegin(), dnp0.lower_nend(),
              dnp1.lower_nbegin(), dnp1.lower_nend()).size();

          assert(num1>0.0);
          retval.insert(make_pair(num0/num1, make_pair(&dnp0, &dnp1)));
        }
      }

      return retval;
    }

  } // The end of the anonemouse namespace


  class CommunityDetectionThreadImpl : public CommunityDetectionThread {
  public:
    typedef unordered_map<StaticNodeProperty*, StaticEdgeProperty*> SNP2SEP;
    typedef unordered_multimap<StaticNodeProperty*, SNP2SEP> CommunityEdgeIdentifier;

    ////////////////////////////////////////////////////////////////////////////////
    CommunityDetectionThreadImpl(shared_ptr<ThreadManager> thread_manager)
         : thread_manager_(thread_manager) {}


    ////////////////////////////////////////////////////////////////////////////////
    ~CommunityDetectionThreadImpl() {}


    ////////////////////////////////////////////////////////////////////////////////
    void terminate(void) const {

      using namespace sociarium_project_thread_manager;

      // Clear the progress message.
      deque<wstring>& status = get_status(COMMUNITY_DETECTION);
      deque<wstring>(status.size()).swap(status);

      // Delete itself via ThreadManager.
      shared_ptr<ThreadManager> tm = thread_manager_.lock();
      assert(tm!=0);
      tm->set(shared_ptr<Thread>());
    }


    ////////////////////////////////////////////////////////////////////////////////
    void operator()(void) {

      shared_ptr<SociariumGraphTimeSeries> ts = sociarium_project_graph_time_series::get();

      ts->read_lock();
      /*
       * Don't forget to call read_unlock().
       */

      deque<wstring>& status
        = sociarium_project_thread_manager::get_status(
          sociarium_project_thread_manager::COMMUNITY_DETECTION);

      // --------------------------------------------------------------------------------
      // Load a community detection module.

      FuncDetectCommunity detect_community = get(get_community_detection_algorithm());

      if (detect_community==0) {
        ts->read_unlock();
        return terminate();
      }

      // --------------------------------------------------------------------------------
      // Detect communities in graphs consisting of "visible" elements in all layers.

      size_t const number_of_layers = ts->number_of_layers();
      vector<shared_ptr<SociariumGraph> > community_series(number_of_layers);

      for (size_t layer=0; layer<number_of_layers; ++layer) {

        // **********  Catch a termination signal  **********
        if (cancel_check()) {
          ts->read_unlock();
          return terminate();
        }

        status[0]
          = number_of_layers<2?
            (boost::wformat(L"%s")
             %get_message(Message::DETECTING_COMMUNITIES)).str()
              :(boost::wformat(L"%s: %d%%")
                %get_message(Message::DETECTING_COMMUNITIES)
                %int((100.0*layer)/number_of_layers)).str();

        // --------------------------------------------------------------------------------
        // Extract marked elements.

        shared_ptr<SociariumGraph> g = ts->get_graph(0, layer);

        unordered_map<Node*, Node const*> node2node; // Map nodes in @g_target to nodes in @g.
        unordered_map<Edge*, Edge const*> edge2edge; // Map edges in @g_target to edges in @g.

        pair<bool, shared_ptr<Graph const> > const ext
          = sociarium_project_graph_extractor::get(
            this, &status[1], g, node2node, edge2edge, ElementFlag::VISIBLE);

        if (ext.first==false) {
          ts->read_unlock();
          return terminate();
        }

        shared_ptr<Graph const> g_target = ext.second;

        if (g_target->nsize()==0) {
          community_series[layer] = SociariumGraph::create(g_target->is_directed());
          continue;
        }

        // **********  Catch a termination signal  **********
        if (cancel_check()) {
          ts->read_unlock();
          return terminate();
        }

        vector<double> edge_weight;
        edge_weight.reserve(g_target->esize());

        if (use_weighted_modularity()) {
          for (edge_iterator i=g_target->ebegin(); i!=g_target->eend(); ++i)
            edge_weight.push_back(double(g->property(edge2edge[*i]).get_weight()));
        }

        // --------------------------------------------------------------------------------
        // Execute the module.

        vector<vector<Node*> > community;
        bool is_canceled = false;

        detect_community(
          this,
          &status[1],
          get_message_object(),
          community,
          is_canceled,
          g_target,
          edge_weight);

        if (is_canceled) {
          ts->read_unlock();
          return terminate();
        }

        // --------------------------------------------------------------------------------
        // Create a community graph.

        size_t const csz = community.size();
        shared_ptr<SociariumGraph> cg(SociariumGraph::create(g_target->is_directed()));
        vector<vector<DynamicNodeProperty*> > upper_nodes(g_target->nsize());

        // Make community nodes.
        for (size_t i=0; i<csz; ++i) {
          Node* n = cg->add_node();
          vector<Node*> const& cn = community[i];
          vector<Edge*> const ce = induced_edges(cn.begin(), cn.end());
          DynamicNodeProperty& dnp
            = link_dynamic_property_and_graph_element<DynamicNodeProperty>(cg, n);

          dnp.set_flag(ElementFlag::VISIBLE);
          dnp.set_weight(0.0);
          dnp.set_size(sqrtf(float(cn.size())));

          for (vector<Node*>::const_iterator j=cn.begin(), jend=cn.end(); j!=jend; ++j) {
            dnp.register_lower_element(&g->property(node2node[*j]));
            upper_nodes[(*j)->index()].push_back(&dnp);
          }

          for (vector<Edge*>::const_iterator j=ce.begin(), jend=ce.end(); j!=jend; ++j)
            dnp.register_lower_element(&g->property(edge2edge[*j]));
          /*
           * Don't forget to call "register_upper_element()" in
           * SociariumGraphTimeSeries::update_community().
           */
        }

        assert(cg->nsize()==csz);

        // **********  Catch a termination signal  **********
        if (cancel_check()) {
          ts->read_unlock();
          return terminate();
        }

        // Make community edges.
        vector<vector<double> > weight(csz, vector<double>(csz, 0.0));

        for (edge_iterator i=g_target->ebegin(); i!=g_target->eend(); ++i) {

          DynamicEdgeProperty& dep = g->property(edge2edge[*i]);

          vector<DynamicNodeProperty*> const& un0 = upper_nodes[(*i)->source()->index()];
          vector<DynamicNodeProperty*> const& un1 = upper_nodes[(*i)->target()->index()];

          double const w = dep.get_weight()/(un0.size()*un1.size());

          vector<DynamicNodeProperty*>::const_iterator j    = un0.begin();
          vector<DynamicNodeProperty*>::const_iterator jend = un0.end();

          for (; j!=jend; ++j) {

            size_t const jj = (*j)->get_graph_element()->index();

            vector<DynamicNodeProperty*>::const_iterator k    = un1.begin();
            vector<DynamicNodeProperty*>::const_iterator kend = un1.end();

            for (; k!=kend; ++k) {
              if (*j!=*k) {
                size_t const kk = (*k)->get_graph_element()->index();
                weight[jj][kk] += w;
              }
            }
          }
        }

        // **********  Catch a termination signal  **********
        if (cancel_check()) {
          ts->read_unlock();
          return terminate();
        }

        for (size_t i=0; i<csz; ++i) {
          for (size_t j=i+1; j<csz; ++j) {

            // Directed graph.
            if (cg->is_directed()) {
              if (weight[i][j]>0.0) {
                Edge* e = cg->add_edge(cg->node(i), cg->node(j));

                DynamicEdgeProperty& dep
                  = link_dynamic_property_and_graph_element<DynamicEdgeProperty>(cg, e);

                dep.set_flag(ElementFlag::VISIBLE);
                dep.set_color_id(
                  sociarium_project_color::get_default_community_edge_color_id());
                dep.set_weight(sqrtf(float(weight[i][j])));
                dep.set_width(sqrtf(dep.get_weight()));
              }

              if (weight[j][i]>0.0) {
                Edge* e = cg->add_edge(cg->node(j), cg->node(i));

                DynamicEdgeProperty& dep
                  = link_dynamic_property_and_graph_element<DynamicEdgeProperty>(cg, e);

                dep.set_flag(ElementFlag::VISIBLE);
                dep.set_color_id(
                  sociarium_project_color::get_default_community_edge_color_id());
                dep.set_weight(sqrtf(float(weight[j][i])));
                dep.set_width(sqrtf(dep.get_weight()));
              }
            }

            // Undirected graph.
            else {
              if (weight[i][j]+weight[j][i]>0.0) {
                Node* c0 = cg->node(i);
                Node* c1 = cg->node(j);
                Edge* e = c0<c1?cg->add_edge(c0, c1):cg->add_edge(c1, c0);
                DynamicEdgeProperty& dep
                  = link_dynamic_property_and_graph_element<DynamicEdgeProperty>(cg, e);

                dep.set_flag(ElementFlag::VISIBLE);
                dep.set_color_id(
                  sociarium_project_color::get_default_community_edge_color_id());
                dep.set_weight(sqrtf(float(weight[i][j]+weight[j][i])));
                dep.set_width(sqrtf(dep.get_weight()));
              }
            }
          }
        }

        community_series[layer] = cg;

        status[0]
          = number_of_layers<2
            ?(boost::wformat(L"%s")
              %get_message(Message::DETECTING_COMMUNITIES)).str()
              :(boost::wformat(L"%s: %d%%")
                %get_message(Message::DETECTING_COMMUNITIES)
                %int(100.0*(layer+1.0)/number_of_layers)).str();
      }

      status[1] = L"";

      // --------------------------------------------------------------------------------
      // Identify community pairs in successive layers.

      static_community_property_.clear();
      static_community_edge_property_.clear();
      community_edge_identifier_.clear();

      size_t const current_layer = ts->index_of_current_layer();

      { // Make a base layer for the identification.
        shared_ptr<SociariumGraph> cg = community_series[current_layer];

        {
          node_property_iterator i   = cg->node_property_begin();
          node_property_iterator end = cg->node_property_end();

          for (; i!=end; ++i) {

            pair<StaticNodePropertySet::iterator, bool> pp
              = static_community_property_.insert(
                StaticNodeProperty(static_community_property_.size()));

            assert(pp.second);

            StaticNodeProperty* snp = &*pp.first;

            DynamicNodeProperty& dnp = i->second;
            link_dynamic_and_static_properties(current_layer, &dnp, snp);

            snp->set_name((boost::wformat(L"%d")%snp->get_id()).str());
            snp->set_texture(sociarium_project_texture::get_default_community_texture());

            dnp.set_color_id(snp->get_id()%predefined_color.number_of_custom_colors()
                             +predefined_color.number_of_reserved_colors());
          }
        }

        // **********  Catch a termination signal  **********
        if (cancel_check()) {
          ts->read_unlock();
          return terminate();
        }

        {
          edge_property_iterator i   = cg->edge_property_begin();
          edge_property_iterator end = cg->edge_property_end();

          for (; i!=end; ++i) {

            pair<StaticEdgePropertySet::iterator, bool> pp
              = static_community_edge_property_.insert(
                StaticEdgeProperty(static_community_edge_property_.size()));

            assert(pp.second);

            StaticEdgeProperty* sep = &*pp.first;

            DynamicEdgeProperty& dep = i->second;
            link_dynamic_and_static_properties(current_layer, &dep, sep);

            sep->set_name((boost::wformat(L"%d")%sep->get_id()).str());
            sep->set_texture(sociarium_project_texture::get_default_edge_texture());

            StaticNodeProperty* snp0
              = cg->property(i->first->source()).get_static_property();
            StaticNodeProperty* snp1
              = cg->property(i->first->target()).get_static_property();

            SNP2SEP m;
            m.insert(make_pair(snp1, sep));
            community_edge_identifier_.insert(make_pair(snp0, m));
          }
        }
      }

      int count = 0;

      // Start iteration of the identification [forward].
      for (size_t layer=current_layer+1; layer<number_of_layers; ++layer) {

        // **********  Catch a termination signal  **********
        if (cancel_check()) {
          ts->read_unlock();
          return terminate();
        }

        status[1]
          = (boost::wformat(L"%s: %d%%")
             %get_message(Message::MAKING_COMMUNITY_TIME_SERIES)
             %int((100.0*count++)/number_of_layers)).str();

        identify_communities(
          layer, community_series[layer-1], community_series[layer]);
      }

      // Start iteration of the identification [backward].
      if (current_layer!=0) {
        for (size_t layer=current_layer-1; layer!=size_t(-1); --layer) {

          // **********  Catch a termination signal  **********
          if (cancel_check()) {
            ts->read_unlock();
            return terminate();
          }

          status[1]
            = (boost::wformat(L"%s: %d%%")
               %get_message(Message::MAKING_COMMUNITY_TIME_SERIES)
               %int((100.0*count++)/number_of_layers)).str();

          identify_communities(
            layer, community_series[layer+1], community_series[layer]);
        }
      }

      ts->update_community(community_series,
                           static_community_property_,
                           static_community_edge_property_);
      ts->read_unlock();
      return terminate();
    }


    ////////////////////////////////////////////////////////////////////////////////
    void identify_communities(
      size_t layer, // The layer of @target.
      shared_ptr<SociariumGraph> source,
      shared_ptr<SociariumGraph> target) {

      vector<int> is_identified0(source->nsize(), 0);
      vector<int> is_identified1(target->nsize(), 0);

      CommunityMatchingList s = similarity(
        source->node_property_begin(), source->node_property_end(),
        target->node_property_begin(), target->node_property_end());

      {
        // Identify community pairs in descending order of their similarity.
        CommunityMatchingList::const_iterator i   = s.begin();
        CommunityMatchingList::const_iterator end = s.end();

        for (; i!=end && i->first>0; ++i) {

          DynamicNodeProperty* dnp0 = i->second.first;
          DynamicNodeProperty* dnp1 = i->second.second;

          if (i->first>similarity_threshold) {

            size_t const index0 = dnp0->get_graph_element()->index();
            size_t const index1 = dnp1->get_graph_element()->index();

            if (is_identified0[index0]==0 && is_identified1[index1]==0) {

              StaticNodeProperty* snp = dnp0->get_static_property();

              link_dynamic_and_static_properties(layer, dnp1, snp);

              dnp1->set_color_id(snp->get_id()%predefined_color.number_of_custom_colors()
                                 +predefined_color.number_of_reserved_colors());

              is_identified0[index0] = 1;
              is_identified1[index1] = 1;
            }
          }
        }
      }

      {
        // If not identified, the community is regarded as a newly appeared community,
        // and make a new static property.
        node_property_iterator i   = target->node_property_begin();
        node_property_iterator end = target->node_property_end();

        for (; i!=end; ++i) {
          if (is_identified1[i->first->index()]==0) {

            pair<StaticNodePropertySet::iterator, bool> pp
              = static_community_property_.insert(
                StaticNodeProperty(static_community_property_.size()));

            assert(pp.second);

            StaticNodeProperty* snp = &*pp.first;

            DynamicNodeProperty& dnp = i->second;

            link_dynamic_and_static_properties(layer, &dnp, snp);

            snp->set_name((boost::wformat(L"%d")%snp->get_id()).str());
            snp->set_texture(sociarium_project_texture::get_default_community_texture());

            dnp.set_color_id(snp->get_id()%predefined_color.number_of_custom_colors()
                             +predefined_color.number_of_reserved_colors());
          }
        }
      }

      {
        // Identify community edges.
        edge_property_iterator i   = target->edge_property_begin();
        edge_property_iterator end = target->edge_property_end();

        for (; i!=end; ++i) {

          DynamicEdgeProperty& dep = i->second;
          DynamicNodeProperty& dnp0 = target->property(i->first->source());
          DynamicNodeProperty& dnp1 = target->property(i->first->target());

          StaticNodeProperty* snp0 = dnp0.get_static_property();
          StaticNodeProperty* snp1 = dnp1.get_static_property();

          CommunityEdgeIdentifier::iterator p0 = community_edge_identifier_.find(snp0);

          if (p0==community_edge_identifier_.end()) {

            pair<StaticEdgePropertySet::iterator, bool> pp
              = static_community_edge_property_.insert(
                StaticEdgeProperty(static_community_edge_property_.size()));

            assert(pp.second);

            StaticEdgeProperty* sep = &*pp.first;

            link_dynamic_and_static_properties(layer, &dep, sep);

            sep->set_name((boost::wformat(L"%d")%sep->get_id()).str());
            sep->set_texture(sociarium_project_texture::get_default_edge_texture());

            SNP2SEP m;
            m.insert(make_pair(snp1, sep));
            community_edge_identifier_.insert(make_pair(snp0, m));

          } else {

            SNP2SEP::iterator p1 = p0->second.find(snp1);

            if (p1==p0->second.end()) {

              pair<StaticEdgePropertySet::iterator, bool> pp
                = static_community_edge_property_.insert(
                  StaticEdgeProperty(static_community_edge_property_.size()));

              assert(pp.second);

              StaticEdgeProperty* sep = &*pp.first;

              link_dynamic_and_static_properties(layer, &dep, sep);

              sep->set_name((boost::wformat(L"%d")%sep->get_id()).str());
              sep->set_texture(sociarium_project_texture::get_default_edge_texture());

              p0->second.insert(make_pair(snp1, sep));

            } else
              link_dynamic_and_static_properties(layer, &dep, p1->second);
          }
        }
      }
    }

  private:
    weak_ptr<ThreadManager> thread_manager_;
    StaticNodePropertySet static_community_property_;
    StaticEdgePropertySet static_community_edge_property_;
    CommunityEdgeIdentifier community_edge_identifier_;
  };


  ////////////////////////////////////////////////////////////////////////////////
  // Factory function of CommunityDetectionThread.
  shared_ptr<CommunityDetectionThread> CommunityDetectionThread::create(
    shared_ptr<ThreadManager> thread_manager) {
    return shared_ptr<CommunityDetectionThread>(
      new CommunityDetectionThreadImpl(thread_manager));
  }

} // The end of the namespace "hashimoto_ut"

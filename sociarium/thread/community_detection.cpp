// s.o.c.i.a.r.i.u.m
// HASHIMOTO, Yasuhiro (E-mail: hy @ sys.t.u-tokyo.ac.jp)
// update: 2009/04/24

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

#include <vector>
#include <map>
#ifdef _MSC_VER
#include <unordered_set>
#include <unordered_map>
#else
#include <tr1/unordered_set>
#include <tr1/unordered_map>
#endif
#include <boost/format.hpp>
#include "community_detection.h"
#include "../module/community_detection.h"
#include "../graph_extractor.h"
#include "../common.h"
#include "../message.h"
#include "../thread.h"
#include "../algorithm_selector.h"
#include "../texture.h"
#include "../draw.h"
#include "../sociarium_graph_time_series.h"
#include "../../shared/msgbox.h"
#include "../../shared/predefined_color.h"
#include "../../graph/graphex.h"

namespace hashimoto_ut {

  using std::vector;
  using std::map;
  using std::wstring;
  using std::pair;
  using std::make_pair;
  using std::tr1::unordered_set;
  using std::tr1::unordered_map;
  using std::tr1::unordered_multimap;
  using std::tr1::shared_ptr;

  typedef SociariumGraph::node_property_iterator node_property_iterator;
  typedef SociariumGraph::edge_property_iterator edge_property_iterator;

  typedef shared_ptr<DynamicNodeProperty> DNP;
  typedef shared_ptr<DynamicEdgeProperty> DEP;
  typedef shared_ptr<StaticNodeProperty> SNP;
  typedef shared_ptr<StaticEdgeProperty> SEP;
  typedef std::multimap<double, pair<DNP, DNP>, std::greater<double> > CommunityMatchingMap;

  namespace {
    double const similarity_threshold = 0.2;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // 共有メンバー数
    template <typename T>
    double number_of_shared_elements(T first0, T last0, T first1, T last1) {
      double retval = 0.0;
      for (; first0!=last0; ++first0) {
        for (T i=first1; i!=last1; ++i) {
          if ((*first0)->get_static_property()==(*i)->get_static_property()) {
            ++retval;
            break;
          }
        }
      }
      return retval;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // ユニークメンバー数
    template <typename T>
    double number_of_unique_elements(T first0, T last0, T first1, T last1) {
      double retval = 0.0;
      unordered_set<StaticNodeProperty*> s;
      for (; first0!=last0; ++first0) s.insert((*first0)->get_static_property().get());
      for (; first1!=last1; ++first1) s.insert((*first1)->get_static_property().get());
      retval = double(s.size());
      return retval;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // コミュニティ一致度(共有メンバー数/ユニークメンバー数)の降順リスト
    CommunityMatchingMap similarity(node_property_iterator first0, node_property_iterator last0,
                                    node_property_iterator first1, node_property_iterator last1) {
      CommunityMatchingMap retval;
      for (; first0!=last0; ++first0) {
        DNP const& dnp0 = first0->second;
        for (node_property_iterator i=first1; i!=last1; ++i) {
          DNP const& dnp1 = i->second;
          double const num0 = number_of_shared_elements(dnp0->lower_nbegin(), dnp0->lower_nend(), dnp1->lower_nbegin(), dnp1->lower_nend());
          double const num1 = number_of_unique_elements(dnp0->lower_nbegin(), dnp0->lower_nend(), dnp1->lower_nbegin(), dnp1->lower_nend());
          assert(num1>0.0);
          retval.insert(make_pair(num0/num1, make_pair(dnp0, dnp1)));
        }
      }
      return retval;
    }
  }


  class CommunityDetectionThreadImpl : public CommunityDetectionThread {
  public:
    typedef unordered_map<SNP, SEP, shared_ptr_hash<StaticNodeProperty> > SNP2SEP;
    typedef unordered_multimap<SNP, SNP2SEP, shared_ptr_hash<StaticNodeProperty> > CommunityEdgeIdentifier;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    CommunityDetectionThreadImpl(shared_ptr<SociariumGraphTimeSeries> const& time_series)
         : time_series_(time_series) {}

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    void terminate(void) const {
      sociarium_project_message::get()->first = L"";
      sociarium_project_message::get()->second = L"";
      time_series_->read_unlock();
      sociarium_project_thread::set_current_community_detection_thread(shared_ptr<Thread>());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    void operator()(void) {

      time_series_->read_lock();

      // ----------------------------------------------------------------------------------------------------
      // コミュニティ検出モジュールのロード
      sociarium_project_message::get()->first = (boost::wformat(L"%s")%sociarium_project_message::LOADING_MODULE).str();
      FuncDetectCommunity detect_community = sociarium_project_module_community_detection::get(sociarium_project_algorithm_selector::get_community_detection_algorithm());
      if (detect_community==0) return terminate();

      // ----------------------------------------------------------------------------------------------------
      // 各レイヤーの表示された要素からなるグラフに対してコミュニティ検出を実行
      size_t const number_of_layers = time_series_->number_of_layers();
      vector<shared_ptr<SociariumGraph> > community_series(number_of_layers);
      vector<double> modularity(number_of_layers, 0.0);

      for (size_t layer=0; layer<number_of_layers; ++layer) {

        sociarium_project_message::get()->first = number_of_layers>1?
          (boost::wformat(L"%s: %d%%")%sociarium_project_message::DETECTING_COMMUNITIES%int((100.0*layer)/number_of_layers)).str()
            :(boost::wformat(L"%s")%sociarium_project_message::DETECTING_COMMUNITIES).str();

        // ----------------------------------------------------------------------------------------------------
        if (cancel_check()) return terminate();
        // ----------------------------------------------------------------------------------------------------

        // 表示された要素のみからなるグラフを構築
        shared_ptr<SociariumGraph> const& g = time_series_->get_graph(0, layer);
        unordered_map<Node*, Node const*> node2node; // g_targetのノードからgのノードを取得
        unordered_map<Edge*, Edge const*> edge2edge; // g_targetのエッジからgのエッジを取得
        shared_ptr<Graph const> g_target = sociarium_project_graph_extractor::get(this, &sociarium_project_message::get()->second, g, node2node, edge2edge, ElementFlag::VISIBLE);

        if (g_target==0) {
          return terminate();
        } else if (g_target->nsize()==0) {
          community_series[layer] = SociariumGraph::create(g_target->is_directed());
          continue;
        }

        vector<double> edge_weight;
        edge_weight.reserve(g_target->esize());
        if (sociarium_project_algorithm_selector::use_weighted_modularity()) {
          for (edge_iterator i=g_target->ebegin(); i!=g_target->eend(); ++i)
            edge_weight.push_back(double(g->property(edge2edge[*i])->get_weight()));
        }

        // 表示された要素のみからなるグラフに対してコミュニティ検出
        vector<vector<Node*> > community;
        bool is_canceled = false;
        detect_community(this, &sociarium_project_message::get()->second, g_target, edge_weight, community, is_canceled);
        if (is_canceled) return terminate();
        //modularity[layer],
        
        // コミュニティをノードとするコミュニティグラフを構築
        size_t const csz = community.size();
        shared_ptr<SociariumGraph> cg(SociariumGraph::create(g_target->is_directed()));
        vector<vector<DynamicNodeProperty*> > upper_nodes(g_target->nsize());
        for (size_t i=0; i<csz; ++i) {
          Node* n = cg->add_node();
          vector<Node*> const& cn = community[i];
          vector<Edge*> const ce = induced_edges(cn.begin(), cn.end());
          DNP const& dnp = cg->property(n);
          dnp->set_graph_element(n);
          dnp->set_flag(ElementFlag::VISIBLE);
          dnp->set_text(L"");
          dnp->set_weight(0.0);
          dnp->set_size(sociarium_project_draw::get_default_node_size()*(float)sqrt(double(cn.size())));
          // ↓register_upper_elementをupdate時に忘れずに
          for (vector<Node*>::const_iterator j=cn.begin(), jend=cn.end(); j!=jend; ++j)
            dnp->register_lower_element(g->property(node2node[*j]).get()), upper_nodes[(*j)->index()].push_back(dnp.get());
          for (vector<Edge*>::const_iterator j=ce.begin(), jend=ce.end(); j!=jend; ++j)
            dnp->register_lower_element(g->property(edge2edge[*j]).get());
        }

        assert(cg->nsize()==csz);

        // コミュニティ間のエッジ数をカウントしてコミュニティエッジを作成
        vector<vector<double> > weight(csz, vector<double>(csz, 0.0));
        for (edge_iterator i=g_target->ebegin(), end=g_target->eend(); i!=end; ++i) {
          DEP const& dep = g->property(edge2edge[*i]);
          vector<DynamicNodeProperty*> const& un0 = upper_nodes[(*i)->source()->index()];
          vector<DynamicNodeProperty*> const& un1 = upper_nodes[(*i)->target()->index()];
          double const w = dep->get_weight()/(un0.size()*un1.size());
          for (vector<DynamicNodeProperty*>::const_iterator j=un0.begin(), jend=un0.end(); j!=jend; ++j)
            for (vector<DynamicNodeProperty*>::const_iterator k=un1.begin(), kend=un1.end(); k!=kend; ++k)
              if (*j!=*k) weight[(*j)->get_graph_element()->index()][(*k)->get_graph_element()->index()] += w;
        }

        for (size_t i=0; i<csz; ++i) {
          for (size_t j=i+1; j<csz; ++j) {
            if (cg->is_directed()) {
              // 有向グラフの場合, コミュニティグラフも有向グラフ
              if (weight[i][j]>0.0) {
                Edge* e = cg->add_edge(cg->node(i), cg->node(j));
                DEP const& dep = cg->property(e);
                dep->set_graph_element(e);
                dep->set_flag(ElementFlag::VISIBLE);
                dep->set_color_id(PredefinedColor::LIGHT_GRAY);
                dep->set_text(L"");
                dep->set_weight(float(weight[i][j]));
                dep->set_length(1.0f);
              }
              if (weight[j][i]>0.0) {
                Edge* e = cg->add_edge(cg->node(j), cg->node(i));
                DEP const& dep = cg->property(e);
                dep->set_graph_element(e);
                dep->set_flag(ElementFlag::VISIBLE);
                dep->set_color_id(PredefinedColor::LIGHT_GRAY);
                dep->set_text(L"");
                dep->set_weight(float(weight[j][i]));
                dep->set_length(1.0f);
              }
            } else {
              if (weight[i][j]>0.0 || weight[j][i]>0.0) {
                Node* c0 = cg->node(i);
                Node* c1 = cg->node(j);
                Edge* e = c0<c1?cg->add_edge(c0, c1):cg->add_edge(c1, c0);
                DEP const& dep = cg->property(e);
                dep->set_graph_element(e);
                dep->set_flag(ElementFlag::VISIBLE);
                dep->set_color_id(PredefinedColor::LIGHT_GRAY);
                dep->set_text(L"");
                dep->set_weight(float(weight[i][j]+weight[j][i]));
                dep->set_length(1.0f);
              }
            }
          }
        }

        community_series[layer] = cg;

        sociarium_project_message::get()->first = number_of_layers>1?
          (boost::wformat(L"%s: %d%%")%sociarium_project_message::DETECTING_COMMUNITIES%int(100.0*(layer+1.0)/number_of_layers)).str()
            :(boost::wformat(L"%s")%sociarium_project_message::DETECTING_COMMUNITIES).str();
      }

      // ----------------------------------------------------------------------------------------------------
      // 時刻の前後でコミュニティを関連付ける
      static_community_property_.clear();
      static_community_edge_property_.clear();
      community_edge_identifier_.clear();

      // 時刻間同定の起点となるレイヤーのコミュニティを決定
      size_t const layer = time_series_->index_of_current_layer();
      shared_ptr<SociariumGraph> const& cg = community_series[layer];

      for (SociariumGraph::node_property_iterator i=cg->node_property_begin(), end=cg->node_property_end(); i!=end; ++i) {
        SNP snp(new StaticNodeProperty(static_community_property_.size()));
        static_community_property_.insert(snp);
        snp->set_name((boost::wformat(L"%d")%snp->get_id()).str());
        snp->set_texture(sociarium_project_texture::get_default_community_texture());
        DNP const& dnp = i->second;
        dnp->set_static_property(snp);
        dnp->set_color_id(snp->get_id()%predefined_color.number_of_custom_colors()+predefined_color.number_of_reserved_colors());
        snp->add_dynamic_property(dnp.get(), layer);
      }

      for (SociariumGraph::edge_property_iterator i=cg->edge_property_begin(), end=cg->edge_property_end(); i!=end; ++i) {
        SEP sep(new StaticEdgeProperty(static_community_edge_property_.size()));
        static_community_edge_property_.insert(sep);
        sep->set_name((boost::wformat(L"%d")%sep->get_id()).str());
        sep->set_texture(sociarium_project_texture::get_default_edge_texture());
        DEP const& dep = i->second;
        dep->set_static_property(sep);
        sep->add_dynamic_property(dep.get(), layer);

        SNP const& snp0 = cg->property(i->first->source())->get_static_property();
        SNP const& snp1 = cg->property(i->first->target())->get_static_property();
        SNP2SEP m;
        m.insert(make_pair(snp1, sep));
        community_edge_identifier_.insert(make_pair(snp0, m));
      }

      // 現在のレイヤーを起点に他のレイヤーのコミュニティとコミュニティエッジを同定
      int const current_layer = time_series_->index_of_current_layer();
      int count = 0;
      for (size_t layer=current_layer+1; layer<number_of_layers; ++layer) {

        sociarium_project_message::get()->first =
          (boost::wformat(L"%s: %d%%")%sociarium_project_message::IDENTIFYING_COMMUNITIES%int((100.0*count++)/number_of_layers)).str();

        // ----------------------------------------------------------------------------------------------------
        if (cancel_check()) return terminate();
        // ----------------------------------------------------------------------------------------------------

        identify_communities(layer, community_series[layer-1], community_series[layer]);
      }

      if (current_layer!=0) {
        for (size_t layer=current_layer-1; layer!=size_t(-1); --layer) {

          sociarium_project_message::get()->first =
            (boost::wformat(L"%s: %d%%")%sociarium_project_message::IDENTIFYING_COMMUNITIES%int((100.0*count++)/number_of_layers)).str();

          // ----------------------------------------------------------------------------------------------------
          if (cancel_check()) return terminate();
          // ----------------------------------------------------------------------------------------------------

          identify_communities(layer, community_series[layer+1], community_series[layer]);
        }
      }

      time_series_->update_community(community_series, static_community_property_, static_community_edge_property_);
      return terminate();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    void identify_communities(size_t layer, shared_ptr<SociariumGraph>& source, shared_ptr<SociariumGraph>& target) {
      vector<int> is_identified0(source->nsize(), 0);
      vector<int> is_identified1(target->nsize(), 0);
      CommunityMatchingMap s = similarity(source->node_property_begin(), source->node_property_end(),
                                          target->node_property_begin(), target->node_property_end());
      // 一致度の高いコミュニティから同定
      for (CommunityMatchingMap::const_iterator i=s.begin(); i!=s.end() && i->first>similarity_threshold; ++i) {
        DNP const& dnp0 = i->second.first;
        DNP const& dnp1 = i->second.second;
        size_t const index0 = dnp0->get_graph_element()->index();
        size_t const index1 = dnp1->get_graph_element()->index();
        if (is_identified0[index0]==0 && is_identified1[index1]==0) {
          SNP const& snp = dnp0->get_static_property();
          dnp1->set_static_property(snp);
          dnp1->set_color_id(snp->get_id()%predefined_color.number_of_custom_colors()+predefined_color.number_of_reserved_colors());
          snp->add_dynamic_property(dnp1.get(), layer);
          is_identified0[index0] = 1;
          is_identified1[index1] = 1;
        }
      }

      // 同定されなかったコミュニティに対して静的属性を生成
      for (node_property_iterator i=target->node_property_begin(); i!=target->node_property_end(); ++i) {
        if (is_identified1[i->first->index()]==0) {
          SNP snp(new StaticNodeProperty(static_community_property_.size()));
          static_community_property_.insert(snp);
          snp->set_name((boost::wformat(L"%d")%snp->get_id()).str());
          snp->set_texture(sociarium_project_texture::get_default_community_texture());
          DNP const& dnp = i->second;
          dnp->set_static_property(snp);
          dnp->set_color_id(snp->get_id()%predefined_color.number_of_custom_colors()+predefined_color.number_of_reserved_colors());
          snp->add_dynamic_property(dnp.get(), layer);
        }
      }

      // コミュニティエッジを同定
      for (edge_property_iterator i=target->edge_property_begin(); i!=target->edge_property_end(); ++i) {
        DEP const& dep = i->second;
        DNP const& dnp0 = target->property(i->first->source());
        DNP const& dnp1 = target->property(i->first->target());
        SNP const& snp0 = dnp0->get_static_property();
        SNP const& snp1 = dnp1->get_static_property();
        CommunityEdgeIdentifier::iterator p0 = community_edge_identifier_.find(snp0);

        if (p0==community_edge_identifier_.end()) {
          // コミュニティエッジの静的属性を生成
          SEP sep(new StaticEdgeProperty(static_community_edge_property_.size()));
          static_community_edge_property_.insert(sep);
          sep->set_name((boost::wformat(L"%d")%sep->get_id()).str());
          sep->set_texture(sociarium_project_texture::get_default_edge_texture());
          dep->set_static_property(sep);
          sep->add_dynamic_property(dep.get(), layer);

          SNP2SEP m;
          m.insert(make_pair(snp1, sep));
          community_edge_identifier_.insert(make_pair(snp0, m));

        } else {
          SNP2SEP::iterator p1 = p0->second.find(snp1);
          if (p1==p0->second.end()) {
            // コミュニティエッジの静的属性を生成
            SEP sep(new StaticEdgeProperty(static_community_edge_property_.size()));
            static_community_edge_property_.insert(sep);
            sep->set_name((boost::wformat(L"%d")%sep->get_id()).str());
            sep->set_texture(sociarium_project_texture::get_default_edge_texture());
            dep->set_static_property(sep);
            sep->add_dynamic_property(dep.get(), layer);
            p0->second.insert(make_pair(snp1, sep));
          } else {
            // コミュニティエッジを同定
            SEP const& sep = p1->second;
            dep->set_static_property(sep);
            sep->add_dynamic_property(dep.get(), layer);
          }
        }
      }
    }

  private:
    shared_ptr<SociariumGraphTimeSeries> const& time_series_;
    SociariumGraphTimeSeries::StaticNodePropertyContainer static_community_property_;
    SociariumGraphTimeSeries::StaticEdgePropertyContainer static_community_edge_property_;
    CommunityEdgeIdentifier community_edge_identifier_;
  };


  ////////////////////////////////////////////////////////////////////////////////////////////////////
  shared_ptr<CommunityDetectionThread> CommunityDetectionThread::create(shared_ptr<SociariumGraphTimeSeries> const& time_series) {
    return shared_ptr<CommunityDetectionThread>(new CommunityDetectionThreadImpl(time_series));
  }

} // The end of the namespace "hashimoto_ut"

// s.o.c.i.a.r.i.u.m
// HASHIMOTO, Yasuhiro (E-mail: hy @ sys.t.u-tokyo.ac.jp)
// update: 2009/05/11

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

#include <ctime>
#include <vector>
#include <map>
#include <boost/format.hpp>
#include <boost/random.hpp>
#include "layout.h"
#include "../graph_utility.h"
#include "../message.h"
#include "../../shared/thread.h"
#include "../../shared/msgbox.h"
#include "../../graph/util/traverser.h"

BOOL WINAPI DllMain (HINSTANCE hinstDll, DWORD fdwReason, LPVOID lpvReserved) { return TRUE; }

namespace hashimoto_ut {

  using std::vector;
  using std::wstring;
  using std::multimap;
  using std::make_pair;
  using std::pair;
  using std::greater;
  using std::tr1::shared_ptr;

  extern "C" __declspec(dllexport)
    void __cdecl layout_graph(
      Thread* parent,
      shared_ptr<pair<wstring, wstring> > const& message,
      shared_ptr<Graph const> const& graph,
      vector<double> const& input_values,
      vector<Vector2<double> >& position) {

      size_t const nsz = graph->nsize();
      size_t const esz = graph->esize();

      assert(nsz==position.size());
      if (nsz<2) return;
      if (esz==0) return;

      assert(input_values.size()==1 && input_values[0]>0.0);

      if (message)
        message->first = (boost::wformat(L"%s")%sociarium_project_message::LAYOUT_WITH_KAMADA_KAWAI_METHOD).str();

      time_t tm;
      boost::mt19937 generator((unsigned long)time(&tm));
      boost::uniform_real<> distribution(0.0, 1.0);
      boost::variate_generator<boost::mt19937, boost::uniform_real<> > rand(generator, distribution);

      // 連結成分を求める
      pair<bool, vector<vector<Node*> > > result =
        sociarium_project_graph_utility::connected_components(parent, &message->second, BFSTraverser::create<bidirectional_tag>(graph));
      if (!result.first) return;

      size_t max_index = 0;
      size_t max_size = 0;
      for (size_t i=0; i<result.second.size(); ++i) {
        if (result.second[i].size()>max_size) {
          max_index = i;
          max_size = result.second[i].size();
        }
      }

      swap(result.second[0], result.second[max_index]); // 最大コミュニティを先頭
      double mps = 0.0; // 最大コミュニティの平均最短経路長
      double unit_length = 1.0;

      vector<vector<Node*> > const& cc = result.second;

      double const threshold = 1.0;
      int const max_iteration = 10000;
      int const recal_spring_length = 100;
      double const spring_length_min = 1e-3;

      // 初期配置に揺らぎ
      for (size_t i=0; i<position.size(); ++i)
        position[i] += Vector2<double>(rand()-0.5, rand()-0.5);

      // 各連結成分を個別にレイアウト
      for (size_t cid=0; cid<cc.size(); ++cid) {

        message->first = (boost::wformat(L"%s: %d/%d")
                          %sociarium_project_message::LAYOUT_WITH_KAMADA_KAWAI_METHOD
                          %(cid+1)%cc.size()).str();

        // ----------------------------------------------------------------------------------------------------
        if (parent->cancel_check()) return;
        // ----------------------------------------------------------------------------------------------------

        // 連結成分内のノード間のグラフ距離からバネ定数を計算
        vector<Node*> const& c = cc[cid];
        size_t const csz = c.size();
        if (csz==1) continue;
        size_t const isz = csz-1;
        vector<vector<double> > spring_length(isz);
        vector<vector<double> > spring_strength(isz);
        shared_ptr<BFSTraverser> t = BFSTraverser::create<bidirectional_tag>(graph);

        for (size_t i=0; i<isz; ++i) {

          message->second = (boost::wformat(L"%s: %d%%")
                             %sociarium_project_message::KAMADA_KAWAI_CALCULATING_SPRING_STRENGTH
                             %int(100.0*(i+1.0)/isz)).str();

          // ----------------------------------------------------------------------------------------------------
          if (parent->cancel_check()) return;
          // ----------------------------------------------------------------------------------------------------

          vector<double> dst(c.size(), 0.0);
          t->reset();
          // 最大連結成分の平均最短経路長をエッジ長のスケールに用いる
          if (cid==0) for (t->start(c[i], 0.0); !t->end(); t->advance()) mps += t->distance();
          else for (t->start(c[i], 0.0); !t->end(); t->advance());
          size_t const jsz = isz-i;
          spring_length[i].resize(jsz);
          spring_strength[i].resize(jsz);
          for (size_t j=0; j<jsz; ++j) {
            Node const* n = graph->node(c[i+j+1]->index());
            spring_length[i][j] = t->distance(n)-(n->degree()==1?0.7:0.0);
            spring_strength[i][j] = 1.0/(spring_length[i][j]*spring_length[i][j]);
          }
        }

        if (cid==0) {
          mps /= 0.5*cc[0].size()*(cc[0].size()-1);
          unit_length = input_values[0]/mps;
        }

        for (size_t i=0; i<isz; ++i) {

          // ----------------------------------------------------------------------------------------------------
          if (parent->cancel_check()) return;
          // ----------------------------------------------------------------------------------------------------

          size_t const jsz = isz-i;
          for (size_t j=0; j<jsz; ++j) {
            spring_length[i][j] *= unit_length;
          }
        }

        // イテレーション開始
        size_t m = -1; // 移動させるノードのインデクス
        Vector2<double> reserved_position; // 移動前の位置
        vector<Vector2<double> > rhoErho(csz, Vector2<double>(0.0, 0.0));

        for (int count=0; count<max_iteration; ++count) {

          // ----------------------------------------------------------------------------------------------------
          if (parent->cancel_check()) return;
          // ----------------------------------------------------------------------------------------------------

          if (count==0) {
            // イテレーションの最初のステップ（計算量〜O[N^2]）
            for (size_t i=0; i<isz; ++i) {

              // ----------------------------------------------------------------------------------------------------
              if (parent->cancel_check()) return;
              // ----------------------------------------------------------------------------------------------------

              size_t const ii = c[i]->index();
              for (size_t j=0, jsz=isz-i; j<jsz; ++j) {
                size_t const jj = c[i+j+1]->index();
                Vector2<double> const dp = position[ii]-position[jj];
                double norm = dp.norm();
                if (norm<spring_length_min) norm = spring_length_min;
                Vector2<double> const sdp = spring_strength[i][j]*(1.0-spring_length[i][j]/norm)*dp;
                rhoErho[i] += sdp;
                rhoErho[i+j+1] -= sdp;
              }
            }
          } else {
            // 2回目以降のイテレーションでは差分のみ計算（計算量〜O[N]）
            rhoErho[m].set(0.0, 0.0);
            for (size_t i=0; i<csz; ++i) {

              // ----------------------------------------------------------------------------------------------------
              if (parent->cancel_check()) return;
              // ----------------------------------------------------------------------------------------------------

              if (i==m) continue;
              size_t const ii = c[i]->index();
              size_t const mm = c[m]->index();
              // 加算する今回分
              Vector2<double> const dp = position[mm]-position[ii];
              double norm = dp.norm();
              if (norm<spring_length_min) norm = spring_length_min;
              size_t const i2 = m<i?m:i;
              size_t const j2 = m<i?i-m-1:m-i-1;
              Vector2<double> const sdp = spring_strength[i2][j2]*(1.0-spring_length[i2][j2]/norm)*dp;
              // 減算する前回分
              Vector2<double> const dp_prev = reserved_position-position[ii];
              double norm_prev = dp_prev.norm();
              if (norm_prev<spring_length_min) norm_prev = spring_length_min;
              Vector2<double> const sdp_prev = spring_strength[i2][j2]*(1.0-spring_length[i2][j2]/norm_prev)*dp_prev;
              rhoErho[m] += sdp;
              rhoErho[i] += sdp_prev-sdp;
            }
          }

          // Δeの降順でソート
          multimap<double, size_t, greater<double> > de;
          double value_sum = 0.0;
          for (size_t i=0; i<csz; ++i) {

            // ----------------------------------------------------------------------------------------------------
            if (parent->cancel_check()) return;
            // ----------------------------------------------------------------------------------------------------

            double const value = rhoErho[i].norm();
            de.insert(make_pair(value, i));
            value_sum += value;
          }

          // Δeの最大値が閾値以下で計算終了
          if (de.begin()->first<threshold) break;
          assert(de.begin()->first>0.0);

          message->second = (boost::wformat(L"%s: %d%% [%.2f/%.2f]")
                             %sociarium_project_message::KAMADA_KAWAI_ITERATING
                             %int(100.0*(count+1)/max_iteration)
                             %de.begin()->first
                             %threshold
                             ).str();

          m = de.begin()->second; // 常に最大のΔeのノードを移動
          Vector2<double> rho2Erho2(0.0, 0.0);
          double rho2ErhoXrhoY = 0.0;
          size_t const mm = c[m]->index();
          reserved_position = position[mm];
          for (size_t i=0; i<csz; ++i) {

            // ----------------------------------------------------------------------------------------------------
            if (parent->cancel_check()) return;
            // ----------------------------------------------------------------------------------------------------

            if (m==i) continue;
            size_t const ii = c[i]->index();
            Vector2<double> const dp = position[mm]-position[ii];
            double const dx2 = dp.x*dp.x;
            double const dy2 = dp.y*dp.y;
            double const norm32 = pow(dx2+dy2, 1.5);
            assert(norm32>0.0);
            size_t const i2 = m<i?m:i;
            size_t const j2 = m<i?i-m-1:m-i-1;
            double const s = spring_strength[i2][j2];
            double const sd = s*spring_length[i2][j2]/norm32;
            rho2Erho2.x += s-sd*dy2;
            rho2Erho2.y += s-sd*dx2;
            rho2ErhoXrhoY += sd*dp.x*dp.y;
          }
          double const denom = rho2ErhoXrhoY*rho2ErhoXrhoY-rho2Erho2.x*rho2Erho2.y;
          assert(denom!=0.0);
          Vector2<double> const delta((-rho2ErhoXrhoY*rhoErho[m].y+rho2Erho2.y*rhoErho[m].x)/denom,
                                      (-rho2ErhoXrhoY*rhoErho[m].x+rho2Erho2.x*rhoErho[m].y)/denom);
          position[mm] += delta;
        }
      }
    }

} // The end of the namespace "hashimoto_ut"

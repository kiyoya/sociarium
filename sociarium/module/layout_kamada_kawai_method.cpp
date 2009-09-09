// s.o.c.i.a.r.i.u.m: module/layout_kamada_kawai_method.cpp
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

#include <ctime>
#include <vector>
#include <map>
#include <boost/format.hpp>
#include <boost/random.hpp>
#include "layout.h"
#include "../graph_utility.h"
#include "../language.h"
#include "../../shared/thread.h"
#include "../../shared/msgbox.h"
#include "../../graph/util/traverser.h"

BOOL WINAPI DllMain (HINSTANCE hinstDll, DWORD fdwReason, LPVOID lpvReserved) {
  return TRUE;
}

namespace hashimoto_ut {

  using std::vector;
  using std::deque;
  using std::wstring;
  using std::multimap;
  using std::make_pair;
  using std::pair;
  using std::greater;
  using std::tr1::shared_ptr;

  using namespace sociarium_project_module_layout;
  using namespace sociarium_project_language;

  extern "C" __declspec(dllexport)
    void __cdecl layout(

      Thread* parent,
      deque<wstring>& status,
      Message const* message,
      vector<Vector2<double> >& position,
      shared_ptr<Graph const> graph,
      vector<double> const& hint) {

      size_t const nsz = graph->nsize();
      size_t const esz = graph->esize();

      assert(nsz==position.size());
      if (nsz<2) return;
      if (esz==0) return;

      assert(hint.size()==1 && hint[0]>0.0);

      status[0]
        = (boost::wformat(L"%s")
           %message->get(Message::KAMADA_KAWAI_METHOD)).str();

      time_t tm;
      boost::mt19937 generator((unsigned long)time(&tm));
      boost::uniform_real<> distribution(0.0, 1.0);
      boost::variate_generator<boost::mt19937, boost::uniform_real<> >
        rand(generator, distribution);

      // 連結成分を求める
      pair<bool, vector<vector<Node*> > > result =
        sociarium_project_graph_utility::connected_components(
          parent, &status[0], message, BFSTraverser::create<bidirectional_tag>(graph), 1);

      if (!result.first) return;

      // 最大連結成分を求める
      size_t max_index = 0;
      size_t max_size = 0;
      for (size_t i=0; i<result.second.size(); ++i) {
        if (result.second[i].size()>max_size) {
          max_index = i;
          max_size = result.second[i].size();
        }
      }

      swap(result.second[0], result.second[max_index]); // The largest componet is first.

      double mps = 0.0; // Mean shortest path length of the larget component.
      double unit_length = 1.0;

      vector<vector<Node*> > const& cc = result.second;

      double const threshold = 1.0;
      int const max_iteration = 10000;
      int const recal_spring_length = 100;
      double const spring_length_min = 1e-3;

      // Give perturbation to the initial position.
      for (size_t i=0; i<position.size(); ++i)
        position[i] += Vector2<double>(rand()-0.5, rand()-0.5);

      // Layout each component individually.
      for (size_t cid=0; cid<cc.size(); ++cid) {

        status[0]
          = (boost::wformat(L"%s: %d/%d")
             %message->get(Message::KAMADA_KAWAI_METHOD)
             %(cid+1)%cc.size()).str();

        // **********  Catch a termination signal  **********
        if (parent->cancel_check()) return;

        // Calculate spring coefficients from geodesic distance.
        vector<Node*> const& c = cc[cid];
        size_t const csz = c.size();
        if (csz==1) continue;
        size_t const isz = csz-1;
        vector<vector<double> > spring_length(isz);
        vector<vector<double> > spring_strength(isz);
        shared_ptr<BFSTraverser> t = BFSTraverser::create<bidirectional_tag>(graph);

        for (size_t i=0; i<isz; ++i) {

          status[1]
            = (boost::wformat(L"%s: %d%%")
               %message->get(Message::KAMADA_KAWAI_METHOD_CALCULATING_SPRING_STRENGTH)
               %int(100.0*(i+1.0)/isz)).str();

          // **********  Catch a termination signal  **********
          if (parent->cancel_check()) return;

          vector<double> dst(c.size(), 0.0);
          t->reset();

          // Use mean shortest path length of the largest component as
          // the base length scale.
          if (cid==0)
            for (t->start(c[i]); !t->end(); t->advance())
              mps += t->distance();
          else
            for (t->start(c[i]); !t->end(); t->advance());

          size_t const jsz = isz-i;
          spring_length[i].resize(jsz);
          spring_strength[i].resize(jsz);

          for (size_t j=0; j<jsz; ++j) {
            Node const* n = c[i+j+1];
            spring_length[i][j]
              = t->distance(n)-(c[i]->degree()==1||n->degree()==1?0.5:0.0);
            spring_strength[i][j] = 1.0/(spring_length[i][j]*spring_length[i][j]);
          }
        }

        if (cid==0) {
          mps /= 0.5*cc[0].size()*(cc[0].size()-1);
          unit_length = hint[0]/mps;
        }

        for (size_t i=0; i<isz; ++i) {

          // **********  Catch a termination signal  **********
          if (parent->cancel_check()) return;

          size_t const jsz = isz-i;
          for (size_t j=0; j<jsz; ++j)
            spring_length[i][j] *= unit_length;
        }

        // Start iteration.
        size_t m = -1; // Index of the moved node.
        Vector2<double> reserved_position;
        vector<Vector2<double> > rhoErho(csz, Vector2<double>(0.0, 0.0));

        for (int count=0; count<max_iteration; ++count) {

          // **********  Catch a termination signal  **********
          if (parent->cancel_check()) return;

          if (count==0) {
            // First step of iteration (O[N^2])
            for (size_t i=0; i<isz; ++i) {

              // **********  Catch a termination signal  **********
              if (parent->cancel_check()) return;

              size_t const ii = c[i]->index();
              for (size_t j=0, jsz=isz-i; j<jsz; ++j) {
                size_t const jj = c[i+j+1]->index();

                Vector2<double> const dp = position[ii]-position[jj];
                double norm = dp.norm();

                if (norm<spring_length_min)
                  norm = spring_length_min;

                Vector2<double> const sdp
                  = spring_strength[i][j]*(1.0-spring_length[i][j]/norm)*dp;
                rhoErho[i] += sdp;
                rhoErho[i+j+1] -= sdp;
              }
            }
          } else {
            // After the second step of iteration (O[N])
            rhoErho[m].set(0.0, 0.0);

            for (size_t i=0; i<csz; ++i) {

              // **********  Catch a termination signal  **********
              if (parent->cancel_check()) return;

              if (i==m) continue;
              size_t const ii = c[i]->index();
              size_t const mm = c[m]->index();

              // Add current value.
              Vector2<double> const dp = position[mm]-position[ii];
              double norm = dp.norm();

              if (norm<spring_length_min)
                norm = spring_length_min;

              size_t const i2 = m<i?m:i;
              size_t const j2 = m<i?i-m-1:m-i-1;

              double const str = spring_strength[i2][j2];
              double const len = spring_length[i2][j2];

              Vector2<double> const sdp = str*(1.0-len/norm)*dp;

              // Subtract previous value.
              Vector2<double> const dp_prev = reserved_position-position[ii];
              double norm_prev = dp_prev.norm();

              if (norm_prev<spring_length_min)
                norm_prev = spring_length_min;

              Vector2<double> const sdp_prev = str*(1.0-len/norm_prev)*dp_prev;
              rhoErho[m] += sdp;
              rhoErho[i] += sdp_prev-sdp;
            }
          }

          // Sort each movement in descending order of @de.
          multimap<double, size_t, greater<double> > de;
          double value_sum = 0.0;

          for (size_t i=0; i<csz; ++i) {

            // **********  Catch a termination signal  **********
            if (parent->cancel_check()) return;

            double const value = rhoErho[i].norm();
            de.insert(make_pair(value, i));
            value_sum += value;
          }

          // The iteration finishes if the maximum value of @de is less than
          // the threshold value.
          if (de.begin()->first<threshold) break;
          assert(de.begin()->first>0.0);

          status[1]
            = (boost::wformat(L"%s: %d%% [%.2f/%.2f]")
               %message->get(Message::KAMADA_KAWAI_METHOD_ITERATING)
               %int(100.0*(count+1)/max_iteration)
               %de.begin()->first
               %threshold
               ).str();

          m = de.begin()->second; // Move the node of max @de.
          Vector2<double> rho2Erho2(0.0, 0.0);
          double rho2ErhoXrhoY = 0.0;
          size_t const mm = c[m]->index();
          reserved_position = position[mm];

          for (size_t i=0; i<csz; ++i) {

            // **********  Catch a termination signal  **********
            if (parent->cancel_check()) return;

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
          Vector2<double> const delta(
            (-rho2ErhoXrhoY*rhoErho[m].y+rho2Erho2.y*rhoErho[m].x)/denom,
            (-rho2ErhoXrhoY*rhoErho[m].x+rho2Erho2.x*rhoErho[m].y)/denom);
          position[mm] += delta;
        }
      }
    }

} // The end of the namespace "hashimoto_ut"

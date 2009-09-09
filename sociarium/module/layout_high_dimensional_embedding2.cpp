// s.o.c.i.a.r.i.u.m
// HASHIMOTO, Yasuhiro (E-mail: hy @ sys.t.u-tokyo.ac.jp)

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
#include <boost/format.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#define USE_OPEN_CV
#ifdef USE_OPEN_CV
#include<cv.h>
#else
#include <boost/numeric/ublas/symmetric.hpp>
#define BOOST_NUMERIC_BINDINGS_USE_CLAPACK
/* "BOOST_NUMERIC_BINDINGS_USE_CLAPACK" if you use clapack,
 * When you also want to use the "Reference BLAS" implementation that comes with clapack,
 * you probably have to define "BIND_FORTRAN_LOWERCASE_UNDERSCORE" instead of
 * "BOOST_NUMERIC_BINDINGS_USE_CLAPACK" and link against the "*_nowrap.lib" libraries.
 */
#include <boost/numeric/bindings/lapack/geev.hpp>
#include <boost/numeric/bindings/traits/std_vector.hpp>
#include <boost/numeric/bindings/traits/ublas_matrix.hpp>
#undef BOOST_NUMERIC_BINDINGS_USE_CLAPACK
#endif

#include "../graph_layout.h"
#include "../../common.h"
#include "../../message.h"
#include "../../graph_util.h"
#include "../../../shared/thread.h"
#include "../../../shared/mtrand.h"
#include "../../../shared/msgbox.h"
#include "../../../graph/util/traverser.h"
#ifdef USE_OPEN_CV
#pragma comment(lib, "cv.lib")
#pragma comment(lib, "cxcore.lib")
#pragma comment(lib, "cvaux.lib")
#pragma comment(lib, "highgui.lib")
#else
#ifdef NDEBUG
#pragma comment(lib, "libf2c.lib")
#pragma comment(lib, "BLAS.lib")
#pragma comment(lib, "clapack.lib")
#else
#pragma comment(lib, "libf2cd.lib")
#pragma comment(lib, "BLASd.lib")
#pragma comment(lib, "clapackd.lib")
#pragma comment(linker, "/NODEFAULTLIB:LIBCMTD.lib")
#endif
#endif

BOOL WINAPI DllMain (HINSTANCE hinstDll, DWORD fdwReason, LPVOID lpvReserved) { return TRUE; }

namespace hashimoto_ut {

  using std::vector;
  using std::wstring;
  using std::multimap;
  using std::complex;
  using std::make_pair;
  using std::pair;
  using std::swap;
  using std::greater;
  using std::tr1::shared_ptr;

#ifndef USE_OPEN_CV
  namespace ublas = boost::numeric::ublas;
  namespace lapack = boost::numeric::bindings::lapack;
#endif

  extern "C" __declspec(dllexport)
    void __cdecl layout_graph(Thread* parent,
                              shared_ptr<pair<wstring, wstring> > const& message,
                              shared_ptr<Graph const> const& graph,
                              vector<double> const& input_values,
                              vector<Vector2<double> >& position) {

      size_t const nsz = graph->nsize();
      size_t const esz = graph->esize();

      assert(nsz==position.size());
      if (nsz==0) return;
      if (esz==0) return;

      if (message)
        message->first = (boost::wformat(L"%s")%sociarium_project_message::LAYOUT_WITH_HDE).str();

      // 連結成分を求める
      pair<bool, vector<vector<Node*> > > const result = connected_component(parent, &message->second, graph);
      if (!result.first) return;
      vector<vector<Node*> > const& cc = result.second;

      // 最大の連結成分に対して平均グラフ距離を求め，それを用いてエッジ長を決める
      size_t csz_max = 0;
      size_t index_max = 0;
      for (size_t i=0, sz=cc.size(); i<sz; ++i) if (cc[i].size()>csz_max) csz_max = cc[i].size(), index_max = i;
      shared_ptr<BFSTraverser> t = BFSRecordingTraverser::create<bidirectional_tag>(graph);
      pair<bool, double> const msp = mean_shortest_path_length(parent, &message->second, t, cc[index_max].begin(), cc[index_max].end());
      if (!msp.first || msp.second<=0.0) return;;

      size_t const number_of_centers_ = 50>nsz?nsz:50;
      double const scale_ = 0.36/msp.second;

      // 中心ノードをランダムに選択
      vector<size_t> center(number_of_centers_);
      {
        vector<size_t> v(nsz);
        for (size_t i=0; i<nsz; ++i) v[i] = i;
        for (size_t i=0; i<number_of_centers_; ++i) swap(v[i], v[size_t(mt::rand()*(nsz-i))+i]);
        center.assign(v.begin(), v.begin()+number_of_centers_);
      }

      // 中心ノードからのグラフ距離をすべてのノードについて求める
#ifdef USE_OPEN_CV
      CvMat* distance = cvCreateMat(number_of_centers_, nsz, CV_32F);
      vector<double> mean_distance(number_of_centers_, 0.0);
#else
      ublas::matrix<double, ublas::column_major> distance(ublas::zero_matrix<double, ublas::column_major>(number_of_centers_, nsz));
      ublas::vector<double> mean_distance(number_of_centers_, 0.0);
#endif

      vector<int> count(number_of_centers_, 0);

      for (size_t i=0; i<number_of_centers_; ++i) {

        if (message)
          message->second = (boost::wformat(L"%s: %d%%")
                             %sociarium_project_message::HDE_CALCULATING_GRAPH_DISTANCE
                             %int(100.0*(i+1)/number_of_centers_)).str();

        // ----------------------------------------------------------------------------------------------------
        if (parent->cancel_check()) return;
        // ----------------------------------------------------------------------------------------------------

        shared_ptr<BFSTraverser> t = BFSTraverser::create<bidirectional_tag>(graph);
        for (t->start(graph->node(center[i]), 0.0); !t->end(); t->advance()) {
          ++count[i];
#ifdef USE_OPEN_CV
          cvmSet(distance, i, t->node()->index(), t->distance());
#else
          distance(i, t->node()->index()) = t->distance();
#endif
        }

        for (size_t j=0; j<nsz; ++j) {
#ifdef USE_OPEN_CV
          mean_distance[i] += cvmGet(distance, i, j);
#else
          mean_distance[i] += distance(i, j);
#endif
        }
      }

      for (size_t i=0; i<number_of_centers_; ++i) {
        mean_distance[i] /= count[i];
        for (size_t j=0; j<nsz; ++j)
#ifdef USE_OPEN_CV
          cvmSet(distance, i, j, cvmGet(distance, i, j)-mean_distance[i]);
#else
          distance(i, j) -= mean_distance[i];
#endif
      }

      // 中心ノードからのグラフ距離に対する分散・共分散行列を計算
#ifdef USE_OPEN_CV
      CvMat* distanceT = cvCreateMat(nsz, number_of_centers_, CV_32F);
      cvmTranspose(distance, distanceT);
      CvMat* cm = cvCreateMat(number_of_centers_, number_of_centers_, CV_32F);
      cvmMul(distance, distanceT, cm);
#else
      ublas::matrix<double, ublas::column_major> distanceT(trans(distance));
      ublas::matrix<double, ublas::column_major> cm(prod(distance, distanceT));
#endif
      for (size_t i=0; i<number_of_centers_; ++i) {

        if (message)
          message->second = (boost::wformat(L"%s: %d%%")
                             %sociarium_project_message::HDE_CALCULATING_VARIANCE_MATRIX
                             %int(100.0*(i+1)/number_of_centers_)).str();

        // ----------------------------------------------------------------------------------------------------
        if (parent->cancel_check()) return;
        // ----------------------------------------------------------------------------------------------------

        for (size_t j=0; j<number_of_centers_; ++j) {
#ifdef USE_OPEN_CV
          cvmSet(cm, i, j, cvmGet(cm, i, j)/nsz);
#else
          cm(i, j) /= nsz;
#endif
        }
      }

      if (message)
        message->second = (boost::wformat(L"%s")
                           %sociarium_project_message::HDE_CALCULATING_EIGEN_VECTOR).str();

      // 分散・共分散行列の固有値と固有ベクトルを計算
#ifdef USE_OPEN_CV
      CvMat* eigv = cvCreateMat(number_of_centers_, number_of_centers_, CV_32F);
      CvMat* eig = cvCreateMat(number_of_centers_, 1, CV_32F);
      cvmEigenVV(cm, eigv, eig, 0.0001); // cvmEigenVVでcmを破壊
#else
      ublas::matrix<complex<double>, ublas::column_major> eigv(number_of_centers_, number_of_centers_);
      ublas::vector<complex<double> > eig(number_of_centers_);
      int err = lapack::geev(cm, eig, (ublas::matrix<complex<double>, ublas::column_major>*)0, &eigv, lapack::optimal_workspace());
      BOOST_UBLAS_CHECK(err==0, ublas::internal_logic());
#endif
      // 第1・2固有値に対応する固有ベクトルをそれぞれx・y座標に対する重みとする
      multimap<double, size_t, greater<double> > pca;
      for (size_t i=0; i<number_of_centers_; ++i) {
#ifdef USE_OPEN_CV
        pca.insert(make_pair(cvmGet(eig, i, 0), i));
#else
        //assert(eig[i].imag()!=0.0);
        //pca.insert(make_pair(eig[i].real(), i));
        pca.insert(make_pair(sqrt(eig[i].real()*eig[i].real()+eig[i].imag()*eig[i].imag()), i));
#endif
      }
      multimap<double, size_t, greater<double> >::const_iterator mit = pca.begin();
      size_t const c1 = mit->second; ++mit; // 第1主成分
      size_t const c2 = mit->second; ++mit; // 第2主成分
      size_t const c3 = mit->second; ++mit; // 第3主成分
      size_t const c4 = mit->second; ++mit; // 第4主成分
      for (size_t i=0; i<nsz; ++i) {

        if (message)
          message->second = (boost::wformat(L"%s: %d%%")
                             %sociarium_project_message::HDE_CALCULATING_POSITION
                             %int(100.0*(i+1)/nsz)).str();

        // ----------------------------------------------------------------------------------------------------
        if (parent->cancel_check()) return;
        // ----------------------------------------------------------------------------------------------------

        position[i].x = 0.0;
        position[i].y = 0.0;

        for (size_t j=0; j<number_of_centers_; ++j) {
#ifdef USE_OPEN_CV
          position[i].x += scale_*cvmGet(distance, j, i)*cvmGet(eigv, c1, j);
          position[i].y += scale_*cvmGet(distance, j, i)*cvmGet(eigv, c2, j);
#else
          position[i].x += scale_*distance(j, i)*eigv(j, c1).real();
          position[i].y += scale_*distance(j, i)*eigv(j, c2).real();
          //position[i].x += scale_*distance(j, i)*eigv(j, c1);
          //position[i].y += scale_*distance(j, i)*eigv(j, c2);
#endif
        }
      }

#ifdef USE_OPEN_CV
      cvReleaseMat(&cm);
      cvReleaseMat(&distance);
      cvReleaseMat(&distanceT);
      cvReleaseMat(&eigv);
      cvReleaseMat(&eig);
#endif
    }

} // The end of the namespace "hashimoto_ut"

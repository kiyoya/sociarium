// s.o.c.i.a.r.i.u.m: module/layout_high_dimensional_embedding.cpp
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
#include <boost/format.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/symmetric.hpp>
#define BOOST_NUMERIC_BINDINGS_USE_CLAPACK
/* "BOOST_NUMERIC_BINDINGS_USE_CLAPACK" if you use clapack,
 * When you also want to use the "Reference BLAS" implementation that comes with clapack,
 * you probably have to define "BIND_FORTRAN_LOWERCASE_UNDERSCORE" instead of
 * "BOOST_NUMERIC_BINDINGS_USE_CLAPACK" and link against the "*_nowrap.lib" libraries.
 */
// http://mathema.tician.de/dl/software/boost-numeric-bindings
#include <boost/numeric/bindings/lapack/geev.hpp>
#include <boost/numeric/bindings/traits/std_vector.hpp>
#include <boost/numeric/bindings/traits/ublas_matrix.hpp>
#undef BOOST_NUMERIC_BINDINGS_USE_CLAPACK

#include "layout.h"
#include "../graph_utility.h"
#include "../menu_and_message.h"
#include "../../shared/mtrand.h"
#include "../../shared/thread.h"
#include "../../graph/util/traverser.h"

// http://www.netlib.org/clapack/LIB_WINDOWS/prebuilt_libraries_windows.html
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

BOOL WINAPI DllMain (HINSTANCE hinstDll, DWORD fdwReason, LPVOID lpvReserved) {
  return TRUE;
}

namespace hashimoto_ut {

  using std::vector;
  using std::deque;
  using std::wstring;
  using std::multimap;
  using std::complex;
  using std::make_pair;
  using std::swap;
  using std::greater;
  using std::tr1::shared_ptr;

  using namespace sociarium_project_module_layout;
  using namespace sociarium_project_menu_and_message;

  namespace ublas = boost::numeric::ublas;
  namespace lapack = boost::numeric::bindings::lapack;

  extern "C" __declspec(dllexport)
    void __cdecl layout(

      Thread& parent,
      wstring& status,
      Message const& message,
      vector<Vector2<double> >& position,
      shared_ptr<Graph const> graph,
      vector<double> const& hint) {

      size_t const nsz = graph->nsize();
      size_t const esz = graph->esize();

      assert(nsz==position.size());

      if (nsz<4) return;
      if (esz==0) return;

      status = (boost::wformat(L"%s")
                %message.get(Message::HDE)).str();

      size_t const number_of_centers_ = 50>nsz?nsz:50;

      // Select the center nodes randomly.
      vector<size_t> center(number_of_centers_);
      {
        vector<size_t> v(nsz);

        for (size_t i=0; i<nsz; ++i)
          v[i] = i;

        for (size_t i=0; i<number_of_centers_; ++i)
          swap(v[i], v[size_t(mt::rand()*(nsz-i))+i]);

        center.assign(v.begin(), v.begin()+number_of_centers_);
      }

      // Calculate distance between each node and the center nodes.
      ublas::matrix<double, ublas::column_major>
        distance(ublas::zero_matrix<double, ublas::column_major>(number_of_centers_, nsz));
      ublas::vector<double> mean_distance(number_of_centers_, 0.0);

      vector<int> count(number_of_centers_, 0);

      for (size_t i=0; i<number_of_centers_; ++i) {

        // **********  Catch a termination signal  **********
        if (parent.cancel_check()) return;

        status = (boost::wformat(L"%s: %d%%")
                  %message.get(Message::HDE_CALCULATING_GRAPH_DISTANCE)
                  %int(100.0*(i+1)/number_of_centers_)).str();

        shared_ptr<BFSTraverser> t = BFSTraverser::create<bidirectional_tag>(graph);

        for (t->start(graph->node(center[i]), 0.0); !t->end(); t->advance()) {
          ++count[i];
          distance(i, t->node()->index()) = t->distance();
        }

        for (size_t j=0; j<nsz; ++j)
          mean_distance[i] += distance(i, j);
      }

      for (size_t i=0; i<number_of_centers_; ++i) {
        mean_distance[i] /= count[i];
        for (size_t j=0; j<nsz; ++j)
          distance(i, j) -= mean_distance[i];
      }

      // Calculate variance-covariance matrix of distance from the center nodes.
      ublas::matrix<double, ublas::column_major> distanceT(trans(distance));
      ublas::matrix<double, ublas::column_major> m(prod(distance, distanceT));

      for (size_t i=0; i<number_of_centers_; ++i) {

        // **********  Catch a termination signal  **********
        if (parent.cancel_check()) return;

        status = (boost::wformat(L"%s: %d%%")
                  %message.get(Message::HDE_CALCULATING_MATRIX)
                  %int(100.0*(i+1)/number_of_centers_)).str();

        for (size_t j=0; j<number_of_centers_; ++j)
          m(i, j) /= nsz;
      }

      status = (boost::wformat(L"%s")
                %message.get(Message::HDE_CALCULATING_PRINCIPAL_COMPONENTS)).str();

      // Calculate eigen values and eigen vectors of variance-covariance matrix.
      ublas::matrix<complex<double>, ublas::column_major>
        eigv(number_of_centers_, number_of_centers_);
      ublas::vector<complex<double> > eig(number_of_centers_);
      int const err = lapack::geev(
        m, eig, (ublas::matrix<complex<double>, ublas::column_major>*)0,
        &eigv, lapack::optimal_workspace());
      BOOST_UBLAS_CHECK(err==0, ublas::internal_logic());

      // X and Y coordinates are weighted by the eigenvector.
      multimap<double, size_t, greater<double> > pca;

      for (size_t i=0; i<number_of_centers_; ++i) {
        //assert(eig[i].imag()!=0.0);
        //pca.insert(make_pair(eig[i].real(), i));
        pca.insert(make_pair(sqrt(eig[i].real()*eig[i].real()
                                  +eig[i].imag()*eig[i].imag()), i));
      }

      multimap<double, size_t, greater<double> >::const_iterator mit = pca.begin();
      assert(mit!=pca.end());

      size_t const c1 = mit->second; ++mit; assert(mit!=pca.end());
      size_t const c2 = mit->second; ++mit; assert(mit!=pca.end());
      size_t const c3 = mit->second;

      size_t const cc1 = hint[0]==3?c2:c1;
      size_t const cc2 = hint[0]==1?c2:c3;

      /* If hint[0]==1, use 1st and 2nd principal eigenvectors.
       * If hint[0]==2, use 1st and 3rd principal eigenvectors.
       * If hint[0]==3, use 2nd and 3rd principal eigenvectors.
       */

      for (size_t i=0; i<nsz; ++i) {

        // **********  Catch a termination signal  **********
        if (parent.cancel_check()) return;

        status  = (boost::wformat(L"%s: %d%%")
                   %message.get(Message::HDE_CALCULATING_POSITION)
                   %int(100.0*(i+1)/nsz)).str();

        position[i].x = 0.0;
        position[i].y = 0.0;

        for (size_t j=0; j<number_of_centers_; ++j) {
          position[i].x += distance(j, i)*eigv(j, cc1).real();
          position[i].y += distance(j, i)*eigv(j, cc2).real();
        }
      }
    }

} // The end of the namespace "hashimoto_ut"

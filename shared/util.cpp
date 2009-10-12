// util.cpp
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
#include <vector>
#include <map>
#include <fstream>
#include <algorithm>
#include <numeric>
#include "math.h"
#include "util.h"

namespace hashimoto_ut {

  using namespace std;

  ////////////////////////////////////////////////////////////////////////////////
  bool is_text(char const* filename) {
    static int const BUFFER_SIZE = 1000;
    char buf[BUFFER_SIZE];
    FILE* fp;
    errno_t err = fopen_s(&fp, filename, "rb");
    size_t const sz = fread(buf, 1, BUFFER_SIZE, fp);
    fclose(fp);
    for (size_t i=0; i<sz; ++i)
      if (buf[i]=='\0') return false;
    return true;
  }

  ////////////////////////////////////////////////////////////////////////////////
  int number_of_lines(char const* filename) {
    int retval = 0;
    ifstream ifs(filename);
    if (ifs.fail()) return -1;
    char c;

    while (ifs.get(c))
      if (c=='\n')
        ++retval;

    return retval;
  }

  ////////////////////////////////////////////////////////////////////////////////
  int number_of_lines(wchar_t const* filename) {
    int retval = 0;
    wifstream ifs(filename);
    if (ifs.fail()) return -1;
    wchar_t c;

    while (ifs.get(c))
      if (c==L'\n')
        ++retval;

    return retval;
  }

  ////////////////////////////////////////////////////////////////////////////////
  vector<string> tokenize(string const& text, char delim) {
    vector<string> retval;
    string::const_iterator begin = text.begin();
    string::const_iterator end = text.end();
    string::const_iterator head = begin;
    string::const_iterator pos = begin;

    for (; pos!=end; ++pos) {
      if (*pos==delim) {
        retval.push_back(text.substr(head-begin, pos-head));
        head = pos+1;
      }
    }

    retval.push_back(text.substr(head-begin, pos-head));
    return retval;
  }

  ////////////////////////////////////////////////////////////////////////////////
  vector<wstring> tokenize(wstring const& text, wchar_t delim) {
    vector<wstring> retval;
    wstring::const_iterator begin = text.begin();
    wstring::const_iterator end = text.end();
    wstring::const_iterator head = begin;
    wstring::const_iterator pos = begin;

    for (; pos!=end; ++pos) {
      if (*pos==delim) {
        retval.push_back(text.substr(head-begin, pos-head));
        head = pos+1;
      }
    }

    retval.push_back(text.substr(head-begin, pos-head));
    return retval;
  }

  ////////////////////////////////////////////////////////////////////////////////
  void trim(string& text) {
    if (text.empty()) return;
    size_t pos = 0;
    while (text[pos]==' ') ++pos; // Delete left spaces.
    text = text.substr(pos);
    if (text.empty()) return;
    pos = text.find('\r'); // Delete a carriage return.
    text = text.substr(0, pos);
    if (text.empty()) return;
    pos = text.size()-1;
    while (text[pos]==' ') --pos; // Delete right spaces.
    text = text.substr(0, pos+1);
  }

  ////////////////////////////////////////////////////////////////////////////////
  void trim(wstring& text) {
    if (text.empty()) return;
    size_t pos = 0;
    while (text[pos]==L' ') ++pos; // Delete left spaces.
    text = text.substr(pos);
    if (text.empty()) return;
    pos = text.find(L'\r'); // Delete a carriage return.
    text = text.substr(0, pos);
    if (text.empty()) return;
    pos = text.size()-1;
    while (text[pos]==L' ') --pos; // Delete right spaces.
    text = text.substr(0, pos+1);
  }

  ////////////////////////////////////////////////////////////////////////////////
  vector<double> floating_number_series(char const* filename) {
    vector<double> retval;
    ifstream ifs(filename);
    if (ifs.fail()) return retval;
    string line;

    while (getline(ifs, line))
      retval.push_back(atof(line.c_str()));

    return retval;
  }


  ////////////////////////////////////////////////////////////////////////////////
  vector<vector<double> >
    floating_number_matrix(char const* filename, size_t offset, char delim) {

      vector<vector<double> > retval;
      ifstream ifs(filename);
      if (ifs.fail()) return retval;
      string line;

      while (getline(ifs, line)) {
        vector<string> tok(tokenize(line, delim));
        assert(offset<tok.size());
        size_t const tsz = tok.size()-offset;
        vector<double> array(tsz);

        for (size_t i=0; i<tsz; ++i)
          array[i] = atof(tok[i+offset].c_str());

        retval.push_back(array);
      }

      return retval;
    }


  ////////////////////////////////////////////////////////////////////////////////
  vector<vector<int> >
    integer_matrix(char const* filename, size_t offset, char delim) {

      vector<vector<int> > retval;
      ifstream ifs(filename);
      if (ifs.fail()) return retval;
      string line;

      while (getline(ifs, line)) {
        vector<string> tok(tokenize(line, delim));
        assert(offset<tok.size());
        size_t const tsz = tok.size()-offset;
        vector<int> array(tsz);

        for (size_t i=0; i<tsz; ++i)
          array[i] = atoi(tok[i+offset].c_str());

        retval.push_back(array);
      }

      return retval;
    }


  ////////////////////////////////////////////////////////////////////////////////
  vector<int> bitstring2decimal(vector<bool> const& bs, int length) {
    vector<int> retval;
    int value = 0;
    int u = 0;

    for (size_t i=0, sz=bs.size(); i<sz; ++i) {
      value += int(((unsigned int)bs[i])<<u);
      if (++u==length) {
        retval.push_back(value);
        value = 0;
        u = 0;
      }
    }

    return retval;
  }


  ////////////////////////////////////////////////////////////////////////////////
  double ipow(double base, int n) {
    if (n==0) return 1.0;
    return base*ipow(base, n-1);
  }


  ////////////////////////////////////////////////////////////////////////////////
  double factorial(double value) {
    if (value<2) return 1;
    return value*factorial(value-1);
  }


  ////////////////////////////////////////////////////////////////////////////////
  double bin(int k, int n, double p) {
    assert(k<=n);
    double const q = 1.0-p;
    double p0 = 1.0;

    for (int i=0; i<k; ++i)
      p0 *= (p*(n-i))/(k-i);

    return p0*ipow(q, n-k);
  }


  ////////////////////////////////////////////////////////////////////////////////
  double gauss(double x, double mean, double variance) {
    return (1.0/sqrt(M_2PI*variance))*exp(-(x-mean)*(x-mean)/(2.0*variance));
  }


  ////////////////////////////////////////////////////////////////////////////////
  double dsqrsum(vector<double>::const_iterator first,
                 vector<double>::const_iterator last) {

    double retval = 0.0;

    for (; first!=last; ++first)
      retval += (*first)*(*first);

    return retval;
  }


  ////////////////////////////////////////////////////////////////////////////////
  double variance(vector<double>::const_iterator first,
                  vector<double>::const_iterator last,
                  double ref) {

    size_t const size = last-first;
    assert(size>0);
    double retval = 0.0;

    for (; first!=last; ++first)
      retval += (*first-ref)*(*first-ref);

    return retval/size;
  }


  ////////////////////////////////////////////////////////////////////////////////
  vector<double> autocorrelation_function(
    vector<double>::const_iterator first,
    vector<double>::const_iterator last) {

    size_t const size = last-first;
    double const mean = accumulate(first, last, 0.0)/size;
    double const var = dsqrsum(first, last)/size-mean*mean;

    // Calculate autocorrelation for the lag shorter than 1/10 of whole data length.
    size_t const lag_max = size/10;
    vector<double> retval(lag_max);

    for (size_t lag=0; lag<lag_max; ++lag) {
      double covar = 0.0;

      for (size_t i=lag; i<size; ++i)
        covar += (*(first+i)-mean)*(*(first+i-lag)-mean);

      covar /= size-lag;
      retval[lag] = covar/var; // Normalize co-variance by variance.
    }

    return retval;
  }


  ////////////////////////////////////////////////////////////////////////////////
  vector<vector<double> >
    probability_distribution(
      vector<double>::const_iterator first,
      vector<double>::const_iterator last,
      double interval) {

      size_t const size = last-first;

      map<long, long> histogram;

      for (; first!=last; ++first) {
        long value = (long)((*first)/interval+0.5);
        if ((*first)/interval<-0.5) value -= 1;
        ++histogram[value];
      }

      // 0:value, 1:probability, 2:cumulative probability.
      vector<vector<double> > retval(histogram.size(), vector<double>(3, 0.0));
      map<long, long>::const_iterator pos = histogram.begin();
      map<long, long>::const_iterator end = histogram.end();
      double cum = 0.0;

      for (size_t i=0; pos!=end; ++pos, ++i) {
        double const p = double(pos->second)/size;
        retval[i][0] = interval*(pos->first);
        retval[i][1] = p;
        retval[i][2] = (cum+=p);
      }

      return retval;
    }


  ////////////////////////////////////////////////////////////////////////////////
  vector<vector<double> >
    probability_distribution(
      vector<double>::const_iterator first,
      vector<double>::const_iterator last, int points) {

      double const v_max = *max_element(first, last);
      double const v_min = *min_element(first, last);
      return probability_distribution(first, last, (v_max-v_min)/points);
    }

} // The end of the namespace "hashimoto_ut"

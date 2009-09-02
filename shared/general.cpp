// HASHIMOTO, Yasuhiro (E-mail: hy @ sys.t.u-tokyo.ac.jp)
// update: 2009/02/28

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

#include <cassert>
#include <vector>
#include <map>
#include <fstream>
#include <algorithm>
#include <numeric>
#include "general.h"
#include "math.h"

namespace hashimoto_ut {

  using namespace std;

  /////////////////////////////////////////////////////////////////////////////////////////////
  bool is_text(char const* filename) {
    vector<char> v;
    ifstream ifs(filename, ios::in|ios::binary);
    copy(istreambuf_iterator<char>(ifs), istreambuf_iterator<char>(), back_inserter(v));
    if (v.empty()) return true;
    for (size_t i=0, sz=v.size()-1; i<sz; ++i) if (v[i]=='\0') return false;
    return true;
  }

  /////////////////////////////////////////////////////////////////////////////////////////////
  int number_of_lines(char const* filename) {
    int retval = 0;
    ifstream ifs(filename);
    if (ifs.fail()) return -1;
    char c;
    while (ifs.get(c)) { if (c=='\n') ++retval; }
    return retval;
  }

  /////////////////////////////////////////////////////////////////////////////////////////////
  vector<string> tokenize(string const& input, char delim) {
    vector<string> retval;
    string::const_iterator begin = input.begin();
    string::const_iterator end = input.end();
    string::const_iterator head = begin;
    string::const_iterator pos = begin;
    for (; pos!=end; ++pos) {
      if (*pos==delim) {
        // 区切り文字を見つけたらトークン先頭マークとイテレータの間の文字列を切り出して配列に格納，
        // 同時にトークン先頭マークを次のトークン先頭に移動します．
        retval.push_back(input.substr(head-begin, pos-head));
        head = pos+1;
      }
    }
    retval.push_back(input.substr(head-begin, pos-head));
    return retval;
  }

  namespace {
    // コメントアウトする条件
    bool is_comment(string& line) {
      size_t const pos0 = line.find("//");
      size_t const pos1 = line.find('\r');
      line = line.substr(0, pos0<pos1?pos0:pos1);
      if (line.empty()) return true;
      if (line[0]=='#') return true;
      return false;
    }
  }

  /////////////////////////////////////////////////////////////////////////////////////////////
  vector<double> floating_number_series(char const* filename) {
    vector<double> retval;
    ifstream ifs(filename);
    if (ifs.fail()) return retval;
    string line;
    while (getline(ifs, line)) {
      if (is_comment(line)) continue;
      retval.push_back(atof(line.c_str()));
    }
    return retval;
  }


  /////////////////////////////////////////////////////////////////////////////////////////////
  vector<vector<double> > floating_number_matrix(char const* filename, size_t offset, char delim) {
    vector<vector<double> > retval;
    ifstream ifs(filename);
    if (ifs.fail()) return retval;
    string line;
    while (getline(ifs, line)) {
      if (is_comment(line)) continue;
      vector<string> tok(tokenize(line, delim));
      assert(offset<tok.size());
      size_t const tsz = tok.size()-offset;
      vector<double> array(tsz);
      for (size_t i=0; i<tsz; ++i) array[i] = atof(tok[i+offset].c_str());
      retval.push_back(array);
    }
    return retval;
  }


  /////////////////////////////////////////////////////////////////////////////////////////////
  vector<vector<int> > integer_matrix(char const* filename, size_t offset, char delim) {
    vector<vector<int> > retval;
    ifstream ifs(filename);
    if (ifs.fail()) return retval;
    string line;
    while (getline(ifs, line)) {
      if (is_comment(line)) continue;
      vector<string> tok(tokenize(line, delim));
      assert(offset<tok.size());
      size_t const tsz = tok.size()-offset;
      vector<int> array(tsz);
      for (size_t i=0; i<tsz; ++i) array[i] = atoi(tok[i+offset].c_str());
      retval.push_back(array);
    }
    return retval;
  }



  /////////////////////////////////////////////////////////////////////////////////////////////
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


  /////////////////////////////////////////////////////////////////////////////////////////////
  double ipow(double base, int n) {
    if (n==0) return 1.0;
    return base*ipow(base, n-1);
  }


  /////////////////////////////////////////////////////////////////////////////////////////////
  double factorial(double value) {
    if (value<2) return 1;
    return value*factorial(value-1);
  }


  /////////////////////////////////////////////////////////////////////////////////////////////
  double bin(int k, int n, double p) {
    assert(k<=n);
    double const q = 1.0-p;
    double p0 = 1.0;
    for (int i=0; i<k; ++i) p0 *= (p*(n-i))/(k-i);
    return p0*ipow(q, n-k);
  }


  /////////////////////////////////////////////////////////////////////////////////////////////
  double gauss(double x, double mean, double variance) {
    return (1.0/sqrt(M_2PI*variance))*exp(-(x-mean)*(x-mean)/(2.0*variance));
  }


  /////////////////////////////////////////////////////////////////////////////////////////////
  double dsqrsum(vector<double>::const_iterator first, vector<double>::const_iterator last) {
    double retval = 0.0;
    for (; first!=last; ++first) retval += (*first)*(*first);
    return retval;
  }


  /////////////////////////////////////////////////////////////////////////////////////////////
  double variance(vector<double>::const_iterator first, vector<double>::const_iterator last, double ref) {
    size_t const size = last-first;
    assert(size>0);
    double retval = 0.0;
    for (; first!=last; ++first) retval += (*first-ref)*(*first-ref);
    return retval/size;
  }


  /////////////////////////////////////////////////////////////////////////////////////////////
  vector<double> autocorrelation_function(vector<double>::const_iterator first, vector<double>::const_iterator last) {
    size_t const size = last-first;
    double const mean = accumulate(first, last, 0.0)/size; // 時系列データの平均値
    double const var = dsqrsum(first, last)/size-mean*mean; // 時系列データの標本分散
    // 時系列の長さに対して1/10のタイムラグまで求めます．
    size_t const lag_max = size/10;
    vector<double> retval(lag_max);
    for (size_t lag=0; lag<lag_max; ++lag) {
      double covar = 0.0;
      for (size_t i=lag; i<size; ++i) covar += (*(first+i)-mean)*(*(first+i-lag)-mean);
      covar /= size-lag; // 共分散
      retval[lag] = covar/var; // 共分散を全体の分散で規格化
    }
    return retval;
  }


  /////////////////////////////////////////////////////////////////////////////////////////////
  vector<vector<double> >
    probability_distribution(vector<double>::const_iterator first, vector<double>::const_iterator last, double interval) {
      size_t const size = last-first;
      // サンプリング区間ごとのヒストグラムを求めます．
      map<long, long> histogram;
      for (; first!=last; ++first) {
        long value = (long)((*first)/interval+0.5); // 値の属する区間
        if ((*first)/interval<-0.5) { value -= 1; }
        ++histogram[value];
      }
      // 3(=区間中心値，確率，累積確率)×区間数の行列を作成します．
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


  /////////////////////////////////////////////////////////////////////////////////////////////
  vector<vector<double> >
    probability_distribution(vector<double>::const_iterator first, vector<double>::const_iterator last, int points) {
      double const v_max = *max_element(first, last);
      double const v_min = *min_element(first, last);
      return probability_distribution(first, last, (v_max-v_min)/points);
    }

} // The end of the namespace "hashimoto_ut"

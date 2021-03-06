﻿// util.h
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

#ifndef INCLUDE_GUARD_SHARED_UTIL_H
#define INCLUDE_GUARD_SHARED_UTIL_H

#include <vector>
#include <string>

namespace hashimoto_ut {

  ////////////////////////////////////////////////////////////////////////////////
  // テキストファイルならtrue，バイナリファイルならfalse（ヌル文字の有無で判定）
  bool is_text(char const* filename);

  ////////////////////////////////////////////////////////////////////////////////
  // テキストファイルの行数を返します．
  int number_of_lines(char const* filename);
  int number_of_lines(wchar_t const* filename);

  ////////////////////////////////////////////////////////////////////////////////
  // 文字列をトークン分割します．
  // [retval] トークン分割された文字列の配列
  // [args]   @text=入力文字列，@delim=区切り文字
  std::vector<std::string> tokenize(std::string const& text, char delim=' ');
  std::vector<std::wstring> tokenize(std::wstring const& text, wchar_t delim=L' ');

  ////////////////////////////////////////////////////////////////////////////////
  // 文字列両端のスペースを削除します．(右端の\rも削除)
  // [retval] なし
  // [args]   @text=入力文字列
  void trim(std::string& text);
  void trim(std::wstring& text);

  ////////////////////////////////////////////////////////////////////////////////
  // ファイルから改行で区切られた浮動小数点数の列を読み込みます．
  // "#"で始まる行はコメントアウトされます．
  // [retval] 浮動小数点数の配列
  // [args]   @filename=ファイル名
  std::vector<double> floating_number_series(char const* filename);

  ////////////////////////////////////////////////////////////////////////////////
  // ファイルから改行と区切り文字で区切られた浮動小数点数の行列を読み込みます．
  // "#"で始まる行はコメントアウトされます．
  // [retval] 浮動小数点数の二次元配列
  // [args]   @filename=ファイル名，@offset=読み飛ばす列の数，@delim=一行の区切り文字
  std::vector<std::vector<double> >
    floating_number_matrix(char const* filename, size_t offset=0, char delim=' ');

  ////////////////////////////////////////////////////////////////////////////////
  // ファイルから改行と区切り文字で区切られた整数の行列を読み込みます．
  // "#"で始まる行はコメントアウトされます．
  // [retval] 整数の二次元配列
  // [args]   @filename=ファイル名，@offset=読み飛ばす列の数，@delim=一行の区切り文字
  std::vector<std::vector<int> >
    integer_matrix(char const* filename, size_t offset=0, char delim=' ');

  ////////////////////////////////////////////////////////////////////////////////
  // ビット列を指定ビット数で区切り10進数に変換します．
  // 例. length==3の場合: "001010110" ⇒ "001 / 010 / 110" ⇒ { 1, 2, 6 }
  // [retval] 整数の配列
  // [args]   @bs=ビット列，@length=10進数に変換するビット数の単位
  std::vector<int> bitstring2decimal(std::vector<bool> const& bs, int length);

  ////////////////////////////////////////////////////////////////////////////////
  // 指数が整数の場合の累乗を求めます．
  // [retval] 累乗
  // [args]   @base=底，@n=指数
  double ipow(double base, int n);

  ////////////////////////////////////////////////////////////////////////////////
  // 階乗を求めます．
  // [retval] 階乗
  // [args]   @value=階乗を求める数
  double factorial(double value);

  ////////////////////////////////////////////////////////////////////////////////
  // 二項分布の確率を求めます．
  // [retval] 事象がk回発生する確率
  // [args]   @k=事象の発生回数，@n=試行数，@p=事象の発生確率
  double bin(int k, int n, double p);

  ////////////////////////////////////////////////////////////////////////////////
  // 正規分布の確率密度を求めます．
  // [retval] 確率変数xの確率密度
  // [args]   @x=確率変数，@mean=平均値，@variance=分散
  double gauss(double x, double mean, double variance);

  ////////////////////////////////////////////////////////////////////////////////
  // 自乗和を求めます．
  // [retval] 自乗和
  // [args]   @first=データ配列の先頭，@last=データ配列の終端
  double dsqrsum(std::vector<double>::const_iterator first,
                 std::vector<double>::const_iterator last);

  ////////////////////////////////////////////////////////////////////////////////
  // 標本分散を求めます．
  // [retval] 分散
  // [args]   @first=データ配列の先頭，@last=データ配列の終端，@ref=基準値
  double variance(std::vector<double>::const_iterator first,
                  std::vector<double>::const_iterator last,
                  double ref=0.0);

  ////////////////////////////////////////////////////////////////////////////////
  // 自己相関関数（時間相関関数）を求めます．
  // [retval] タイムラグをインデクスとする自己相関係数の配列
  // [args]   @first=時系列データ配列の先頭，@last=時系列データ配列の終端
  std::vector<double> autocorrelation_function(
    std::vector<double>::const_iterator first,
    std::vector<double>::const_iterator last);

  ////////////////////////////////////////////////////////////////////////////////
  // 確率分布関数を求めます．
  // [retval] 浮動小数点数の二次元配列[0~size-1][0~2]
  //          1行が一つのデータ点に対応し，1列目は区間の代表値，2列目は区間の確率，3列目は累積確率を表します．
  // [args]   @first=データ配列の先頭，@last=データ配列の終端，@interval=サンプリング間隔
  std::vector<std::vector<double> >
    probability_distribution(
      std::vector<double>::const_iterator first,
      std::vector<double>::const_iterator last, double interval);
  // [args]   @first=データ配列の先頭，@last=データ配列の終端，@points=出力点数
  std::vector<std::vector<double> >
    probability_distribution(
      std::vector<double>::const_iterator first,
      std::vector<double>::const_iterator last, int points);

} // The end of the namespace "hashimoto_ut"

#endif // INCLUDE_GUARD_SHARED_UTIL_H

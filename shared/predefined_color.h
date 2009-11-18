// predefined_color.h
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

#ifndef INCLUDE_GUARD_SHARED_PREDEFINED_COLOR_H
#define INCLUDE_GUARD_SHARED_PREDEFINED_COLOR_H

#include <cassert>
#include <vector>
#include "vector3.h"

namespace hashimoto_ut {

  class PredefinedColor {
  public:
    enum {
      BLACK=0, WHITE=1, GRAY=2, LIGHT_GRAY=3, DARK_GRAY=4, RED=5, GREEN=6, BLUE=7, YELLOW=8, SNOW=9,
      RESERVED0=10, RESERVED1=11, RESERVED2=12, RESERVED3=13, RESERVED4=14,
      RESERVED5=15, RESERVED6=16, RESERVED7=17, RESERVED8=18, RESERVED9=19,
      NUMBER_OF_RESERVED_COLOR=20
    };

    ////////////////////////////////////////////////////////////////////////////////
    PredefinedColor(void) : number_of_reserved_colors_(NUMBER_OF_RESERVED_COLOR) {
      // Reserved Colors.
      color_.push_back(Vector3<float>(float(0x00)/255, float(0x00)/255, float(0x00)/255)); // BLACK
      color_.push_back(Vector3<float>(float(0xff)/255, float(0xff)/255, float(0xff)/255)); // WHITE
      color_.push_back(Vector3<float>(float(0x80)/255, float(0x80)/255, float(0x80)/255)); // GRAY
      color_.push_back(Vector3<float>(float(0xa0)/255, float(0xa0)/255, float(0xa0)/255)); // LIGHT_GRAY
      color_.push_back(Vector3<float>(float(0x40)/255, float(0x40)/255, float(0x40)/255)); // DARK_GRAY
      color_.push_back(Vector3<float>(float(0xff)/255, float(0x00)/255, float(0x00)/255)); // RED
      color_.push_back(Vector3<float>(float(0x00)/255, float(0xff)/255, float(0x00)/255)); // GREEN
      color_.push_back(Vector3<float>(float(0x00)/255, float(0x00)/255, float(0xff)/255)); // BLUE
      color_.push_back(Vector3<float>(float(0xff)/255, float(0xff)/255, float(0x00)/255)); // YELLOW
      color_.push_back(Vector3<float>(float(0xff)/255, float(0xfa)/255, float(0xfa)/255)); // SNOW
      color_.push_back(Vector3<float>(float(0xff)/255, float(0xff)/255, float(0xff)/255)); // 予約10
      color_.push_back(Vector3<float>(float(0xff)/255, float(0xff)/255, float(0xff)/255)); // 予約11
      color_.push_back(Vector3<float>(float(0xff)/255, float(0xff)/255, float(0xff)/255)); // 予約12
      color_.push_back(Vector3<float>(float(0xff)/255, float(0xff)/255, float(0xff)/255)); // 予約13
      color_.push_back(Vector3<float>(float(0xff)/255, float(0xff)/255, float(0xff)/255)); // 予約14
      color_.push_back(Vector3<float>(float(0xff)/255, float(0xff)/255, float(0xff)/255)); // 予約15
      color_.push_back(Vector3<float>(float(0xff)/255, float(0xff)/255, float(0xff)/255)); // 予約16
      color_.push_back(Vector3<float>(float(0xff)/255, float(0xff)/255, float(0xff)/255)); // 予約17
      color_.push_back(Vector3<float>(float(0xff)/255, float(0xff)/255, float(0xff)/255)); // 予約18
      color_.push_back(Vector3<float>(float(0xff)/255, float(0xff)/255, float(0xff)/255)); // 予約19
      assert(number_of_reserved_colors_==color_.size());

      // Custom Colors.
      color_.push_back(Vector3<float>(float(0xf2)/255, float(0x9b)/255, float(0x76)/255)); // パステルレッド
      //color_.push_back(Vector3<float>(float(0xff)/255, float(0xf7)/255, float(0x99)/255)); // パステルイエロー
      color_.push_back(Vector3<float>(float(0x89)/255, float(0xc9)/255, float(0x97)/255)); // パステルグリーン
      color_.push_back(Vector3<float>(float(0x8c)/255, float(0x97)/255, float(0xcb)/255)); // パステルブルー
      color_.push_back(Vector3<float>(float(0xA0)/255, float(0x88)/255, float(0xBA)/255)); // パステルバイオレット
      color_.push_back(Vector3<float>(float(0xf1)/255, float(0x9e)/255, float(0xc2)/255)); // パステルマゼンダ
      color_.push_back(Vector3<float>(float(0xf6)/255, float(0xb3)/255, float(0x7f)/255)); // パステルレッドオレンジ
      color_.push_back(Vector3<float>(float(0x88)/255, float(0xab)/255, float(0xda)/255)); // パステルシアンブルー
      //color_.push_back(Vector3<float>(float(0xfa)/255, float(0xcd)/255, float(0x89)/255)); // パステルイエローオレンジ
      color_.push_back(Vector3<float>(float(0xcc)/255, float(0xe1)/255, float(0x98)/255)); // パステルピーグリーン
      color_.push_back(Vector3<float>(float(0x7e)/255, float(0xce)/255, float(0xf4)/255)); // パステルシアン
      color_.push_back(Vector3<float>(float(0x84)/255, float(0xcc)/255, float(0xc9)/255)); // パステルグリーンシアン
      color_.push_back(Vector3<float>(float(0xf2)/255, float(0x9c)/255, float(0x9f)/255)); // パステルマゼンダレッド
      color_.push_back(Vector3<float>(float(0xac)/255, float(0xd5)/255, float(0x98)/255)); // パステルイエローグリーン
      color_.push_back(Vector3<float>(float(0x8f)/255, float(0x82)/255, float(0xbc)/255)); // パステルブルーバイオレット
      color_.push_back(Vector3<float>(float(0xc4)/255, float(0x90)/255, float(0xbf)/255)); // パステルバイオレットマゼンダ
      color_.push_back(Vector3<float>(float(0xec)/255, float(0x69)/255, float(0x41)/255)); // レッド(明)
      color_.push_back(Vector3<float>(float(0xf1)/255, float(0x91)/255, float(0x49)/255)); // レッドオレンジ(明)
      color_.push_back(Vector3<float>(float(0xf8)/255, float(0xb5)/255, float(0x51)/255)); // イエローオレンジ(明)
      //color_.push_back(Vector3<float>(float(0xff)/255, float(0xf4)/255, float(0x5c)/255)); // イエロー(明)
      color_.push_back(Vector3<float>(float(0xb3)/255, float(0xd4)/255, float(0x65)/255)); // ピーグリーン(明)
      color_.push_back(Vector3<float>(float(0x80)/255, float(0xc2)/255, float(0x69)/255)); // イエローグリーン(明)
      color_.push_back(Vector3<float>(float(0x32)/255, float(0xb1)/255, float(0x6c)/255)); // グリーン(明)
      color_.push_back(Vector3<float>(float(0x13)/255, float(0xb5)/255, float(0xb1)/255)); // グリーンシアン(明)
      color_.push_back(Vector3<float>(float(0x00)/255, float(0xb7)/255, float(0xee)/255)); // ライトシアン
      color_.push_back(Vector3<float>(float(0x44)/255, float(0x8a)/255, float(0xca)/255)); // シアンブルー(明)
      color_.push_back(Vector3<float>(float(0x55)/255, float(0x6f)/255, float(0xb5)/255)); // ブルー(明)
      color_.push_back(Vector3<float>(float(0x5f)/255, float(0x52)/255, float(0xa0)/255)); // ブルーバイオレット(明)
      color_.push_back(Vector3<float>(float(0x89)/255, float(0x57)/255, float(0xa1)/255)); // バイオレット(明)
      color_.push_back(Vector3<float>(float(0xae)/255, float(0x5d)/255, float(0xa1)/255)); // バイオレットマゼンダ(明)
      color_.push_back(Vector3<float>(float(0xea)/255, float(0x68)/255, float(0xa2)/255)); // マゼンダ(明)
      color_.push_back(Vector3<float>(float(0xeb)/255, float(0x68)/255, float(0x77)/255)); // マゼンダレッド(明)
      color_.push_back(Vector3<float>(float(0xe6)/255, float(0x00)/255, float(0x12)/255)); // レッド(純色)
      color_.push_back(Vector3<float>(float(0xeb)/255, float(0x61)/255, float(0x00)/255)); // レッドオレンジ(純色)
      color_.push_back(Vector3<float>(float(0xf3)/255, float(0x98)/255, float(0x00)/255)); // イエローオレンジ(純色)
      //color_.push_back(Vector3<float>(float(0xff)/255, float(0xf1)/255, float(0x00)/255)); // イエロー(純色)
      color_.push_back(Vector3<float>(float(0x8f)/255, float(0xc3)/255, float(0x1f)/255)); // ピーグリーン(純色)
      color_.push_back(Vector3<float>(float(0x22)/255, float(0xac)/255, float(0x38)/255)); // イエローグリーン(純色)
    }

    ////////////////////////////////////////////////////////////////////////////////
    Vector3<float> const& operator[](int id) const {
      assert(0<=id && id<size());
      return color_[id];
    }

    ////////////////////////////////////////////////////////////////////////////////
    int size(void) const {
      return int(color_.size());
    }

    ////////////////////////////////////////////////////////////////////////////////
    Vector3<float> const& reserved(int id) const {
      assert(0<=id && id<number_of_reserved_colors_);
      return color_[id];
    }

    Vector3<float> const& custom(int id) const {
      id += number_of_reserved_colors_;
      assert(0<=id && id<size());
      return color_[id];
    }

    ////////////////////////////////////////////////////////////////////////////////
    int number_of_reserved_colors(void) const {
      return number_of_reserved_colors_;
    }

    int number_of_custom_colors(void) const {
      return size()-number_of_reserved_colors_;
    }

  private:
    int const number_of_reserved_colors_;
    std::vector<Vector3<float> > color_;
  };

  namespace {
    PredefinedColor predefined_color;
  }

} // The end of the namespace "hashimoto_ut"

#endif // INCLUDE_GUARD_SHARED_PREDEFINED_COLOR_H

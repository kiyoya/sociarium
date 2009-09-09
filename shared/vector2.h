// vector2.h
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

#ifndef INCLUDE_GUARD_SHARED_VECTOR2_H
#define INCLUDE_GUARD_SHARED_VECTOR2_H

#include <cstdlib>
#include <cassert>
#include <iostream>
#include "math.h"

namespace hashimoto_ut {

  ////////////////////////////////////////////////////////////////////////////////
  // 2-dimensional vector
  template <typename T>
  struct Vector2 {
    typedef T value_type;

    union {
      struct { T x,y; };
      T data[2];
    };

    // コンストラクタ
    Vector2(void) : x(T(0)), y(T(0)) {}

    template <typename U> Vector2(U const* ptr)
         : x(T(ptr[0])), y(T(ptr[1])) {}

    template <typename U> Vector2(Vector2<U> const& v)
         : x(T(v.x)), y(T(v.y)) {}

    template <typename U> Vector2(U vx, U vy)
         : x(T(vx)), y(T(vy)) {}

    // デストラクタ
    ~Vector2() {}

    // 配列風参照
    T& operator[](size_t index) {
      assert(index<2);
      return data[index];
    }

    T const& operator[](size_t index) const {
      assert(index<2);
      return data[index];
    }

    // 符号
    Vector2 operator+(void) const {
      return *this;
    }

    Vector2 operator-(void) const {
      return Vector2(-x, -y);
    }

    // 代入
    Vector2& operator=(Vector2 const& v) {
      x = v.x;
      y = v.y;
      return *this;
    }

    Vector2& operator+=(Vector2 const& v) {
      x += v.x;
      y += v.y;
      return *this;
    }

    Vector2& operator-=(Vector2 const& v) {
      x -= v.x;
      y -= v.y;
      return *this;
    }

    Vector2& operator*=(T value) {
      x *= value;
      y *= value;
      return *this;
    }

    Vector2& operator/=(T value) {
      assert(value!=0);
      x /= value;
      y /= value;
      return *this;
    }

    // 初期化
    Vector2& set(T const vx, T const vy) {
      x = vx;
      y = vy;
      return *this;
    }

    Vector2& set(Vector2 const& v) {
      x = v.x;
      y = v.y;
      return *this;
    }

    // ベクトル同士の和差
    Vector2 operator+(Vector2 const& v) const {
      return Vector2(x+v.x, y+v.y);
    }

    Vector2 operator-(Vector2 const& v) const {
      return Vector2(x-v.x, y-v.y);
    }

    // 固有演算
    T operator*(Vector2 const& v) const {
      return x*v.x+y*v.y;
    }

    T operator%(Vector2 const& v) const {
      return x*v.y-y*v.x;
    }

    // 論理演算
    bool operator==(Vector2 const& v) const {
      return (x==v.x)&&(y==v.y);
    }

    bool operator!=(Vector2 const& v) const {
      return !(operator==(v));
    }

    bool operator<(Vector2 const& v) const {
      return norm2()<v.norm2();
    }

    bool operator<=(Vector2 const& v) const {
      return norm2()<=v.norm2();
    }

    bool operator>(Vector2 const& v) const {
      return norm2()>v.norm2();
    }

    bool operator>=(Vector2 const& v) const {
      return norm2()>=v.norm2();
    }

    // キャスト演算
    Vector2<double> dcast(void) const {
      return Vector2<double>(double(x), double(y));
    }

    Vector2<float> fcast(void) const {
      return Vector2<float>(float(x), float(y));
    }

    Vector2<long> lcast(void) const {
      return Vector2<long>(long(x), long(y));
    }

    Vector2<int> icast(void) const {
      return Vector2<int>(int(x), int(y));
    }

    // 暗黙のキャスト
    operator Vector2<double>(void) const {
      return Vector2<double>(double(x), double(y));
    }

    operator Vector2<float>(void) const {
      return Vector2<float>(float(x), float(y));
    }

    operator Vector2<long>(void) const {
      return Vector2<long>(long(x), long(y));
    }

    operator Vector2<int>(void) const {
      return Vector2<int>(int(x), int(y));
    }

    // ノルム演算
    double norm(void) const {
      return sqrt(norm2());
    }

    double norm2(void) const {
      return x*x+y*y;
    }

    // 正規化
    Vector2<double> normalized_vector(void) const {
      double n = norm();
      if (n>0.0) return Vector2<double>(x/n, y/n);
      return Vector2<double>(0);
    }

    // 回転
    Vector2<double> rot(double radian) const {
      double const c = cos(radian);
      double const s = sin(radian);
      return Vector2<double>(x*c-y*s, x*s+y*c);
    }

    Vector2<double> rot_discrete(int degree) const {
      assert(0<=degree && degree<360);
      float const c = COS360[degree];
      float const s = SIN360[degree];
      return Vector2<double>(x*c-y*s, x*s+y*c);
    }

    double angle(void) const {
      double const theta = acos(x/norm());
      if (y<0.0) return -theta;
      return theta;
    }

    void dump(void) const {
      std::cout << "(x, y) = (" << x << ", " << y << ")" << std::endl;
    }
  };

  template <typename T>	Vector2<T> operator*(Vector2<T> const& v, T value) {
    return Vector2<T>(v) *= value;
  }

  template <typename T>	Vector2<T> operator*(T value, Vector2<T> const& v) {
    return Vector2<T>(v) *= value;
  }

  template <typename T>	Vector2<T> operator/(Vector2<T> const& v, T value) {
    assert(value!=0);
    return Vector2<T>(v) /= value;
  }


  ////////////////////////////////////////////////////////////////////////////////
  // 2-dimensional radius vector
  template <typename T>
  struct RadiusVector2 {
    typedef T value_type;

    Vector2<T> v; // radius vector
    Vector2<T> u; // unit vector
    Vector2<T> n; // unit normal vector
    double const norm;

    RadiusVector2(Vector2<T> const& v0, Vector2<T> const& v1) : v(v1-v0), norm(v.norm()) {
      if (norm>0) u = v/T(norm);
      else u = Vector2<T>(T(0), T(0));
      n.x = u.y;
      n.y = -u.x;
    }
  };


  ////////////////////////////////////////////////////////////////////////////////
  template <typename T>
  bool point_is_in_triangle(Vector2<T> const& pt, Vector2<T> const* t) {
    double const a = (t[0].x-pt.x)*(t[1].y-pt.y)-(t[1].x-pt.x)*(t[0].y-pt.y);
    double const b = (t[1].x-pt.x)*(t[2].y-pt.y)-(t[2].x-pt.x)*(t[1].y-pt.y);
    double const c = (t[2].x-pt.x)*(t[0].y-pt.y)-(t[0].x-pt.x)*(t[2].y-pt.y);
    return a*b>0 && b*c>0;
  }
  /* returns true, if @pt is inside of the triangle bounded by @t[0], @t[1], @t[2].
   */


  ////////////////////////////////////////////////////////////////////////////////
  template <typename T>
  bool point_is_in_trapezoid(Vector2<T> const& pt, Vector2<T> const* t) {
    Vector2<T> const p[3] = { t[3], t[1], t[2] };
    return point_is_in_triangle(pt, t) || point_is_in_triangle(pt, p);
  }
  /* returns true, if @pt is inside of the trapezoid bounded by @t[0] (top left), @t[1] (top right),
   * @t[2] (bottom left), @t[3] (bottom right).
   */

} // The end of the namespace "hashimoto_ut"

#endif // INCLUDE_GUARD_SHARED_VECTOR2_H

// HASHIMOTO, Yasuhiro (E-mail: hy @ sys.t.u-tokyo.ac.jp)
// update: 2009/03/04

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

#ifndef INCLUDE_GUARD_SHARED_VECTOR3_H
#define INCLUDE_GUARD_SHARED_VECTOR3_H

#include <cstdlib>
#include <cassert>
#include <iostream>
#include "math.h"

namespace hashimoto_ut {

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // 3-dimensional vector
  template <typename T>
  struct Vector3 {
    typedef T value_type;

    union {
      struct { T x, y, z; };
      T data[3];
    };

    // コンストラクタ
    Vector3(void) : x(T(0)), y(T(0)), z(T(0)) {}
    template <typename U> Vector3(U const* p) : x(T(p[0])), y(T(p[1])), z(T(p[2])) {}
    template <typename U> Vector3(Vector3<U> const& v) : x(T(v.x)), y(T(v.y)), z(T(v.z)) {}
    template <typename U> Vector3(U vx, U vy, U vz) : x(T(vx)), y(T(vy)), z(T(vz)) {}

    // デストラクタ
    ~Vector3() {}

    // 配列風参照
    T& operator[](size_t index) { assert(index<3); return data[index]; }
    const T& operator[](size_t index) const { assert(index<3); return data[index]; }

    // 符号
    Vector3 operator+(void) const { return *this; }
    Vector3 operator-(void) const { return Vector3(-x, -y, -z); }

    // 代入
    Vector3& operator=(Vector3<T> const& v) { if (this!=&v) x=v.x, y=v.y, z=v.z; return *this; }
    Vector3& operator+=(Vector3<T> const& v) { x+=v.x; y+=v.y; z+=v.z; return *this; }
    Vector3& operator-=(Vector3<T> const& v) { x-=v.x; y-=v.y; z-=v.z; return *this; }
    Vector3& operator*=(T value) { x*=value; y*=value; z*=value; return *this; }
    Vector3& operator/=(T value) { assert(value!=0); x/=value; y/=value; z/=value; return *this; }

    // 初期化
    Vector3& set(T vx, T vy, T vz) { x=vx; y=vy; z=vz; return *this; }
    Vector3& set(Vector3 const& v) { x=v.x; y=v.y; z=v.z; return *this; }

    // ベクトル同士の和差
    Vector3 operator+(Vector3 const& v) const { return Vector3(x+v.x, y+v.y, z+v.z); }
    Vector3 operator-(Vector3 const& v) const { return Vector3(x-v.x, y-v.y, z-v.z); }

    // 特殊演算
    T operator*(Vector3 const& v) const { return x*v.x+y*v.y+z*v.z; } // 内積
    Vector3 operator%(Vector3 const& v) const { return Vector3(y*v.z-z*v.y, z*v.x-x*v.z, x*v.y-y*v.x); } // 外積

    // 論理演算
    bool operator==(Vector3 const& v) const { return (x==v.x)&&(y==v.y)&&(z==v.z); }
    bool operator!=(Vector3 const& v) const { return !(operator==(v)); }
    bool operator<(Vector3 const& v) const { return norm2()<v.norm2(); }
    bool operator<=(Vector3 const& v) const { return norm2()<=v.norm2(); }
    bool operator>(Vector3 const& v) const { return norm2()>v.norm2(); }
    bool operator>=(Vector3 const& v) const { return norm2()>=v.norm2(); }

    // キャスト演算
    Vector3<double> dcast(void) const { return Vector3<double>(double(x), double(y), double(z)); }
    Vector3<float> fcast(void) const { return Vector3<float>(float(x), float(y), float(z)); }
    Vector3<long> lcast(void) const { return Vector3<long>(long(x), long(y), long(z)); }
    Vector3<int> icast(void) const { return Vector3<int>(int(x), int(y), int(z)); }

    // 暗黙のキャスト
    operator Vector3<double>(void) const { return Vector3<double>(double(x), double(y), double(z)); }
    operator Vector3<float>(void) const { return Vector3<float>(float(x), float(y), float(z)); }
    operator Vector3<long>(void) const { return Vector3<long>(long(x), long(y), long(z)); }
    operator Vector3<int>(void) const { return Vector3<int>(int(x), int(y), int(z)); }

    // ノルム演算
    double norm(void) const { return sqrt(norm2()); } // 絶対値
    double norm2(void) const { return x*x+y*y+z*z; } // 絶対値の二乗

    // 正規化
    Vector3<double> normalized_vector(void) const {
      double n = norm();
      if (n>0.0) return Vector3<double>(x/n, y/n, z/n);
      return Vector3<double>(0.0);
    }

    // 回転
    Vector3<double> rot(double radian, Vector3<double> const& axis) const;

    // プリント
    void dump(void) const {
      std::cout << "(x, y, z) = (" << x << ", " << y << ", " << z << ")" << std::endl;
    }
  };

  template <typename T> const Vector3<T> operator*(Vector3<T> const& v, T value) { return Vector3<T>(v) *= value; }
  template <typename T> const Vector3<T> operator*(T value, Vector3<T> const& v) { return Vector3<T>(v) *= value; }
  template <typename T> const Vector3<T> operator/(Vector3<T> const& v, T value) { assert(value!=0.0); return Vector3<T>(v) /= value; }


  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // Quaternion and its operations
  namespace {
    struct Quaternion {
      double t_;
      Vector3<double> v_;
      Quaternion(double t, Vector3<double> const& v) : t_(t), v_(v) {}
    };

    Quaternion operator*(Quaternion const& lhs, Quaternion const& rhs) {
      return Quaternion(
        lhs.t_*rhs.t_-lhs.v_.x*rhs.v_.x-lhs.v_.y*rhs.v_.y-lhs.v_.z*rhs.v_.z,
        Vector3<double>(
          lhs.t_*rhs.v_.x+rhs.t_*lhs.v_.x+lhs.v_.y*rhs.v_.z-lhs.v_.z*rhs.v_.y,
          lhs.t_*rhs.v_.y+rhs.t_*lhs.v_.y+lhs.v_.z*rhs.v_.x-lhs.v_.x*rhs.v_.z,
          lhs.t_*rhs.v_.z+rhs.t_*lhs.v_.z+lhs.v_.x*rhs.v_.y-lhs.v_.y*rhs.v_.x));
    }

    Quaternion rotation_quaternion(double radian, Vector3<double> const& axis) {
      double const n = axis.norm();
      if (n==0.0) return Quaternion(0.0f, Vector3<double>(0.0, 0.0, 0.0));
      return Quaternion(cos(0.5*radian), sin(0.5*radian)*axis/n);
    }
  } // The end of the anonymous namespace


  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // Vector rotation in 3-dimension
  template <typename T>
  Vector3<double> Vector3<T>::rot(double radian, Vector3<double > const& axis) const {
    return (rotation_quaternion(-radian, axis)*Quaternion(0.0, *this)*rotation_quaternion(radian, axis)).v_;
  }

} // The end of the namespace "hashimoto_ut"

#endif // INCLUDE_GUARD_SHARED_VECTOR3_H

// HASHIMOTO, Yasuhiro (E-mail: hy @ sys.t.u-tokyo.ac.jp)
// update: 2008/12/01

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

#ifndef INCLUDE_GUARD_SHARED_GL_GLVIEW_H
#define INCLUDE_GUARD_SHARED_GL_GLVIEW_H

#ifdef _MSC_VER
#include <windows.h>
#endif
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif
#include "../vector2.h"
#include "../vector3.h"

#ifdef _MSC_VER
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#endif

namespace hashimoto_ut {

  namespace {
    int const DEGREE_RESOLUTION = 10; // 角度1度の分割数
    int const DEGREEH_MAX = 360*DEGREE_RESOLUTION;
    int const DEGREEV_MAX =  90*DEGREE_RESOLUTION;
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  class GLView {
  public:
    GLView(void);
    ~GLView();

    void initialize_matrix(void) const;
    void push_matrix(void) const;
    void pop_matrix(void) const;
    void set_perspective_matrix(void) const;
    void set_orthogonal_matrix(void) const; // 視点は規定値に固定

    void set_angleH(int degree);
    void set_angleV(int degree);
    void set_angle(int degreeH, int degreeV);
    void set_distance(double dst);
    void set_fov(double fov);
    void set_znear(double znear);
    void set_zfar(double zfar);
    void set_viewport(Vector2<int> const& origin, Vector2<int> const& size);

    Vector3<double> const& eye(void) const;
    double distance(void) const;
    double fov(void) const;
    int angleH(void) const;
    int angleV(void) const;
    double radianH(void) const;
    double radianV(void) const;
    Vector2<int> const& viewport_origin(void) const;
    Vector2<int> const& viewport_size(void) const;
    double viewport_aspect(void) const;

    bool scr2world_z0(Vector2<double>& pos, Vector2<int> const& scr) const;
    /* 視点eye_（ワールド座標）とビューポート内の点scr（スクリーン座標）を結ぶ直線が
     * ワールド座標のz=0平面に交わるワールド座標をposに代入．
     * 返り値は視線がz=0平面と交わる場合にtrue，交わらない場合にfalse．
     */

  private:
    void set_eye(void);

  private:
    // ※ワールド座標（z軸手前が正，奥が負）
    int angleH_;       // xy平面上の視線の角度（0≦angle<360*DEGREE_RESOLUTION）
    int angleV_;       // xy平面に対する視線の角度（0≦angle<360*DEGREE_RESOLUTION）
    double distance_;  // 視点から参照点までの距離
    double fov_;       // 視野角
    double znear_;     // 視体積手前までの距離
    double zfar_;      // 視体積奥までの距離
    Vector3<double> eye_; // 視点の位置
    Vector3<double> top_; // 頭頂の向き

    // ※スクリーン座標
    Vector2<int> viewport_origin_; // ビューポート原点
    Vector2<int> viewport_size_;   // ビューポートサイズ
    double viewport_aspect_;       // ビューポートサイズのアスペクト比
  };

} // The end of the namespace "hashimoto_ut"

#endif // INCLUDE_GUARD_SHARED_GL_GLVIEW_H

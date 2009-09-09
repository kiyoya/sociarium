// glview.cpp
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

#include "glview.h"
#include "../math.h"

namespace hashimoto_ut {

  ////////////////////////////////////////////////////////////////////////////////
  GLView::GLView(void) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
  }

  ////////////////////////////////////////////////////////////////////////////////
  GLView::~GLView() {
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
  }

  ////////////////////////////////////////////////////////////////////////////////
  void GLView::initialize_matrix(void) const {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
  }

  ////////////////////////////////////////////////////////////////////////////////
  void GLView::push_matrix(void) const {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
  }

  ////////////////////////////////////////////////////////////////////////////////
  void GLView::pop_matrix(void) const {
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
  }

  ////////////////////////////////////////////////////////////////////////////////
  void GLView::set_perspective_matrix(void) const {
    glMatrixMode(GL_PROJECTION);
    gluPerspective(fov_, viewport_aspect_, znear_, zfar_);
    glMatrixMode(GL_MODELVIEW);
    gluLookAt(eye_.x, eye_.y, eye_.z, 0.0, 0.0, 0.0, top_.x, top_.y, top_.z);
  }

  ////////////////////////////////////////////////////////////////////////////////
  void GLView::set_orthogonal_matrix(void) const {
    glMatrixMode(GL_PROJECTION);
    glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    gluLookAt(0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    // [メモ] PROJECTIONマトリクスがglOrthoの場合，gluLookAtは視点を視体積内に置かないと駄目
  }

  ////////////////////////////////////////////////////////////////////////////////
  void GLView::set_angleH(int degree) {
    if (degree<-DEGREEH_MAX) degree = -DEGREEH_MAX;
    else if (degree>DEGREEH_MAX) degree = DEGREEH_MAX;
    angleH_ = degree;
    set_eye();
  }

  ////////////////////////////////////////////////////////////////////////////////
  void GLView::set_angleV(int degree) {
    if (degree<-DEGREEV_MAX) degree = -DEGREEV_MAX;
    else if (degree>DEGREEV_MAX) degree = DEGREEV_MAX;
    angleV_ = degree;
    set_eye();
  }

  ////////////////////////////////////////////////////////////////////////////////
  void GLView::set_angle(int degreeH, int degreeV) {
    while (degreeH<0) degreeH += DEGREEH_MAX;
    while (degreeH>DEGREEH_MAX) degreeH -= DEGREEH_MAX;
    if (degreeV<-DEGREEV_MAX) degreeV = -DEGREEV_MAX;
    else if (degreeV>DEGREEV_MAX) degreeV = DEGREEV_MAX;
    angleH_ = degreeH;
    angleV_ = degreeV;
    set_eye();
  }

  ////////////////////////////////////////////////////////////////////////////////
  void GLView::set_distance(double dst) {
    distance_ = dst;
    set_eye();
  }

  ////////////////////////////////////////////////////////////////////////////////
  void GLView::set_fov(double fov) {
    fov_ = fov;
  }

  ////////////////////////////////////////////////////////////////////////////////
  void GLView::set_znear(double znear) {
    znear_ = znear;
  }

  ////////////////////////////////////////////////////////////////////////////////
  void GLView::set_zfar(double zfar) {
    zfar_ = zfar;
  }

  ////////////////////////////////////////////////////////////////////////////////
  void GLView::set_viewport(Vector2<int> const& origin, Vector2<int> const& size) {
    viewport_origin_ = origin;
    viewport_size_ = size;
    viewport_aspect_ = (viewport_size_.y==0)?0:double(viewport_size_.x)/viewport_size_.y;
    glViewport(viewport_origin_.x, viewport_origin_.y, viewport_size_.x, viewport_size_.y);
  }

  ////////////////////////////////////////////////////////////////////////////////
  Vector3<double> const& GLView::eye(void) const { return eye_; }
  double GLView::distance(void) const { return distance_; }
  double GLView::fov(void) const { return fov_; }
  int GLView::angleV(void) const { return angleV_; }
  int GLView::angleH(void) const { return angleH_; }
  double GLView::radianV(void) const { return M_2PI*angleV_/DEGREEH_MAX; }
  double GLView::radianH(void) const { return M_2PI*angleH_/DEGREEH_MAX; }
  Vector2<int> const& GLView::viewport_origin(void) const { return viewport_origin_; }
  Vector2<int> const& GLView::viewport_size(void) const { return viewport_size_; }
  double GLView::viewport_aspect(void) const { return viewport_aspect_; }

  ////////////////////////////////////////////////////////////////////////////////
  bool GLView::scr2world_z0(Vector2<double>& pos, Vector2<int> const& scr) const {
    push_matrix();
    initialize_matrix();
    set_perspective_matrix();
    double model[16], proj[16];
    int view[4];
    glGetDoublev(GL_MODELVIEW_MATRIX, model);
    glGetDoublev(GL_PROJECTION_MATRIX, proj);
    glGetIntegerv(GL_VIEWPORT, view);
    Vector3<double> v; // マウス座標に対応するカメラのznear面上のワールド座標
    gluUnProject(GLdouble(scr.x), GLdouble(scr.y), 0.0, model, proj, view, &v.x, &v.y, &v.z);
    // 透視射影の場合（正射影の場合は単純に(v.x, v.y)）
    if ((0.0<v.z && v.z<eye_.z) || (eye_.z<v.z && v.z<0.0)) {
      // 視線の先がz=0平面に交わる場合
      double const denom = eye_.z-v.z;
      pos.set((eye_.z*v.x-v.z*eye_.x)/denom, (eye_.z*v.y-v.z*eye_.y)/denom);
      pop_matrix();
      return true;
    } else {
      // 交わらない場合
      pos.set(v.x-eye_.x, v.y-eye_.y);
      pop_matrix();
      return false;
    }
  }

  ////////////////////////////////////////////////////////////////////////////////
  void GLView::set_eye(void) {
    double const theta = radianH();
    double const phi   = radianV();
    double const r = distance()*cos(phi);
    eye_.x = r*cos(theta);
    eye_.y = r*sin(theta);
    eye_.z = distance()*sin(phi);
    static Vector3<double> const zaxis(0.0, 0.0, 1.0);
    static Vector3<double> const yaxis(0.0, 1.0, 0.0);
    top_ = zaxis.rot(phi, yaxis.rot(-theta, zaxis));
  }

} // The end of the namespace "hashimoto_ut"

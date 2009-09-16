// s.o.c.i.a.r.i.u.m: agent.cpp
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

#ifdef _MSC_VER
#include <windows.h>
#endif
#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <gl/gl.h>
#endif
#include "agent.h"

namespace hashimoto_ut {

  using std::vector;
  using std::tr1::shared_ptr;


  namespace sociarium_project_agent {

    namespace {
      shared_ptr<DancingMan> dancing_man;
      shared_ptr<WalkingMan> walking_man;
    }

    ////////////////////////////////////////////////////////////////////////////////
    shared_ptr<DancingMan> get_dancing_man(void) {
      return dancing_man;
    }

    void set_dancing_man(vector<shared_ptr<GLTexture> > const& texture) {
      dancing_man.reset(new DancingMan(texture));
    }

    ////////////////////////////////////////////////////////////////////////////////
    shared_ptr<WalkingMan> get_walking_man(void) {
      return walking_man;
    }

    void set_walking_man(vector<shared_ptr<GLTexture> > const& texture) {
      walking_man.reset(new WalkingMan(texture));
    }

  } // The end of the namespace "sociarium_project_agent"


  ////////////////////////////////////////////////////////////////////////////////
  DancingMan::DancingMan(vector<shared_ptr<GLTexture> > const& textures)
       : textures_(textures) {}

  DancingMan::~DancingMan() {}

  int DancingMan::number_of_frames(void) const {
    return int(textures_.size());
  }

  void DancingMan::draw(Vector2<float> const& pos, float size, int frame) const {
    if (textures_.empty()) return;
    if (frame<0 || int(textures_.size())<=frame)
      frame = 0;

    GLTexture const* texture = textures_[frame].get();
    float const aspect = float(texture->width())/texture->height();
    Vector2<float> const sz(
      aspect<1.0f?Vector2<float>(aspect*size, size):Vector2<float>(size, size/aspect));

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture->get());
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0.0f, texture->ycoord());
    glVertex2f(pos.x-0.5f*sz.x, pos.y+0.5f*sz.y);
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(pos.x-0.5f*sz.x, pos.y-0.5f*sz.y);
    glTexCoord2f(texture->xcoord(), texture->ycoord());
    glVertex2f(pos.x+0.5f*sz.x, pos.y+0.5f*sz.y);
    glTexCoord2f(texture->xcoord(), 0.0f);
    glVertex2f(pos.x+0.5f*sz.x, pos.y-0.5f*sz.y);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
  }


  ////////////////////////////////////////////////////////////////////////////////
  WalkingMan::WalkingMan(vector<shared_ptr<GLTexture> > const& textures)
       : textures_(textures) {}

  WalkingMan::~WalkingMan() {}

  int WalkingMan::number_of_frames(void) const {
    return int(textures_.size());
  }

  void WalkingMan::draw(Vector2<float> const& pos, float size, int frame, int dir) const {
    if (textures_.empty()) return;
    if (frame<0 || int(textures_.size())<=frame)
      frame = 0;

    GLTexture const* texture = textures_[frame].get();
    float const aspect = float(texture->width())/texture->height();
    Vector2<float> const sz(
      aspect<1.0f?Vector2<float>(aspect*size, size):Vector2<float>(size, size/aspect));

    float const x0 = dir==LEFT?0.0f:texture->xcoord();
    float const x1 = dir==LEFT?texture->xcoord():0.0f;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture->get());
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(x0, texture->ycoord());
    glVertex2f(pos.x-0.5f*sz.x, pos.y+0.5f*sz.y);
    glTexCoord2f(x0, 0.0f);
    glVertex2f(pos.x-0.5f*sz.x, pos.y-0.5f*sz.y);
    glTexCoord2f(x1, texture->ycoord());
    glVertex2f(pos.x+0.5f*sz.x, pos.y+0.5f*sz.y);
    glTexCoord2f(x1, 0.0f);
    glVertex2f(pos.x+0.5f*sz.x, pos.y-0.5f*sz.y);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
  }

} // The end of the namespace "hashimoto_ut"

// s.o.c.i.a.r.i.u.m: agent.h
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

#ifndef INCLUDE_GUARD_SOCIARIUM_PROJECT_AGENT_H
#define INCLUDE_GUARD_SOCIARIUM_PROJECT_AGENT_H

#include <memory>
#include <vector>
#include <string>
#include "../shared/vector2.h"
#include "../shared/gl/gltexture.h"

namespace hashimoto_ut {

  ////////////////////////////////////////////////////////////////////////////////
  class DancingMan {
  public:
    DancingMan(std::vector<std::tr1::shared_ptr<GLTexture> > const& textures);
    ~DancingMan();
    int number_of_frames(void) const;
    void draw(Vector2<float> const& pos, float size, int frame) const;

  private:
    std::vector<std::tr1::shared_ptr<GLTexture> > textures_;
  };


  ////////////////////////////////////////////////////////////////////////////////
  class WalkingMan {
  public:
    enum { LEFT, RIGHT };

    WalkingMan(std::vector<std::tr1::shared_ptr<GLTexture> > const& textures);
    ~WalkingMan();
    int number_of_frames(void) const;
    void draw(Vector2<float> const& pos, float size, int frame, int dir) const;

  private:
    std::vector<std::tr1::shared_ptr<GLTexture> > textures_;
  };


  ////////////////////////////////////////////////////////////////////////////////
  class Balloon {
  public:
    Balloon(std::tr1::shared_ptr<GLTexture> const& textures,
            Vector2<float> const& velocity, float duration);
    ~Balloon();
    int number_of_frames(void) const;
    void update(void);
    void draw(void) const;

  private:
    std::wstring text_;
    Vector2<float> position_;
    Vector2<float> velocity_;
    float duration_;
  };

  ////////////////////////////////////////////////////////////////////////////////
  namespace sociarium_project_agent {

    std::tr1::shared_ptr<DancingMan> get_dancing_man(void);
    void set_dancing_man(std::vector<std::tr1::shared_ptr<GLTexture> > const& texture);

    std::tr1::shared_ptr<WalkingMan> get_walking_man(void);
    void set_walking_man(std::vector<std::tr1::shared_ptr<GLTexture> > const& texture);

  } // The end of the namespace "sociarium_project_agent"

} // The end of the namespace "hashimoto_ut"

#endif // INCLUDE_GUARD_SOCIARIUM_PROJECT_AGENT_H

// fbo.cpp
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
#include <windows.h>
#include <gl/glew.h>
#include "fbo.h"
#include "texture.h"

namespace hashimoto_ut {

  using std::tr1::shared_ptr;

  class FrameBufferObjectImpl : public FrameBufferObject {
  public:

    ////////////////////////////////////////////////////////////////////////////////
    FrameBufferObjectImpl(GLsizei width, GLsizei height, GLenum wrap_s, GLenum wrap_t) {

      // Create a texture.
      texture_ = Texture::create();
      int const err = texture_->set(width, height, wrap_s, wrap_t);
      assert(err==Texture::SUCCEEDED);

      // Bind the texture to FBO.
      glGenFramebuffersEXT(1, &id_);
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, id_);
      glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                                GL_TEXTURE_2D, texture_->get(), 0);
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    }


    ////////////////////////////////////////////////////////////////////////////////
    ~FrameBufferObjectImpl() {
      glDeleteFramebuffersEXT(1, &id_);
    }


    ////////////////////////////////////////////////////////////////////////////////
    GLuint get(void) const {
      return id_;
    }


    ////////////////////////////////////////////////////////////////////////////////
    shared_ptr<Texture> get_texture(void) const {
      return texture_;
    }

  private:
    GLuint id_;
    shared_ptr<Texture> texture_;
  };


  ////////////////////////////////////////////////////////////////////////////////
  // Factory function of FrameBufferObject.
  shared_ptr<FrameBufferObject>
    FrameBufferObject::create(GLsizei width, GLsizei height, GLenum wrap_s, GLenum wrap_t) {
      return shared_ptr<FrameBufferObject>(
        new FrameBufferObjectImpl(width, height, wrap_s, wrap_t));
    }

} // The end of the namespace "hashimoto_ut"

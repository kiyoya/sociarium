// texture.h
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

// #define SOCIAIRUM_PROJECT_USES_OPENCV

#ifndef INCLUDE_GUARD_SHARED_GL_TEXTURE_H
#define INCLUDE_GUARD_SHARED_GL_TEXTURE_H

#include <memory>
#include <windows.h>
#include <GL/gl.h>

#ifdef SOCIAIRUM_PROJECT_USES_OPENCV
#include <cv.h>
#endif

namespace hashimoto_ut {

  class Texture {
  public:
    enum {
      SUCCEEDED,
      FILE_NOT_FOUND,
      FAILED_TO_LOAD_IMAGE,
      UNSUPPORTED_FILE_FORMAT
    };

  public:
    virtual ~Texture() {}

    virtual int create(wchar_t const* filename, GLenum wrap_s, GLenum wrap_t) = 0;
    virtual int create_mipmap(wchar_t const* filename, GLenum wrap_s, GLenum wrap_t) = 0;
    virtual int create_subimage(wchar_t const* filename) = 0;
    virtual int create(GLuint width, GLuint height, GLenum wrap_s, GLenum wrap_t) = 0;

#ifdef SOCIAIRUM_PROJECT_USES_OPENCV
    virtual int create(IplImage* image, GLenum wrap_s, GLenum wrap_t) = 0;
    virtual int create_mipmap(IplImage* image, GLenum wrap_s, GLenum wrap_t) = 0;
    virtual int create_subimage(IplImage* image) = 0;
#endif

    virtual GLuint get(void) const = 0;
    virtual GLsizei width(void) const = 0;
    virtual GLsizei height(void) const = 0;
    virtual GLfloat xcoord(void) const = 0;
    virtual GLfloat ycoord(void) const = 0;

    static std::tr1::shared_ptr<Texture> create(void);
  };

} // The end of the namespace "hashimoto_ut"

#endif // INCLUDE_GUARD_SHARED_GL_TEXTURE_H

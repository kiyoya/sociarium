// s.o.c.i.a.r.i.u.m: cvframe.h
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

#ifndef INCLUDE_GUARD_SOCIARIUM_PROJECT_CVFRAME_H
#define INCLUDE_GUARD_SOCIARIUM_PROJECT_CVFRAME_H

#include <windows.h>
#include <boost/thread.hpp>
#include <GL/gl.h>
#include "../shared/thread.h"
#include "../shared/mutex.h"

namespace hashimoto_ut {

  class GLTexture;

  ////////////////////////////////////////////////////////////////////////////////
  class CVFrame : public Thread, public Mutex {
  public:
    virtual ~CVFrame() {}
    virtual void set_camera(void) = 0;
    virtual void set_movie(char const* movie_filename) = 0;
    virtual bool set_masking_image(char const* image_filename) = 0;
    virtual bool set_chroma_key_background_image(char const* image_filename) = 0;
    virtual GLTexture const* get_texture(void) const = 0;
  };

  ////////////////////////////////////////////////////////////////////////////////
  namespace sociarium_project_cvframe {

    std::tr1::shared_ptr<CVFrame> create(HDC hdc, HGLRC hrc);
    std::tr1::shared_ptr<CVFrame> get_current_frame(void);
    void invoke(std::tr1::shared_ptr<CVFrame> cvframe);
    void terminate(void);
    void join(void);
    bool joinable(void);

  } // The end of the namespace "sociarium_project_cvframe"

} // The end of the namespace "hashimoto_ut"

#endif // INCLUDE_GUARD_SOCIARIUM_PROJECT_CVFRAME_H

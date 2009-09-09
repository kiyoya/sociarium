// s.o.c.i.a.r.i.u.m: font.h
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

#ifndef INCLUDE_GUARD_SOCIARIUM_PROJECT_FONT_H
#define INCLUDE_GUARD_SOCIARIUM_PROJECT_FONT_H

#include <memory>

class FTFont;

namespace hashimoto_ut {

  namespace sociarium_project_font {

    ////////////////////////////////////////////////////////////////////////////////
    namespace FontCategory {
      enum {
        NODE_NAME = 0,
        EDGE_NAME,
        COMMUNITY_NAME,
        COMMUNITY_EDGE_NAME,
        MISC,
        NUMBER_OF_CATEGORIES
      };
    }

    ////////////////////////////////////////////////////////////////////////////////
    namespace FontType {
      enum {
        POLYGON_FONT = 0, // use 'FTPolygonFont'
        TEXTURE_FONT,     // use 'FTTextureFont'
        NUMBER_OF_FONT_TYPES
      };
    }

    ////////////////////////////////////////////////////////////////////////////////
    namespace FontScale {
      enum {
        TINY = 0,
        SMALL,
        NORMAL,
        LARGE,
        XLARGE,
        NUMBER_OF_FONT_SCALES
      };
    }

    ////////////////////////////////////////////////////////////////////////////////
    void initialize(void);

    ////////////////////////////////////////////////////////////////////////////////
    int get_font_type(void);
    void set_font_type(int type_id);

    ////////////////////////////////////////////////////////////////////////////////
    std::tr1::shared_ptr<FTFont> get_font(int category_id);

    ////////////////////////////////////////////////////////////////////////////////
    float get_font_scale(int category_id);
    void set_font_scale(int category_id, int scale_id);

    ////////////////////////////////////////////////////////////////////////////////
    float get_default_font_scale(int scale_id);

  } // The end of the namespace "sociarium_project_font"

} // The end of the namespace "hashimoto_ut"

#endif // INCLUDE_GUARD_SOCIARIUM_PROJECT_FONT_H

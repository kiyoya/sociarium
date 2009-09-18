// s.o.c.i.a.r.i.u.m: font.cpp
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
#include <string>
#include <windows.h>
#include <FTGL/ftgl.h>
#include "font.h"
#include "common.h"
#include "language.h"
#include "../shared/msgbox.h"
#include "../shared/win32api.h"

namespace hashimoto_ut {

  using std::wstring;
  using std::tr1::shared_ptr;

  using namespace sociarium_project_common;
  using namespace sociarium_project_language;

  namespace sociarium_project_font {

    namespace {
      ////////////////////////////////////////////////////////////////////////////////
      char const FONT_OPTION[][_MAX_PATH] = {
        // Priority sequence of default used fonts.
        "C:\\Windows\\Fonts\\meiryo.ttc",
        "C:\\Windows\\Fonts\\msgothic.ttc",
        "C:\\Windows\\Fonts\\arial.ttf"
        };

      size_t const NUMBER_OF_FONT_OPTIONS
        = sizeof(FONT_OPTION)/sizeof(char[_MAX_PATH]);


      ////////////////////////////////////////////////////////////////////////////////
      int const default_face_size = 24;

      float const default_font_scale[FontScale::NUMBER_OF_FONT_SCALES] = {
        0.01f, // FontScale::TINY
        0.02f, // FontScale::SMALL
        0.04f, // FontScale::NORMAL
        0.08f, // FontScale::LARGE
        0.16f  // FontScale::XLARGE
        };

      int font_scale_id[FontCategory::NUMBER_OF_CATEGORIES] = {
        FontScale::NORMAL, // FontCategory::NODE_NAME
        FontScale::SMALL,  // FontCategory::EDGE_NAME
        FontScale::NORMAL, // FontCategory::COMMUNITY_NAME
        FontScale::SMALL,  // FontCategory::COMMUNITY_EDGE_NAME
        FontScale::NORMAL  // FontCategory::MISC
        };


      ////////////////////////////////////////////////////////////////////////////////
      int font_type_id = FontType::POLYGON_FONT; // or FontType::TEXTURE_FONT;


      ////////////////////////////////////////////////////////////////////////////////
      shared_ptr<FTFont> font[FontCategory::NUMBER_OF_CATEGORIES];


      ////////////////////////////////////////////////////////////////////////////////
      void set_font(shared_ptr<FTFont>& f, int type) {

        // Try to create a font object following the priority order of font options.
        for (size_t i=0; i<NUMBER_OF_FONT_OPTIONS; ++i) {
          switch (type) {
          case FontType::POLYGON_FONT:
            f.reset(new FTPolygonFont(FONT_OPTION[i]));
            break;
          case FontType::TEXTURE_FONT:
            f.reset(new FTTextureFont(FONT_OPTION[i]));
            break;
          default:
            assert(0 && "never reach");
          }

          wstring const filename = mbcs2wcs(FONT_OPTION[i], strlen(FONT_OPTION[i]));

          if (!f->Error()) {
            if (f->FaceSize(default_face_size)) {
              if (f->CharMap(ft_encoding_unicode)) break; // Successfully finished.
              else if (i==NUMBER_OF_FONT_OPTIONS-1) {
                // No option available any more.
                message_box(
                  get_window_handle(),
                  MessageType::CRITICAL,
                  APPLICATION_TITLE,
                  L"%s: %s [ft_encoding_unicode]",
                  get_message(Message::FTGL_ERROR_CHARMAP),
                  filename.c_str());
                exit(1);
              } else continue; // Try the next option.
            } else if (i==NUMBER_OF_FONT_OPTIONS-1) {
              // No option available any more.
              message_box(
                get_window_handle(),
                MessageType::CRITICAL,
                APPLICATION_TITLE,
                L"%s: %s [%d]",
                get_message(Message::FTGL_ERROR_FACESIZE),
                filename.c_str(), default_face_size);
              exit(1);
            } else continue; // Try the next option.
          } else if (i==NUMBER_OF_FONT_OPTIONS-1) {
            // No option available any more.
            message_box(
              get_window_handle(),
              MessageType::CRITICAL,
              APPLICATION_TITLE,
              L"%s",
              get_message(Message::FTGL_ERROR_CREATE));
            exit(1);
          } else continue; // Try the next option.
        }
      }

    } // The end of the anonymous namespace


    ////////////////////////////////////////////////////////////////////////////////
    void initialize(void) {
      set_font_type(font_type_id);
    }


    ////////////////////////////////////////////////////////////////////////////////
    int get_font_type(void) {
      return font_type_id;
    }

    void set_font_type(int type_id) {
      assert(0<=type_id && type_id<FontType::NUMBER_OF_FONT_TYPES);
      font_type_id = type_id;
      set_font(font[FontCategory::NODE_NAME], font_type_id);
      set_font(font[FontCategory::EDGE_NAME], font_type_id);
      set_font(font[FontCategory::COMMUNITY_NAME], font_type_id);
      set_font(font[FontCategory::COMMUNITY_EDGE_NAME], font_type_id);
      set_font(font[FontCategory::MISC], font_type_id);
    }


    ////////////////////////////////////////////////////////////////////////////////
    shared_ptr<FTFont> get_font(int category_id) {
      assert(0<=category_id && category_id<FontCategory::NUMBER_OF_CATEGORIES);
      return font[category_id];
    }


    ////////////////////////////////////////////////////////////////////////////////
    float get_font_scale(int category_id) {
      assert(0<=category_id && category_id<FontCategory::NUMBER_OF_CATEGORIES);
      int const scale_id = font_scale_id[category_id];
      assert(0<=scale_id && scale_id<FontScale::NUMBER_OF_FONT_SCALES);
      return default_font_scale[scale_id];
    }

    void set_font_scale(int category_id, int scale_id) {
      assert(0<=category_id && category_id<FontCategory::NUMBER_OF_CATEGORIES);
      assert(0<=scale_id && scale_id<=FontScale::NUMBER_OF_FONT_SCALES);
      font_scale_id[category_id] = scale_id;
    }


    ////////////////////////////////////////////////////////////////////////////////
    float get_default_font_scale(int scale_id) {
      assert(0<=scale_id && scale_id<=FontScale::NUMBER_OF_FONT_SCALES);
      return default_font_scale[scale_id];
    }

  } // The end of the namespace "sociarium_project_font"

} // The end of the namespace "hashimoto_ut"

// s.o.c.i.a.r.i.u.m
// HASHIMOTO, Yasuhiro (E-mail: hy @ sys.t.u-tokyo.ac.jp)
// update: 2009/03/15

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

#include <cassert>
#include <string>
#ifdef _MSC_VER
#include <windows.h>
#endif
#include <FTGL/ftgl.h>
#include "common.h"
#include "message.h"
#include "font.h"
#include "../shared/msgbox.h"
#include "../shared/win32api.h"

namespace hashimoto_ut {

  using std::wstring;
  using std::tr1::shared_ptr;

  namespace {

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // ローカル変数

#ifdef __APPLE__
    size_t const NUMBER_OF_FONT_OPTION = 1;
	char const FONT_LIST[NUMBER_OF_FONT_OPTION][256] = {
      "/Library/Fonts/Osaka.ttf"
    };
#elif _MSC_VER
    size_t const NUMBER_OF_FONT_OPTION = 2;
    char const FONT_LIST[NUMBER_OF_FONT_OPTION][_MAX_PATH] = {
      // 使用するフォントの優先順位
      "C:\\Windows\\Fonts\\meiryob.ttc",
      "C:\\Windows\\Fonts\\msgothic.ttc"
      };
#endif

    int const default_font_size = 24;
    float const font_scale[4] = { 0.02f, 0.04f, 0.08f, 0.24f };

    float node_font_scale = font_scale[1];
    float edge_font_scale = font_scale[0];
    float community_font_scale = font_scale[2];
    float community_edge_font_scale = font_scale[1];
    float misc_font_scale = font_scale[3];

    struct Font {
      shared_ptr<FTFont> body;
      wstring filename;
    };

    Font node_font;
    Font edge_font;
    Font misc_font;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // ローカル関数

    void set_font(Font& f) {
      for (size_t i=0; i<NUMBER_OF_FONT_OPTION; ++i) {
        f.body.reset(new FTGLPolygonFont(FONT_LIST[i]));
        f.filename = mbcs2wcs(FONT_LIST[i], strlen(FONT_LIST[i]));
        if (!f.body->Error()) {
          if (f.body->FaceSize(default_font_size)) {
            if (f.body->CharMap(ft_encoding_unicode)) break; // 正常にフォントを生成した場合はここから抜ける
            else if (i==NUMBER_OF_FONT_OPTION-1) {
              MsgBox(sociarium_project_common::get_window_handle(), sociarium_project_common::APPLICATION_TITLE, L"%s [%s, ft_encoding_unicode]",
                     sociarium_project_message::FTGL_ERROR_CHARMAP, f.filename.c_str());
              exit(1);
            } else continue;
          } else if (i==NUMBER_OF_FONT_OPTION-1) {
            MsgBox(sociarium_project_common::get_window_handle(), sociarium_project_common::APPLICATION_TITLE, L"%s [%s, %d]",
                   sociarium_project_message::FTGL_ERROR_FACESIZE, f.filename.c_str(), default_font_size);
            exit(1);
          } else continue;
        } else if (i==NUMBER_OF_FONT_OPTION-1) {
          MsgBox(sociarium_project_common::get_window_handle(), sociarium_project_common::APPLICATION_TITLE, L"%s", sociarium_project_message::FTGL_ERROR_CREATE);
          exit(1);
        } else continue;
      }
    }

  } // The end of the anonymous namespace

  namespace sociarium_project_font {

    void create(void) {
      set_font(node_font);
      set_font(edge_font);
      set_font(misc_font);
    }

    shared_ptr<FTFont> const& get_node_font(void) { return node_font.body; }
    shared_ptr<FTFont> const& get_edge_font(void) { return edge_font.body; }
    shared_ptr<FTFont> const& get_misc_font(void) { return misc_font.body; }

    float get_node_font_scale(void) { return node_font_scale; }
    float get_edge_font_scale(void) { return edge_font_scale; }
    float get_community_font_scale(void) { return community_font_scale; }
    float get_community_edge_font_scale(void) { return community_edge_font_scale; }
    float get_misc_font_scale(void) { return misc_font_scale; }

    float get_font_scale(int index) { return font_scale[index]; }

    void set_node_font_scale(int index) { node_font_scale = font_scale[index]; }
    void set_edge_font_scale(int index) { edge_font_scale = font_scale[index]; }
    void set_community_font_scale(int index) { community_font_scale = font_scale[index]; }
    void set_community_edge_font_scale(int index) { community_edge_font_scale = font_scale[index]; }
    void set_misc_font_scale(int index) { misc_font_scale = font_scale[index]; }

  } // The end of the namespace "sociarium_project_font"

} // The end of the namespace "hashimoto_ut"

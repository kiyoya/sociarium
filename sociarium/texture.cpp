// s.o.c.i.a.r.i.u.m
// HASHIMOTO, Yasuhiro (E-mail: hy @ sys.t.u-tokyo.ac.jp)
// update: 2009/05/01

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
#include <memory>
#include <unordered_map>
#include <windows.h>
#include "common.h"
#include "message.h"
#include "texture.h"
#include "../shared/msgbox.h"
#include "../shared/GL/gltexture.h"

namespace hashimoto_ut {

  using std::wstring;
  using std::tr1::shared_ptr;
  using std::tr1::unordered_map;

  namespace {
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // ローカル変数

    shared_ptr<GLTexture> default_node_texture;
    shared_ptr<GLTexture> default_edge_texture;
    shared_ptr<GLTexture> default_community_texture;
    shared_ptr<GLTexture> slider_texture;
    unordered_map<wstring, shared_ptr<GLTexture> > filename2texture;
    wstring texture_folder_path = L"";;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // ローカル関数

    void set_texture_dispatch(GLTexture* texture, wstring const& filename, GLenum wrap) {
      assert(texture!=0);
      wstring const full_path = sociarium_project_texture::get_texture_folder_path()+filename;
      int const err = texture->create_mipmap(full_path.c_str(), wrap);
      if (err!=GLTexture::SUCCEEDED)
        MsgBox(sociarium_project_common::get_window_handle(), sociarium_project_common::APPLICATION_TITLE,
               L"%s [%s, %d]", sociarium_project_message::TEXTURE_ERROR_CREATE, full_path.c_str(), err);
    }

    GLTexture const* get_texture_dispatch(wstring const& name, GLenum wrap, GLTexture const* default_retval) {
      wstring filename = name+L".png";
      if (filename2texture.find(filename)==filename2texture.end()) {
        wstring const full_path = sociarium_project_texture::get_texture_folder_path()+filename;
        shared_ptr<GLTexture> texture(new GLTexture());
        int const err = texture->create_mipmap(full_path.c_str(), wrap);
        if (err!=GLTexture::SUCCEEDED) return default_retval;
        filename2texture[filename] = texture;
        return texture.get();
      }
      return filename2texture[filename].get();
    }
  }

  namespace sociarium_project_texture {

    wstring get_texture_folder_path(void) {
      if (texture_folder_path.empty()) return sociarium_project_common::get_module_path()+L"texture\\";
      return texture_folder_path;
    }

    void set_texture_folder_path(wstring const& path) {
      texture_folder_path = path;
    }

    GLTexture const* get_default_node_texture(void) {
      assert(default_node_texture!=0);
      return default_node_texture.get();
    }

    GLTexture const* get_default_edge_texture(void) {
      assert(default_edge_texture!=0);
      return default_edge_texture.get();
    }

    GLTexture const* get_default_community_texture(void) {
      assert(default_community_texture!=0);
      return default_community_texture.get();
    }

    void set_default_node_texture(wstring const& filename, GLenum wrap) {
      default_node_texture.reset(new GLTexture());
      set_texture_dispatch(default_node_texture.get(), filename, wrap);
    }

    void set_default_edge_texture(wstring const& filename, GLenum wrap) {
      default_edge_texture.reset(new GLTexture());
      set_texture_dispatch(default_edge_texture.get(), filename, wrap);
    }

    void set_default_community_texture(wstring const& filename, GLenum wrap) {
      default_community_texture.reset(new GLTexture());
      set_texture_dispatch(default_community_texture.get(), filename, wrap);
    }

    void set_slider_texture(wstring const& filename, GLenum wrap) {
      slider_texture.reset(new GLTexture());
      set_texture_dispatch(slider_texture.get(), filename, wrap);
    }

    GLTexture const* get_node_texture(wstring const& name) {
      return get_texture_dispatch(name, GL_CLAMP, default_node_texture.get());
    }

    GLTexture const* get_edge_texture(wstring const& name) {
      return get_texture_dispatch(name, GL_REPEAT, default_edge_texture.get());
    }

    GLTexture const* get_texture(wstring const& name) {
      return get_texture_dispatch(name, GL_REPEAT, 0);
    }

    GLTexture const* get_slider_texture(void) {
      return slider_texture.get();
    }

  } // The end of the namespace "sociarium_project_texture"

} // The end of the namespace "hashimoto_ut"

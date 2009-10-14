// s.o.c.i.a.r.i.u.m: texture.cpp
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
#ifdef _MSC_VER
#include <memory>
#include <unordered_map>
#else
#include <tr1/memory>
#include <tr1/unordered_map>
#endif
#include <boost/format.hpp>
#ifdef __APPLE__
#include <GL/glew.h>
#else
#include <gl/glew.h>
#endif
#include "common.h"
#include "menu_and_message.h"
#include "texture.h"
#include "../shared/msgbox.h"
#include "../shared/gl/texture.h"

#define SOCIARIUM_PROJECT_USE_MIPMAP_TEXTURE

namespace hashimoto_ut {

  using std::wstring;
  using std::pair;
  using std::make_pair;
  using std::tr1::shared_ptr;
  using std::tr1::unordered_map;

  using namespace sociarium_project_common;
  using namespace sociarium_project_menu_and_message;

  namespace {

    ////////////////////////////////////////////////////////////////////////////////
    wstring texture_folder_path = L"";

    ////////////////////////////////////////////////////////////////////////////////
    GLint wrap_s = GL_CLAMP_TO_EDGE_EXT;
    GLint wrap_t = GL_CLAMP_TO_EDGE_EXT;

    ////////////////////////////////////////////////////////////////////////////////
    shared_ptr<Texture> default_node_texture;
    shared_ptr<Texture> default_edge_texture;
    shared_ptr<Texture> default_community_texture;
    shared_ptr<Texture> default_community_edge_texture;

    shared_ptr<Texture> default_node_texture_tmp;
    shared_ptr<Texture> default_edge_texture_tmp;
    shared_ptr<Texture> default_community_texture_tmp;
    shared_ptr<Texture> default_community_edge_texture_tmp;

    ////////////////////////////////////////////////////////////////////////////////
    typedef unordered_map<wstring, shared_ptr<Texture> > TextureMap;
    TextureMap identification_name2texture;

    ////////////////////////////////////////////////////////////////////////////////
    wstring get_full_path_of_texture_folder(void) {
#ifdef __APPLE__
      return get_module_path()+L"texture/"+texture_folder_path;
#elif _MSC_VER
      return get_module_path()+L"texture\\"+texture_folder_path;
#else
#error Not implemented
#endif
    }

    ////////////////////////////////////////////////////////////////////////////////
    void set_texture_dispatch(
      shared_ptr<Texture> texture,
      wstring const& filename,
      GLenum wrap_s, GLenum wrap_t) {

      assert(texture!=0);
      wstring const full_path = get_full_path_of_texture_folder()+filename;

#ifdef SOCIARIUM_PROJECT_USE_MIPMAP_TEXTURE
      int const err = texture->set_mipmap(full_path.c_str(), wrap_s, wrap_t);
#else
      int const err = texture->set(full_path.c_str(), wrap_s, wrap_t);
#endif

      if (err!=Texture::SUCCEEDED)
        texture.reset();
    }

    ////////////////////////////////////////////////////////////////////////////////
    Texture const* set_texture_dispatch(
      wstring const& filename,
      GLenum wrap_s, GLenum wrap_t) {

      wstring const identification_name = texture_folder_path+filename;
      TextureMap::iterator i = identification_name2texture.find(identification_name);

      if (i==identification_name2texture.end()) {
        wstring const full_path = get_full_path_of_texture_folder()+filename;
        shared_ptr<Texture> texture = Texture::create();

#ifdef SOCIARIUM_PROJECT_USE_MIPMAP_TEXTURE
        int const err = texture->set_mipmap(full_path.c_str(), wrap_s, wrap_t);
#else
        int const err = texture->set(full_path.c_str(), wrap_s, wrap_t);
#endif

        if (err!=Texture::SUCCEEDED)
          return 0;

        pair<TextureMap::iterator, bool> pp
          = identification_name2texture.insert(
            make_pair(identification_name, texture));

        if (!pp.second)
          return 0;

        return pp.first->second.get();
      }

      return i->second.get();
    }

  } // The end of the anonymous namespace


  namespace sociarium_project_texture {

    ////////////////////////////////////////////////////////////////////////////////
    wstring const& get_texture_folder_path(void) {
      return texture_folder_path;
    }

    void set_texture_folder_path(wstring const& path) {
      texture_folder_path = path;
    }

    void initialize_texture_folder_path(void) {
      texture_folder_path = L"";
    }


    ////////////////////////////////////////////////////////////////////////////////
    GLint get_texture_parameter_wrap_s(void) {
      return wrap_s;
    }

    void set_texture_parameter_wrap_s(GLint value) {
      wrap_s = value;
    }

    GLint get_texture_parameter_wrap_t(void) {
      return wrap_t;
    }

    void set_texture_parameter_wrap_t(GLint value) {
      wrap_t = value;
    }


    ////////////////////////////////////////////////////////////////////////////////
    Texture const* get_texture_by_name(wstring const& name) {
      Texture const* texture = set_texture_dispatch(name+L".png", wrap_s, wrap_t);
      if (texture) return texture;
      return set_texture_dispatch(name+L".jpg", wrap_s, wrap_t);
    }

    Texture const* get_texture(wstring const& filename) {
      return set_texture_dispatch(filename, wrap_s, wrap_t);
    }


    ////////////////////////////////////////////////////////////////////////////////
    void update_default_textures(void) {
      default_node_texture = default_node_texture_tmp;
      default_edge_texture = default_edge_texture_tmp;
      default_community_texture = default_community_texture_tmp;
      default_community_edge_texture = default_community_edge_texture_tmp;
    }


    ////////////////////////////////////////////////////////////////////////////////
    Texture const* get_default_node_texture(void) {
      assert(default_node_texture!=0);
      return default_node_texture.get();
    }

    Texture const* get_default_node_texture_tmp(void) {
      assert(default_node_texture_tmp!=0);
      return default_node_texture_tmp.get();
    }

    void set_default_node_texture(wstring const& filename) {
      default_node_texture = Texture::create();
      set_texture_dispatch(default_node_texture, filename, wrap_s, wrap_t);
    }

    void set_default_node_texture_tmp(wstring const& filename) {
      default_node_texture_tmp = Texture::create();
      set_texture_dispatch(default_node_texture_tmp, filename, wrap_s, wrap_t);
    }


    ////////////////////////////////////////////////////////////////////////////////
    Texture const* get_default_edge_texture(void) {
      assert(default_edge_texture!=0);
      return default_edge_texture.get();
    }

    Texture const* get_default_edge_texture_tmp(void) {
      assert(default_edge_texture_tmp!=0);
      return default_edge_texture_tmp.get();
    }

    void set_default_edge_texture(wstring const& filename) {
      default_edge_texture = Texture::create();
      set_texture_dispatch(default_edge_texture, filename, wrap_s, wrap_t);
    }

    void set_default_edge_texture_tmp(wstring const& filename) {
      default_edge_texture_tmp = Texture::create();
      set_texture_dispatch(default_edge_texture_tmp, filename, wrap_s, wrap_t);
    }


    ////////////////////////////////////////////////////////////////////////////////
    Texture const* get_default_community_texture(void) {
      assert(default_community_texture!=0);
      return default_community_texture.get();
    }

    Texture const* get_default_community_texture_tmp(void) {
      assert(default_community_texture_tmp!=0);
      return default_community_texture_tmp.get();
    }

    void set_default_community_texture(wstring const& filename) {
      default_community_texture = Texture::create();
      set_texture_dispatch(default_community_texture, filename, wrap_s, wrap_t);
    }

    void set_default_community_texture_tmp(wstring const& filename) {
      default_community_texture_tmp = Texture::create();
      set_texture_dispatch(default_community_texture_tmp, filename, wrap_s, wrap_t);
    }


    ////////////////////////////////////////////////////////////////////////////////
    Texture const* get_default_community_edge_texture(void) {
      assert(default_community_edge_texture!=0);
      return default_community_edge_texture.get();
    }

    Texture const* get_default_community_edge_texture_tmp(void) {
      assert(default_community_edge_texture_tmp!=0);
      return default_community_edge_texture_tmp.get();
    }

    void set_default_community_edge_texture(wstring const& filename) {
      default_community_edge_texture = Texture::create();
      set_texture_dispatch(
        default_community_edge_texture, filename, wrap_s, wrap_t);
    }

    void set_default_community_edge_texture_tmp(wstring const& filename) {
      default_community_edge_texture_tmp = Texture::create();
      set_texture_dispatch(
        default_community_edge_texture_tmp, filename, wrap_s, wrap_t);
    }

  } // The end of the namespace "sociarium_project_texture"

} // The end of the namespace "hashimoto_ut"

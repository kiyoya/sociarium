// s.o.c.i.a.r.i.u.m: texture.h
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

#ifndef INCLUDE_GUARD_SOCIARIUM_PROJECT_TEXTURE_H
#define INCLUDE_GUARD_SOCIARIUM_PROJECT_TEXTURE_H

#include <string>
#include <windows.h>
#include <GL/gl.h>

namespace hashimoto_ut {

  class Texture;

  namespace sociarium_project_texture {

    ////////////////////////////////////////////////////////////////////////////////
    // Relative path against the default texture folder path.
    // The default texture path is module_path/texture/.
    std::wstring const& get_texture_folder_path(void);
    void set_texture_folder_path(std::wstring const& path);
    void initialize_texture_folder_path(void);

    ////////////////////////////////////////////////////////////////////////////////
    GLint get_texture_parameter_wrap_s(void);
    void set_texture_parameter_wrap_s(GLint wrap);
    GLint get_texture_parameter_wrap_t(void);
    void set_texture_parameter_wrap_t(GLint wrap);

    ////////////////////////////////////////////////////////////////////////////////
    Texture const* get_texture_by_name(std::wstring const& name);
    Texture const* get_texture(std::wstring const& filename);

    ////////////////////////////////////////////////////////////////////////////////
    // Make temporary assigned default textures fix.
    void update_default_textures(void);

    ////////////////////////////////////////////////////////////////////////////////
    Texture const* get_default_node_texture(void);
    Texture const* get_default_node_texture_tmp(void);
    void set_default_node_texture(std::wstring const& filename);
    void set_default_node_texture_tmp(std::wstring const& filename);

    ////////////////////////////////////////////////////////////////////////////////
    Texture const* get_default_edge_texture(void);
    Texture const* get_default_edge_texture_tmp(void);
    void set_default_edge_texture(std::wstring const& filename);
    void set_default_edge_texture_tmp(std::wstring const& filename);

    ////////////////////////////////////////////////////////////////////////////////
    Texture const* get_default_community_texture(void);
    Texture const* get_default_community_texture_tmp(void);
    void set_default_community_texture(std::wstring const& filename);
    void set_default_community_texture_tmp(std::wstring const& filename);

    ////////////////////////////////////////////////////////////////////////////////
    Texture const* get_default_community_edge_texture(void);
    Texture const* get_default_community_edge_texture_tmp(void);
    void set_default_community_edge_texture(std::wstring const& filename);
    void set_default_community_edge_texture_tmp(std::wstring const& filename);

  } // The end of the namespace "sociarium_project_texture"

} // The end of the namespace "hashimoto_ut"

#endif // INCLUDE_GUARD_SOCIARIUM_PROJECT_TEXTURE_H

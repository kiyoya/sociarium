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

#ifndef INCLUDE_GUARD_SOCIARIUM_PROJECT_TEXTURE_H
#define INCLUDE_GUARD_SOCIARIUM_PROJECT_TEXTURE_H

#include <string>
#include <GL/gl.h>

namespace hashimoto_ut {

  class GLTexture;

  namespace sociarium_project_texture {

    std::wstring get_texture_folder_path(void);
    void set_texture_folder_path(std::wstring const& path);

    GLTexture const* get_default_node_texture(void);
    GLTexture const* get_default_edge_texture(void);
    GLTexture const* get_default_community_texture(void);

    void set_default_node_texture(std::wstring const& filename, GLenum wrap);
    void set_default_edge_texture(std::wstring const& filename, GLenum wrap);
    void set_default_community_texture(std::wstring const& filename, GLenum wrap);
    void set_slider_texture(std::wstring const& filename, GLenum wrap);

    GLTexture const* get_node_texture(std::wstring const& name);
    GLTexture const* get_edge_texture(std::wstring const& name);
    GLTexture const* get_texture(std::wstring const& name);
    GLTexture const* get_slider_texture(void);

  } // The end of the namespace "sociarium_project_texture"

} // The end of the namespace "hashimoto_ut"

#endif // INCLUDE_GUARD_SOCIARIUM_PROJECT_TEXTURE_H

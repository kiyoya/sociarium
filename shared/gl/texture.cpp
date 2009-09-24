// gltexture.cpp
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
#include <atlimage.h>
#include <GL/glu.h>
#include <GL/glext.h>
#include "texture.h"
#include "../win32api.h"

#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "opengl32.lib")

#ifdef SOCIAIRUM_PROJECT_USES_OPENCV
#pragma comment(lib, "cv.lib")
#pragma comment(lib, "cxcore.lib")
#pragma comment(lib, "highgui.lib")
#endif

namespace hashimoto_ut {

  using std::tr1::shared_ptr;

  class TextureImpl : public Texture {
  public:

    ////////////////////////////////////////////////////////////////////////////////
    TextureImpl(void) :
    width_(0), height_(0), xcoord_(0.0f), ycoord_(0.0f) {

      glGenTextures(1, &id_);
      GLenum err = glGetError();
      assert(err!=GL_INVALID_VALUE);
      assert(err!=GL_INVALID_OPERATION);
    }


    ////////////////////////////////////////////////////////////////////////////////
    ~TextureImpl() {
      glDeleteTextures(1, &id_);
    }


    ////////////////////////////////////////////////////////////////////////////////
    int create(wchar_t const* filename, GLenum wrap_s, GLenum wrap_t) {

      DWORD dw = GetFileAttributes(filename);

      if (dw==-1)
        return FILE_NOT_FOUND;

      CImage image;
      HRESULT res = image.Load(filename);

      if (res!=S_OK)
        return FAILED_TO_LOAD_IMAGE;

      width_  = image.GetWidth();
      height_ = image.GetHeight();

      GLsizei w_pot = 1;
      while (w_pot<width_) w_pot *= 2;

      GLsizei h_pot = 1;
      while (h_pot<height_) h_pot *= 2;

      xcoord_ = GLfloat(width_)/w_pot;
      ycoord_ = GLfloat(height_)/h_pot;

      glBindTexture(GL_TEXTURE_2D, id_);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

      int const bpp = image.GetBPP();
      int const pitch = image.GetPitch();
      char const* pixels = (char*)image.GetBits()+(pitch<0?pitch*(height_-1):0);

      switch (bpp) {
      case 24:
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w_pot, h_pot,
                     0, GL_BGR, GL_UNSIGNED_BYTE, 0);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width_, height_,
                        GL_BGR, GL_UNSIGNED_BYTE, (GLvoid*)pixels);
        break;
      case 32:
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_, height_, 0,
                     GL_BGRA, GL_UNSIGNED_BYTE, 0);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width_, height_,
                        GL_BGRA, GL_UNSIGNED_BYTE, (GLvoid*)pixels);
        break;
      default:
        glBindTexture(GL_TEXTURE_2D, 0);
        return UNSUPPORTED_FILE_FORMAT;
      }

      glBindTexture(GL_TEXTURE_2D, 0);
      return SUCCEEDED;
    }


    ////////////////////////////////////////////////////////////////////////////////
    int create_mipmap(wchar_t const* filename, GLenum wrap_s, GLenum wrap_t) {

      DWORD dw = GetFileAttributes(filename);

      if (dw==-1)
        return FILE_NOT_FOUND;

      CImage image;
      HRESULT res = image.Load(filename);

      if (res!=S_OK)
        return FAILED_TO_LOAD_IMAGE;

      width_  = image.GetWidth();
      height_ = image.GetHeight();

      xcoord_ = 1.0f;
      ycoord_ = 1.0f;

      glBindTexture(GL_TEXTURE_2D, id_);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

      int const bpp = image.GetBPP();
      int const pitch = image.GetPitch();
      char const* pixels = (char*)image.GetBits()+(pitch<0?pitch*(height_-1):0);

      switch (bpp) {
      case 24:
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, width_, height_,
                          GL_BGR, GL_UNSIGNED_BYTE, (GLvoid*)pixels);
        break;
      case 32:
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, width_, height_,
                          GL_BGRA, GL_UNSIGNED_BYTE, (GLvoid*)pixels);
        break;
      default:
        glBindTexture(GL_TEXTURE_2D, 0);
        return UNSUPPORTED_FILE_FORMAT;
      }

      glBindTexture(GL_TEXTURE_2D, 0);
      return SUCCEEDED;
    }


    ////////////////////////////////////////////////////////////////////////////////
    int create_subimage(wchar_t const* filename) {

      DWORD dw = GetFileAttributes(filename);

      if (dw==-1)
        return FILE_NOT_FOUND;

      CImage image;
      HRESULT res = image.Load(filename);

      if (res!=S_OK)
        return FAILED_TO_LOAD_IMAGE;

      GLsizei const w = image.GetWidth();
      GLsizei const h = image.GetHeight();

      if (w>width_ || h>height_)
        return FAILED_TO_LOAD_IMAGE;

      xcoord_ = GLfloat(w)/width_;
      ycoord_ = GLfloat(h)/height_;

      glBindTexture(GL_TEXTURE_2D, id_);

      int const bpp = image.GetBPP();
      int const pitch = image.GetPitch();
      char const* pixels = (char*)image.GetBits()+(pitch<0?pitch*(height_-1):0);

      switch (bpp) {
      case 3:
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h,
                        GL_BGR, GL_UNSIGNED_BYTE, pixels);
        break;
      case 4:
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h,
                        GL_BGRA, GL_UNSIGNED_BYTE, pixels);
        break;
      default:
        glBindTexture(GL_TEXTURE_2D, 0);
        return UNSUPPORTED_FILE_FORMAT;
      }

      glBindTexture(GL_TEXTURE_2D, 0);
      return SUCCEEDED;
    }


    ////////////////////////////////////////////////////////////////////////////////
    int create(GLuint width, GLuint height, GLenum wrap_s, GLenum wrap_t) {

      width_  = width;
      height_ = height;

      GLsizei w_pot = 1;
      while (w_pot<width_) w_pot *= 2;

      GLsizei h_pot = 1;
      while (h_pot<height_) h_pot *= 2;

      xcoord_ = GLfloat(width_)/w_pot;
      ycoord_ = GLfloat(height_)/h_pot;

      glBindTexture(GL_TEXTURE_2D, id_);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w_pot, h_pot, 0,
                   GL_BGRA, GL_UNSIGNED_BYTE, 0);
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width_, height_,
                      GL_BGRA, GL_UNSIGNED_BYTE, 0);
      glBindTexture(GL_TEXTURE_2D, 0);
      return SUCCEEDED;
    }


#ifdef SOCIAIRUM_PROJECT_USES_OPENCV
    ////////////////////////////////////////////////////////////////////////////////
    int create(IplImage* image, GLenum wrap_s, GLenum wrap_t) {

      assert(image!=0);

      width_  = image->width;
      height_ = image->height;

      GLsizei w_pot = 1;
      while (w_pot<width_) w_pot *= 2;

      GLsizei h_pot = 1;
      while (h_pot<height_) h_pot *= 2;

      xcoord_ = GLfloat(width_)/w_pot;
      ycoord_ = GLfloat(height_)/h_pot;

      glBindTexture(GL_TEXTURE_2D, id_);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

      int const nChannels = image->nChannels;
      GLvoid* pixels = (GLvoid*)image->imageData;

      switch (nChannels) {
      case 3:
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w_pot, h_pot, 0,
                     GL_BGR, GL_UNSIGNED_BYTE, 0);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width_, height_,
                        GL_BGR, GL_UNSIGNED_BYTE, pixels);
        break;
      case 4:
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w_pot, h_pot, 0,
                     GL_BGRA, GL_UNSIGNED_BYTE, 0);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width_, height_,
                        GL_BGRA, GL_UNSIGNED_BYTE, pixels);
        break;
      default:
        glBindTexture(GL_TEXTURE_2D, 0);
        return UNSUPPORTED_FILE_FORMAT;
      }

      glBindTexture(GL_TEXTURE_2D, 0);
      return SUCCEEDED;
    }


    ////////////////////////////////////////////////////////////////////////////////
    int create_mipmap(IplImage* image, GLenum wrap_s, GLenum wrap_t) {

      assert(image!=0);

      width_  = image->width;
      height_ = image->height;

      xcoord_ = 1.0f;
      ycoord_ = 1.0f;

      glBindTexture(GL_TEXTURE_2D, id_);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

      int const nChannels = image->nChannels;
      GLvoid* pixels = (GLvoid*)image->imageData;

      switch (nChannels) {
      case 3:
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, width_, height_,
                          GL_BGR, GL_UNSIGNED_BYTE, pixels);
        break;
      case 4:
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, width_, height_,
                          GL_BGRA, GL_UNSIGNED_BYTE, pixels);
        break;
      default:
        glBindTexture(GL_TEXTURE_2D, 0);
        return UNSUPPORTED_FILE_FORMAT;
      }

      glBindTexture(GL_TEXTURE_2D, 0);
      return SUCCEEDED;
    }


    ////////////////////////////////////////////////////////////////////////////////
    int create_subimage(IplImage* image) {

      assert(image!=0);

      GLsizei const w = image->width;
      GLsizei const h = image->height;

      if (w>width_ || h>height_)
        return FAILED_TO_LOAD_IMAGE;

      xcoord_ = GLfloat(w)/width_;
      ycoord_ = GLfloat(h)/height_;

      glBindTexture(GL_TEXTURE_2D, id_);

      int const nChannels = image->nChannels;
      GLvoid* pixels = (GLvoid*)image->imageData;

      switch (nChannels) {
      case 3:
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h,
                        GL_BGR, GL_UNSIGNED_BYTE, pixels);
        break;
      case 4:
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h,
                        GL_BGRA, GL_UNSIGNED_BYTE, pixels);
        break;
      default:
        glBindTexture(GL_TEXTURE_2D, 0);
        return UNSUPPORTED_FILE_FORMAT;
      }

      glBindTexture(GL_TEXTURE_2D, 0);
      return SUCCEEDED;
    }
#endif


    ////////////////////////////////////////////////////////////////////////////////
    GLuint get(void) const {
      return id_;
    }

    GLsizei width(void) const {
      return width_;
    }

    GLsizei height(void) const {
      return height_;
    }

    GLfloat xcoord(void) const {
      return xcoord_;
    }

    GLfloat ycoord(void) const {
      return ycoord_;
    }

  private:
    GLuint id_;
    GLsizei width_;
    GLsizei height_;
    GLfloat xcoord_;
    GLfloat ycoord_;
  };


  ////////////////////////////////////////////////////////////////////////////////
  // Factory function of Texture.
  shared_ptr<Texture> Texture::create(void) {
    return shared_ptr<Texture>(new TextureImpl);
  }

} // The end of the namespace "hashimoto_ut"

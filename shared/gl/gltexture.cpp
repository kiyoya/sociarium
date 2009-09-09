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
#ifdef _MSC_VER
#include <atlimage.h>
#endif
#ifdef __APPLE__
#include <ApplicationServices/ApplicationServices.h>
#include <OpenGL/glu.h>
#include <OpenGL/glext.h>
#else
#include <GL/glu.h>
#include <GL/glext.h>
#endif
#include "gltexture.h"
#include "../win32api.h"

#ifdef _MSC_VER
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "opengl32.lib")

#ifdef SOCIAIRUM_PROJECT_USES_OPENCV
#pragma comment(lib, "cv.lib")
#pragma comment(lib, "cxcore.lib")
#pragma comment(lib, "highgui.lib")
#endif
#endif

namespace hashimoto_ut {

  using std::tr1::shared_ptr;

  class GLTextureImpl : public GLTexture {
  public:

    ////////////////////////////////////////////////////////////////////////////////
    GLTextureImpl(void) :
    width_(0), height_(0), xcoord_(0.0f), ycoord_(0.0f) {

      glGenTextures(1, &id_);
      GLenum err = glGetError();
      assert(err!=GL_INVALID_VALUE);
      assert(err!=GL_INVALID_OPERATION);
    }


    ////////////////////////////////////////////////////////////////////////////////
    ~GLTextureImpl() {
      glDeleteTextures(1, &id_);
    }


    ////////////////////////////////////////////////////////////////////////////////
    int create(
      wchar_t const* filename,
      GLenum wrap_s, GLenum wrap_t) {

#ifdef __APPLE__
      CFStringRef path = CFStringCreateWithCString(kCFAllocatorDefault, wcs2mbcs(filename, wcslen(filename)).c_str(), kCFStringEncodingUTF8);
      CFURLRef url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, path, kCFURLPOSIXPathStyle, false);
      CGImageSourceRef imageRef = CGImageSourceCreateWithURL(url, NULL);
      CFRelease(url);
      CFRelease(path);

      switch (CGImageSourceGetStatus(imageRef))
      {
  		case kCGImageStatusComplete:
  			break;
      case kCGImageStatusUnknownType:
        CFRelease(imageRef);
        return UNSUPPORTED_FILE_FORMAT;
	  	default:
        CFRelease(imageRef);
        return FAILED_TO_LOAD_IMAGE;
      }

      CGImageRef image = CGImageSourceCreateImageAtIndex(imageRef, 0, NULL);

      width_ = CGImageGetWidth(image);
      height_ = CGImageGetHeight(image);
#elif _MSC_VER
      DWORD dw = GetFileAttributes(filename);

      if (dw==-1)
        return FILE_NOT_FOUND;

      CImage image;
      HRESULT res = image.Load(filename);

      if (res!=S_OK)
        return FAILED_TO_LOAD_IMAGE;

      width_  = image.GetWidth();
      height_ = image.GetHeight();
#endif

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

#ifdef __APPLE__
      GLubyte * pixels = (GLubyte *)malloc(width_ * height_ * 4);
      CGContextRef context = CGBitmapContextCreate(pixels, width_, height_, 8, width_ * 4, CGImageGetColorSpace(image), kCGImageAlphaPremultipliedLast);
      CGContextDrawImage(context, CGRectMake(0.0f, 0.0f, (CGFloat)width_, (CGFloat)height_), image);
      CGContextRelease(context);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
      free(pixels);
      CGImageRelease(image);
      CFRelease(imageRef);
#elif _MSC_VER
      int const bpp = image.GetBPP();
      int const pitch = image.GetPitch();
      char const* pixels = (char*)image.GetBits()+(pitch<0?pitch*(height_-1):0);

      switch (bpp) {
      case 24:
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(
          GL_TEXTURE_2D, 0, GL_RGB, w_pot, h_pot, 0,
          GL_BGR, GL_UNSIGNED_BYTE, 0);
        glTexSubImage2D(
          GL_TEXTURE_2D, 0, 0, 0, width_, height_,
          GL_BGR, GL_UNSIGNED_BYTE, (GLvoid*)pixels);
        break;
      case 32:
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        glTexImage2D(
          GL_TEXTURE_2D, 0, GL_RGBA, width_, height_, 0,
          GL_BGRA, GL_UNSIGNED_BYTE, 0);
        glTexSubImage2D(
          GL_TEXTURE_2D, 0, 0, 0, width_, height_,
          GL_BGRA, GL_UNSIGNED_BYTE, (GLvoid*)pixels);
        break;
      default:
        glBindTexture(GL_TEXTURE_2D, 0);
        return UNSUPPORTED_FILE_FORMAT;
      }
#endif

      glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      glBindTexture(GL_TEXTURE_2D, 0);
      return SUCCEEDED;
    }


    ////////////////////////////////////////////////////////////////////////////////
    int create_mipmap(
      wchar_t const* filename,
      GLenum wrap_s, GLenum wrap_t) {

#ifdef __APPLE__
      CFStringRef path = CFStringCreateWithCString(kCFAllocatorDefault, wcs2mbcs(filename, wcslen(filename)).c_str(), kCFStringEncodingUTF8);
      CFURLRef url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, path, kCFURLPOSIXPathStyle, false);
      CGImageSourceRef imageRef = CGImageSourceCreateWithURL(url, NULL);
      CFRelease(url);
      CFRelease(path);

      switch (CGImageSourceGetStatus(imageRef))
      {
        case kCGImageStatusComplete:
          break;
        case kCGImageStatusUnknownType:
          CFRelease(imageRef);
          return UNSUPPORTED_FILE_FORMAT;
        default:
          CFRelease(imageRef);
          return FAILED_TO_LOAD_IMAGE;
      }

      CGImageRef image = CGImageSourceCreateImageAtIndex(imageRef, 0, NULL);
    
      width_ = CGImageGetWidth(image);
      height_ = CGImageGetHeight(image);
#elif _MSC_VER
      DWORD dw = GetFileAttributes(filename);

      if (dw==-1)
        return FILE_NOT_FOUND;

      CImage image;
      HRESULT res = image.Load(filename);

      if (res!=S_OK)
        return FAILED_TO_LOAD_IMAGE;

      width_  = image.GetWidth();
      height_ = image.GetHeight();
#endif

      xcoord_ = 1.0f;
      ycoord_ = 1.0f;

      glBindTexture(GL_TEXTURE_2D, id_);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

#ifdef __APPLE__
      GLubyte * pixels = (GLubyte *)malloc(width_ * height_ * 4);
      CGContextRef context = CGBitmapContextCreate(pixels, width_, height_, 8, width_ * 4, CGImageGetColorSpace(image), kCGImageAlphaPremultipliedLast);
      CGContextDrawImage(context, CGRectMake(0.0f, 0.0f, (CGFloat)width_, (CGFloat)height_), image);
      CGContextRelease(context);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
      free(pixels);
      CGImageRelease(image);
      CFRelease(imageRef);
#elif _MSC_VER
      int const bpp = image.GetBPP();
      int const pitch = image.GetPitch();
      char const* pixels = (char*)image.GetBits()+(pitch<0?pitch*(height_-1):0);

      switch (bpp) {
      case 24:
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        gluBuild2DMipmaps(
          GL_TEXTURE_2D, GL_RGB, width_, height_,
          GL_BGR, GL_UNSIGNED_BYTE, (GLvoid*)pixels);
        break;
      case 32:
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        gluBuild2DMipmaps(
          GL_TEXTURE_2D, GL_RGBA, width_, height_,
          GL_BGRA, GL_UNSIGNED_BYTE, (GLvoid*)pixels);
        break;
      default:
        glBindTexture(GL_TEXTURE_2D, 0);
        return UNSUPPORTED_FILE_FORMAT;
      }
#endif

      glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      glBindTexture(GL_TEXTURE_2D, 0);
      return SUCCEEDED;
    }


#ifdef SOCIAIRUM_PROJECT_USES_OPENCV
    ////////////////////////////////////////////////////////////////////////////////
    int create(
      IplImage* image,
      GLenum wrap_s, GLenum wrap_t) {

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

      switch (nChannels) {
      case 3:
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(
          GL_TEXTURE_2D, 0, GL_RGB, w_pot, h_pot, 0,
          GL_BGR, GL_UNSIGNED_BYTE, 0);
        glTexSubImage2D(
          GL_TEXTURE_2D, 0, 0, 0, width_, height_,
          GL_BGR, GL_UNSIGNED_BYTE, (GLvoid*)image->imageData);
        break;
      case 4:
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        glTexImage2D(
          GL_TEXTURE_2D, 0, GL_RGBA, w_pot, h_pot, 0,
          GL_BGRA, GL_UNSIGNED_BYTE, 0);
        glTexSubImage2D(
          GL_TEXTURE_2D, 0, 0, 0, width_, height_,
          GL_BGRA, GL_UNSIGNED_BYTE, (GLvoid*)image->imageData);
        break;
      default:
        glBindTexture(GL_TEXTURE_2D, 0);
        return UNSUPPORTED_FILE_FORMAT;
      }

      glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      glBindTexture(GL_TEXTURE_2D, 0);
      return SUCCEEDED;
    }


    ////////////////////////////////////////////////////////////////////////////////
    int create_mipmap(
      IplImage* image,
      GLenum wrap_s, GLenum wrap_t) {

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

      switch (nChannels) {
      case 3:
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        gluBuild2DMipmaps(
          GL_TEXTURE_2D, GL_RGB, width_, height_,
          GL_BGR, GL_UNSIGNED_BYTE, (GLvoid*)image->imageData);
        break;
      case 4:
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        gluBuild2DMipmaps(
          GL_TEXTURE_2D, GL_RGBA, width_, height_,
          GL_BGRA, GL_UNSIGNED_BYTE, (GLvoid*)image->imageData);
        break;
      default:
        glBindTexture(GL_TEXTURE_2D, 0);
        return UNSUPPORTED_FILE_FORMAT;
      }

      glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      glBindTexture(GL_TEXTURE_2D, 0);
      return SUCCEEDED;
    }


    ////////////////////////////////////////////////////////////////////////////////
    int create_subimage(IplImage* image) {

      if (image==0)
        return UNSUPPORTED_FILE_FORMAT;

      if (image->width>width_ || image->height>height_)
        return FAILED_TO_LOAD_IMAGE;

      glBindTexture(GL_TEXTURE_2D, id_);

      int const nChannels = image->nChannels;

      switch (nChannels) {
      case 3:
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexSubImage2D(
          GL_TEXTURE_2D, 0, 0, 0, image->width, image->height,
          GL_BGR, GL_UNSIGNED_BYTE, (GLvoid*)image->imageData);
        break;
      case 4:
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        glTexSubImage2D(
          GL_TEXTURE_2D, 0, 0, 0, image->width, image->height,
          GL_BGRA, GL_UNSIGNED_BYTE, (GLvoid*)image->imageData);
        break;
      default:
        glBindTexture(GL_TEXTURE_2D, 0);
        return UNSUPPORTED_FILE_FORMAT;
      }

      glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
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
  // Factory function of GLTexture.
  shared_ptr<GLTexture> GLTexture::create(void) {
    return shared_ptr<GLTexture>(new GLTextureImpl);
  }

} // The end of the namespace "hashimoto_ut"

// HASHIMOTO, Yasuhiro (E-mail: hy @ sys.t.u-tokyo.ac.jp)
// update: 2009/02/28

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
#endif

namespace hashimoto_ut {

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  GLTexture::GLTexture(void) {
    glGenTextures(1, &id_);
    GLenum err = glGetError();
    assert(err!=GL_INVALID_VALUE);
    assert(err!=GL_INVALID_OPERATION);
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  GLTexture::~GLTexture() {
    glDeleteTextures(1, &id_);
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  int GLTexture::create(wchar_t const* filename, GLenum wrap) {
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
    if (dw==-1) return FILE_NOT_FOUND;
    CImage image;
    HRESULT res = image.Load(filename);
    if (res!=S_OK) return FAILED_TO_LOAD_IMAGE;
    width_  = image.GetWidth();
    height_ = image.GetHeight();
#endif

    glBindTexture(GL_TEXTURE_2D, id_);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
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
    case 24: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_, height_, 0, GL_BGR, GL_UNSIGNED_BYTE, (GLvoid*)pixels); break;
    case 32: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_, height_, 0, GL_BGRA, GL_UNSIGNED_BYTE, (GLvoid*)pixels); break;
    default: return UNSUPPORTED_FILE_FORMAT;
    }
#endif

    return SUCCEEDED;
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  int GLTexture::create_mipmap(wchar_t const* filename, GLenum wrap) {
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
    if (dw==-1) return FILE_NOT_FOUND;
    CImage image;
    HRESULT res = image.Load(filename);
    if (res!=S_OK) return FAILED_TO_LOAD_IMAGE;
    width_  = image.GetWidth();
    height_ = image.GetHeight();
#endif

    glBindTexture(GL_TEXTURE_2D, id_);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
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
    case 24: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_, height_, 0, GL_BGR, GL_UNSIGNED_BYTE, (GLvoid*)pixels); break;
    case 32: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_, height_, 0, GL_BGRA, GL_UNSIGNED_BYTE, (GLvoid*)pixels); break;
    default: return UNSUPPORTED_FILE_FORMAT;
    }
#endif

    return SUCCEEDED;
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  GLuint GLTexture::get(void) const { return id_; }
  GLsizei GLTexture::width(void) const { return width_; }
  GLsizei GLTexture::height(void) const { return height_; }

} // The end of the namespace "hashimoto_ut"

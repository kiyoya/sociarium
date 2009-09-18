// s.o.c.i.a.r.i.u.m: cvframe.cpp
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

#ifdef _MSC_VER
#include <windows.h>
#endif
#ifdef __APPLE__
#include <GL/glew.h>
#else
#include <gl/glew.h>
#endif

#include "../shared/gl/gltexture.h"
#ifdef SOCIAIRUM_PROJECT_USES_OPENCV

#ifdef _MSC_VER
#include <memory>
#else
#include <tr1/memory>
#endif
#include <shlwapi.h>
#include <highgui.h>

#include "common.h"
#include "cvframe.h"
#include "../shared/fps.h"
#include "../shared/msgbox.h"
#include "../shared/win32api.h"

#ifdef _MSC_VER
#pragma comment(lib, "cxcore.lib")
#pragma comment(lib, "cv.lib")
#pragma comment(lib, "cvaux.lib")
#pragma comment(lib, "highgui.lib")
#endif

namespace hashimoto_ut {

  using std::wstring;
  using std::tr1::shared_ptr;

  using namespace sociarium_project_common;

  namespace {

    wchar_t const* ERROR_FAILED_TO_CREATE_CAMERA_CAPTURE
      = L"カメラキャプチャに失敗しました";
    wchar_t const* ERROR_FAILED_TO_CREATE_FILE_CAPTURE
      = L"ファイルキャプチャに失敗しました";
    wchar_t const* ERROR_FAILED_TO_LOAD_MASKING_IMAGE
      = L"イメージのロードに失敗しました";
    wchar_t const* ERROR_PATH_FILE_EXISTS
      = L"ファイルが見つかりませんでした";


    class CVFrameImpl : public CVFrame {
    public:

      enum {
        UNDEFINED = 0,
        CAMERA,
        MOVIE
      };

      ////////////////////////////////////////////////////////////////////////////////
      CVFrameImpl(HDC hdc, HGLRC hrc) :
      dc_(hdc), rc_(hrc), mode_(UNDEFINED),
      capture_(0), masking_image_(0), chroma_key_background_(0),
      texture_(GLTexture::create()) {}


      ////////////////////////////////////////////////////////////////////////////////
      ~CVFrameImpl() {
        if (capture_) cvReleaseCapture(&capture_);
        if (masking_image_) cvReleaseImage(&masking_image_);
        if (chroma_key_background_) cvReleaseImage(&chroma_key_background_);
      }


      ////////////////////////////////////////////////////////////////////////////////
      void terminate(void) {
        if (wglGetCurrentContext() && wglMakeCurrent(0, 0)==FALSE)
          show_last_error(L"CVFrame:~CVFrame/wglMakeCurrent");

        if (wglDeleteContext(rc_)==FALSE)
          show_last_error(L"CVFrame::~CVFrame/wglDeleteContext");
      }


      ////////////////////////////////////////////////////////////////////////////////
      void set_camera(void) {
        if ((capture_=cvCreateCameraCapture(0))==0) {
          message_box(
            get_window_handle(),
            MessageType::CRITICAL,
            APPLICATION_TITLE,
            ERROR_FAILED_TO_CREATE_CAMERA_CAPTURE);
          return;
        }

        fps_keeper_.set(30);
        mode_ = CAMERA;
      }


      ////////////////////////////////////////////////////////////////////////////////
      void set_movie(char const* movie_filename) {
        if (PathFileExists(
          mbcs2wcs(movie_filename, strlen(movie_filename)).c_str())==NULL) {
          message_box(
            get_window_handle(),
            MessageType::CRITICAL,
            APPLICATION_TITLE,
            L"%s: %s",
            ERROR_PATH_FILE_EXISTS,
            mbcs2wcs(movie_filename, strlen(movie_filename)).c_str());
          return;
        }

        capture_ = cvCreateFileCapture(movie_filename);

        if (capture_==0) {
          message_box(
            get_window_handle(),
            MessageType::CRITICAL,
            APPLICATION_TITLE,
            L"%s: %s",
            ERROR_FAILED_TO_CREATE_FILE_CAPTURE,
            mbcs2wcs(movie_filename, strlen(movie_filename)).c_str());
          return;
        }

        double const fps = cvGetCaptureProperty(capture_, CV_CAP_PROP_FPS);
        fps_keeper_.set(unsigned long(fps));
        mode_ = MOVIE;
      }


      ////////////////////////////////////////////////////////////////////////////////
      bool set_masking_image(char const* image_filename) {
        IplImage* image = cvLoadImage(image_filename, CV_LOAD_IMAGE_GRAYSCALE);

        if (image==0) {
          message_box(
            get_window_handle(),
            MessageType::CRITICAL,
            APPLICATION_TITLE,
            L"%s: %s",
            ERROR_FAILED_TO_LOAD_MASKING_IMAGE,
            mbcs2wcs(image_filename, strlen(image_filename)).c_str());
          return false;
        }

        int const w = image->width;
        int const h = image->height;
        masking_image_ = cvCreateImage(cvSize(w, h), image->depth, 1);
        cvThreshold(image, masking_image_, 128, 255, CV_THRESH_BINARY);
        cvFlip(masking_image_, NULL, 0);
        if (image) cvReleaseImage(&image);
        return true;
      }


      ////////////////////////////////////////////////////////////////////////////////
      bool set_chroma_key_background_image(char const* image_filename) {
        chroma_key_background_ = cvLoadImage(image_filename);

        if (chroma_key_background_==0) {
          message_box(
            get_window_handle(),
            MessageType::CRITICAL,
            APPLICATION_TITLE,
            L"%s: %s",
            ERROR_FAILED_TO_LOAD_MASKING_IMAGE,
            mbcs2wcs(image_filename, strlen(image_filename)).c_str());
          return false;
        }

        cvFlip(chroma_key_background_, NULL, 0);
        return true;
      }


      ////////////////////////////////////////////////////////////////////////////////
      GLTexture const* get_texture(void) const {
        return texture_.get();
      }


      ////////////////////////////////////////////////////////////////////////////////
      void operator()(void) {

        if (capture_==0) return;

        if (dc_==0) {
          show_last_error(L"CVFrame::operator()/dc_==0");
          return;
        }

        if (rc_==0) {
          show_last_error(L"CVFrame::operator()/rc_==0");
          return;
        }

        if (wglMakeCurrent(dc_, rc_)==FALSE) {
          show_last_error(L"CVFrame::operator()/wglMakeCurrent");
          return;
        }
        /*
         * Don't forget to call 'terminate()' when leave this function.
         * 'terminate()' function release and delete the current rendering context.
         */

        IplImage* frame = cvQueryFrame(capture_);
        IplImage* frameBGRA = 0;

        if (frame) {
          if (mode_==CAMERA)
            texture_->create(frame, GL_CLAMP_TO_EDGE_EXT, GL_CLAMP_TO_EDGE_EXT);
          else if (mode_==MOVIE) {
            cvFlip(frame, NULL, 0);
            frameBGRA = cvCreateImage(cvSize(frame->width, frame->height), frame->depth, 4);
            cvCvtColor(frame, frameBGRA, CV_BGR2BGRA);
            texture_->create(frameBGRA, GL_CLAMP_TO_EDGE_EXT, GL_CLAMP_TO_EDGE_EXT);
          }
        } else {
          return terminate();
        }

        int const w = frame->width;
        int const h = frame->height;
        int const depth = frame->depth;

        IplImage* diff   = 0;
        IplImage* gray   = 0;

        if (chroma_key_background_) {
          IplImage* diff = cvCreateImage(cvSize(w, h), depth, frame->nChannels);
          IplImage* gray = cvCreateImage(cvSize(w, h), depth, 1);
          masking_image_ = cvCreateImage(cvSize(w, h), depth, 1);
        }

        for (;;) {

          if (cancel_check()) break;

          frame = cvQueryFrame(capture_);

          if (frame) {

            write_lock();
            cvFlip(frame, NULL, 0);

            // Camera
            if (mode_==CAMERA)
              texture_->create_subimage(frame);

            // Movie
            else if (mode_==MOVIE) {

              if (chroma_key_background_) {
                // Chroma Key Movie.
                static const int threshold_ = 20;
                cvAbsDiff(frame, chroma_key_background_, diff);
                cvCvtColor(diff, gray, CV_BGR2GRAY);
                cvThreshold(gray, masking_image_, threshold_, 255, CV_THRESH_BINARY);
              }

              int const number_of_pixels = frame->width*frame->height;

              if (masking_image_==0)
                cvCvtColor(frame, frameBGRA, CV_BGR2BGRA);
              else {
                for (int i=0; i<number_of_pixels; ++i) {
                  if (masking_image_->imageData[i]==0) {
                    frameBGRA->imageData[4*i] = 0;
                    frameBGRA->imageData[4*i+1] = 0;
                    frameBGRA->imageData[4*i+2] = 0;
                    frameBGRA->imageData[4*i+3] = 0;
                  } else {
                    frameBGRA->imageData[4*i] = frame->imageData[3*i];
                    frameBGRA->imageData[4*i+1] = frame->imageData[3*i+1];
                    frameBGRA->imageData[4*i+2] = frame->imageData[3*i+2];
                    frameBGRA->imageData[4*i+3] = char(255);
                  }
                }
              }

              texture_->create_subimage(frameBGRA);
              //texture_->create_mipmap(frameBGRA, GL_CLAMP_TO_EDGE_EXT, GL_CLAMP_TO_EDGE_EXT);
            }

            // Never reach
            else assert(0 && "never reach");

            write_unlock();
          }

          fps_keeper_.wait();
        }

        if (diff) cvReleaseImage(&diff);
        if (gray) cvReleaseImage(&gray);

        cvReleaseImage(&frameBGRA);
        terminate();
      }

    private:
      HDC dc_;
      HGLRC rc_;
      int mode_;
      CvCapture* capture_;
      IplImage* masking_image_;
      IplImage* chroma_key_background_;
      shared_ptr<GLTexture> texture_;
      FPSKeeper fps_keeper_;
      float threshold_;
    };


    ////////////////////////////////////////////////////////////////////////////////
    shared_ptr<CVFrame> cvframe_current;
    shared_ptr<CVFrame> cvframe_new;
    boost::thread thread_manager;

  } // The end of the anonymous namespace


  namespace sociarium_project_cvframe {

    ////////////////////////////////////////////////////////////////////////////////
    shared_ptr<CVFrame> create(HDC hdc, HGLRC hrc) {
      cvframe_new.reset(new CVFrameImpl(hdc, hrc));
      return cvframe_new;
    }

    ////////////////////////////////////////////////////////////////////////////////
    shared_ptr<CVFrame> get_current_frame(void) {
      return cvframe_current;
    }

    ////////////////////////////////////////////////////////////////////////////////
    void invoke(shared_ptr<CVFrame> cvframe) {
      terminate();
      cvframe_current = cvframe;
      boost::thread(boost::ref(*cvframe_current)).swap(thread_manager);
    }

    ////////////////////////////////////////////////////////////////////////////////
    void terminate(void) {
      if (cvframe_current) cvframe_current->cancel();
      if (thread_manager.joinable()) thread_manager.join();
    }

    ////////////////////////////////////////////////////////////////////////////////
    void join(void) {
      thread_manager.join();
    }

    ////////////////////////////////////////////////////////////////////////////////
    bool joinable(void) {
      return thread_manager.joinable();
    }

  } // The end of the namespace "sociarium_project_cvframe"

} // The end of the namespace "hashimoto_ut"

#endif // SOCIAIRUM_PROJECT_USES_OPENCV

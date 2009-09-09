// s.o.c.i.a.r.i.u.m: thread/detail/read_file.cpp
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

#include <deque>
#include <sstream>
#include <iostream>
#include <boost/format.hpp>
#include <mlang.h>
#include "read_file.h"
#include "../../common.h"
#include "../../language.h"
#include "../../thread_manager.h"
#include "../../module/graph_creation.h"
#include "../../../shared/general.h"
#include "../../../shared/thread.h"
#include "../../../shared/msgbox.h"
#include "../../../shared/win32api.h"

namespace hashimoto_ut {

  using std::vector;
  using std::deque;
  using std::string;
  using std::wstring;
  using std::getline;
  using std::pair;
  using std::make_pair;
  using std::stringstream;
  using std::tr1::unordered_map;

  using namespace sociarium_project_common;
  using namespace sociarium_project_language;

  namespace sociarium_project_thread_detail_read_file {

    namespace {

      ////////////////////////////////////////////////////////////////////////////////
      char const param_symbol = '@';
      char const comment_out_symbol = '#';

      /*
       * In this case, the following sentence
       *
       *   @abc = 123 # xyz
       *
       * is interpreted as below:
       *   - The name of parameter is "abc".
       *   - The value of parameter is "123".
       *   - The blank is ignored.
       *   - The right part of '#' is ignored.
       */

      ////////////////////////////////////////////////////////////////////////////////
      bool convert2sjis(char const* filename, stringstream& ss, wstring& status) {

        CoInitialize(NULL);

        wstring const filename_w = mbcs2wcs(filename, strlen(filename));
        HANDLE hfile = CreateFile(
          filename_w.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
          OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        DWORD fsize = GetFileSize(hfile, NULL);

        if (fsize<1) {
          CoUninitialize();
          return false;
        }

        HGLOBAL hsrc = GlobalAlloc(GMEM_MOVEABLE, fsize+1);
        char* buf = (char*)GlobalLock(hsrc);
        ReadFile(hfile, buf, fsize, &fsize, NULL);
        buf[fsize-1] = '\0';
        GlobalUnlock(hsrc);
        CloseHandle(hfile);

        string text = buf;

        IMultiLanguage2* ml;

        if (FAILED(CoCreateInstance(
          CLSID_CMultiLanguage, NULL, CLSCTX_INPROC_SERVER,
          IID_IMultiLanguage2, (void**)&ml))) {
          show_last_error(L"convert2sjis");
          CoUninitialize();
          return false;
        }

        // Generate an IStream object for reading.
        IStream* is_src;
        CreateStreamOnHGlobal(hsrc, true, &is_src);
        // Judge character encoding.
        status = get_message(Message::CHECKING_TEXT_ENCODING);
        int encsize = 1;
        DetectEncodingInfo encoding;

        if (FAILED(ml->DetectCodepageInIStream(
          MLDETECTCP_HTML, 0, is_src, &encoding, &encsize))) {
          message_box(
            get_window_handle(),
            MB_OK|MB_ICONERROR|MB_SYSTEMMODAL,
            APPLICATION_TITLE,
            L"%s [%s]",
            get_message(Message::UNKNOWN_CHARACTER_ENCODING),
            filename_w.c_str());
          is_src->Release();
          CoUninitialize();
          return false;
        }

        LARGE_INTEGER pos = { 0 };
        is_src->Seek(pos, STREAM_SEEK_SET, NULL);

        bool cnv = encoding.nCodePage!=932 // not Shift_JIS
          && encoding.nCodePage!=20127;    // not ASCII

        if (cnv) {
          // Convert @text into Shift_JIS encoding.
          status = get_message(Message::CONVERTING_INTO_SJIS_ENCODING);
          HGLOBAL hdst = GlobalAlloc(GMEM_MOVEABLE, 0);
          IStream* is_dst;
          CreateStreamOnHGlobal(hdst, true, &is_dst);
          DWORD mode = 0;
          UINT const destCodePage = 932;
          ml->ConvertStringInIStream(
            &mode, 0, NULL, encoding.nCodePage, destCodePage, is_src, is_dst);
          char const null_char = '\0';
          is_dst->Write(&null_char, sizeof(char), NULL);

          text = (char*)GlobalLock(hdst);

          GlobalUnlock(hdst);
          is_dst->Release();
        }

        ss << text;
        is_src->Release();
        CoUninitialize();
        return true;
      }

    } // The end of the anonymous namespace


    ////////////////////////////////////////////////////////////////////////////////
    // Remove the right part of @comment_out_symbol in a line.
    void remove_comment_part(string& text) {
      if (text.empty()) return;
      text = text.substr(0, text.find(comment_out_symbol));
    }


    ////////////////////////////////////////////////////////////////////////////////
    bool read_file(Thread* parent, char const* filename,
                   unordered_map<string, pair<string, int> >& params,
                   vector<pair<string, int> >& data) {

      using namespace sociarium_project_thread_manager;
      deque<wstring>& status = get_status(GRAPH_CREATION);

      int const num = number_of_lines(filename);
      if (num<1) return false;

      stringstream ss;

      if (!convert2sjis(filename, ss, status[0]))
        return false;

      string line;

      for (int i=1; getline(ss, line); ++i) {

        // **********  Catch a termination signal  **********
        if (parent->cancel_check()) {
          deque<wstring>(status.size()).swap(status);
          return false;
        }

        status[0]
          = (boost::wformat(L"%s: %d%%")
             %get_message(Message::READING_DATA_FILE)
             %int(100.0*i/num)).str();

        remove_comment_part(line);
        trim(line);

        if (!line.empty()) {
          if (line[0]==param_symbol) {
            size_t pos = line.find('=');
            if (pos==string::npos) {
              trim(line=line.substr(1));
              params.insert(make_pair(line, make_pair("", i)));
            } else {
              string value = line.substr(pos+1);
              line = line.substr(1, pos-1);
              trim(line); // key
              trim(value);
              params.insert(make_pair(line, make_pair(value, i)));
            }
          } else if (line[0]!=comment_out_symbol) {
            data.push_back(make_pair(line, i));
          }
        }
      }

      deque<wstring>(status.size()).swap(status);

      return true;
    }

  } // The end of the namespace "sociarium_project_thread_detail_read_file"

} // The end of the namespace "hashimoto_ut"

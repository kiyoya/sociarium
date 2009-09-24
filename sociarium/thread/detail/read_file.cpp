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
#include "../../menu_and_message.h"
#include "../../thread.h"
#include "../../module/graph_creation.h"
#include "../../../shared/msgbox.h"
#include "../../../shared/thread.h"
#include "../../../shared/util.h"
#include "../../../shared/win32api.h"

namespace hashimoto_ut {

  using std::vector;
  using std::deque;
  using std::string;
  using std::wstring;
  using std::getline;
  using std::pair;
  using std::make_pair;
  using std::wstringstream;
  using std::tr1::unordered_map;

  using namespace sociarium_project_common;
  using namespace sociarium_project_menu_and_message;

  namespace sociarium_project_thread_detail_read_file {

    namespace {

      ////////////////////////////////////////////////////////////////////////////////
      wchar_t const param_symbol = L'@';
      wchar_t const comment_out_symbol = L'#';

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
      bool convert2utf16(wchar_t const* filename, wstringstream& ss, wstring& status) {

        CoInitialize(NULL);

        HANDLE hfile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL,
                                  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

        DWORD byte = GetFileSize(hfile, NULL);

        if (byte==0) {
          CoUninitialize();
          return false;
        }

        IMultiLanguage2* ml;

        if (FAILED(CoCreateInstance(
          CLSID_CMultiLanguage, NULL, CLSCTX_INPROC_SERVER,
          IID_IMultiLanguage2, (void**)&ml))) {
          show_last_error(L"convert2utf16/CoCreateInstance");
          CoUninitialize();
          return false;
        }

        HGLOBAL hsrc = GlobalAlloc(GMEM_MOVEABLE, byte);

        char* buf = (char*)GlobalLock(hsrc);
        ReadFile(hfile, buf, byte, &byte, NULL);
        GlobalUnlock(hsrc);
        CloseHandle(hfile);

        // Generate an IStream object for reading.
        IStream* is_src;
        CreateStreamOnHGlobal(hsrc, true, &is_src);

        // Judge character encoding.
        status = get_message(Message::CHECKING_TEXT_ENCODING);

        int encsize = 1;
        DetectEncodingInfo encoding;

        if (FAILED(ml->DetectCodepageInIStream(
          MLDETECTCP_HTML, 0, is_src, &encoding, &encsize))) {
          message_box(get_window_handle(), mb_error, APPLICATION_TITLE,
                      L"%s [%s]", get_message(Message::UNKNOWN_CHARACTER_ENCODING),
                      filename);
          is_src->Release();
          CoUninitialize();
          return false;
        }

        LARGE_INTEGER pos = { 0 };
        wchar_t const null_char = L'\0';

        if (encoding.nCodePage==1200) { // is UTF-16
          // Read as it is.
          is_src->Seek(pos, STREAM_SEEK_END, NULL);
          is_src->Write(&null_char, sizeof(wchar_t), NULL);
          ss << (wchar_t*)GlobalLock(hsrc);

        } else {
          // Convert into UTF-16 encoding.
          status = get_message(Message::CONVERTING_INTO_UTF16_ENCODING);
          is_src->Seek(pos, STREAM_SEEK_SET, NULL);
          HGLOBAL hdst = GlobalAlloc(GMEM_MOVEABLE, 0);
          IStream* is_dst;
          CreateStreamOnHGlobal(hdst, true, &is_dst);
          DWORD mode = 0;
          UINT const destCodePage = 1200;
          ml->ConvertStringInIStream(
            &mode, 0, NULL, encoding.nCodePage, destCodePage, is_src, is_dst);
          is_dst->Write(&null_char, sizeof(wchar_t), NULL);
          ss << (wchar_t*)GlobalLock(hdst);
          GlobalUnlock(hdst);
          is_dst->Release();
        }

        is_src->Release();
        CoUninitialize();
        return true;
      }

    } // The end of the anonymous namespace


    ////////////////////////////////////////////////////////////////////////////////
    // Remove the right part of @comment_out_symbol in a line.
    void remove_comment_part(wstring& text) {
      if (text.empty()) return;
      text = text.substr(0, text.find(comment_out_symbol));
    }


    ////////////////////////////////////////////////////////////////////////////////
    bool read_file(Thread* parent, wchar_t const* filename,
                   unordered_map<wstring, pair<wstring, int> >& params,
                   vector<pair<wstring, int> >& data) {

      using namespace sociarium_project_thread;
      deque<wstring>& status = get_status(GRAPH_CREATION);

      status[0]
        = (boost::wformat(L"%s: 0%%")
           %get_message(Message::READING_DATA_FILE)).str();

      string const filename_mb = wcs2mbcs(filename, wcslen(filename));
      int const num = number_of_lines(filename_mb.c_str());
      if (num<1) return false;

      wstringstream ss;

      if (!convert2utf16(filename, ss, status[0]))
        return false;

      wstring line;

      for (int i=1; getline(ss, line); ++i) {

        // **********  Catch a termination signal  **********
        if (parent && parent->cancel_check()) {
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
          if (i==1 && line[0]==0xFEFF || line[0]==0xFFFE)
            line = line.substr(1); // omit BOM.
          if (line[0]==param_symbol) {
            size_t pos = line.find(L'=');
            if (pos==wstring::npos) {
              trim(line=line.substr(1));
              params.insert(make_pair(line, make_pair(L"", i)));
            } else {
              wstring value = line.substr(pos+1);
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

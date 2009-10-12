// win32api.cpp
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

#include <boost/shared_array.hpp>
#include "win32api.h"

namespace hashimoto_ut {

  using std::vector;
  using std::string;
  using std::wstring;
  using boost::shared_array;

  namespace {
    enum { MAX_PATHLENGTH=65535 };
  };

#ifdef _MSC_VER

  ////////////////////////////////////////////////////////////////////////////////
  wstring GetFileName::for_read(
    HWND hwnd,
    wchar_t const* filename,
    wchar_t const* title,
    wchar_t const* filter,
    wchar_t const* path) {

    wchar_t buf[MAX_PATHLENGTH];
    wcscpy_s(buf, filename);
    OPENFILENAME ofn;
    memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize     = sizeof(ofn);
    ofn.hwndOwner       = hwnd;
    ofn.lpstrFile       = buf;
    ofn.nMaxFile        = MAX_PATHLENGTH;
    ofn.lpstrFilter     = filter;
    ofn.nFilterIndex    = 1;
    ofn.lpstrFileTitle  = NULL;
    ofn.nMaxFileTitle   = 0;
    ofn.lpstrInitialDir = path;
    ofn.lpstrTitle      = title;
    ofn.Flags           =
      OFN_EXPLORER|OFN_PATHMUSTEXIST|OFN_READONLY|OFN_HIDEREADONLY|OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn)) {
      PathSplitter sp(ofn.lpstrFile);
      dir_ = sp.dir();
      filename_ = sp.filename();
      filename_ += sp.ext();
      return wstring(buf);
    }

    return wstring(L"");
  }


  ////////////////////////////////////////////////////////////////////////////////
  vector<wstring> GetFileName::for_read_multi(
    HWND hwnd,
    wchar_t const* title,
    wchar_t const* filter,
    wchar_t const* path) {

    wchar_t buf[MAX_PATHLENGTH] = TEXT("");
    OPENFILENAME ofn;
    memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize     = sizeof(ofn);
    ofn.hwndOwner       = hwnd;
    ofn.lpstrFile       = buf;
    ofn.nMaxFile        = MAX_PATHLENGTH;
    ofn.lpstrFilter     = filter;
    ofn.nFilterIndex    = 1;
    ofn.lpstrFileTitle  = NULL;
    ofn.nMaxFileTitle   = 0;
    ofn.lpstrInitialDir = path;
    ofn.lpstrTitle      = title;
    ofn.Flags           =
      OFN_EXPLORER|OFN_PATHMUSTEXIST|OFN_READONLY|OFN_HIDEREADONLY|OFN_FILEMUSTEXIST|
        OFN_ALLOWMULTISELECT;

    if (GetOpenFileName(&ofn)) {
      vector<wstring> filelist;
      dir_ = wstring(ofn.lpstrFile)+L"\\";
      wchar_t* c = &buf[ofn.nFileOffset];
      wstring filename;
      while (1) {
        if (*c==0) {
          filelist.push_back(filename);
          filename = L"";
          if (*(++c)==0) break;
        } else {
          filename += *c;
          ++c;
        }
      }
      return filelist;
    }

    return vector<wstring>();
  }


  ////////////////////////////////////////////////////////////////////////////////
  wstring GetFileName::for_write(
    HWND hwnd,
    wchar_t const* filename,
    wchar_t const* title,
    wchar_t const* filter,
    wchar_t const* path) {

    wchar_t buf[MAX_PATHLENGTH];
    wcscpy_s(buf, filename);
    OPENFILENAME ofn;
    memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize     = sizeof(ofn);
    ofn.hwndOwner       = hwnd;
    ofn.lpstrFile       = buf;
    ofn.nMaxFile        = MAX_PATHLENGTH;
    ofn.lpstrFilter     = filter;
    ofn.nFilterIndex    = 1;
    ofn.lpstrFileTitle  = NULL;
    ofn.nMaxFileTitle   = 0;
    ofn.lpstrInitialDir = path;
    ofn.lpstrTitle      = title;
    ofn.Flags           = OFN_EXPLORER|OFN_PATHMUSTEXIST|OFN_OVERWRITEPROMPT;

    if (GetSaveFileName(&ofn)) {
      PathSplitter sp(ofn.lpstrFile);
      dir_ = sp.dir();
      filename_ = sp.filename();
      filename_ += sp.ext();
      return wstring(buf);
    }

    return wstring(L"");
  }

  ////////////////////////////////////////////////////////////////////////////////
  wstring const& GetFileName::prev_filename(void) const {
    return filename_;
  }

  ////////////////////////////////////////////////////////////////////////////////
  wstring const& GetFileName::prev_dir(void) const {
    return dir_;
  }


  ////////////////////////////////////////////////////////////////////////////////
  vector<wstring> get_all_filenames(wchar_t const* dir, wchar_t const* condition,
                                    int recursive) {
    vector<wstring> retval;
    wstring path = wstring(dir)+condition;
    WIN32_FIND_DATA lp;
    HANDLE handle = FindFirstFile(path.c_str(), &lp);

    do {
      if ((lp.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
          && recursive>0
          && wcscmp(lp.cFileName, L"..")!=0
          && wcscmp(lp.cFileName, L".")!=0) {
        wstring subpath = wstring(dir)+lp.cFileName;
        vector<wstring> tmp
          = get_all_filenames(subpath.c_str(), condition, recursive-1);
        retval.insert(retval.end(), tmp.begin(), tmp.end());
      } else if ((lp.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)!=FILE_ATTRIBUTE_DIRECTORY)
        retval.push_back(lp.cFileName);
    } while (FindNextFile(handle, &lp));

    FindClose(handle);
    return retval;
  }

#endif // _MSC_VER

  ////////////////////////////////////////////////////////////////////////////////
  wstring mbcs2wcs(char const* cs, size_t length) {
    shared_array<wchar_t> wcs(new wchar_t [length+1]);
#ifdef _MSC_VER
    size_t c;
    errno_t err = mbstowcs_s(&c, wcs.get(), length+1, cs, _TRUNCATE);
#else
    mbstowcs(wcs.get(), cs, length+1);
#endif
    return wstring(wcs.get());
  }

  string wcs2mbcs(wchar_t const* cs, size_t length) {
    shared_array<char> mbcs(new char [2*length+1]);
#ifdef _MSC_VER
    size_t c;
    errno_t err = wcstombs_s(&c, mbcs.get(), 2*length+1, cs, _TRUNCATE);
#else
    wcstombs(mbcs.get(), cs, 2*length+1);
#endif
    return string(mbcs.get());
  }

#ifdef __APPLE__
  std::wstring CFStringGetWString(CFStringRef cs) {
    CFIndex len = CFStringGetLength(cs);
    CFIndex max = CFStringGetMaximumSizeForEncoding(len, kCFStringEncodingUTF8);
    shared_array<char> wcs(new char[max+1]);
    CFStringGetCString(cs, wcs.get(), max+1, kCFStringEncodingUTF8);
    return mbcs2wcs(wcs.get(), strlen(wcs.get()));
  }
  
  CFStringRef CFStringCreateWithWString(CFAllocatorRef alloc, wchar_t const* cs, CFStringEncoding encoding) {
    return CFStringCreateWithCString(alloc, wcs2mbcs(cs, wcslen(cs)).c_str(), encoding);
  }
#endif

} // The end of the namespace "hashimoto_ut"

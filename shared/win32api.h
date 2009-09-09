// HASHIMOTO, Yasuhiro (E-mail: hy @ sys.t.u-tokyo.ac.jp)
// update: 2009/02/15

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

#ifndef INCLUDE_GUARD_SHARED_WIN32API_H
#define INCLUDE_GUARD_SHARED_WIN32API_H

#include <vector>
#include <string>
#ifdef _MSC_VER
#include <windows.h>
#endif

namespace hashimoto_ut {

#ifdef _MSC_VER
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // ファイルのパスをドライブ・ディレクトリ，ファイル名，拡張子に分割
  class PathSplitter {
  public:
    PathSplitter(wchar_t const* path) { _wsplitpath_s(path, drive_, dir_, filename_, ext_); }
    ~PathSplitter() {}
    std::wstring drive(void) const { return drive_; }
    std::wstring dir(void)   const { return dir_; }
    std::wstring filename(void)  const { return filename_; }
    std::wstring ext(void)   const { return ext_; }

  private:
    wchar_t drive_[_MAX_DRIVE];
    wchar_t dir_[_MAX_DIR];
    wchar_t filename_[_MAX_FNAME];
    wchar_t ext_[_MAX_EXT];
  };

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // ファイル入出力ダイアログからファイル名を取得するクラス
  class GetFileName {
  public:
    GetFileName(void) {}
    ~GetFileName() {}
    std::wstring for_read(HWND hwnd, wchar_t const* filename, wchar_t const* title, wchar_t const* filter, wchar_t const* path);
    std::vector<std::wstring> for_read_multi(HWND, wchar_t const* title, wchar_t const* filter, wchar_t const* path);
    std::wstring for_write(HWND hwnd, wchar_t const* filename, wchar_t const* title, wchar_t const* filter, wchar_t const* path);
    std::wstring const& prev_filename(void) const;
    std::wstring const& prev_dir(void) const;

  private:
    std::wstring filename_;
    std::wstring dir_;
  };

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // 指定したフォルダから条件にマッチするすべてのファイル名を取得
  std::vector<std::wstring> get_all_filenames(wchar_t const* dir, int recursive);

#endif // _MSC_VER
  
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // マルチバイト文字列⇔ワイド文字列
  std::wstring mbcs2wcs(char const* cs, size_t length);
  std::string wcs2mbcs(wchar_t const* cs, size_t length);

  struct MBCS2WCS {
    std::wstring operator()(const std::string& s) const { return mbcs2wcs(s.c_str(), s.size()); }
  };

  struct WCS2MBCS {
    std::string operator()(const std::wstring& s) const { return wcs2mbcs(s.c_str(), s.size()); }
  };

} // The end of the namespace "hashimoto_ut"

#endif // INCLUDE_GUARD_SHARED_WIN32API_H

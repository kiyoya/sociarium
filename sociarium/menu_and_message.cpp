// s.o.c.i.a.r.i.u.m: menu_and_message.cpp
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
#include <unordered_map>
#include <windows.h>
#include "menu_and_message.h"
#include "resource.h"

namespace hashimoto_ut {

  using std::vector;
  using std::wstring;
  using std::tr1::shared_ptr;
  using std::tr1::unordered_map;

  namespace sociarium_project_menu_and_message {

    typedef __declspec(dllimport)
      void (__cdecl* FuncLoadMessage)(vector<wstring>& message);

    typedef __declspec(dllimport)
      void (__cdecl* FuncLoadMenu)(unordered_map<int, wstring>& menu);

    namespace {
      shared_ptr<Message> message_object;
    }

    ////////////////////////////////////////////////////////////////////////////////
    void set_menu(HWND hwnd, wchar_t const* filename) {

      wstring path = wstring(L"dll\\")+filename;

      HMODULE handle = LoadLibrary(path.c_str());

      FuncLoadMenu func = (FuncLoadMenu)GetProcAddress(handle, "load_menu");

      if (func==0)
        throw path;

      unordered_map<int, wstring> menu;

      func(menu);

      HMENU hmenu           = CreateMenu();
      HMENU hmenu_file      = CreateMenu();
      HMENU hmenu_edit      = CreateMenu();
      HMENU hmenu_view      = CreateMenu();
      HMENU hmenu_string    = CreateMenu();
      HMENU hmenu_layout    = CreateMenu();
      HMENU hmenu_community = CreateMenu();
      HMENU hmenu_timeline  = CreateMenu();

      UINT main_menu_id = 0;

      MENUITEMINFO mii;

      ////////////////////////////////////////////////////////////////////////////////
      // FILE

      ZeroMemory(&mii, sizeof(mii));
      mii.cbSize = sizeof(mii);
      mii.fMask = MIIM_TYPE|MIIM_SUBMENU;
      mii.fType = MFT_STRING;
      mii.hSubMenu = hmenu_file;
      mii.dwTypeData = (LPWSTR)menu[IDM_FILE].c_str();
      InsertMenuItem(hmenu, ++main_menu_id, FALSE, &mii);

      {
        UINT pos = 0;

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_ID;
        mii.fType = MFT_STRING;
        mii.wID = IDM_FILE_CANCEL;
        mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
        InsertMenuItem(hmenu_file, ++pos, FALSE, &mii);

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE;
        mii.fType = MFT_SEPARATOR;
        InsertMenuItem(hmenu_file, ++pos, FALSE, &mii);

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_ID;
        mii.fType = MFT_STRING;
        mii.wID = IDM_FILE_OUTPUT_DEGREE_DISTRIBUTION;
        mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
        InsertMenuItem(hmenu_file, ++pos, FALSE, &mii);

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_ID;
        mii.fType = MFT_STRING;
        mii.wID = IDM_FILE_OUTPUT_COMMUNITY_INFORMATION;
        mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
        InsertMenuItem(hmenu_file, ++pos, FALSE, &mii);

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE;
        mii.fType = MFT_SEPARATOR;
        InsertMenuItem(hmenu_file, ++pos, FALSE, &mii);

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_ID;
        mii.fType = MFT_STRING;
        mii.wID = IDM_FILE_QUIT;
        mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
        InsertMenuItem(hmenu_file, ++pos, FALSE, &mii);
      }


      ////////////////////////////////////////////////////////////////////////////////
      // EDIT

      ZeroMemory(&mii, sizeof(mii));
      mii.cbSize = sizeof(mii);
      mii.fMask = MIIM_TYPE|MIIM_SUBMENU;
      mii.fType = MFT_STRING;
      mii.hSubMenu = hmenu_edit;
      mii.dwTypeData = (LPWSTR)menu[IDM_EDIT].c_str();
      InsertMenuItem(hmenu, ++main_menu_id, FALSE, &mii);

      {
        UINT pos = 0;

        HMENU hmenu_edit_select_current = CreateMenu();

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_SUBMENU;
        mii.fType = MFT_STRING;
        mii.hSubMenu = hmenu_edit_select_current;
        mii.dwTypeData = (LPWSTR)menu[IDM_EDIT_MARK_ON_CURRENT_LAYER].c_str();
        InsertMenuItem(hmenu_edit, ++pos, FALSE, &mii);

        {
          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_EDIT_MARK_ALL_NODES_ON_CURRENT_LAYER;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_edit_select_current, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_EDIT_MARK_ALL_EDGES_ON_CURRENT_LAYER;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_edit_select_current, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE;
          mii.fType = MFT_SEPARATOR;
          InsertMenuItem(hmenu_edit_select_current, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_EDIT_MARK_NODES_INSIDE_COMMUNITY_ON_CURRENT_LAYER;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_edit_select_current, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_EDIT_MARK_EDGES_INSIDE_COMMUNITY_ON_CURRENT_LAYER;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_edit_select_current, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_EDIT_MARK_ELEMENTS_INSIDE_COMMUNITY_ON_CURRENT_LAYER;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_edit_select_current, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE;
          mii.fType = MFT_SEPARATOR;
          InsertMenuItem(hmenu_edit_select_current, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_EDIT_MARK_NODES_OUTSIDE_COMMUNITY_ON_CURRENT_LAYER;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_edit_select_current, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_EDIT_MARK_EDGES_OUTSIDE_COMMUNITY_ON_CURRENT_LAYER;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_edit_select_current, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_EDIT_MARK_ELEMENTS_OUTSIDE_COMMUNITY_ON_CURRENT_LAYER;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_edit_select_current, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE;
          mii.fType = MFT_SEPARATOR;
          InsertMenuItem(hmenu_edit_select_current, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_EDIT_MARK_NODES_IN_SELECTED_COMMUNITY_CONTINUUMS_ON_CURRENT_LAYER;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_edit_select_current, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_EDIT_MARK_EDGES_IN_SELECTED_COMMUNITY_CONTINUUMS_ON_CURRENT_LAYER;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_edit_select_current, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_EDIT_MARK_ELEMENTS_IN_SELECTED_COMMUNITY_CONTINUUMS_ON_CURRENT_LAYER;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_edit_select_current, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE;
          mii.fType = MFT_SEPARATOR;
          InsertMenuItem(hmenu_edit_select_current, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_EDIT_MARK_ALL_ELEMENTS_ON_CURRENT_LAYER;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_edit_select_current, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_EDIT_INVERT_MARK_ON_CURRENT_LAYER;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_edit_select_current, ++pos, FALSE, &mii);
        }

        HMENU hmenu_edit_select_all = CreateMenu();

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_SUBMENU;
        mii.fType = MFT_STRING;
        mii.hSubMenu = hmenu_edit_select_all;
        mii.dwTypeData = (LPWSTR)menu[IDM_EDIT_MARK_ON_EACH_LAYER].c_str();
        InsertMenuItem(hmenu_edit, ++pos, FALSE, &mii);

        {
          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_EDIT_MARK_ALL_NODES_ON_EACH_LAYER;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_edit_select_all, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_EDIT_MARK_ALL_EDGES_ON_EACH_LAYER;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_edit_select_all, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE;
          mii.fType = MFT_SEPARATOR;
          InsertMenuItem(hmenu_edit_select_all, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_EDIT_MARK_NODES_INSIDE_COMMUNITY_ON_EACH_LAYER;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_edit_select_all, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_EDIT_MARK_EDGES_INSIDE_COMMUNITY_ON_EACH_LAYER;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_edit_select_all, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_EDIT_MARK_ELEMENTS_INSIDE_COMMUNITY_ON_EACH_LAYER;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_edit_select_all, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE;
          mii.fType = MFT_SEPARATOR;
          InsertMenuItem(hmenu_edit_select_all, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_EDIT_MARK_NODES_OUTSIDE_COMMUNITY_ON_EACH_LAYER;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_edit_select_all, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_EDIT_MARK_EDGES_OUTSIDE_COMMUNITY_ON_EACH_LAYER;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_edit_select_all, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_EDIT_MARK_ELEMENTS_OUTSIDE_COMMUNITY_ON_EACH_LAYER;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_edit_select_all, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE;
          mii.fType = MFT_SEPARATOR;
          InsertMenuItem(hmenu_edit_select_all, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_EDIT_MARK_NODES_IN_SELECTED_COMMUNITY_CONTINUUMS_ON_EACH_LAYER;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_edit_select_all, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_EDIT_MARK_EDGES_IN_SELECTED_COMMUNITY_CONTINUUMS_ON_EACH_LAYER;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_edit_select_all, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_EDIT_MARK_ELEMENTS_IN_SELECTED_COMMUNITY_CONTINUUMS_ON_EACH_LAYER;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_edit_select_all, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE;
          mii.fType = MFT_SEPARATOR;
          InsertMenuItem(hmenu_edit_select_all, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_EDIT_MARK_ALL_ELEMENTS_ON_EACH_LAYER;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_edit_select_all, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_EDIT_INVERT_MARK_ON_EACH_LAYER;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_edit_select_all, ++pos, FALSE, &mii);
        }

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE;
        mii.fType = MFT_SEPARATOR;
        InsertMenuItem(hmenu_edit, ++pos, FALSE, &mii);

        HMENU hmenu_edit_hide = CreateMenu();

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_SUBMENU;
        mii.fType = MFT_STRING;
        mii.hSubMenu = hmenu_edit_hide;
        mii.dwTypeData = (LPWSTR)menu[IDM_EDIT_HIDE_MARKED_ELEMENTS].c_str();
        InsertMenuItem(hmenu_edit, ++pos, FALSE, &mii);

        {
          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_EDIT_HIDE_MARKED_NODES_ON_CURRENT_LAYER;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_edit_hide, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_EDIT_HIDE_MARKED_EDGES_ON_CURRENT_LAYER;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_edit_hide, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_EDIT_HIDE_MARKED_ELEMENTS_ON_CURRENT_LAYER;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_edit_hide, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE;
          mii.fType = MFT_SEPARATOR;
          InsertMenuItem(hmenu_edit_hide, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_EDIT_HIDE_MARKED_NODES_ON_EACH_LAYER;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_edit_hide, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_EDIT_HIDE_MARKED_EDGES_ON_EACH_LAYER;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_edit_hide, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_EDIT_HIDE_MARKED_ELEMENTS_ON_EACH_LAYER;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_edit_hide, ++pos, FALSE, &mii);
        }

        HMENU hmenu_edit_hide_all = CreateMenu();

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_SUBMENU;
        mii.fType = MFT_STRING;
        mii.hSubMenu = hmenu_edit_hide_all;
        mii.dwTypeData = (LPWSTR)menu[IDM_EDIT_HIDE_MARKED_ELEMENTS_ALL].c_str();
        InsertMenuItem(hmenu_edit, ++pos, FALSE, &mii);

        {
          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_EDIT_HIDE_MARKED_NODES_ON_CURRENT_LAYER_ALL;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_edit_hide_all, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_EDIT_HIDE_MARKED_EDGES_ON_CURRENT_LAYER_ALL;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_edit_hide_all, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_EDIT_HIDE_MARKED_ELEMENTS_ON_CURRENT_LAYER_ALL;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_edit_hide_all, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE;
          mii.fType = MFT_SEPARATOR;
          InsertMenuItem(hmenu_edit_hide_all, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_EDIT_HIDE_MARKED_NODES_ON_EACH_LAYER_ALL;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_edit_hide_all, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_EDIT_HIDE_MARKED_EDGES_ON_EACH_LAYER_ALL;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_edit_hide_all, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_EDIT_HIDE_MARKED_ELEMENTS_ON_EACH_LAYER_ALL;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_edit_hide_all, ++pos, FALSE, &mii);
        }

        HMENU hmenu_edit_show = CreateMenu();

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_SUBMENU;
        mii.fType = MFT_STRING;
        mii.hSubMenu = hmenu_edit_show;
        mii.dwTypeData = (LPWSTR)menu[IDM_EDIT_SHOW_HIDDEN_ELEMENTS].c_str();
        InsertMenuItem(hmenu_edit, ++pos, FALSE, &mii);

        {
          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_EDIT_SHOW_HIDDEN_NODES_ON_CURRENT_LAYER;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_edit_show, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_EDIT_SHOW_HIDDEN_EDGES_ON_CURRENT_LAYER;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_edit_show, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_EDIT_SHOW_HIDDEN_ELEMENTS_ON_CURRENT_LAYER;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_edit_show, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE;
          mii.fType = MFT_SEPARATOR;
          InsertMenuItem(hmenu_edit_show, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_EDIT_SHOW_HIDDEN_NODES_ON_EACH_LAYER;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_edit_show, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_EDIT_SHOW_HIDDEN_EDGES_ON_EACH_LAYER;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_edit_show, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_EDIT_SHOW_HIDDEN_ELEMENTS_ON_EACH_LAYER;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_edit_show, ++pos, FALSE, &mii);
        }


        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE;
        mii.fType = MFT_SEPARATOR;
        InsertMenuItem(hmenu_edit, ++pos, FALSE, &mii);

        HMENU hmenu_edit_remove = CreateMenu();

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_SUBMENU;
        mii.fType = MFT_STRING;
        mii.hSubMenu = hmenu_edit_remove;
        mii.dwTypeData = (LPWSTR)menu[IDM_EDIT_REMOVE_MARKED_ELEMENTS].c_str();
        InsertMenuItem(hmenu_edit, ++pos, FALSE, &mii);

        {
          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_EDIT_REMOVE_MARKED_NODES_ON_CURRENT_LAYER;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_edit_remove, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_EDIT_REMOVE_MARKED_EDGES_ON_CURRENT_LAYER;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_edit_remove, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_EDIT_REMOVE_MARKED_ELEMENTS_ON_CURRENT_LAYER;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_edit_remove, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE;
          mii.fType = MFT_SEPARATOR;
          InsertMenuItem(hmenu_edit_remove, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_EDIT_REMOVE_MARKED_NODES_ON_EACH_LAYER;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_edit_remove, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_EDIT_REMOVE_MARKED_EDGES_ON_EACH_LAYER;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_edit_remove, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_EDIT_REMOVE_MARKED_ELEMENTS_ON_EACH_LAYER;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_edit_remove, ++pos, FALSE, &mii);
        }

        HMENU hmenu_edit_remove_all = CreateMenu();

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_SUBMENU;
        mii.fType = MFT_STRING;
        mii.hSubMenu = hmenu_edit_remove_all;
        mii.dwTypeData = (LPWSTR)menu[IDM_EDIT_REMOVE_MARKED_ELEMENTS_ALL].c_str();
        InsertMenuItem(hmenu_edit, ++pos, FALSE, &mii);

        {
          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_EDIT_REMOVE_MARKED_NODES_ON_CURRENT_LAYER_ALL;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_edit_remove_all, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_EDIT_REMOVE_MARKED_EDGES_ON_CURRENT_LAYER_ALL;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_edit_remove_all, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_EDIT_REMOVE_MARKED_ELEMENTS_ON_CURRENT_LAYER_ALL;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_edit_remove_all, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE;
          mii.fType = MFT_SEPARATOR;
          InsertMenuItem(hmenu_edit_remove_all, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_EDIT_REMOVE_MARKED_NODES_ON_EACH_LAYER_ALL;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_edit_remove_all, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_EDIT_REMOVE_MARKED_EDGES_ON_EACH_LAYER_ALL;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_edit_remove_all, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_EDIT_REMOVE_MARKED_ELEMENTS_ON_EACH_LAYER_ALL;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_edit_remove_all, ++pos, FALSE, &mii);
        }
      }


      ////////////////////////////////////////////////////////////////////////////////
      // VIEW

      ZeroMemory(&mii, sizeof(mii));
      mii.cbSize = sizeof(mii);
      mii.fMask = MIIM_TYPE|MIIM_SUBMENU;
      mii.fType = MFT_STRING;
      mii.hSubMenu = hmenu_view;
      mii.dwTypeData = (LPWSTR)menu[IDM_VIEW].c_str();
      InsertMenuItem(hmenu, ++main_menu_id, FALSE, &mii);

      {
        UINT pos = 0;

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_ID;
        mii.fType = MFT_STRING;
        mii.wID = IDM_VIEW_SHOW_NODE;
        mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
        InsertMenuItem(hmenu_view, ++pos, FALSE, &mii);

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_ID;
        mii.fType = MFT_STRING;
        mii.wID = IDM_VIEW_SHOW_EDGE;
        mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
        InsertMenuItem(hmenu_view, ++pos, FALSE, &mii);

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_ID;
        mii.fType = MFT_STRING;
        mii.wID = IDM_VIEW_SHOW_COMMUNITY;
        mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
        InsertMenuItem(hmenu_view, ++pos, FALSE, &mii);

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_ID;
        mii.fType = MFT_STRING;
        mii.wID = IDM_VIEW_SHOW_COMMUNITY_EDGE;
        mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
        InsertMenuItem(hmenu_view, ++pos, FALSE, &mii);

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE;
        mii.fType = MFT_SEPARATOR;
        InsertMenuItem(hmenu_view, ++pos, FALSE, &mii);

        HMENU hmenu_view_node_size = CreateMenu();

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_SUBMENU;
        mii.fType = MFT_STRING;
        mii.hSubMenu = hmenu_view_node_size;
        mii.dwTypeData = (LPWSTR)menu[IDM_VIEW_NODE_SIZE].c_str();
        InsertMenuItem(hmenu_view, ++pos, FALSE, &mii);

        {
          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_VIEW_NODE_SIZE_UPDATE;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_view_node_size, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_VIEW_NODE_SIZE_CANCEL;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_view_node_size, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE;
          mii.fType = MFT_SEPARATOR;
          InsertMenuItem(hmenu_view_node_size, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_VIEW_NODE_SIZE_UNIFORM;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_view_node_size, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_VIEW_NODE_SIZE_WEIGHT;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_view_node_size, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_VIEW_NODE_SIZE_DEGREE_CENTRALITY;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_view_node_size, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_VIEW_NODE_SIZE_CLOSENESS_CENTRALITY;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_view_node_size, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_VIEW_NODE_SIZE_BETWEENNESS_CENTRALITY;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_view_node_size, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_VIEW_NODE_SIZE_PAGERANK;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_view_node_size, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_VIEW_NODE_SIZE_POINT;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_view_node_size, ++pos, FALSE, &mii);
        }

        HMENU hmenu_view_edge_width = CreateMenu();

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_SUBMENU;
        mii.fType = MFT_STRING;
        mii.hSubMenu = hmenu_view_edge_width;
        mii.dwTypeData = (LPWSTR)menu[IDM_VIEW_EDGE_WIDTH].c_str();
        InsertMenuItem(hmenu_view, ++pos, FALSE, &mii);

        {
          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_VIEW_EDGE_WIDTH_UPDATE;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_view_edge_width, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_VIEW_EDGE_WIDTH_CANCEL;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_view_edge_width, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE;
          mii.fType = MFT_SEPARATOR;
          InsertMenuItem(hmenu_view_edge_width, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_VIEW_EDGE_WIDTH_UNIFORM;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_view_edge_width, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_VIEW_EDGE_WIDTH_WEIGHT;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_view_edge_width, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_VIEW_EDGE_WIDTH_BETWEENNESS_CENTRALITY;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_view_edge_width, ++pos, FALSE, &mii);
        }

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE;
        mii.fType = MFT_SEPARATOR;
        InsertMenuItem(hmenu_view, ++pos, FALSE, &mii);

        HMENU hmenu_view_node_style = CreateMenu();

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_SUBMENU;
        mii.fType = MFT_STRING;
        mii.hSubMenu = hmenu_view_node_style;
        mii.dwTypeData = (LPWSTR)menu[IDM_VIEW_NODE_STYLE].c_str();
        InsertMenuItem(hmenu_view, ++pos, FALSE, &mii);

        {
          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_VIEW_NODE_STYLE_POLYGON;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_view_node_style, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_VIEW_NODE_STYLE_TEXTURE;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_view_node_style, ++pos, FALSE, &mii);
        }

        HMENU hmenu_view_edge_style = CreateMenu();

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_SUBMENU;
        mii.fType = MFT_STRING;
        mii.hSubMenu = hmenu_view_edge_style;
        mii.dwTypeData = (LPWSTR)menu[IDM_VIEW_EDGE_STYLE].c_str();
        InsertMenuItem(hmenu_view, ++pos, FALSE, &mii);

        {
          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_VIEW_EDGE_STYLE_LINE;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_view_edge_style, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_VIEW_EDGE_STYLE_POLYGON;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_view_edge_style, ++pos, FALSE, &mii);
        }

        HMENU hmenu_view_community_style = CreateMenu();

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_SUBMENU;
        mii.fType = MFT_STRING;
        mii.hSubMenu = hmenu_view_community_style;
        mii.dwTypeData = (LPWSTR)menu[IDM_VIEW_COMMUNITY_STYLE].c_str();
        InsertMenuItem(hmenu_view, ++pos, FALSE, &mii);

        {
          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_VIEW_COMMUNITY_STYLE_POLYGON_CIRCLE;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_view_community_style, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_VIEW_COMMUNITY_STYLE_TEXTURE;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_view_community_style, ++pos, FALSE, &mii);
        }

        HMENU hmenu_view_community_edge_style = CreateMenu();

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_SUBMENU;
        mii.fType = MFT_STRING;
        mii.hSubMenu = hmenu_view_community_edge_style;
        mii.dwTypeData = (LPWSTR)menu[IDM_VIEW_COMMUNITY_EDGE_STYLE].c_str();
        InsertMenuItem(hmenu_view, ++pos, FALSE, &mii);

        {
          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_VIEW_COMMUNITY_EDGE_STYLE_LINE;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_view_community_edge_style, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_VIEW_COMMUNITY_EDGE_STYLE_POLYGON;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_view_community_edge_style, ++pos, FALSE, &mii);
        }
      }


      ////////////////////////////////////////////////////////////////////////////////
      // STRING

      ZeroMemory(&mii, sizeof(mii));
      mii.cbSize = sizeof(mii);
      mii.fMask = MIIM_TYPE|MIIM_SUBMENU;
      mii.fType = MFT_STRING;
      mii.hSubMenu = hmenu_string;
      mii.dwTypeData = (LPWSTR)menu[IDM_STRING].c_str();
      InsertMenuItem(hmenu, ++main_menu_id, FALSE, &mii);

      {
        UINT pos = 0;

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_ID;
        mii.fType = MFT_STRING;
        mii.wID = IDM_STRING_SHOW_NODE_NAME;
        mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
        InsertMenuItem(hmenu_string, ++pos, FALSE, &mii);

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_ID;
        mii.fType = MFT_STRING;
        mii.wID = IDM_STRING_SHOW_EDGE_NAME;
        mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
        InsertMenuItem(hmenu_string, ++pos, FALSE, &mii);

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_ID;
        mii.fType = MFT_STRING;
        mii.wID = IDM_STRING_SHOW_COMMUNITY_NAME;
        mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
        InsertMenuItem(hmenu_string, ++pos, FALSE, &mii);

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_ID;
        mii.fType = MFT_STRING;
        mii.wID = IDM_STRING_SHOW_COMMUNITY_EDGE_NAME;
        mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
        InsertMenuItem(hmenu_string, ++pos, FALSE, &mii);

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE;
        mii.fType = MFT_SEPARATOR;
        InsertMenuItem(hmenu_string, ++pos, FALSE, &mii);

        HMENU hmenu_string_node_name_size = CreateMenu();

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_SUBMENU;
        mii.fType = MFT_STRING;
        mii.hSubMenu = hmenu_string_node_name_size;
        mii.dwTypeData = (LPWSTR)menu[IDM_STRING_NODE_NAME_SIZE].c_str();
        InsertMenuItem(hmenu_string, ++pos, FALSE, &mii);

        {
          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_STRING_NODE_NAME_SIZE_VARIABLE;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_string_node_name_size, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE;
          mii.fType = MFT_SEPARATOR;
          InsertMenuItem(hmenu_string_node_name_size, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_STRING_NODE_NAME_SIZE_0;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_string_node_name_size, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_STRING_NODE_NAME_SIZE_1;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_string_node_name_size, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_STRING_NODE_NAME_SIZE_2;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_string_node_name_size, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_STRING_NODE_NAME_SIZE_3;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_string_node_name_size, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_STRING_NODE_NAME_SIZE_4;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_string_node_name_size, ++pos, FALSE, &mii);
        }

        HMENU hmenu_string_edge_name_size = CreateMenu();

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_SUBMENU;
        mii.fType = MFT_STRING;
        mii.hSubMenu = hmenu_string_edge_name_size;
        mii.dwTypeData = (LPWSTR)menu[IDM_STRING_EDGE_NAME_SIZE].c_str();
        InsertMenuItem(hmenu_string, ++pos, FALSE, &mii);

        {
          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_STRING_EDGE_NAME_SIZE_VARIABLE;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_string_edge_name_size, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE;
          mii.fType = MFT_SEPARATOR;
          InsertMenuItem(hmenu_string_edge_name_size, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_STRING_EDGE_NAME_SIZE_0;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_string_edge_name_size, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_STRING_EDGE_NAME_SIZE_1;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_string_edge_name_size, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_STRING_EDGE_NAME_SIZE_2;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_string_edge_name_size, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_STRING_EDGE_NAME_SIZE_3;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_string_edge_name_size, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_STRING_EDGE_NAME_SIZE_4;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_string_edge_name_size, ++pos, FALSE, &mii);
        }

        HMENU hmenu_string_community_name_size = CreateMenu();

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_SUBMENU;
        mii.fType = MFT_STRING;
        mii.hSubMenu = hmenu_string_community_name_size;
        mii.dwTypeData = (LPWSTR)menu[IDM_STRING_COMMUNITY_NAME_SIZE].c_str();
        InsertMenuItem(hmenu_string, ++pos, FALSE, &mii);

        {
          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_STRING_COMMUNITY_NAME_SIZE_VARIABLE;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_string_community_name_size, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE;
          mii.fType = MFT_SEPARATOR;
          InsertMenuItem(hmenu_string_community_name_size, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_STRING_COMMUNITY_NAME_SIZE_0;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_string_community_name_size, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_STRING_COMMUNITY_NAME_SIZE_1;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_string_community_name_size, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_STRING_COMMUNITY_NAME_SIZE_2;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_string_community_name_size, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_STRING_COMMUNITY_NAME_SIZE_3;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_string_community_name_size, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_STRING_COMMUNITY_NAME_SIZE_4;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_string_community_name_size, ++pos, FALSE, &mii);
        }

        HMENU hmenu_string_community_edge_name_size = CreateMenu();

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_SUBMENU;
        mii.fType = MFT_STRING;
        mii.hSubMenu = hmenu_string_community_edge_name_size;
        mii.dwTypeData = (LPWSTR)menu[IDM_STRING_COMMUNITY_EDGE_NAME_SIZE].c_str();
        InsertMenuItem(hmenu_string, ++pos, FALSE, &mii);

        {
          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_STRING_COMMUNITY_EDGE_NAME_SIZE_VARIABLE;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_string_community_edge_name_size, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE;
          mii.fType = MFT_SEPARATOR;
          InsertMenuItem(hmenu_string_community_edge_name_size, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_STRING_COMMUNITY_EDGE_NAME_SIZE_0;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_string_community_edge_name_size, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_STRING_COMMUNITY_EDGE_NAME_SIZE_1;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_string_community_edge_name_size, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_STRING_COMMUNITY_EDGE_NAME_SIZE_2;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_string_community_edge_name_size, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_STRING_COMMUNITY_EDGE_NAME_SIZE_3;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_string_community_edge_name_size, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_STRING_COMMUNITY_EDGE_NAME_SIZE_4;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_string_community_edge_name_size, ++pos, FALSE, &mii);
        }

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE;
        mii.fType = MFT_SEPARATOR;
        InsertMenuItem(hmenu_string, ++pos, FALSE, &mii);

        HMENU hmenu_string_type = CreateMenu();

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_SUBMENU;
        mii.fType = MFT_STRING;
        mii.hSubMenu = hmenu_string_type;
        mii.dwTypeData = (LPWSTR)menu[IDM_STRING_FONT_TYPE].c_str();
        InsertMenuItem(hmenu_string, ++pos, FALSE, &mii);

        {
          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_STRING_FONT_TYPE_POLYGON;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_string_type, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_STRING_FONT_TYPE_TEXTURE;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_string_type, ++pos, FALSE, &mii);
        }

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE;
        mii.fType = MFT_SEPARATOR;
        InsertMenuItem(hmenu_string, ++pos, FALSE, &mii);

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_ID;
        mii.fType = MFT_STRING;
        mii.wID = IDM_STRING_SHOW_LAYER_NAME;
        mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
        InsertMenuItem(hmenu_string, ++pos, FALSE, &mii);

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_ID;
        mii.fType = MFT_STRING;
        mii.wID = IDM_STRING_SHOW_FPS;
        mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
        InsertMenuItem(hmenu_string, ++pos, FALSE, &mii);
      }


      ////////////////////////////////////////////////////////////////////////////////
      // LAYOUT

      ZeroMemory(&mii, sizeof(mii));
      mii.cbSize = sizeof(mii);
      mii.fMask = MIIM_TYPE|MIIM_SUBMENU;
      mii.fType = MFT_STRING;
      mii.hSubMenu = hmenu_layout;
      mii.dwTypeData = (LPWSTR)menu[IDM_LAYOUT].c_str();
      InsertMenuItem(hmenu, ++main_menu_id, FALSE, &mii);

      {
        UINT pos = 0;

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_ID;
        mii.fType = MFT_STRING;
        mii.wID = IDM_LAYOUT_UPDATE;
        mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
        InsertMenuItem(hmenu_layout, ++pos, FALSE, &mii);

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_ID;
        mii.fType = MFT_STRING;
        mii.wID = IDM_LAYOUT_CANCEL;
        mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
        InsertMenuItem(hmenu_layout, ++pos, FALSE, &mii);

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE;
        mii.fType = MFT_SEPARATOR;
        InsertMenuItem(hmenu_layout, ++pos, FALSE, &mii);

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_ID;
        mii.fType = MFT_STRING;
        mii.wID = IDM_LAYOUT_KAMADA_KAWAI_METHOD;
        mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
        InsertMenuItem(hmenu_layout, ++pos, FALSE, &mii);

        HMENU hmenu_layout_hde = CreateMenu();

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_SUBMENU;
        mii.fType = MFT_STRING;
        mii.hSubMenu = hmenu_layout_hde;
        mii.dwTypeData = (LPWSTR)menu[IDM_LAYOUT_HIGH_DIMENSIONAL_EMBEDDING].c_str();
        InsertMenuItem(hmenu_layout, ++pos, FALSE, &mii);

        {
          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_LAYOUT_HIGH_DIMENSIONAL_EMBEDDING_1_2;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_layout_hde, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_LAYOUT_HIGH_DIMENSIONAL_EMBEDDING_1_3;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_layout_hde, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_LAYOUT_HIGH_DIMENSIONAL_EMBEDDING_2_3;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_layout_hde, ++pos, FALSE, &mii);
        }

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE;
        mii.fType = MFT_SEPARATOR;
        InsertMenuItem(hmenu_layout, ++pos, FALSE, &mii);

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_ID;
        mii.fType = MFT_STRING;
        mii.wID = IDM_LAYOUT_CIRCLE;
        mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
        InsertMenuItem(hmenu_layout, ++pos, FALSE, &mii);

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_ID;
        mii.fType = MFT_STRING;
        mii.wID = IDM_LAYOUT_CIRCLE_IN_SIZE_ORDER;
        mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
        InsertMenuItem(hmenu_layout, ++pos, FALSE, &mii);

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_ID;
        mii.fType = MFT_STRING;
        mii.wID = IDM_LAYOUT_LATTICE;
        mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
        InsertMenuItem(hmenu_layout, ++pos, FALSE, &mii);

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_ID;
        mii.fType = MFT_STRING;
        mii.wID = IDM_LAYOUT_RANDOM;
        mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
        InsertMenuItem(hmenu_layout, ++pos, FALSE, &mii);

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_ID;
        mii.fType = MFT_STRING;
        mii.wID = IDM_LAYOUT_CARTOGRAMS;
        mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
        InsertMenuItem(hmenu_layout, ++pos, FALSE, &mii);

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE;
        mii.fType = MFT_SEPARATOR;
        InsertMenuItem(hmenu_layout, ++pos, FALSE, &mii);

        HMENU hmenu_layout_fde = CreateMenu();

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_SUBMENU;
        mii.fType = MFT_STRING;
        mii.hSubMenu = hmenu_layout_fde;
        mii.dwTypeData = (LPWSTR)menu[IDM_LAYOUT_FORCE_DIRECTION].c_str();
        InsertMenuItem(hmenu_layout, ++pos, FALSE, &mii);

        {
          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_LAYOUT_FORCE_DIRECTION_RUN;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_layout_fde, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE;
          mii.fType = MFT_SEPARATOR;
          InsertMenuItem(hmenu_layout_fde, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_LAYOUT_FORCE_DIRECTION_KAMADA_KAWAI_METHOD;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_layout_fde, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.wID = IDM_LAYOUT_FORCE_DIRECTION_KAMADA_KAWAI_METHOD_WITH_COMMUNITY_SEPARATION;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_layout_fde, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_LAYOUT_FORCE_DIRECTION_COMMUNITY_ORIENTED;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_layout_fde, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE;
          mii.fType = MFT_SEPARATOR;
          InsertMenuItem(hmenu_layout_fde, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_LAYOUT_FORCE_DIRECTION_SPRING_AND_REPULSIVE_FORCE;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_layout_fde, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_LAYOUT_FORCE_DIRECTION_LATTICE_GAS_METHOD;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_layout_fde, ++pos, FALSE, &mii);
#if 0
          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_LAYOUT_FORCE_DIRECTION_DESIGNTIDE;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_layout_fde, ++pos, FALSE, &mii);
#endif
        }

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE;
        mii.fType = MFT_SEPARATOR;
        InsertMenuItem(hmenu_layout, ++pos, FALSE, &mii);

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_ID;
        mii.fType = MFT_STRING;
        mii.wID = IDM_LAYOUT_SHOW_LAYOUT_FRAME;
        mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
        InsertMenuItem(hmenu_layout, ++pos, FALSE, &mii);

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_ID;
        mii.fType = MFT_STRING;
        mii.wID = IDM_LAYOUT_INITIALIZE_LAYOUT_FRAME;
        mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
        InsertMenuItem(hmenu_layout, ++pos, FALSE, &mii);

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE;
        mii.fType = MFT_SEPARATOR;
        InsertMenuItem(hmenu_layout, ++pos, FALSE, &mii);

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_ID;
        mii.fType = MFT_STRING;
        mii.wID = IDM_LAYOUT_SHOW_GRID;
        mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
        InsertMenuItem(hmenu_layout, ++pos, FALSE, &mii);

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_ID;
        mii.fType = MFT_STRING;
        mii.wID = IDM_LAYOUT_SHOW_CENTER;
        mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
        InsertMenuItem(hmenu_layout, ++pos, FALSE, &mii);

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE;
        mii.fType = MFT_SEPARATOR;
        InsertMenuItem(hmenu_layout, ++pos, FALSE, &mii);

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_ID;
        mii.fType = MFT_STRING;
        mii.wID = IDM_LAYOUT_INITIALIZE_EYEPOINT;
        mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
        InsertMenuItem(hmenu_layout, ++pos, FALSE, &mii);

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_ID;
        mii.fType = MFT_STRING;
        mii.wID = IDM_LAYOUT_ZOOM_IN;
        mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
        InsertMenuItem(hmenu_layout, ++pos, FALSE, &mii);

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_ID;
        mii.fType = MFT_STRING;
        mii.wID = IDM_LAYOUT_ZOOM_OUT;
        mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
        InsertMenuItem(hmenu_layout, ++pos, FALSE, &mii);
      }


      ////////////////////////////////////////////////////////////////////////////////
      // COMMUNITY

      ZeroMemory(&mii, sizeof(mii));
      mii.cbSize = sizeof(mii);
      mii.fMask = MIIM_TYPE|MIIM_SUBMENU;
      mii.fType = MFT_STRING;
      mii.hSubMenu = hmenu_community;
      mii.dwTypeData = (LPWSTR)menu[IDM_COMMUNITY_DETECTION].c_str();
      InsertMenuItem(hmenu, ++main_menu_id, FALSE, &mii);

      {
        UINT pos = 0;

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_ID;
        mii.fType = MFT_STRING;
        mii.wID = IDM_COMMUNITY_DETECTION_UPDATE;
        mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
        InsertMenuItem(hmenu_community, ++pos, FALSE, &mii);

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_ID;
        mii.fType = MFT_STRING;
        mii.wID = IDM_COMMUNITY_DETECTION_CANCEL;
        mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
        InsertMenuItem(hmenu_community, ++pos, FALSE, &mii);

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE;
        mii.fType = MFT_SEPARATOR;
        InsertMenuItem(hmenu_community, ++pos, FALSE, &mii);

        HMENU hmenu_community_connected_components = CreateMenu();

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_SUBMENU;
        mii.fType = MFT_STRING;
        mii.hSubMenu = hmenu_community_connected_components;
        mii.dwTypeData = (LPWSTR)menu[IDM_COMMUNITY_DETECTION_CONNECTED_COMPONENTS].c_str();
        InsertMenuItem(hmenu_community, ++pos, FALSE, &mii);

        {
          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_COMMUNITY_DETECTION_WEAKLY_CONNECTED_COMPONENTS;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_community_connected_components, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_COMMUNITY_DETECTION_STRONGLY_CONNECTED_COMPONENTS;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_community_connected_components, ++pos, FALSE, &mii);
        }

        HMENU hmenu_community_modularity_maximization = CreateMenu();

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_SUBMENU;
        mii.fType = MFT_STRING;
        mii.hSubMenu = hmenu_community_modularity_maximization;
        mii.dwTypeData = (LPWSTR)menu[IDM_COMMUNITY_DETECTION_MODULARITY_MAXIMIZATION].c_str();
        InsertMenuItem(hmenu_community, ++pos, FALSE, &mii);

        {
          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_COMMUNITY_DETECTION_MODULARITY_MAXIMIZATION_USING_GREEDY_METHOD;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_community_modularity_maximization, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_COMMUNITY_DETECTION_MODULARITY_MAXIMIZATION_USING_TEO_METHOD;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_community_modularity_maximization, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE;
          mii.fType = MFT_SEPARATOR;
          InsertMenuItem(hmenu_community_modularity_maximization, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_COMMUNITY_DETECTION_USE_WEIGHTED_MODULARITY;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_community_modularity_maximization, ++pos, FALSE, &mii);
        }

        HMENU hmenu_community_clique_percolaion = CreateMenu();

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_SUBMENU;
        mii.fType = MFT_STRING;
        mii.hSubMenu = hmenu_community_clique_percolaion;
        mii.dwTypeData = (LPWSTR)menu[IDM_COMMUNITY_DETECTION_CLIQUE_PERCOLATION].c_str();
        InsertMenuItem(hmenu_community, ++pos, FALSE, &mii);

        {
          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_COMMUNITY_DETECTION_CLIQUE_PERCOLATION_3;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_community_clique_percolaion, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_COMMUNITY_DETECTION_CLIQUE_PERCOLATION_4;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_community_clique_percolaion, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_COMMUNITY_DETECTION_CLIQUE_PERCOLATION_5;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_community_clique_percolaion, ++pos, FALSE, &mii);
        }

        HMENU hmenu_community_others = CreateMenu();

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_SUBMENU;
        mii.fType = MFT_STRING;
        mii.hSubMenu = hmenu_community_others;
        mii.dwTypeData = (LPWSTR)menu[IDM_COMMUNITY_DETECTION_OTHERS].c_str();
        InsertMenuItem(hmenu_community, ++pos, FALSE, &mii);

        {
          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_COMMUNITY_DETECTION_BETWEENNESS_CENTRALITY_SEPARATION;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_community_others, ++pos, FALSE, &mii);
#if 0
          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_COMMUNITY_DETECTION_INFORMATION_FLOW_MAPPING;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_community_others, ++pos, FALSE, &mii);
#endif
        }

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE;
        mii.fType = MFT_SEPARATOR;
        InsertMenuItem(hmenu_community, ++pos, FALSE, &mii);

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_ID;
        mii.fType = MFT_STRING;
        mii.wID = IDM_COMMUNITY_TRANSITION_DIAGRAM;
        mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
        InsertMenuItem(hmenu_community, ++pos, FALSE, &mii);

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_ID;
        mii.fType = MFT_STRING;
        mii.wID = IDM_COMMUNITY_TRANSITION_DIAGRAM_SCOPE_WIDER;
        mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
        InsertMenuItem(hmenu_community, ++pos, FALSE, &mii);

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_ID;
        mii.fType = MFT_STRING;
        mii.wID = IDM_COMMUNITY_TRANSITION_DIAGRAM_SCOPE_NARROWER;
        mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
        InsertMenuItem(hmenu_community, ++pos, FALSE, &mii);

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE;
        mii.fType = MFT_SEPARATOR;
        InsertMenuItem(hmenu_community, ++pos, FALSE, &mii);

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_ID;
        mii.fType = MFT_STRING;
        mii.wID = IDM_COMMUNITY_DETECTION_CLEAR;
        mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
        InsertMenuItem(hmenu_community, ++pos, FALSE, &mii);
      }


      ////////////////////////////////////////////////////////////////////////////////
      // TIMELINE

      ZeroMemory(&mii, sizeof(mii));
      mii.cbSize = sizeof(mii);
      mii.fMask = MIIM_TYPE|MIIM_SUBMENU;
      mii.fType = MFT_STRING;
      mii.hSubMenu = hmenu_timeline;
      mii.dwTypeData = (LPWSTR)menu[IDM_TIMELINE].c_str();
      InsertMenuItem(hmenu, ++main_menu_id, FALSE, &mii);

      {
        UINT pos = 0;

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_ID;
        mii.fType = MFT_STRING;
        mii.wID = IDM_TIMELINE_NEXT;
        mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
        InsertMenuItem(hmenu_timeline, ++pos, FALSE, &mii);

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_ID;
        mii.fType = MFT_STRING;
        mii.wID = IDM_TIMELINE_PREV;
        mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
        InsertMenuItem(hmenu_timeline, ++pos, FALSE, &mii);

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE;
        mii.fType = MFT_SEPARATOR;
        InsertMenuItem(hmenu_timeline, ++pos, FALSE, &mii);

        HMENU hmenu_timeline_auto = CreateMenu();

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_SUBMENU;
        mii.fType = MFT_STRING;
        mii.hSubMenu = hmenu_timeline_auto;
        mii.dwTypeData = (LPWSTR)menu[IDM_TIMELINE_AUTO_RUN].c_str();
        InsertMenuItem(hmenu_timeline, ++pos, FALSE, &mii);

        {
          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_TIMELINE_STOP;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_timeline_auto, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE;
          mii.fType = MFT_SEPARATOR;
          InsertMenuItem(hmenu_timeline_auto, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_TIMELINE_FORWARD_1;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_timeline_auto, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_TIMELINE_FORWARD_2;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_timeline_auto, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_TIMELINE_FORWARD_3;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_timeline_auto, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_TIMELINE_FORWARD_4;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_timeline_auto, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE;
          mii.fType = MFT_SEPARATOR;
          InsertMenuItem(hmenu_timeline_auto, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_TIMELINE_BACKWARD_1;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_timeline_auto, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_TIMELINE_BACKWARD_2;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_timeline_auto, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_TIMELINE_BACKWARD_3;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_timeline_auto, ++pos, FALSE, &mii);

          ZeroMemory(&mii, sizeof(mii));
          mii.cbSize = sizeof(mii);
          mii.fMask = MIIM_TYPE|MIIM_ID;
          mii.fType = MFT_STRING;
          mii.wID = IDM_TIMELINE_BACKWARD_4;
          mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
          InsertMenuItem(hmenu_timeline_auto, ++pos, FALSE, &mii);
        }

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE;
        mii.fType = MFT_SEPARATOR;
        InsertMenuItem(hmenu_timeline, ++pos, FALSE, &mii);

        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE|MIIM_ID;
        mii.fType = MFT_STRING;
        mii.wID = IDM_TIMELINE_SHOW_SLIDER;
        mii.dwTypeData = (LPWSTR)menu[mii.wID].c_str();
        InsertMenuItem(hmenu_timeline, ++pos, FALSE, &mii);
      }

      SetMenu(hwnd, hmenu);
      FreeLibrary(handle);
    }


    ////////////////////////////////////////////////////////////////////////////////
    void set_message(wchar_t const* filename) {

      wstring path = wstring(L"dll\\")+filename;

      HMODULE handle = LoadLibrary(path.c_str());

      if (handle==0)
        throw path;

      FuncLoadMessage func = (FuncLoadMessage)GetProcAddress(handle, "load_message");

      if (func==0)
        throw path.c_str();

      vector<wstring> message;
      message.resize(Message::NUMBER_OF_MESSAGES);

      func(message);

      message_object.reset(new Message);
      message_object->set(message);
    }


    ////////////////////////////////////////////////////////////////////////////////
    Message const& get_message_object(void) {
      assert(message_object!=0);
      return *message_object;
    }


    ////////////////////////////////////////////////////////////////////////////////
    wchar_t const* get_message(int message_id) {
      assert(message_object!=0);
      return message_object->get(message_id);
    }

  } // The end of the namespace "sociarium_project_menu_and_message"

} // The end of the namespace "hashimoto_ut"

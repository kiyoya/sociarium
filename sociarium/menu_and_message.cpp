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
#ifdef _MSC_VER
#include <unordered_map>
#include <windows.h>
#else
#include <tr1/unordered_map>
#endif
#include "menu_and_message.h"
#include "resource.h"

namespace hashimoto_ut {

  using std::vector;
  using std::wstring;
  using std::tr1::shared_ptr;
  using std::tr1::unordered_map;

  namespace sociarium_project_menu_and_message {

#ifdef _MSC_VER
    typedef __declspec(dllimport)
      void (__cdecl* FuncLoadMessage)(vector<wstring>& message);

    typedef __declspec(dllimport)
      void (__cdecl* FuncLoadMenu)(unordered_map<int, wstring>& menu);
#else
    typedef void (*FuncLoadMessage)(vector<wstring>& message);
    typedef void (*FuncLoadMenu)(unordered_map<int, wstring>& menu);
#endif

    namespace {
      shared_ptr<Message> message_object;
    }

    ////////////////////////////////////////////////////////////////////////////////
#ifdef __APPLE__
    // Implemented at menu_and_message.mm
    HMENU get_file_menu(HMENU hmenu);
    unsigned int get_file_menu_pos(HMENU hmenu);
    HMENU get_main_menu();
    void create_menuitem(int mii, HMENU hmenu, unsigned int& menu_pos, /*const */unordered_map<int, wstring>& menu, int menu_id);
    void create_separator(int mii, HMENU hmenu, unsigned int& menu_pos);
    HMENU create_submenu(int mii, HMENU hmenu, unsigned int& menu_pos, /*const */unordered_map<int, wstring>& menu, int menu_id);
#elif _MSC_VER
    inline void create_menuitem(MENUITEMINFO& mii, HMENU hmenu, UINT& menu_pos, /*const */unordered_map<int, wstring>& menu, int menu_id) {
      ZeroMemory(&mii, sizeof(mii));
      mii.cbSize = sizeof(mii);
      mii.fMask = MIIM_TYPE|MIIM_ID;
      mii.fType = MFT_STRING;
      mii.wID = menu_id;
      mii.dwTypeData = (LPWSTR)menu[menu_id].c_str();
      InsertMenuItem(hmenu, ++menu_pos, FALSE, &mii);
    }

    inline void create_separator(MENUITEMINFO& mii, HMENU hmenu, UINT& menu_pos) {
      ZeroMemory(&mii, sizeof(mii));
      mii.cbSize = sizeof(mii);
      mii.fMask = MIIM_TYPE;
      mii.fType = MFT_SEPARATOR;
      InsertMenuItem(hmenu, ++menu_pos, FALSE, &mii);
    }

    inline HMENU create_submenu(MENUITEMINFO& mii, HMENU hmenu, UINT& menu_pos, /*const */unordered_map<int, wstring>& menu, int menu_id) {
      HMENU hsubmenu = CreateMenu();
      ZeroMemory(&mii, sizeof(mii));
      mii.cbSize = sizeof(mii);
      mii.fMask = MIIM_TYPE|MIIM_SUBMENU;
      mii.fType = MFT_STRING;
      mii.hSubMenu = hsubmenu;
      mii.dwTypeData = (LPWSTR)menu[menu_id].c_str();
      InsertMenuItem(hmenu, ++menu_pos, FALSE, &mii);
      return hsubmenu;
    }
#else
#error Not implemented
#endif

    ////////////////////////////////////////////////////////////////////////////////
#ifdef __APPLE__
    void set_menu(void* hwnd, wchar_t const* filename) {

      CFBundleRef mainBundle = CFBundleGetMainBundle();
      CFURLRef pluginURL = CFBundleCopyBuiltInPlugInsURL(mainBundle);
      CFStringRef fn = CFStringCreateWithWString(kCFAllocatorDefault, filename, kCFStringEncodingUTF8);
      CFURLRef path = CFURLCreateCopyAppendingPathComponent(NULL, pluginURL, fn, FALSE);
      
      CFBundleRef bundle = CFBundleCreate(kCFAllocatorSystemDefault, path);
      CFRelease(path);
      CFRelease(fn);
      CFRelease(pluginURL);
      
      if (bundle==NULL)
        throw filename;
      
      FuncLoadMenu func = reinterpret_cast<FuncLoadMenu>(CFBundleGetFunctionPointerForName(bundle, CFSTR("load_menu")));
      
      if (func==NULL) {
        CFRelease(bundle);
        throw filename;
      }
#elif _MSC_VER
    void set_menu(HWND hwnd, wchar_t const* filename) {

      wstring path = wstring(L"dll\\")+filename;

      HMODULE handle = LoadLibrary(path.c_str());

      FuncLoadMenu func = (FuncLoadMenu)GetProcAddress(handle, "load_menu");

      if (func==0)
        throw path.c_str();
#endif

      unordered_map<int, wstring> menu;

      func(menu);

#ifdef __APPLE__
      HMENU hmenu = get_main_menu();
      
      unsigned int main_menu_id = get_file_menu_pos(hmenu);
      
      int mii = NULL;
#elif _MSC_VER
      HMENU hmenu = CreateMenu();

      UINT main_menu_id = 0;

      MENUITEMINFO mii;
#else
#error Not implemented
#endif

      ////////////////////////////////////////////////////////////////////////////////
      // FILE

#ifdef __APPLE__
      HMENU hmenu_file = get_file_menu(hmenu);
      ++main_menu_id;
#else
      HMENU hmenu_file = create_submenu(mii, hmenu, main_menu_id, menu, IDM_FILE);
#endif

      {
#ifndef __APPLE__
        unsigned int pos = 0;

        create_menuitem(mii, hmenu_file, pos, menu, IDM_FILE_CANCEL);

        create_separator(mii, hmenu_file, pos);

        create_menuitem(mii, hmenu_file, pos, menu, IDM_FILE_QUIT);
#endif
      }


      ////////////////////////////////////////////////////////////////////////////////
      // EDIT

      HMENU hmenu_edit = create_submenu(mii, hmenu, main_menu_id, menu, IDM_EDIT);

      {
        unsigned int pos = 0;

        HMENU hmenu_edit_select_current = create_submenu(mii, hmenu_edit, pos, menu, IDM_EDIT_MARK_ON_CURRENT_LAYER);

        {
#ifdef __APPLE__
          unsigned int pos = 0;
#endif
          create_menuitem(mii, hmenu_edit_select_current, pos, menu, IDM_EDIT_MARK_ALL_NODES_ON_CURRENT_LAYER);

          create_menuitem(mii, hmenu_edit_select_current, pos, menu, IDM_EDIT_MARK_ALL_EDGES_ON_CURRENT_LAYER);

          create_separator(mii, hmenu_edit_select_current, pos);

          create_menuitem(mii, hmenu_edit_select_current, pos, menu, IDM_EDIT_MARK_NODES_INSIDE_COMMUNITY_ON_CURRENT_LAYER);

          create_menuitem(mii, hmenu_edit_select_current, pos, menu, IDM_EDIT_MARK_EDGES_INSIDE_COMMUNITY_ON_CURRENT_LAYER);

          create_menuitem(mii, hmenu_edit_select_current, pos, menu, IDM_EDIT_MARK_ELEMENTS_INSIDE_COMMUNITY_ON_CURRENT_LAYER);

          create_separator(mii, hmenu_edit_select_current, pos);

          create_menuitem(mii, hmenu_edit_select_current, pos, menu, IDM_EDIT_MARK_NODES_OUTSIDE_COMMUNITY_ON_CURRENT_LAYER);

          create_menuitem(mii, hmenu_edit_select_current, pos, menu, IDM_EDIT_MARK_EDGES_OUTSIDE_COMMUNITY_ON_CURRENT_LAYER);

          create_menuitem(mii, hmenu_edit_select_current, pos, menu, IDM_EDIT_MARK_ELEMENTS_OUTSIDE_COMMUNITY_ON_CURRENT_LAYER);

          create_separator(mii, hmenu_edit_select_current, pos);

          create_menuitem(mii, hmenu_edit_select_current, pos, menu, IDM_EDIT_MARK_NODES_IN_SELECTED_COMMUNITY_CONTINUUMS_ON_CURRENT_LAYER);

          create_menuitem(mii, hmenu_edit_select_current, pos, menu, IDM_EDIT_MARK_EDGES_IN_SELECTED_COMMUNITY_CONTINUUMS_ON_CURRENT_LAYER);

          create_menuitem(mii, hmenu_edit_select_current, pos, menu, IDM_EDIT_MARK_ELEMENTS_IN_SELECTED_COMMUNITY_CONTINUUMS_ON_CURRENT_LAYER);

          create_separator(mii, hmenu_edit_select_current, pos);

          create_menuitem(mii, hmenu_edit_select_current, pos, menu, IDM_EDIT_MARK_ALL_ELEMENTS_ON_CURRENT_LAYER);

          create_menuitem(mii, hmenu_edit_select_current, pos, menu, IDM_EDIT_INVERT_MARK_ON_CURRENT_LAYER);
        }

        HMENU hmenu_edit_select_all = create_submenu(mii, hmenu_edit, pos, menu, IDM_EDIT_MARK_ON_EACH_LAYER);

        {
#ifdef __APPLE__
          unsigned int pos = 0;
#endif
          create_menuitem(mii, hmenu_edit_select_all, pos, menu, IDM_EDIT_MARK_ALL_NODES_ON_EACH_LAYER);

          create_menuitem(mii, hmenu_edit_select_all, pos, menu, IDM_EDIT_MARK_ALL_EDGES_ON_EACH_LAYER);

          create_separator(mii, hmenu_edit_select_all, pos);

          create_menuitem(mii, hmenu_edit_select_all, pos, menu, IDM_EDIT_MARK_NODES_INSIDE_COMMUNITY_ON_EACH_LAYER);

          create_menuitem(mii, hmenu_edit_select_all, pos, menu, IDM_EDIT_MARK_EDGES_INSIDE_COMMUNITY_ON_EACH_LAYER);

          create_menuitem(mii, hmenu_edit_select_all, pos, menu, IDM_EDIT_MARK_ELEMENTS_INSIDE_COMMUNITY_ON_EACH_LAYER);

          create_separator(mii, hmenu_edit_select_all, pos);

          create_menuitem(mii, hmenu_edit_select_all, pos, menu, IDM_EDIT_MARK_NODES_OUTSIDE_COMMUNITY_ON_EACH_LAYER);

          create_menuitem(mii, hmenu_edit_select_all, pos, menu, IDM_EDIT_MARK_EDGES_OUTSIDE_COMMUNITY_ON_EACH_LAYER);

          create_menuitem(mii, hmenu_edit_select_all, pos, menu, IDM_EDIT_MARK_ELEMENTS_OUTSIDE_COMMUNITY_ON_EACH_LAYER);

          create_separator(mii, hmenu_edit_select_all, pos);

          create_menuitem(mii, hmenu_edit_select_all, pos, menu, IDM_EDIT_MARK_NODES_IN_SELECTED_COMMUNITY_CONTINUUMS_ON_EACH_LAYER);

          create_menuitem(mii, hmenu_edit_select_all, pos, menu, IDM_EDIT_MARK_EDGES_IN_SELECTED_COMMUNITY_CONTINUUMS_ON_EACH_LAYER);

          create_menuitem(mii, hmenu_edit_select_all, pos, menu, IDM_EDIT_MARK_ELEMENTS_IN_SELECTED_COMMUNITY_CONTINUUMS_ON_EACH_LAYER);

          create_separator(mii, hmenu_edit_select_all, pos);

          create_menuitem(mii, hmenu_edit_select_all, pos, menu, IDM_EDIT_MARK_ALL_ELEMENTS_ON_EACH_LAYER);

          create_menuitem(mii, hmenu_edit_select_all, pos, menu, IDM_EDIT_INVERT_MARK_ON_EACH_LAYER);
        }

        create_separator(mii, hmenu_edit, pos);

        HMENU hmenu_edit_hide = create_submenu(mii, hmenu_edit, pos, menu, IDM_EDIT_HIDE_MARKED_ELEMENTS);

        {
#ifdef __APPLE__
          unsigned int pos = 0;
#endif
          create_menuitem(mii, hmenu_edit_hide, pos, menu, IDM_EDIT_HIDE_MARKED_NODES_ON_CURRENT_LAYER);

          create_menuitem(mii, hmenu_edit_hide, pos, menu, IDM_EDIT_HIDE_MARKED_EDGES_ON_CURRENT_LAYER);

          create_menuitem(mii, hmenu_edit_hide, pos, menu, IDM_EDIT_HIDE_MARKED_ELEMENTS_ON_CURRENT_LAYER);

          create_separator(mii, hmenu_edit_hide, pos);

          create_menuitem(mii, hmenu_edit_hide, pos, menu, IDM_EDIT_HIDE_MARKED_NODES_ON_EACH_LAYER);

          create_menuitem(mii, hmenu_edit_hide, pos, menu, IDM_EDIT_HIDE_MARKED_EDGES_ON_EACH_LAYER);

          create_menuitem(mii, hmenu_edit_hide, pos, menu, IDM_EDIT_HIDE_MARKED_ELEMENTS_ON_EACH_LAYER);
        }

        HMENU hmenu_edit_hide_all = create_submenu(mii, hmenu_edit, pos, menu, IDM_EDIT_HIDE_MARKED_ELEMENTS_ALL);

        {
#ifdef __APPLE__
          unsigned int pos = 0;
#endif
          create_menuitem(mii, hmenu_edit_hide_all, pos, menu, IDM_EDIT_HIDE_MARKED_NODES_ON_CURRENT_LAYER_ALL);

          create_menuitem(mii, hmenu_edit_hide_all, pos, menu, IDM_EDIT_HIDE_MARKED_EDGES_ON_CURRENT_LAYER_ALL);

          create_menuitem(mii, hmenu_edit_hide_all, pos, menu, IDM_EDIT_HIDE_MARKED_ELEMENTS_ON_CURRENT_LAYER_ALL);

          create_separator(mii, hmenu_edit_hide_all, pos);

          create_menuitem(mii, hmenu_edit_hide_all, pos, menu, IDM_EDIT_HIDE_MARKED_NODES_ON_EACH_LAYER_ALL);

          create_menuitem(mii, hmenu_edit_hide_all, pos, menu, IDM_EDIT_HIDE_MARKED_EDGES_ON_EACH_LAYER_ALL);

          create_menuitem(mii, hmenu_edit_hide_all, pos, menu, IDM_EDIT_HIDE_MARKED_ELEMENTS_ON_EACH_LAYER_ALL);
        }

        HMENU hmenu_edit_show = create_submenu(mii, hmenu_edit, pos, menu, IDM_EDIT_SHOW_HIDDEN_ELEMENTS);

        {
#ifdef __APPLE__
          unsigned int pos = 0;
#endif
          create_menuitem(mii, hmenu_edit_show, pos, menu, IDM_EDIT_SHOW_HIDDEN_NODES_ON_CURRENT_LAYER);

          create_menuitem(mii, hmenu_edit_show, pos, menu, IDM_EDIT_SHOW_HIDDEN_EDGES_ON_CURRENT_LAYER);

          create_menuitem(mii, hmenu_edit_show, pos, menu, IDM_EDIT_SHOW_HIDDEN_ELEMENTS_ON_CURRENT_LAYER);

          create_separator(mii, hmenu_edit_show, pos);

          create_menuitem(mii, hmenu_edit_show, pos, menu, IDM_EDIT_SHOW_HIDDEN_NODES_ON_EACH_LAYER);

          create_menuitem(mii, hmenu_edit_show, pos, menu, IDM_EDIT_SHOW_HIDDEN_EDGES_ON_EACH_LAYER);

          create_menuitem(mii, hmenu_edit_show, pos, menu, IDM_EDIT_SHOW_HIDDEN_ELEMENTS_ON_EACH_LAYER);
        }

        create_separator(mii, hmenu_edit, pos);

        HMENU hmenu_edit_remove = create_submenu(mii, hmenu_edit, pos, menu, IDM_EDIT_REMOVE_MARKED_ELEMENTS);

        {
#ifdef __APPLE__
          unsigned int pos = 0;
#endif
          create_menuitem(mii, hmenu_edit_remove, pos, menu, IDM_EDIT_REMOVE_MARKED_NODES_ON_CURRENT_LAYER);

          create_menuitem(mii, hmenu_edit_remove, pos, menu, IDM_EDIT_REMOVE_MARKED_EDGES_ON_CURRENT_LAYER);

          create_menuitem(mii, hmenu_edit_remove, pos, menu, IDM_EDIT_REMOVE_MARKED_ELEMENTS_ON_CURRENT_LAYER);

          create_separator(mii, hmenu_edit_remove, pos);

          create_menuitem(mii, hmenu_edit_remove, pos, menu, IDM_EDIT_REMOVE_MARKED_NODES_ON_EACH_LAYER);

          create_menuitem(mii, hmenu_edit_remove, pos, menu, IDM_EDIT_REMOVE_MARKED_EDGES_ON_EACH_LAYER);

          create_menuitem(mii, hmenu_edit_remove, pos, menu, IDM_EDIT_REMOVE_MARKED_ELEMENTS_ON_EACH_LAYER);
        }

        HMENU hmenu_edit_remove_all = create_submenu(mii, hmenu_edit, pos, menu, IDM_EDIT_REMOVE_MARKED_ELEMENTS_ALL);

        {
#ifdef __APPLE__
          unsigned int pos = 0;
#endif
          create_menuitem(mii, hmenu_edit_remove_all, pos, menu, IDM_EDIT_REMOVE_MARKED_NODES_ON_CURRENT_LAYER_ALL);

          create_menuitem(mii, hmenu_edit_remove_all, pos, menu, IDM_EDIT_REMOVE_MARKED_EDGES_ON_CURRENT_LAYER_ALL);

          create_menuitem(mii, hmenu_edit_remove_all, pos, menu, IDM_EDIT_REMOVE_MARKED_ELEMENTS_ON_CURRENT_LAYER_ALL);

          create_separator(mii, hmenu_edit_remove_all, pos);

          create_menuitem(mii, hmenu_edit_remove_all, pos, menu, IDM_EDIT_REMOVE_MARKED_NODES_ON_EACH_LAYER_ALL);

          create_menuitem(mii, hmenu_edit_remove_all, pos, menu, IDM_EDIT_REMOVE_MARKED_EDGES_ON_EACH_LAYER_ALL);

          create_menuitem(mii, hmenu_edit_remove_all, pos, menu, IDM_EDIT_REMOVE_MARKED_ELEMENTS_ON_EACH_LAYER_ALL);
        }
      }


      ////////////////////////////////////////////////////////////////////////////////
      // VIEW

      HMENU hmenu_view = create_submenu(mii, hmenu, main_menu_id, menu, IDM_VIEW);

      {
        unsigned int pos = 0;

        create_menuitem(mii, hmenu_view, pos, menu, IDM_VIEW_SHOW_NODE);

        create_menuitem(mii, hmenu_view, pos, menu, IDM_VIEW_SHOW_EDGE);

        create_menuitem(mii, hmenu_view, pos, menu, IDM_VIEW_SHOW_COMMUNITY);

        create_menuitem(mii, hmenu_view, pos, menu, IDM_VIEW_SHOW_COMMUNITY_EDGE);

        create_separator(mii, hmenu_view, pos);

        HMENU hmenu_view_node_size = create_submenu(mii, hmenu_view, pos, menu, IDM_VIEW_NODE_SIZE);

        {
#ifdef __APPLE__
          unsigned int pos = 0;
#endif
          create_menuitem(mii, hmenu_view_node_size, pos, menu, IDM_VIEW_NODE_SIZE_UPDATE);

          create_menuitem(mii, hmenu_view_node_size, pos, menu, IDM_VIEW_NODE_SIZE_CANCEL);

          create_separator(mii, hmenu_view_node_size, pos);

          create_menuitem(mii, hmenu_view_node_size, pos, menu, IDM_VIEW_NODE_SIZE_UNIFORM);

          create_menuitem(mii, hmenu_view_node_size, pos, menu, IDM_VIEW_NODE_SIZE_WEIGHT);

          create_menuitem(mii, hmenu_view_node_size, pos, menu, IDM_VIEW_NODE_SIZE_DEGREE_CENTRALITY);

          create_menuitem(mii, hmenu_view_node_size, pos, menu, IDM_VIEW_NODE_SIZE_CLOSENESS_CENTRALITY);

          create_menuitem(mii, hmenu_view_node_size, pos, menu, IDM_VIEW_NODE_SIZE_BETWEENNESS_CENTRALITY);

          create_menuitem(mii, hmenu_view_node_size, pos, menu, IDM_VIEW_NODE_SIZE_PAGERANK);

          create_menuitem(mii, hmenu_view_node_size, pos, menu, IDM_VIEW_NODE_SIZE_POINT);
        }

        HMENU hmenu_view_edge_width = create_submenu(mii, hmenu_view, pos, menu, IDM_VIEW_EDGE_WIDTH);

        {
#ifdef __APPLE__
          unsigned int pos = 0;
#endif
          create_menuitem(mii, hmenu_view_edge_width, pos, menu, IDM_VIEW_EDGE_WIDTH_UPDATE);

          create_menuitem(mii, hmenu_view_edge_width, pos, menu, IDM_VIEW_EDGE_WIDTH_CANCEL);

          create_separator(mii, hmenu_view_edge_width, pos);

          create_menuitem(mii, hmenu_view_edge_width, pos, menu, IDM_VIEW_EDGE_WIDTH_UNIFORM);

          create_menuitem(mii, hmenu_view_edge_width, pos, menu, IDM_VIEW_EDGE_WIDTH_WEIGHT);

          create_menuitem(mii, hmenu_view_edge_width, pos, menu, IDM_VIEW_EDGE_WIDTH_BETWEENNESS_CENTRALITY);
        }

        create_separator(mii, hmenu_view, pos);
        
        HMENU hmenu_view_node_style = create_submenu(mii, hmenu_view, pos, menu, IDM_VIEW_NODE_STYLE);

        {
#ifdef __APPLE__
          unsigned int pos = 0;
#endif
          create_menuitem(mii, hmenu_view_node_style, pos, menu, IDM_VIEW_NODE_STYLE_POLYGON);

          create_menuitem(mii, hmenu_view_node_style, pos, menu, IDM_VIEW_NODE_STYLE_TEXTURE);
        }

        HMENU hmenu_view_edge_style = create_submenu(mii, hmenu_view, pos, menu, IDM_VIEW_EDGE_STYLE);

        {
#ifdef __APPLE__
          unsigned int pos = 0;
#endif
          create_menuitem(mii, hmenu_view_edge_style, pos, menu, IDM_VIEW_EDGE_STYLE_LINE);

          create_menuitem(mii, hmenu_view_edge_style, pos, menu, IDM_VIEW_EDGE_STYLE_POLYGON);
        }

        HMENU hmenu_view_community_style = create_submenu(mii, hmenu_view, pos, menu, IDM_VIEW_COMMUNITY_STYLE);

        {
#ifdef __APPLE__
          unsigned int pos = 0;
#endif
          create_menuitem(mii, hmenu_view_community_style, pos, menu, IDM_VIEW_COMMUNITY_STYLE_POLYGON_CIRCLE);

          create_menuitem(mii, hmenu_view_community_style, pos, menu, IDM_VIEW_COMMUNITY_STYLE_TEXTURE);
        }

        HMENU hmenu_view_community_edge_style = create_submenu(mii, hmenu_view, pos, menu, IDM_VIEW_COMMUNITY_EDGE_STYLE);

        {
#ifdef __APPLE__
          unsigned int pos = 0;
#endif
          create_menuitem(mii, hmenu_view_community_edge_style, pos, menu, IDM_VIEW_COMMUNITY_EDGE_STYLE_LINE);

          create_menuitem(mii, hmenu_view_community_edge_style, pos, menu, IDM_VIEW_COMMUNITY_EDGE_STYLE_POLYGON);
        }
      }


      ////////////////////////////////////////////////////////////////////////////////
      // STRING

      HMENU hmenu_string = create_submenu(mii, hmenu, main_menu_id, menu, IDM_STRING);

      {
        unsigned int pos = 0;

        create_menuitem(mii, hmenu_string, pos, menu, IDM_STRING_SHOW_NODE_NAME);

        create_menuitem(mii, hmenu_string, pos, menu, IDM_STRING_SHOW_EDGE_NAME);

        create_menuitem(mii, hmenu_string, pos, menu, IDM_STRING_SHOW_COMMUNITY_NAME);

        create_menuitem(mii, hmenu_string, pos, menu, IDM_STRING_SHOW_COMMUNITY_EDGE_NAME);

        create_separator(mii, hmenu_string, pos);

        HMENU hmenu_string_node_name_size = create_submenu(mii, hmenu_string, pos, menu, IDM_STRING_NODE_NAME_SIZE);

        {
#ifdef __APPLE__
          unsigned int pos = 0;
#endif
          create_menuitem(mii, hmenu_string_node_name_size, pos, menu, IDM_STRING_NODE_NAME_SIZE_VARIABLE);

          create_separator(mii, hmenu_string_node_name_size, pos);

          create_menuitem(mii, hmenu_string_node_name_size, pos, menu, IDM_STRING_NODE_NAME_SIZE_0);

          create_menuitem(mii, hmenu_string_node_name_size, pos, menu, IDM_STRING_NODE_NAME_SIZE_1);

          create_menuitem(mii, hmenu_string_node_name_size, pos, menu, IDM_STRING_NODE_NAME_SIZE_2);

          create_menuitem(mii, hmenu_string_node_name_size, pos, menu, IDM_STRING_NODE_NAME_SIZE_3);

          create_menuitem(mii, hmenu_string_node_name_size, pos, menu, IDM_STRING_NODE_NAME_SIZE_4);
        }

        HMENU hmenu_string_edge_name_size = create_submenu(mii, hmenu_string, pos, menu, IDM_STRING_EDGE_NAME_SIZE);

        {
#ifdef __APPLE__
          unsigned int pos = 0;
#endif
          create_menuitem(mii, hmenu_string_edge_name_size, pos, menu, IDM_STRING_EDGE_NAME_SIZE_VARIABLE);

          create_separator(mii, hmenu_string_edge_name_size, pos);

          create_menuitem(mii, hmenu_string_edge_name_size, pos, menu, IDM_STRING_EDGE_NAME_SIZE_0);

          create_menuitem(mii, hmenu_string_edge_name_size, pos, menu, IDM_STRING_EDGE_NAME_SIZE_1);

          create_menuitem(mii, hmenu_string_edge_name_size, pos, menu, IDM_STRING_EDGE_NAME_SIZE_2);

          create_menuitem(mii, hmenu_string_edge_name_size, pos, menu, IDM_STRING_EDGE_NAME_SIZE_3);

          create_menuitem(mii, hmenu_string_edge_name_size, pos, menu, IDM_STRING_EDGE_NAME_SIZE_4);
        }

        HMENU hmenu_string_community_name_size = create_submenu(mii, hmenu_string, pos, menu, IDM_STRING_COMMUNITY_NAME_SIZE);

        {
#ifdef __APPLE__
          unsigned int pos = 0;
#endif
          create_menuitem(mii, hmenu_string_community_name_size, pos, menu, IDM_STRING_COMMUNITY_NAME_SIZE_VARIABLE);

          create_separator(mii, hmenu_string_community_name_size, pos);

          create_menuitem(mii, hmenu_string_community_name_size, pos, menu, IDM_STRING_COMMUNITY_NAME_SIZE_0);

          create_menuitem(mii, hmenu_string_community_name_size, pos, menu, IDM_STRING_COMMUNITY_NAME_SIZE_1);

          create_menuitem(mii, hmenu_string_community_name_size, pos, menu, IDM_STRING_COMMUNITY_NAME_SIZE_2);

          create_menuitem(mii, hmenu_string_community_name_size, pos, menu, IDM_STRING_COMMUNITY_NAME_SIZE_3);

          create_menuitem(mii, hmenu_string_community_name_size, pos, menu, IDM_STRING_COMMUNITY_NAME_SIZE_4);
        }

        HMENU hmenu_string_community_edge_name_size = create_submenu(mii, hmenu_string, pos, menu, IDM_STRING_COMMUNITY_EDGE_NAME_SIZE);

        {
#ifdef __APPLE__
          unsigned int pos = 0;
#endif
          create_menuitem(mii, hmenu_string_community_edge_name_size, pos, menu, IDM_STRING_COMMUNITY_EDGE_NAME_SIZE_VARIABLE);

          create_separator(mii, hmenu_string_community_edge_name_size, pos);

          create_menuitem(mii, hmenu_string_community_edge_name_size, pos, menu, IDM_STRING_COMMUNITY_EDGE_NAME_SIZE_0);

          create_menuitem(mii, hmenu_string_community_edge_name_size, pos, menu, IDM_STRING_COMMUNITY_EDGE_NAME_SIZE_1);

          create_menuitem(mii, hmenu_string_community_edge_name_size, pos, menu, IDM_STRING_COMMUNITY_EDGE_NAME_SIZE_2);

          create_menuitem(mii, hmenu_string_community_edge_name_size, pos, menu, IDM_STRING_COMMUNITY_EDGE_NAME_SIZE_3);

          create_menuitem(mii, hmenu_string_community_edge_name_size, pos, menu, IDM_STRING_COMMUNITY_EDGE_NAME_SIZE_4);
        }

        create_separator(mii, hmenu_string, pos);

        HMENU hmenu_string_type = create_submenu(mii, hmenu_string, pos, menu, IDM_STRING_FONT_TYPE);

        {
#ifdef __APPLE__
          unsigned int pos = 0;
#endif
          create_menuitem(mii, hmenu_string_type, pos, menu, IDM_STRING_FONT_TYPE_POLYGON);

          create_menuitem(mii, hmenu_string_type, pos, menu, IDM_STRING_FONT_TYPE_TEXTURE);
        }

        create_separator(mii, hmenu_string, pos);

        create_menuitem(mii, hmenu_string, pos, menu, IDM_STRING_SHOW_LAYER_NAME);

        create_menuitem(mii, hmenu_string, pos, menu, IDM_STRING_SHOW_FPS);
      }


      ////////////////////////////////////////////////////////////////////////////////
      // LAYOUT

      HMENU hmenu_layout = create_submenu(mii, hmenu, main_menu_id, menu, IDM_LAYOUT);

      {
        unsigned int pos = 0;

        create_menuitem(mii, hmenu_layout, pos, menu, IDM_LAYOUT_UPDATE);

        create_menuitem(mii, hmenu_layout, pos, menu, IDM_LAYOUT_CANCEL);

        create_separator(mii, hmenu_layout, pos);

        create_menuitem(mii, hmenu_layout, pos, menu, IDM_LAYOUT_KAMADA_KAWAI_METHOD);

        HMENU hmenu_layout_hde = create_submenu(mii, hmenu_layout, pos, menu, IDM_LAYOUT_HIGH_DIMENSIONAL_EMBEDDING);

        {
#ifdef __APPLE__
          unsigned int pos = 0;
#endif
          create_menuitem(mii, hmenu_layout_hde, pos, menu, IDM_LAYOUT_HIGH_DIMENSIONAL_EMBEDDING_1_2);

          create_menuitem(mii, hmenu_layout_hde, pos, menu, IDM_LAYOUT_HIGH_DIMENSIONAL_EMBEDDING_1_3);

          create_menuitem(mii, hmenu_layout_hde, pos, menu, IDM_LAYOUT_HIGH_DIMENSIONAL_EMBEDDING_2_3);
        }

        create_separator(mii, hmenu_layout, pos);

        create_menuitem(mii, hmenu_layout, pos, menu, IDM_LAYOUT_CIRCLE);

        create_menuitem(mii, hmenu_layout, pos, menu, IDM_LAYOUT_CIRCLE_IN_SIZE_ORDER);

        create_menuitem(mii, hmenu_layout, pos, menu, IDM_LAYOUT_LATTICE);

        create_menuitem(mii, hmenu_layout, pos, menu, IDM_LAYOUT_RANDOM);

        create_menuitem(mii, hmenu_layout, pos, menu, IDM_LAYOUT_CARTOGRAMS);

        create_separator(mii, hmenu_layout, pos);

        HMENU hmenu_layout_fde = create_submenu(mii, hmenu_layout, pos, menu, IDM_LAYOUT_FORCE_DIRECTION);

        {
#ifdef __APPLE__
          unsigned int pos = 0;
#endif
          create_menuitem(mii, hmenu_layout_fde, pos, menu, IDM_LAYOUT_FORCE_DIRECTION_RUN);

          create_separator(mii, hmenu_layout_fde, pos);

          create_menuitem(mii, hmenu_layout_fde, pos, menu, IDM_LAYOUT_FORCE_DIRECTION_KAMADA_KAWAI_METHOD);

          create_menuitem(mii, hmenu_layout_fde, pos, menu, IDM_LAYOUT_FORCE_DIRECTION_KAMADA_KAWAI_METHOD_WITH_COMMUNITY_SEPARATION);

          create_menuitem(mii, hmenu_layout_fde, pos, menu, IDM_LAYOUT_FORCE_DIRECTION_COMMUNITY_ORIENTED);

          create_separator(mii, hmenu_layout_fde, pos);

          create_menuitem(mii, hmenu_layout_fde, pos, menu, IDM_LAYOUT_FORCE_DIRECTION_SPRING_AND_REPULSIVE_FORCE);

          create_menuitem(mii, hmenu_layout_fde, pos, menu, IDM_LAYOUT_FORCE_DIRECTION_LATTICE_GAS_METHOD);
#if 0
          create_menuitem(mii, hmenu_layout_fde, pos, menu, IDM_LAYOUT_FORCE_DIRECTION_DESIGNTIDE);
#endif
        }

        create_separator(mii, hmenu_layout, pos);

        create_menuitem(mii, hmenu_layout, pos, menu, IDM_LAYOUT_SHOW_LAYOUT_FRAME);

        create_menuitem(mii, hmenu_layout, pos, menu, IDM_LAYOUT_INITIALIZE_LAYOUT_FRAME);

        create_separator(mii, hmenu_layout, pos);

        create_menuitem(mii, hmenu_layout, pos, menu, IDM_LAYOUT_SHOW_GRID);

        create_menuitem(mii, hmenu_layout, pos, menu, IDM_LAYOUT_SHOW_CENTER);

        create_separator(mii, hmenu_layout, pos);

        create_menuitem(mii, hmenu_layout, pos, menu, IDM_LAYOUT_INITIALIZE_EYEPOINT);

        create_menuitem(mii, hmenu_layout, pos, menu, IDM_LAYOUT_ZOOM_IN);

        create_menuitem(mii, hmenu_layout, pos, menu, IDM_LAYOUT_ZOOM_OUT);
      }


      ////////////////////////////////////////////////////////////////////////////////
      // COMMUNITY

      HMENU hmenu_community = create_submenu(mii, hmenu, main_menu_id, menu, IDM_COMMUNITY_DETECTION);

      {
        unsigned int pos = 0;

        create_menuitem(mii, hmenu_community, pos, menu, IDM_COMMUNITY_DETECTION_UPDATE);

        create_menuitem(mii, hmenu_community, pos, menu, IDM_COMMUNITY_DETECTION_CANCEL);

        create_separator(mii, hmenu_community, pos);

        HMENU hmenu_community_connected_components = create_submenu(mii, hmenu_community, pos, menu, IDM_COMMUNITY_DETECTION_CONNECTED_COMPONENTS);

        {
#ifdef __APPLE__
          unsigned int pos = 0;
#endif
          create_menuitem(mii, hmenu_community_connected_components, pos, menu, IDM_COMMUNITY_DETECTION_WEAKLY_CONNECTED_COMPONENTS);

          create_menuitem(mii, hmenu_community_connected_components, pos, menu, IDM_COMMUNITY_DETECTION_STRONGLY_CONNECTED_COMPONENTS);
        }

        HMENU hmenu_community_modularity_maximization = create_submenu(mii, hmenu_community, pos, menu, IDM_COMMUNITY_DETECTION_MODULARITY_MAXIMIZATION);

        {
#ifdef __APPLE__
          unsigned int pos = 0;
#endif
          create_menuitem(mii, hmenu_community_modularity_maximization, pos, menu, IDM_COMMUNITY_DETECTION_MODULARITY_MAXIMIZATION_USING_GREEDY_METHOD);

          create_menuitem(mii, hmenu_community_modularity_maximization, pos, menu, IDM_COMMUNITY_DETECTION_MODULARITY_MAXIMIZATION_USING_TEO_METHOD);

          create_separator(mii, hmenu_community_modularity_maximization, pos);

          create_menuitem(mii, hmenu_community_modularity_maximization, pos, menu, IDM_COMMUNITY_DETECTION_USE_WEIGHTED_MODULARITY);
        }

        HMENU hmenu_community_clique_percolaion = create_submenu(mii, hmenu_community, pos, menu, IDM_COMMUNITY_DETECTION_CLIQUE_PERCOLATION);

        {
#ifdef __APPLE__
          unsigned int pos = 0;
#endif
          create_menuitem(mii, hmenu_community_clique_percolaion, pos, menu, IDM_COMMUNITY_DETECTION_CLIQUE_PERCOLATION_3);

          create_menuitem(mii, hmenu_community_clique_percolaion, pos, menu, IDM_COMMUNITY_DETECTION_CLIQUE_PERCOLATION_4);

          create_menuitem(mii, hmenu_community_clique_percolaion, pos, menu, IDM_COMMUNITY_DETECTION_CLIQUE_PERCOLATION_5);
        }

        HMENU hmenu_community_others = create_submenu(mii, hmenu_community, pos, menu, IDM_COMMUNITY_DETECTION_OTHERS);

        {
#ifdef __APPLE__
          unsigned int pos = 0;
#endif
          create_menuitem(mii, hmenu_community_others, pos, menu, IDM_COMMUNITY_DETECTION_BETWEENNESS_CENTRALITY_SEPARATION);
#if 0
          create_menuitem(mii, hmenu_community_others, pos, menu, IDM_COMMUNITY_DETECTION_INFORMATION_FLOW_MAPPING);
#endif
        }

        create_separator(mii, hmenu_community, pos);

        create_menuitem(mii, hmenu_community, pos, menu, IDM_COMMUNITY_TRANSITION_DIAGRAM);

        create_menuitem(mii, hmenu_community, pos, menu, IDM_COMMUNITY_TRANSITION_DIAGRAM_SCOPE_WIDER);

        create_menuitem(mii, hmenu_community, pos, menu, IDM_COMMUNITY_TRANSITION_DIAGRAM_SCOPE_NARROWER);

        create_separator(mii, hmenu_community, pos);

        create_menuitem(mii, hmenu_community, pos, menu, IDM_COMMUNITY_DETECTION_CLEAR);
      }


      ////////////////////////////////////////////////////////////////////////////////
      // TIMELINE

      HMENU hmenu_timeline = create_submenu(mii, hmenu, main_menu_id, menu, IDM_TIMELINE);

      {
        unsigned int pos = 0;

        create_menuitem(mii, hmenu_timeline, pos, menu, IDM_TIMELINE_NEXT);

        create_menuitem(mii, hmenu_timeline, pos, menu, IDM_TIMELINE_PREV);

        create_separator(mii, hmenu_timeline, pos);

        HMENU hmenu_timeline_auto = create_submenu(mii, hmenu_timeline, pos, menu, IDM_TIMELINE_AUTO_RUN);

        {
#ifdef __APPLE__
          unsigned int pos = 0;
#endif
          create_menuitem(mii, hmenu_timeline_auto, pos, menu, IDM_TIMELINE_STOP);

          create_separator(mii, hmenu_timeline_auto, pos);

          create_menuitem(mii, hmenu_timeline_auto, pos, menu, IDM_TIMELINE_FORWARD_1);

          create_menuitem(mii, hmenu_timeline_auto, pos, menu, IDM_TIMELINE_FORWARD_2);

          create_menuitem(mii, hmenu_timeline_auto, pos, menu, IDM_TIMELINE_FORWARD_3);

          create_menuitem(mii, hmenu_timeline_auto, pos, menu, IDM_TIMELINE_FORWARD_4);

          create_separator(mii, hmenu_timeline_auto, pos);

          create_menuitem(mii, hmenu_timeline_auto, pos, menu, IDM_TIMELINE_BACKWARD_1);

          create_menuitem(mii, hmenu_timeline_auto, pos, menu, IDM_TIMELINE_BACKWARD_2);

          create_menuitem(mii, hmenu_timeline_auto, pos, menu, IDM_TIMELINE_BACKWARD_3);

          create_menuitem(mii, hmenu_timeline_auto, pos, menu, IDM_TIMELINE_BACKWARD_4);
        }

        create_separator(mii, hmenu_timeline, pos);

        create_menuitem(mii, hmenu_timeline, pos, menu, IDM_TIMELINE_SHOW_SLIDER);
      }

#ifdef __APPLE__
      CFRelease(bundle);
#elif _MSC_VER
      SetMenu(hwnd, hmenu);
      FreeLibrary(handle);
#else
#error Not implemented
#endif
    }

    ////////////////////////////////////////////////////////////////////////////////
    void set_message(wchar_t const* filename) {

#ifdef __APPLE__
      CFBundleRef mainBundle = CFBundleGetMainBundle();
      CFURLRef pluginURL = CFBundleCopyBuiltInPlugInsURL(mainBundle);
      CFStringRef fn = CFStringCreateWithWString(kCFAllocatorDefault, filename, kCFStringEncodingUTF8);
      CFURLRef path = CFURLCreateCopyAppendingPathComponent(NULL, pluginURL, fn, FALSE);
      
      CFBundleRef bundle = CFBundleCreate(kCFAllocatorSystemDefault, path);
      CFRelease(path);
      CFRelease(fn);
      CFRelease(pluginURL);
      
      if (bundle==NULL)
        throw filename;
      
      FuncLoadMessage func = (FuncLoadMessage)CFBundleGetFunctionPointerForName(bundle, CFSTR("load_message"));
      
      if (func==NULL) {
        CFRelease(bundle);
        throw filename;
      }
#elif _MSC_VER
      wstring path = wstring(L"dll\\")+filename;

      HMODULE handle = LoadLibrary(path.c_str());

      if (handle==0)
        throw path.c_str();

      FuncLoadMessage func = (FuncLoadMessage)GetProcAddress(handle, "load_message");

      if (func==0)
        throw path.c_str();
#else
#error Not implemented
#endif

      vector<wstring> message;
      message.resize(Message::NUMBER_OF_MESSAGES);

      func(message);

      message_object.reset(new Message);
      message_object->set(message);

#ifdef __APPLE__
      CFRelease(bundle);
#endif
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

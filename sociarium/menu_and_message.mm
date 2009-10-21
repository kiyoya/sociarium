//
//  menu_and_message.mm
//  sociarium
//
//  Created by YAMAGUCHI Kiyohiro on 2009-09-16.
//  Copyright 2009 YAMAGUCHI Kiyohiro. All rights reserved.
//

#ifdef __APPLE__
#import <CoreFoundation/CoreFoundation.h>
#import "SociariumAppDelegate.h"

#define FILE_MENU_TAG 1

typedef NSMenu* HMENU;

#import "win32api.h"
#import "menu_and_message.cpp"

namespace hashimoto_ut {
  
  using std::string;
  using std::vector;
  using std::wstring;
  using std::tr1::shared_ptr;
  using std::tr1::unordered_map;
  
  namespace sociarium_project_menu_and_message {
    
    NSString *stringWithWString(const wstring& str) {
      static NSCharacterSet* separatorSet = nil;
      if (!separatorSet) {
        separatorSet = [[NSCharacterSet characterSetWithCharactersInString:@"\t("] retain];
      }

      string s = wcs2mbcs(str.c_str(), str.length());
      NSString* t;
      t = [[[NSString alloc] initWithCString:s.c_str() encoding:NSUTF8StringEncoding] autorelease];
      t = [t stringByReplacingOccurrencesOfString:@"&" withString:@""];

      NSRange r = [t rangeOfCharacterFromSet:separatorSet];
      if (r.location != NSNotFound) {
        t = [t substringToIndex:r.location];
      }

      return t;
    }
    
    HMENU get_file_menu(HMENU hmenu) {
      return [[hmenu itemWithTag:FILE_MENU_TAG] submenu];
    }
    
    unsigned int get_file_menu_pos(HMENU hmenu) {
      return [hmenu indexOfItemWithTag:FILE_MENU_TAG];
    }
    
    HMENU get_main_menu() {
      return reinterpret_cast<HMENU>([NSApp mainMenu]);
    }
    
    void create_menuitem(int mii, HMENU hmenu, unsigned int& menu_pos, /*const */unordered_map<int, wstring>& menu, int menu_id) {
      NSString* title = stringWithWString(menu[menu_id]);
      NSMenuItem* menuItem = [[[NSMenuItem alloc] initWithTitle:title action:NULL keyEquivalent:@""] autorelease];
      [menuItem setTag:menu_id];
      SociariumAppDelegate *d = reinterpret_cast<SociariumAppDelegate *>([NSApp delegate]);
      if (d) {
        [menuItem setAction:@selector(doCommand:)];
        [menuItem setTarget:d]; 
      }
      [hmenu insertItem:menuItem atIndex:menu_pos];
      ++menu_pos;
    }
    
    void create_separator(int mii, HMENU hmenu, unsigned int& menu_pos) {
      [hmenu insertItem:[NSMenuItem separatorItem] atIndex:menu_pos];
      ++menu_pos;
    }
    
    HMENU create_submenu(int mii, HMENU hmenu, unsigned int& menu_pos, /*const */unordered_map<int, wstring>& menu, int menu_id) {
      NSString* title = stringWithWString(menu[menu_id]);
      NSMenu* subMenu = [[[NSMenu alloc] initWithTitle:title] autorelease];
      [subMenu setAutoenablesItems:YES];
      NSMenuItem* subMenuItem = [[[NSMenuItem alloc] initWithTitle:title action:NULL keyEquivalent:@""] autorelease];
      [subMenuItem setSubmenu:subMenu];
      [hmenu insertItem:subMenuItem atIndex:menu_pos];
      ++menu_pos;
      return subMenu;
    }
  }
}

#endif // __APPLE__

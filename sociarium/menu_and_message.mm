//
//  menu_and_message.mm
//  sociarium
//
//  Created by YAMAGUCHI Kiyohiro on 2009-09-16.
//  Copyright 2009 YAMAGUCHI Kiyohiro. All rights reserved.
//

#ifdef __APPLE__
#import "menu_and_message.cpp"
#import "win32api.h"

namespace hashimoto_ut {
  
  using std::string;
  using std::vector;
  using std::wstring;
  using std::tr1::shared_ptr;
  using std::tr1::unordered_map;
  
  namespace sociarium_project_menu_and_message {
    
    NSString *stringWithWString(const wstring& str) {
      string s = wcs2mbcs(str.c_str(), str.length());
      NSString* t = [[[NSString alloc] initWithCString:s.c_str() encoding:NSUTF8StringEncoding] autorelease];
      return [t stringByReplacingOccurrencesOfString:@"&" withString:@""];
    }
    
    void set_menu(void* window, wchar_t const* filename) {
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
      
      unordered_map<int, wstring> menu;
      
      func(menu);
      
      NSMenu* mainMenu = [NSApp mainMenu];
      NSInteger mainMenuIndex = [mainMenu indexOfItemWithTag:1];
      
      ////////////////////////////////////////////////////////////////////////////////
      // FILE

      NSMenuItem* fileMenuItem = [mainMenu itemWithTag:1];
      NSMenu* fileMenu = [fileMenuItem submenu];
      [fileMenu setTitle:stringWithWString(menu[IDM_FILE])];


      ////////////////////////////////////////////////////////////////////////////////
      // EDIT

      NSMenu* editMenu = [[[NSMenu alloc] initWithTitle:stringWithWString(menu[IDM_EDIT])] autorelease];
      {
        NSMenuItem *editMenuItem = [[NSMenuItem alloc] initWithTitle:@"" action:NULL keyEquivalent:@""];
        [editMenuItem setSubmenu:editMenu];
        [mainMenu insertItem:editMenuItem atIndex:(++mainMenuIndex)];
      }


      ////////////////////////////////////////////////////////////////////////////////
      // VIEW

      NSMenu* viewMenu = [[[NSMenu alloc] initWithTitle:stringWithWString(menu[IDM_VIEW])] autorelease];
      {
        NSMenuItem *viewMenuItem = [[NSMenuItem alloc] initWithTitle:@"" action:NULL keyEquivalent:@""];
        [viewMenuItem setSubmenu:viewMenu];
        [mainMenu insertItem:viewMenuItem atIndex:(++mainMenuIndex)];
      }
      
      
      ////////////////////////////////////////////////////////////////////////////////
      // STRING
      
      NSMenu* stringMenu = [[[NSMenu alloc] initWithTitle:stringWithWString(menu[IDM_STRING])] autorelease];
      {
        NSMenuItem *stringMenuItem = [[NSMenuItem alloc] initWithTitle:@"" action:NULL keyEquivalent:@""];
        [stringMenuItem setSubmenu:stringMenu];
        [mainMenu insertItem:stringMenuItem atIndex:(++mainMenuIndex)];
      }


      ////////////////////////////////////////////////////////////////////////////////
      // LAYOUT
      
      NSMenu* layoutMenu = [[[NSMenu alloc] initWithTitle:stringWithWString(menu[IDM_LAYOUT])] autorelease];
      {
        NSMenuItem *layoutMenuItem = [[NSMenuItem alloc] initWithTitle:@"" action:NULL keyEquivalent:@""];
        [layoutMenuItem setSubmenu:layoutMenu];
        [mainMenu insertItem:layoutMenuItem atIndex:(++mainMenuIndex)];
      }


      ////////////////////////////////////////////////////////////////////////////////
      // COMMUNITY

      NSMenu* communityMenu = [[[NSMenu alloc] initWithTitle:stringWithWString(menu[IDM_COMMUNITY_DETECTION])] autorelease];
      {
        NSMenuItem *communityMenuItem = [[NSMenuItem alloc] initWithTitle:@"" action:NULL keyEquivalent:@""];
        [communityMenuItem setSubmenu:communityMenu];
        [mainMenu insertItem:communityMenuItem atIndex:(++mainMenuIndex)];
      }
      
      
      ////////////////////////////////////////////////////////////////////////////////
      // TIMELINE

      NSMenu* timelineMenu = [[[NSMenu alloc] initWithTitle:stringWithWString(menu[IDM_TIMELINE])] autorelease];
      {
        NSMenuItem *timelineMenuItem = [[NSMenuItem alloc] initWithTitle:@"" action:NULL keyEquivalent:@""];
        [timelineMenuItem setSubmenu:timelineMenu];
        [mainMenu insertItem:timelineMenuItem atIndex:(++mainMenuIndex)];
      }

      CFRelease(bundle);
    }
    
    void set_message(wchar_t const *filename) {
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
        
      vector<wstring> message;
      message.resize(Message::NUMBER_OF_MESSAGES, wstring(L" "));
        
      func(message);
        
      message_object.reset(new Message);
      message_object->set(message);
      
      CFRelease(bundle);
    }
  }
}

#endif // __APPLE__

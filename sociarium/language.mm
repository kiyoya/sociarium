//
//  language.mm
//  sociarium
//
//  Created by YAMAGUCHI Kiyohiro on 2009-09-16.
//  Copyright 2009 YAMAGUCHI Kiyohiro. All rights reserved.
//

#import "language.cpp"
#import "win32api.h"

namespace hashimoto_ut {
  
  using std::vector;
  using std::wstring;
  using std::tr1::shared_ptr;
  using std::tr1::unordered_map;
  
  using namespace sociarium_project_common;
  
  namespace sociarium_project_language {
    
    void initialize(wchar_t const *filename) {
      CFBundleRef mainBundle = CFBundleGetMainBundle();
      CFURLRef pluginURL = CFBundleCopyBuiltInPlugInsURL(mainBundle);
      CFStringRef fn = CFStringCreateWithWString(kCFAllocatorDefault, filename, kCFStringEncodingUTF8);
      CFURLRef path = CFURLCreateCopyAppendingPathComponent(NULL, pluginURL, fn, FALSE);
      CFBundleRef bundle = CFBundleCreate(kCFAllocatorSystemDefault, path);
      if ( ! bundle) {
        sociarium_project_common::show_last_error(filename);
        exit(1);
      }
      
      {
        FuncLoadMessage func = (FuncLoadMessage)CFBundleGetFunctionPointerForName(bundle, CFSTR("load_message"));
        if ( ! func) {
          sociarium_project_common::show_last_error(filename);
          exit(1);
        }
        
        vector<wstring> message;
        message.resize(Message::NUMBER_OF_MESSAGES, wstring(L" "));
        
        func(message);
        
        message_object.reset(new Message);
        message_object->set(message);
      }
      
      {
        FuncLoadMenu func = (FuncLoadMenu)CFBundleGetFunctionPointerForName(bundle, CFSTR("load_menu"));
        
        if ( ! func) {
          sociarium_project_common::show_last_error(filename);
          exit(1);
        }
        
        unordered_map<int, wstring> menu;
        
        func(menu);
      }
      
      CFRelease(bundle);
      CFRelease(path);
      CFRelease(fn);
      CFRelease(pluginURL);
    }
  }
}

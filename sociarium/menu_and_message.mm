//
//  menu_and_message.mm
//  sociarium
//
//  Created by YAMAGUCHI Kiyohiro on 2009-09-16.
//  Copyright 2009 YAMAGUCHI Kiyohiro. All rights reserved.
//

#import "menu_and_message.cpp"
#import "win32api.h"

namespace hashimoto_ut {
  
  using std::vector;
  using std::wstring;
  using std::tr1::shared_ptr;
  using std::tr1::unordered_map;
  
  namespace sociarium_project_menu_and_message {
    
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

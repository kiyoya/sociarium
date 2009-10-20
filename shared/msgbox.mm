#ifdef __APPLE__
#import "common.h"
#import "msgbox.cpp"

namespace hashimoto_ut {

  using namespace sociarium_project_common;
  
  bool message_box(void* window, int type, char const* title, char const* fmt, ...) {
    NSWindow *docWindow = reinterpret_cast<NSWindow*>(window);
    if (docWindow) {
      if (type == mb_ok_cancel_error) {
        NSBeginAlertSheet([NSString stringWithCString:title encoding:NSUTF8StringEncoding],
                          @"OK", @"Cancel", nil, docWindow, nil, nil, nil, NULL,
                          [NSString stringWithCString:fmt encoding:NSUTF8StringEncoding]);
      }
      else if (type == mb_ok_cancel) {
        NSBeginAlertSheet([NSString stringWithCString:title encoding:NSUTF8StringEncoding],
                          @"OK", @"Cancel", nil, docWindow, nil, nil, nil, NULL,
                          [NSString stringWithCString:fmt encoding:NSUTF8StringEncoding]);
      }
      else //if (type == mb_notice || type == mb_info || type == mb_error)
      {
        NSBeginAlertSheet([NSString stringWithCString:title encoding:NSUTF8StringEncoding],
                          @"OK", nil, nil, docWindow, nil, nil, nil, NULL,
                          [NSString stringWithCString:fmt encoding:NSUTF8StringEncoding]);
      }
    }
#warning Not implemented
    return true;
  }
  
  bool message_box(void* window, int type, wchar_t const* title, wchar_t const* fmt, ...) {
#warning Not implemented
    return true;
  }

}

#endif

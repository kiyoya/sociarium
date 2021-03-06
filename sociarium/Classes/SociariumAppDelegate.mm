//
//  SociariumAppDelegate.mm
//  sociarium
//
//  Created by YAMAGUCHI Kiyohiro on 2009-09-16.
//  Copyright 2009 YAMAGUCHI Kiyohiro. All rights reserved.
//

#import "SociariumAppDelegate.h"
#import "SociariumDocument.h"
#import "common.h"
#import "menu_and_message.h"
#import "win32api.h"

using namespace hashimoto_ut;

@implementation SociariumAppDelegate

#pragma mark IBAction

- (IBAction) doCommand:(NSMenuItem *)menuItem;
{
  NSDocument *doc = [[NSDocumentController sharedDocumentController] currentDocument];
  if (doc) {
    SociariumDocument *d = reinterpret_cast<SociariumDocument *>(doc);
    if (d) {
      [d doCommand:menuItem.tag];
    }
  }
}

#pragma mark NSApplicationDelegate

- (void)applicationWillFinishLaunching:(NSNotification *)aNotification
{
  CFBundleRef mainBundle = CFBundleGetMainBundle();
  CFURLRef resourceURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
  CFURLRef resourceAbsoluteURL = CFURLCopyAbsoluteURL(resourceURL);
  sociarium_project_common::set_module_path(CFStringGetWString(CFURLGetString(resourceAbsoluteURL)));
  CFRelease(resourceAbsoluteURL);
  CFRelease(resourceURL);
  
#warning Not implemented
  
  try {
    sociarium_project_menu_and_message::set_message(L"LanguageEn.plugin");
  } catch (wchar_t const* errmsg) {
    exit(1);
  }
  
  try {
    sociarium_project_menu_and_message::set_menu(NULL, L"LanguageEn.plugin");
  } catch (wchar_t const* errmsg) {
    exit(1);
  }
}

#pragma mark NSMenuValidationProtocol

- (BOOL) validateMenuItem:(NSMenuItem *)menuItem
{
  NSDocument *doc = [[NSDocumentController sharedDocumentController] currentDocument];
  if (doc) {
    SociariumDocument *d = reinterpret_cast<SociariumDocument *>(doc);
    if (d) {
      return [d validateMenuItem:menuItem];
    }
  }
  return FALSE;
}

@end

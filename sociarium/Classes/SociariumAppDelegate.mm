//
//  SociariumAppDelegate.mm
//  sociarium
//
//  Created by YAMAGUCHI Kiyohiro on 2009-09-16.
//  Copyright 2009 YAMAGUCHI Kiyohiro. All rights reserved.
//

#import "SociariumAppDelegate.h"
#import "common.h"
#import "language.h"
#import "win32api.h"

using namespace hashimoto_ut;

@implementation SociariumAppDelegate

- (void)applicationWillFinishLaunching:(NSNotification *)aNotification
{
  CFBundleRef mainBundle = CFBundleGetMainBundle();
  CFURLRef resourceURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
  sociarium_project_common::set_module_path(CFStringGetWString(CFURLGetString(resourceURL)));
  CFRelease(resourceURL);
  
  sociarium_project_language::initialize(L"LanguageEn.plugin");
}

@end

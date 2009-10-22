//
//  SociariumView.h
//  sociarium
//
//  Created by YAMAGUCHI Kiyohiro on 2009-09-06.
//  Copyright 2009 YAMAGUCHI Kiyohiro. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "world.h"

@interface SociariumView : NSOpenGLView
{
  NSOpenGLContext *fullScreenContext;
  
  NSURL * fileURL;
  
  NSTimer * redrawTimer;
  
  hashimoto_ut::World *world_;
  NSObject * world_mutex;
}

- (IBAction) doCommand:(int)tag;

- (void) destroy;
- (void) timerDidFireRedraw:(NSTimer *)timer;
- (void) toggleFullscreen;

@property (nonatomic, copy) NSURL * fileURL;
@property (nonatomic, readonly) hashimoto_ut::World * world;

@end

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
  hashimoto_ut::World * world_;
}

@property (nonatomic, readonly) hashimoto_ut::World * world;

@end

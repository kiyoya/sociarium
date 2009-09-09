//
//  creation.mm
//  sociarium
//
//  Created by YAMAGUCHI Kiyohiro on 2009-09-06.
//  Copyright 2009 YAMAGUCHI Kiyohiro. All rights reserved.
//

#import <AppKit/NSOpenGL.h>
#import "creation.cpp"

namespace hashimoto_ut {
  void CreationThread::create_context(void) const {
    NSOpenGLContext *trg = [[NSOpenGLContext alloc] initWithCGLContextObj:rc_src];
    rc_trg = trg;
    [trg makeCurrentContext];
  }
  
  void CreationThread::terminate_context(void) const {
    [NSOpenGLContext clearCurrentContext];
    NSOpenGLContext * trg = (NSOpenGLContext *)rc_trg;
    [trg release];
  }
}

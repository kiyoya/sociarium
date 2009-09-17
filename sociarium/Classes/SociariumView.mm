//
//  SociariumView.mm
//  sociarium
//
//  Created by YAMAGUCHI Kiyohiro on 2009-09-06.
//  Copyright 2009 YAMAGUCHI Kiyohiro. All rights reserved.
//

#import "SociariumView.h"
#import "selection.h"
#import "win32api.h"
#import <string>

using namespace hashimoto_ut;

@implementation SociariumView

@synthesize fileURL;
@synthesize world = world_;

- (void) timerDidFireRedraw:(NSTimer *)timer
{
  [self setNeedsDisplay:YES];
}

#pragma mark NSOpenGLView

- (void)prepareOpenGL
{
  [super prepareOpenGL];
  
  World::destroy(world_);
  world_ = World::create(static_cast<CGLContextObj>([[self openGLContext] CGLContextObj]));
  
  if (redrawTimer)
  {
    [redrawTimer invalidate];
    [redrawTimer release], redrawTimer = nil;
  }
  redrawTimer = [NSTimer scheduledTimerWithTimeInterval:0.001f target:self selector:@selector(timerDidFireRedraw:) userInfo:nil repeats:YES];
  [redrawTimer retain];
  
  if (fileURL)
  {
    std::wstring filename(CFStringGetWString((CFStringRef)[fileURL path]));
    world_->create_graph(filename.c_str());
  }
}

- (void)reshape
{
  [super reshape];
  NSRect bounds = [self bounds];
  Vector2<int> wsize(bounds.size.width, bounds.size.height);
  world_->resize_window(wsize);
}

#pragma mark NSView

- (void)drawRect:(NSRect)aRect
{
  [[self openGLContext] makeCurrentContext];
  [[self openGLContext] setView:self];
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  world_->draw();
  glFinish();
  [[self openGLContext] flushBuffer];
  [super drawRect:aRect];
}

#pragma mark NSResponder

- (void)mouseDown:(NSEvent *)event
{
  NSPoint pt = [self convertPointToBase:[event locationInWindow]];
  world_->do_mouse_action(MouseAction::LBUTTON_DOWN, Vector2<int>(pt.x, pt.y), ([event modifierFlags]&NSControlKeyMask) ? MouseModifier::CONTROL : MouseModifier::NONE);
}

- (void)mouseDragged:(NSEvent *)event
{
  NSPoint pt = [self convertPointToBase:[event locationInWindow]];
  world_->do_mouse_action(MouseAction::LBUTTON_DRAG, Vector2<int>(pt.x, pt.y), ([event modifierFlags]&NSControlKeyMask) ? MouseModifier::CONTROL : MouseModifier::NONE);
}

- (void)mouseEntered:(NSEvent *)event
{
}

- (void)mouseExited:(NSEvent *)event
{
}

- (void)mouseMoved:(NSEvent *)event
{
  NSPoint pt = [self convertPointToBase:[event locationInWindow]];
	world_->select(Vector2<int>(pt.x, pt.y));
}

- (void)mouseUp:(NSEvent *)event
{
  NSPoint pt = [self convertPointToBase:[event locationInWindow]];
  world_->do_mouse_action(MouseAction::LBUTTON_UP, Vector2<int>(pt.x, pt.y), ([event modifierFlags]&NSControlKeyMask) ? MouseModifier::CONTROL : MouseModifier::NONE);
}

- (void)rightMouseDown:(NSEvent *)event
{
  NSPoint pt = [self convertPointToBase:[event locationInWindow]];
  world_->do_mouse_action(MouseAction::RBUTTON_DOWN, Vector2<int>(pt.x, pt.y), ([event modifierFlags]&NSControlKeyMask) ? MouseModifier::CONTROL : MouseModifier::NONE);
}

- (void)rightMouseDragged:(NSEvent *)event
{
  NSPoint pt = [self convertPointToBase:[event locationInWindow]];
  world_->do_mouse_action(MouseAction::RBUTTON_DRAG, Vector2<int>(pt.x, pt.y), ([event modifierFlags]&NSControlKeyMask) ? MouseModifier::CONTROL : MouseModifier::NONE);
}

- (void)rightMouseUp:(NSEvent *)event
{
  NSPoint pt = [self convertPointToBase:[event locationInWindow]];
  world_->do_mouse_action(MouseAction::RBUTTON_UP, Vector2<int>(pt.x, pt.y), ([event modifierFlags]&NSControlKeyMask) ? MouseModifier::CONTROL : MouseModifier::NONE);
}

#pragma mark NSObject

- (id)initWithFrame:(NSRect)frame
{
  if (self = [super initWithFrame:frame])
  {
    world_ = NULL;
  }
  return self;
}

- (void) dealloc
{
  if (redrawTimer)
  {
    [redrawTimer invalidate];
    [redrawTimer release];
  }
  
  World::destroy(world_), world_ = NULL;
  [super dealloc];
  [fileURL release];
}

@end

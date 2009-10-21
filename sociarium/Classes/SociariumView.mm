//
//  SociariumView.mm
//  sociarium
//
//  Created by YAMAGUCHI Kiyohiro on 2009-09-06.
//  Copyright 2009 YAMAGUCHI Kiyohiro. All rights reserved.
//

#import <tr1/memory>
#import "SociariumView.h"
#import "common.h"
#import "menu_and_message.h"
#import "msgbox.h"
#import "resource.h"
#import "thread.h"
#import "selection.h"
#import "win32api.h"
#import "../../shared/thread.h"
#import <string>

using namespace hashimoto_ut;
using namespace sociarium_project_common;
using namespace sociarium_project_menu_and_message;
using std::tr1::shared_ptr;

@implementation SociariumView

@synthesize fileURL;
@synthesize world = world_;

- (IBAction) doCommand:(int)tag
{
  if (world_)
  {
    world_->do_command(tag);
  }
}

- (void) destroy
{
  @synchronized(world_mutex)
  {
    if (world_)
    {
      World::destroy(world_), world_ = NULL;
    }
  }
}

- (void) timerDidFireRedraw:(NSTimer *)timer
{
  [self setNeedsDisplay:YES];
}

#pragma mark NSOpenGLView

- (void)prepareOpenGL
{
  [super prepareOpenGL];

  @synchronized(world_mutex)
  {
    if ( ! world_mutex)
    {
      world_mutex = [NSObject new];
    }
  }
  
  [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(destroy) name:NSApplicationWillTerminateNotification object:NSApp];

  [self.window setAcceptsMouseMovedEvents:YES];

  CGLContextObj context = reinterpret_cast<CGLContextObj>([[self openGLContext] CGLContextObj]);

  // Enable Multi-threaded OpenGL Execution
  if (CGLEnable(context, kCGLCEMPEngine) != kCGLNoError)
  { }

  @synchronized(world_mutex)
  {
    if (world_)
    {
      World::destroy(world_);
    }
    hashimoto_ut::World *wo = World::create([self window], context);
    if (fileURL)
    {
      std::wstring filename(CFStringGetWString((CFStringRef)[fileURL path]));
      wo->create_graph(filename.c_str());
    }
    world_ = wo;
  }

  if (redrawTimer)
  {
    [redrawTimer invalidate];
    [redrawTimer release], redrawTimer = nil;
  }
  redrawTimer = [NSTimer scheduledTimerWithTimeInterval:0.001f target:self selector:@selector(timerDidFireRedraw:) userInfo:nil repeats:YES];
  [redrawTimer retain];
}

- (void)reshape
{
  [super reshape];
  if (world_)
  {
    NSRect bounds = [self bounds];
    Vector2<int> wsize(bounds.size.width, bounds.size.height);
    [[self openGLContext] makeCurrentContext];
    world_->resize_window(wsize);
  }
}

#pragma mark NSView

- (void)drawRect:(NSRect)aRect
{
  if (world_)
  {
    [[self openGLContext] makeCurrentContext];
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    world_->draw();
    glFinish();
    [[self openGLContext] flushBuffer];
  }
  [super drawRect:aRect];
}

#pragma mark NSResponder

- (BOOL)acceptsFirstResponder
{
  return YES;
}

- (void)mouseDown:(NSEvent *)event
{
  if (world_)
  {
    NSPoint pt = [self convertPointFromBase:[event locationInWindow]];
    [[self openGLContext] makeCurrentContext];
    world_->do_mouse_action(MouseAction::LBUTTON_DOWN, Vector2<int>(pt.x, pt.y), ([event modifierFlags]&NSControlKeyMask) ? MouseModifier::CONTROL : MouseModifier::NONE);
  }
}

- (void)mouseDragged:(NSEvent *)event
{
  if (world_)
  {
    NSPoint pt = [self convertPointFromBase:[event locationInWindow]];
    [[self openGLContext] makeCurrentContext];
    world_->do_mouse_action(MouseAction::LBUTTON_DRAG, Vector2<int>(pt.x, pt.y), ([event modifierFlags]&NSControlKeyMask) ? MouseModifier::CONTROL : MouseModifier::NONE);
  }
}

- (void)mouseEntered:(NSEvent *)event
{
}

- (void)mouseExited:(NSEvent *)event
{
}

- (void)mouseMoved:(NSEvent *)event
{
  if (world_)
  {
    NSPoint pt = [self convertPointFromBase:[event locationInWindow]];
    NSSize size = self.bounds.size;
    if (0 <= pt.x && pt.x <= size.width && 0 <= pt.y && pt.y <= size.height) {
      [[self openGLContext] makeCurrentContext];
      world_->select(Vector2<int>((int)pt.x, (int)pt.y));
    }
  }
}

- (void)mouseUp:(NSEvent *)event
{
  if (world_)
  {
    NSPoint pt = [self convertPointFromBase:[event locationInWindow]];
    [[self openGLContext] makeCurrentContext];
    world_->do_mouse_action(MouseAction::LBUTTON_UP, Vector2<int>(pt.x, pt.y), ([event modifierFlags]&NSControlKeyMask) ? MouseModifier::CONTROL : MouseModifier::NONE);
  }
}

- (void)rightMouseDown:(NSEvent *)event
{
  if (world_)
  {
    NSPoint pt = [self convertPointFromBase:[event locationInWindow]];
    [[self openGLContext] makeCurrentContext];
    world_->do_mouse_action(MouseAction::RBUTTON_DOWN, Vector2<int>(pt.x, pt.y), ([event modifierFlags]&NSControlKeyMask) ? MouseModifier::CONTROL : MouseModifier::NONE);
  }
}

- (void)rightMouseDragged:(NSEvent *)event
{
  if (world_)
  {
    NSPoint pt = [self convertPointFromBase:[event locationInWindow]];
    [[self openGLContext] makeCurrentContext];
    world_->do_mouse_action(MouseAction::RBUTTON_DRAG, Vector2<int>(pt.x, pt.y), ([event modifierFlags]&NSControlKeyMask) ? MouseModifier::CONTROL : MouseModifier::NONE);
  }
}

- (void)rightMouseUp:(NSEvent *)event
{
  if (world_)
  {
    NSPoint pt = [self convertPointFromBase:[event locationInWindow]];
    [[self openGLContext] makeCurrentContext];
    world_->do_mouse_action(MouseAction::RBUTTON_UP, Vector2<int>(pt.x, pt.y), ([event modifierFlags]&NSControlKeyMask) ? MouseModifier::CONTROL : MouseModifier::NONE);
  }
}

- (void)scrollWheel:(NSEvent *)event
{
  if (world_)
  {
    NSPoint pt = [self convertPointFromBase:[event locationInWindow]];
    [[self openGLContext] makeCurrentContext];
    world_->do_mouse_action(MouseAction::WHEEL, Vector2<int>(pt.x, pt.y), -event.deltaY);
  }
}

#pragma mark NSObject

- (void) dealloc
{
  if (redrawTimer)
  {
    [redrawTimer invalidate];
    [redrawTimer release];
  }
  
  [self destroy];
  [world_mutex release];
  
  [super dealloc];
  [fileURL release];
}

@end

//
//  SociariumView.mm
//  sociarium
//
//  Created by YAMAGUCHI Kiyohiro on 2009-09-06.
//  Copyright 2009 YAMAGUCHI Kiyohiro. All rights reserved.
//

#import "SociariumView.h"
#import "mouse_and_selection.h"

using namespace hashimoto_ut;

@implementation SociariumView

@synthesize world = world_;

#pragma mark NSOpenGLView

- (void)prepareOpenGL
{
  if (world_)
  {
    delete world_;
  }
	
  world_ = new World([[self openGLContext] CGLContextObj]);
}

- (void)update
{
  world_->draw();
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
  delete world_, world_ = NULL;
  [super dealloc];
}

@end

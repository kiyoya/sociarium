//
//  MyDocument.m
//  sociarium
//
//  Created by YAMAGUCHI Kiyohiro on 2009-09-03.
//  Copyright 2009 YAMAGUCHI Kiyohiro. All rights reserved.
//

#import "SociariumDocument.h"
#import "algorithm_selector.h"
#import "view.h"
#import "win32api.h"
#import <string>

using namespace std;
using namespace hashimoto_ut;

@implementation SociariumDocument

@synthesize fileURL;

#pragma mark IBAction

// http://www.cocoadev.com/index.pl?CreatingPDFsFromNSOpenGLView
- (IBAction) copyScreenshotToClipboard:(id)sender
{
  NSSize imageSize = [sociarium bounds].size;
  imageSize.width = ( imageSize.width - ( ((int)imageSize.width) % 4 ));
  
  int imageWidth = imageSize.width;
  int imageHeight = imageSize.height;
  int bytesPerPixel = 4;
  int bytesPerImage = imageWidth * imageHeight * bytesPerPixel;
  unsigned char *imageBuffer = (unsigned char *)malloc(bytesPerImage);
  
  [[sociarium openGLContext] makeCurrentContext];
  glReadPixels(0, 0, imageWidth, imageHeight, GL_RGBA, GL_UNSIGNED_BYTE, imageBuffer);
  
  NSBitmapImageRep *rep = [[NSBitmapImageRep alloc]
                           initWithBitmapDataPlanes:nil
                           pixelsWide:imageWidth
                           pixelsHigh:imageHeight
                           bitsPerSample:8
                           samplesPerPixel:3
                           hasAlpha:NO
                           isPlanar:NO
                           colorSpaceName:NSCalibratedRGBColorSpace
                           bytesPerRow:0
                           bitsPerPixel:0];
  
  unsigned char *src, *end, *dest;
  src = imageBuffer;
  end = src + bytesPerImage;
  dest = [rep bitmapData];
  
  while ( src < end )
  {
    *dest = *src; dest++; src++; //R
    *dest = *src; dest++; src++; //G
    *dest = *src; dest++; src++; //B
    ++src;                       //A
  }
  
  NSImage *image = [[NSImage alloc] init];
  [image addRepresentation:rep];
  
  [image setFlipped:YES];
  [image lockFocusOnRepresentation:rep];
  [image unlockFocus];
  
  NSImage *flipped = [[NSImage alloc] initWithData:[image TIFFRepresentation]];
  NSPasteboard *pb = [NSPasteboard generalPasteboard];
  [pb declareTypes:[NSArray arrayWithObjects:NSTIFFPboardType, nil] owner:nil];
  [pb setData:[flipped TIFFRepresentation] forType:NSTIFFPboardType];
  
  [rep release];
  [image release];
  free( imageBuffer );
  [flipped release];
}

- (IBAction) doCommand:(int)tag
{
  [sociarium doCommand:tag];
}

#pragma mark NSDocument

/*
- (NSData *)dataOfType:(NSString *)typeName error:(NSError **)outError
{
  if (outError)
  {
    *outError = [NSError errorWithDomain:NSOSStatusErrorDomain code:unimpErr userInfo:NULL];
  }
  return nil;
}
 */

- (BOOL)readFromURL:(NSURL *)absoluteURL ofType:(NSString *)typeName error:(NSError **)outError
{
  if ([absoluteURL isFileURL])
  {
    fileURL = [absoluteURL copy];
    
    return YES;
  }
  
  if (outError)
  {
    *outError = [NSError errorWithDomain:NSOSStatusErrorDomain code:unimpErr userInfo:NULL];
  }
  
  return NO;
}

- (NSString *)windowNibName
{
  return @"SociariumDocument";
}

- (void)windowControllerDidLoadNib:(NSWindowController *)aController
{
  [super windowControllerDidLoadNib:aController];
  
  sociarium.fileURL = fileURL;
}

#pragma mark NSObject

- (id)init
{
  if (self = [super init])
  {
  }
  return self;
}

- (void)dealloc
{
  [sociarium destroy];
  [super dealloc];
  [fileURL release];
}

@end

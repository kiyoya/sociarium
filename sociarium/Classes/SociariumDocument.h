//
//  MyDocument.h
//  sociarium
//
//  Created by YAMAGUCHI Kiyohiro on 2009-09-03.
//  Copyright 2009 YAMAGUCHI Kiyohiro. All rights reserved.
//


#import <Cocoa/Cocoa.h>
#import "SociariumView.h"

@interface SociariumDocument : NSDocument
{
  NSURL * fileURL;
  IBOutlet SociariumView * sociarium;
}

- (IBAction) copyScreenshotToClipboard:(id)sender;
- (IBAction) doCommand:(int)tag;
- (IBAction) toggleFullscreen:(id)sender;

@property (nonatomic, readonly) NSURL * fileURL;

@end

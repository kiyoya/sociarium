//
//  MyDocument.m
//  sociarium
//
//  Created by YAMAGUCHI Kiyohiro on 2009-09-03.
//  Copyright 2009 YAMAGUCHI Kiyohiro. All rights reserved.
//

#import "SociariumDocument.h"
#import "algorithm_selector.h"
#import "font.h"
#import "force_direction.h"
#import "resource.h"
#import "thread.h"
#import "timeline.h"
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

#pragma mark NSMenuValidationProtocol

- (BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
  using namespace sociarium_project_algorithm_selector;
  using namespace sociarium_project_font;
  using namespace sociarium_project_thread;
  using namespace sociarium_project_timeline;
  using namespace sociarium_project_view;
  
  switch (menuItem.tag)
  {
    case IDM_COMMUNITY_DETECTION_BETWEENNESS_CENTRALITY_SEPARATION:
      [menuItem setState:(get_community_detection_algorithm()==CommunityDetectionAlgorithm::BETWEENNESS_CENTRALITY_SEPARATION ? NSOnState : NSOffState)];
      break;
    case IDM_COMMUNITY_DETECTION_CANCEL:
      return joinable(COMMUNITY_DETECTION);
    case IDM_COMMUNITY_DETECTION_CLIQUE_PERCOLATION_3:
      [menuItem setState:(get_community_detection_algorithm()==CommunityDetectionAlgorithm::CLIQUE_PERCOLATION_3 ? NSOnState : NSOffState)];
      break;
    case IDM_COMMUNITY_DETECTION_CLIQUE_PERCOLATION_4:
      [menuItem setState:(get_community_detection_algorithm()==CommunityDetectionAlgorithm::CLIQUE_PERCOLATION_4 ? NSOnState : NSOffState)];
      break;
    case IDM_COMMUNITY_DETECTION_CLIQUE_PERCOLATION_5:
      [menuItem setState:(get_community_detection_algorithm()==CommunityDetectionAlgorithm::CLIQUE_PERCOLATION_5 ? NSOnState : NSOffState)];
      break;
    case IDM_COMMUNITY_DETECTION_INFORMATION_FLOW_MAPPING:
      [menuItem setState:(get_community_detection_algorithm()==CommunityDetectionAlgorithm::INFORMATION_FLOW_MAPPING ? NSOnState : NSOffState)];
      break;
    case IDM_COMMUNITY_DETECTION_MODULARITY_MAXIMIZATION_USING_GREEDY_METHOD:
      [menuItem setState:(get_community_detection_algorithm()==CommunityDetectionAlgorithm::MODULARITY_MAXIMIZATION_USING_GREEDY_METHOD ? NSOnState : NSOffState)];
      break;
    case IDM_COMMUNITY_DETECTION_MODULARITY_MAXIMIZATION_USING_TEO_METHOD:
      [menuItem setState:(get_community_detection_algorithm()==CommunityDetectionAlgorithm::MODULARITY_MAXIMIZATION_USING_TEO_METHOD ? NSOnState : NSOffState)];
      break;
    case IDM_COMMUNITY_DETECTION_STRONGLY_CONNECTED_COMPONENTS:
      [menuItem setState:(get_community_detection_algorithm()==CommunityDetectionAlgorithm::STRONGLY_CONNECTED_COMPONENTS ? NSOnState : NSOffState)];
      break;
    case IDM_COMMUNITY_DETECTION_USE_WEIGHTED_MODULARITY:
      [menuItem setState:(use_weighted_modularity() ? NSOnState : NSOffState)];
      break;
    case IDM_COMMUNITY_DETECTION_UPDATE:
      return !joinable(COMMUNITY_DETECTION);
    case IDM_COMMUNITY_DETECTION_WEAKLY_CONNECTED_COMPONENTS:
      [menuItem setState:(get_community_detection_algorithm()==CommunityDetectionAlgorithm::CONNECTED_COMPONENTS ? NSOnState : NSOffState)];
      break;
    case IDM_COMMUNITY_TRANSITION_DIAGRAM:
      [menuItem setState:(get_show_diagram() ? NSOnState : NSOffState)];
      break;
    case IDM_FILE_CANCEL:
      return (joinable(GRAPH_CREATION) || joinable(GRAPH_RETOUCH));
    case IDM_LAYOUT_CANCEL:
      return joinable(LAYOUT);
    case IDM_LAYOUT_CARTOGRAMS:
      [menuItem setState:(get_layout_algorithm()==LayoutAlgorithm::CARTOGRAMS ? NSOnState : NSOffState)];
      break;
    case IDM_LAYOUT_CIRCLE:
      [menuItem setState:(get_layout_algorithm()==LayoutAlgorithm::CIRCLE ? NSOnState : NSOffState)];
      break;
    case IDM_LAYOUT_CIRCLE_IN_SIZE_ORDER:
      [menuItem setState:(get_layout_algorithm()==LayoutAlgorithm::CIRCLE_IN_SIZE_ORDER ? NSOnState : NSOffState)];
      break;
    case IDM_LAYOUT_FORCE_DIRECTION_COMMUNITY_ORIENTED:
      [menuItem setState:(get_force_direction_algorithm()==RealTimeForceDirectionAlgorithm::COMMUNITY_ORIENTED ? NSOnState : NSOffState)];
      break;
    case IDM_LAYOUT_FORCE_DIRECTION_DESIGNTIDE:
      [menuItem setState:(get_force_direction_algorithm()==RealTimeForceDirectionAlgorithm::DESIGNTIDE ? NSOnState : NSOffState)];
      break;
    case IDM_LAYOUT_FORCE_DIRECTION_KAMADA_KAWAI_METHOD:
      [menuItem setState:(get_force_direction_algorithm()==RealTimeForceDirectionAlgorithm::KAMADA_KAWAI_METHOD ? NSOnState : NSOffState)];
      break;
    case IDM_LAYOUT_FORCE_DIRECTION_KAMADA_KAWAI_METHOD_WITH_COMMUNITY_SEPARATION:
      [menuItem setState:(get_force_direction_algorithm()==RealTimeForceDirectionAlgorithm::KAMADA_KAWAI_METHOD_WITH_COMMUNITY_SEPARATION ? NSOnState : NSOffState)];
      break;
    case IDM_LAYOUT_FORCE_DIRECTION_LATTICE_GAS_METHOD:
      [menuItem setState:(get_force_direction_algorithm()==RealTimeForceDirectionAlgorithm::LATTICE_GAS_METHOD ? NSOnState : NSOffState)];
      break;
    case IDM_LAYOUT_FORCE_DIRECTION_RUN:
      [menuItem setState:(sociarium_project_force_direction::is_active() ? NSOnState : NSOffState)];
      break;
    case IDM_LAYOUT_FORCE_DIRECTION_SPRING_AND_REPULSIVE_FORCE:
      [menuItem setState:(get_force_direction_algorithm()==RealTimeForceDirectionAlgorithm::SPRING_AND_REPULSIVE_FORCE ? NSOnState : NSOffState)];
      break;
    case IDM_LAYOUT_HIGH_DIMENSIONAL_EMBEDDING_1_2:
      [menuItem setState:(get_layout_algorithm()==LayoutAlgorithm::HIGH_DIMENSIONAL_EMBEDDING_1_2 ? NSOnState : NSOffState)];
      break;
    case IDM_LAYOUT_HIGH_DIMENSIONAL_EMBEDDING_1_3:
      [menuItem setState:(get_layout_algorithm()==LayoutAlgorithm::HIGH_DIMENSIONAL_EMBEDDING_1_3 ? NSOnState : NSOffState)];
      break;
    case IDM_LAYOUT_HIGH_DIMENSIONAL_EMBEDDING_2_3:
      [menuItem setState:(get_layout_algorithm()==LayoutAlgorithm::HIGH_DIMENSIONAL_EMBEDDING_2_3 ? NSOnState : NSOffState)];
      break;
    case IDM_LAYOUT_KAMADA_KAWAI_METHOD:
      [menuItem setState:(get_layout_algorithm()==LayoutAlgorithm::KAMADA_KAWAI_METHOD ? NSOnState : NSOffState)];
      break;
    case IDM_LAYOUT_LATTICE:
      [menuItem setState:(get_layout_algorithm()==LayoutAlgorithm::LATTICE ? NSOnState : NSOffState)];
      break;
    case IDM_LAYOUT_RANDOM:
      [menuItem setState:(get_layout_algorithm()==LayoutAlgorithm::RANDOM ? NSOnState : NSOffState)];
      break;
    case IDM_LAYOUT_SHOW_CENTER:
      [menuItem setState:(get_show_center() ? NSOnState : NSOffState)];
      break;
    case IDM_LAYOUT_SHOW_GRID:
      [menuItem setState:(get_show_grid() ? NSOnState : NSOffState)];
      break;
    case IDM_LAYOUT_SHOW_LAYOUT_FRAME:
      [menuItem setState:(get_show_layout_frame() ? NSOnState : NSOffState)];
      break;
    case IDM_LAYOUT_UPDATE:
      return !joinable(LAYOUT);
    case IDM_STRING_COMMUNITY_EDGE_NAME_SIZE_0:
      [menuItem setState:(get_font_scale(FontCategory::COMMUNITY_EDGE_NAME)==get_default_font_scale(0) ? NSOnState : NSOffState)];
      break;
    case IDM_STRING_COMMUNITY_EDGE_NAME_SIZE_1:
      [menuItem setState:(get_font_scale(FontCategory::COMMUNITY_EDGE_NAME)==get_default_font_scale(1) ? NSOnState : NSOffState)];
      break;
    case IDM_STRING_COMMUNITY_EDGE_NAME_SIZE_2:
      [menuItem setState:(get_font_scale(FontCategory::COMMUNITY_EDGE_NAME)==get_default_font_scale(2) ? NSOnState : NSOffState)];
      break;
    case IDM_STRING_COMMUNITY_EDGE_NAME_SIZE_3:
      [menuItem setState:(get_font_scale(FontCategory::COMMUNITY_EDGE_NAME)==get_default_font_scale(3) ? NSOnState : NSOffState)];
      break;
    case IDM_STRING_COMMUNITY_EDGE_NAME_SIZE_4:
      [menuItem setState:(get_font_scale(FontCategory::COMMUNITY_EDGE_NAME)==get_default_font_scale(4) ? NSOnState : NSOffState)];
      break;
    case IDM_STRING_COMMUNITY_EDGE_NAME_SIZE_VARIABLE:
      [menuItem setState:(get_community_edge_name_size_variable() ? NSOnState : NSOffState)];
      break;
    case IDM_STRING_COMMUNITY_NAME_SIZE_0:
      [menuItem setState:(get_font_scale(FontCategory::COMMUNITY_NAME)==get_default_font_scale(0) ? NSOnState : NSOffState)];
      break;
    case IDM_STRING_COMMUNITY_NAME_SIZE_1:
      [menuItem setState:(get_font_scale(FontCategory::COMMUNITY_NAME)==get_default_font_scale(1) ? NSOnState : NSOffState)];
      break;
    case IDM_STRING_COMMUNITY_NAME_SIZE_2:
      [menuItem setState:(get_font_scale(FontCategory::COMMUNITY_NAME)==get_default_font_scale(2) ? NSOnState : NSOffState)];
      break;
    case IDM_STRING_COMMUNITY_NAME_SIZE_3:
      [menuItem setState:(get_font_scale(FontCategory::COMMUNITY_NAME)==get_default_font_scale(3) ? NSOnState : NSOffState)];
      break;
    case IDM_STRING_COMMUNITY_NAME_SIZE_4:
      [menuItem setState:(get_font_scale(FontCategory::COMMUNITY_NAME)==get_default_font_scale(4) ? NSOnState : NSOffState)];
      break;
    case IDM_STRING_COMMUNITY_NAME_SIZE_VARIABLE:
      [menuItem setState:(get_community_name_size_variable() ? NSOnState : NSOffState)];
      break;
    case IDM_STRING_EDGE_NAME_SIZE_0:
      [menuItem setState:(get_font_scale(FontCategory::EDGE_NAME)==get_default_font_scale(0) ? NSOnState : NSOffState)];
      break;
    case IDM_STRING_EDGE_NAME_SIZE_1:
      [menuItem setState:(get_font_scale(FontCategory::EDGE_NAME)==get_default_font_scale(1) ? NSOnState : NSOffState)];
      break;
    case IDM_STRING_EDGE_NAME_SIZE_2:
      [menuItem setState:(get_font_scale(FontCategory::EDGE_NAME)==get_default_font_scale(2) ? NSOnState : NSOffState)];
      break;
    case IDM_STRING_EDGE_NAME_SIZE_3:
      [menuItem setState:(get_font_scale(FontCategory::EDGE_NAME)==get_default_font_scale(3) ? NSOnState : NSOffState)];
      break;
    case IDM_STRING_EDGE_NAME_SIZE_4:
      [menuItem setState:(get_font_scale(FontCategory::EDGE_NAME)==get_default_font_scale(4) ? NSOnState : NSOffState)];
      break;
    case IDM_STRING_EDGE_NAME_SIZE_VARIABLE:
      [menuItem setState:(get_edge_name_size_variable() ? NSOnState : NSOffState)];
      break;
    case IDM_STRING_FONT_TYPE_POLYGON:
      [menuItem setState:(get_font_type()==FontType::POLYGON_FONT ? NSOnState : NSOffState)];
      break;
    case IDM_STRING_FONT_TYPE_TEXTURE:
      [menuItem setState:(get_font_type()==FontType::TEXTURE_FONT ? NSOnState : NSOffState)];
      break;
    case IDM_STRING_NODE_NAME_SIZE_0:
      [menuItem setState:(get_font_scale(FontCategory::NODE_NAME)==get_default_font_scale(0) ? NSOnState : NSOffState)];
      break;
    case IDM_STRING_NODE_NAME_SIZE_1:
      [menuItem setState:(get_font_scale(FontCategory::NODE_NAME)==get_default_font_scale(1) ? NSOnState : NSOffState)];
      break;
    case IDM_STRING_NODE_NAME_SIZE_2:
      [menuItem setState:(get_font_scale(FontCategory::NODE_NAME)==get_default_font_scale(2) ? NSOnState : NSOffState)];
      break;
    case IDM_STRING_NODE_NAME_SIZE_3:
      [menuItem setState:(get_font_scale(FontCategory::NODE_NAME)==get_default_font_scale(3) ? NSOnState : NSOffState)];
      break;
    case IDM_STRING_NODE_NAME_SIZE_4:
      [menuItem setState:(get_font_scale(FontCategory::NODE_NAME)==get_default_font_scale(4) ? NSOnState : NSOffState)];
      break;
    case IDM_STRING_NODE_NAME_SIZE_VARIABLE:
      [menuItem setState:(get_node_name_size_variable() ? NSOnState : NSOffState)];
      break;
    case IDM_STRING_SHOW_COMMUNITY_EDGE_NAME:
      [menuItem setState:(get_show_community_edge_name() ? NSOnState : NSOffState)];
      break;
    case IDM_STRING_SHOW_COMMUNITY_NAME:
      [menuItem setState:(get_show_community_name() ? NSOnState : NSOffState)];
      break;
    case IDM_STRING_SHOW_EDGE_NAME:
      [menuItem setState:(get_show_edge_name() ? NSOnState : NSOffState)];
      break;
    case IDM_STRING_SHOW_FPS:
      [menuItem setState:(get_show_fps() ? NSOnState : NSOffState)];
      break;
    case IDM_STRING_SHOW_LAYER_NAME:
      [menuItem setState:(get_show_layer_name() ? NSOnState : NSOffState)];
      break;
    case IDM_STRING_SHOW_NODE_NAME:
      [menuItem setState:(get_show_node_name() ? NSOnState : NSOffState)];
      break;
    case IDM_TIMELINE_BACKWARD_1:
      [menuItem setState:(get_auto_run_id()==AutoRun::BACKWARD_1 ? NSOnState : NSOffState)];
      break;
    case IDM_TIMELINE_BACKWARD_2:
      [menuItem setState:(get_auto_run_id()==AutoRun::BACKWARD_2 ? NSOnState : NSOffState)];
      break;
    case IDM_TIMELINE_BACKWARD_3:
      [menuItem setState:(get_auto_run_id()==AutoRun::BACKWARD_3 ? NSOnState : NSOffState)];
      break;
    case IDM_TIMELINE_BACKWARD_4:
      [menuItem setState:(get_auto_run_id()==AutoRun::BACKWARD_4 ? NSOnState : NSOffState)];
      break;
    case IDM_TIMELINE_FORWARD_1:
      [menuItem setState:(get_auto_run_id()==AutoRun::FORWARD_1 ? NSOnState : NSOffState)];
      break;
    case IDM_TIMELINE_FORWARD_2:
      [menuItem setState:(get_auto_run_id()==AutoRun::FORWARD_2 ? NSOnState : NSOffState)];
      break;
    case IDM_TIMELINE_FORWARD_3:
      [menuItem setState:(get_auto_run_id()==AutoRun::FORWARD_3 ? NSOnState : NSOffState)];
      break;
    case IDM_TIMELINE_FORWARD_4:
      [menuItem setState:(get_auto_run_id()==AutoRun::FORWARD_4 ? NSOnState : NSOffState)];
      break;
    case IDM_TIMELINE_SHOW_SLIDER:
      [menuItem setState:(get_show_slider() ? NSOnState : NSOffState)];
      break;
    case IDM_TIMELINE_STOP:
      [menuItem setState:(get_auto_run_id()==AutoRun::STOP ? NSOnState : NSOffState)];
      break;
    case IDM_VIEW_COMMUNITY_EDGE_STYLE_LINE:
      [menuItem setState:(get_community_edge_style()&CommunityEdgeStyle::LINE ? NSOnState : NSOffState)];
      break;
    case IDM_VIEW_COMMUNITY_EDGE_STYLE_POLYGON:
      [menuItem setState:(get_community_edge_style()&CommunityEdgeStyle::POLYGON ? NSOnState : NSOffState)];
      break;
    case IDM_VIEW_COMMUNITY_STYLE_POLYGON_CIRCLE:
      [menuItem setState:(get_community_style()&CommunityStyle::POLYGON_CIRCLE ? NSOnState : NSOffState)];
      break;
    case IDM_VIEW_COMMUNITY_STYLE_TEXTURE:
      [menuItem setState:(get_community_style()&CommunityStyle::TEXTURE ? NSOnState : NSOffState)];
      break;
    case IDM_VIEW_EDGE_STYLE_LINE:
      [menuItem setState:(get_edge_style()&EdgeStyle::LINE ? NSOnState : NSOffState)];
      break;
    case IDM_VIEW_EDGE_STYLE_POLYGON:
      [menuItem setState:(get_edge_style()&EdgeStyle::POLYGON ? NSOnState : NSOffState)];
      break;
    case IDM_VIEW_EDGE_WIDTH_BETWEENNESS_CENTRALITY:
      [menuItem setState:(get_edge_width_update_algorithm()==EdgeWidthUpdateAlgorithm::BETWEENNESS_CENTRALITY ? NSOnState : NSOffState)];
      break;
    case IDM_VIEW_EDGE_WIDTH_CANCEL:
      return joinable(EDGE_WIDTH_UPDATE);
    case IDM_VIEW_EDGE_WIDTH_UNIFORM:
      [menuItem setState:(get_edge_width_update_algorithm()==EdgeWidthUpdateAlgorithm::UNIFORM ? NSOnState : NSOffState)];
      break;
    case IDM_VIEW_EDGE_WIDTH_UPDATE:
      return !joinable(EDGE_WIDTH_UPDATE);
    case IDM_VIEW_EDGE_WIDTH_WEIGHT:
      [menuItem setState:(get_edge_width_update_algorithm()==EdgeWidthUpdateAlgorithm::WEIGHT ? NSOnState : NSOffState)];
      break;
    case IDM_VIEW_NODE_SIZE_BETWEENNESS_CENTRALITY:
      [menuItem setState:(get_node_size_update_algorithm()==NodeSizeUpdateAlgorithm::BETWEENNESS_CENTRALITY ? NSOnState : NSOffState)];
      break;
    case IDM_VIEW_NODE_SIZE_CANCEL:
      return joinable(NODE_SIZE_UPDATE);
    case IDM_VIEW_NODE_SIZE_CLOSENESS_CENTRALITY:
      [menuItem setState:(get_node_size_update_algorithm()==NodeSizeUpdateAlgorithm::CLOSENESS_CENTRALITY ? NSOnState : NSOffState)];
      break;
    case IDM_VIEW_NODE_SIZE_DEGREE_CENTRALITY:
      [menuItem setState:(get_node_size_update_algorithm()==NodeSizeUpdateAlgorithm::DEGREE_CENTRALITY ? NSOnState : NSOffState)];
      break;
    case IDM_VIEW_NODE_SIZE_PAGERANK:
      [menuItem setState:(get_node_size_update_algorithm()==NodeSizeUpdateAlgorithm::PAGERANK ? NSOnState : NSOffState)];
      break;
    case IDM_VIEW_NODE_SIZE_POINT:
      [menuItem setState:(get_node_size_update_algorithm()==NodeSizeUpdateAlgorithm::POINT ? NSOnState : NSOffState)];
      break;
    case IDM_VIEW_NODE_SIZE_UNIFORM:
      [menuItem setState:(get_node_size_update_algorithm()==NodeSizeUpdateAlgorithm::UNIFORM ? NSOnState : NSOffState)];
      break;
    case IDM_VIEW_NODE_SIZE_UPDATE:
      return !joinable(NODE_SIZE_UPDATE);
    case IDM_VIEW_NODE_SIZE_WEIGHT:
      [menuItem setState:(get_node_size_update_algorithm()==NodeSizeUpdateAlgorithm::WEIGHT ? NSOnState : NSOffState)];
      break;
    case IDM_VIEW_NODE_STYLE_POLYGON:
      [menuItem setState:(get_node_style()&NodeStyle::POLYGON ? NSOnState : NSOffState)];
      break;
    case IDM_VIEW_NODE_STYLE_TEXTURE:
      [menuItem setState:(get_node_style()&NodeStyle::TEXTURE ? NSOnState : NSOffState)];
      break;
    case IDM_VIEW_SHOW_COMMUNITY:
      [menuItem setState:(get_show_community() ? NSOnState : NSOffState)];
      break;
    case IDM_VIEW_SHOW_COMMUNITY_EDGE:
      [menuItem setState:(get_show_community_edge() ? NSOnState : NSOffState)];
      break;
    case IDM_VIEW_SHOW_EDGE:
      [menuItem setState:(get_show_edge() ? NSOnState : NSOffState)];
      break;
    case IDM_VIEW_SHOW_NODE:
      [menuItem setState:(get_show_node() ? NSOnState : NSOffState)];
      break;
    default:
      break;
  }
  return YES;
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

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

#pragma mark IBActions

- (IBAction)cancelRunningThread:(id)sender
{
}

- (IBAction)clearCommunityDetection:(id)sender
{
  sociarium.world->clear_community();
}

- (IBAction)executeLayout:(id)sender
{
  sociarium.world->layout();
}

- (IBAction)executeCommunityDetection:(id)sender
{
  sociarium.world->detect_community();
}

- (IBAction)initializeView:(id)sender
{
  sociarium.world->initialize_view();
}

- (IBAction)initializeViewport:(id)sender
{
}

- (IBAction)setAlgoritumOfCommunityDetectionToConnectedComponents:(id)sender
{
  sociarium_project_algorithm_selector::set_community_detection_algorithm(sociarium_project_algorithm_selector::CommunityDetectionAlgorithm::CONNECTED_COMPONENTS);
}

- (IBAction)setAlgoritumOfCommunityDetectionToStronglyConnectedComponents:(id)sender
{
  sociarium_project_algorithm_selector::set_community_detection_algorithm(sociarium_project_algorithm_selector::CommunityDetectionAlgorithm::STRONGLY_CONNECTED_COMPONENTS);
}

- (IBAction)setAlgoritumOfCommunityDetectionToCliquePercolation3:(id)sender
{
  sociarium_project_algorithm_selector::set_community_detection_algorithm(sociarium_project_algorithm_selector::CommunityDetectionAlgorithm::CLIQUE_PERCOLATION_3);
}

- (IBAction)setAlgoritumOfCommunityDetectionToCliquePercolation4:(id)sender
{
}

- (IBAction)setAlgoritumOfCommunityDetectionToCliquePercolation5:(id)sender
{
}

- (IBAction)setAlgoritumOfCommunityDetectionToModularityMaximizationUsingGreedyMethod:(id)sender
{
  sociarium_project_algorithm_selector::set_community_detection_algorithm(sociarium_project_algorithm_selector::CommunityDetectionAlgorithm::MODULARITY_MAXIMIZATION_USING_GREEDY_METHOD);
}

- (IBAction)setAlgoritumOfCommunityDetectionToModularityMaximizationUsingTauEoMethod:(id)sender
{
  sociarium_project_algorithm_selector::set_community_detection_algorithm(sociarium_project_algorithm_selector::CommunityDetectionAlgorithm::MODULARITY_MAXIMIZATION_USING_TEO_METHOD);
}

- (IBAction)setAlgoritumOfCommunityDetectionToBetweennessCentralityClustering:(id)sender
{
  sociarium_project_algorithm_selector::set_community_detection_algorithm(sociarium_project_algorithm_selector::CommunityDetectionAlgorithm::BETWEENNESS_CENTRALITY_SEPARATION);
}

- (IBAction)setAlgoritumOfGraphLayoutToKamadaKawaiMethod:(id)sender
{
  sociarium_project_algorithm_selector::set_layout_algorithm(sociarium_project_algorithm_selector::LayoutAlgorithm::KAMADA_KAWAI_METHOD);
}

/*
- (IBAction)setAlgoritumOfGraphLayoutToHighDimensionalEmbedding:(id)sender
{
  sociarium_project_algorithm_selector::set_layout_algorithm(sociarium_project_algorithm_selector::LayoutAlgorithm::HDE);
}
 */

- (IBAction)setAlgoritumOfGraphLayoutToCircle:(id)sender
{
  sociarium_project_algorithm_selector::set_layout_algorithm(sociarium_project_algorithm_selector::LayoutAlgorithm::CIRCLE);
}

- (IBAction)setAlgoritumOfGraphLayoutToCircleInSizeOrder:(id)sender
{
  sociarium_project_algorithm_selector::set_layout_algorithm(sociarium_project_algorithm_selector::LayoutAlgorithm::CIRCLE_IN_SIZE_ORDER);
}

/*
- (IBAction)setAlgoritumOfGraphLayoutToArray:(id)sender
{
  sociarium_project_algorithm_selector::set_layout_algorithm(sociarium_project_algorithm_selector::LayoutAlgorithm::ARRAY);
}
 */

- (IBAction)setAlgoritumOfGraphLayoutToRandom:(id)sender
{
  sociarium_project_algorithm_selector::set_layout_algorithm(sociarium_project_algorithm_selector::LayoutAlgorithm::RANDOM);
}

- (IBAction)setFontSizeOfCommunity:(id)sender
{
}

- (IBAction)setFontSizeOfCommunityEdge:(id)sender
{
}

- (IBAction)setFontSizeOfEdge:(id)sender
{
}

- (IBAction)setFontSizeOfNode:(id)sender
{
}

- (IBAction)setSizeOfNode:(id)sender
{
}

- (IBAction)setStyleOfCommunityEdgeToLine:(id)sender
{  
  sociarium_project_view::set_community_edge_style(CommunityEdgeStyle::LINE);
}

- (IBAction)setStyleOfCommunityEdgeToPolygon:(id)sender
{
  sociarium_project_view::set_community_edge_style(CommunityEdgeStyle::POLYGON);
}

- (IBAction)setStyleOfCommunityToPolygon:(id)sender
{
  sociarium_project_view::set_community_style(CommunityStyle::POLYGON_CIRCLE);  
}

- (IBAction)setStyleOfCommunityToTexture:(id)sender
{  
  sociarium_project_view::set_community_style(CommunityStyle::TEXTURE);
}

- (IBAction)setStyleOfEdgeToLine:(id)sender
{
  sociarium_project_view::set_edge_style(EdgeStyle::LINE);  
}

- (IBAction)setStyleOfEdgeToPolygon:(id)sender
{  
  sociarium_project_view::set_edge_style(EdgeStyle::POLYGON);
}

- (IBAction)setStyleOfNodeToPolygon:(id)sender
{
  sociarium_project_view::set_node_style(NodeStyle::POLYGON);
}

- (IBAction)setStyleOfNodeToTexture:(id)sender
{
  sociarium_project_view::set_node_style(NodeStyle::TEXTURE);
}

- (IBAction)shiftStyle:(id)sender
{
}

- (IBAction)toggleFullscreen:(id)sender
{
}

- (IBAction)toggleUseOfWeightedModularity:(id)sender
{
  sociarium_project_algorithm_selector::use_weighted_modularity(!sociarium_project_algorithm_selector::use_weighted_modularity());
}

- (IBAction)toggleLabelVisibilityOfEdge:(id)sender
{
  sociarium_project_view::set_show_edge_name(!sociarium_project_view::get_show_edge_name());
}

- (IBAction)toggleLabelVisibilityOfNode:(id)sender
{
  sociarium_project_view::set_show_node_name(!sociarium_project_view::get_show_node_name());
}

- (IBAction)toggleLabelVisibilityOfCommunity:(id)sender
{
  sociarium_project_view::set_show_community_name(!sociarium_project_view::get_show_community_name());
}

- (IBAction)toggleLabelVisibilityOfCommunityEdge:(id)sender
{
  sociarium_project_view::set_show_community_edge_name(!sociarium_project_view::get_show_community_edge_name());
}

- (IBAction)toggleVisibilityOfEdge:(id)sender
{
}

- (IBAction)toggleVisibilityOfNode:(id)sender
{
}

- (IBAction)toggleVisibilityOfFPS:(id)sender
{
  sociarium_project_view::set_show_fps(!sociarium_project_view::get_show_fps());
}

- (IBAction)toggleVisibilityOfCommunity:(id)sender
{
}

- (IBAction)toggleVisibilityOfCommunityEdge:(id)sender
{
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
  [super dealloc];
  [fileURL release];
}

@end

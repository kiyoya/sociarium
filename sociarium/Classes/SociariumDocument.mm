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

using namespace hashimoto_ut;

@implementation SociariumDocument

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
  sociarium.world->layout_graph();
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
  sociarium_project_algorithm_selector::set_community_detection_algorithm(sociarium_project_algorithm_selector::CommunityDetectionAlgorithm::BETWEENNESS_CENTRALITY_CLUSTERING);
}

- (IBAction)setAlgoritumOfGraphLayoutToKamadaKawaiMethod:(id)sender
{
  sociarium_project_algorithm_selector::set_graph_layout_algorithm(sociarium_project_algorithm_selector::LayoutAlgorithm::KAMADA_KAWAI_METHOD);
}

- (IBAction)setAlgoritumOfGraphLayoutToHighDimensionalEmbedding:(id)sender
{
  sociarium_project_algorithm_selector::set_graph_layout_algorithm(sociarium_project_algorithm_selector::LayoutAlgorithm::HDE);
}

- (IBAction)setAlgoritumOfGraphLayoutToCircle:(id)sender
{
  sociarium_project_algorithm_selector::set_graph_layout_algorithm(sociarium_project_algorithm_selector::LayoutAlgorithm::CIRCLE);
}

- (IBAction)setAlgoritumOfGraphLayoutToCircleInSizeOrder:(id)sender
{
  sociarium_project_algorithm_selector::set_graph_layout_algorithm(sociarium_project_algorithm_selector::LayoutAlgorithm::CIRCLE_IN_SIZE_ORDER);
}

- (IBAction)setAlgoritumOfGraphLayoutToArray:(id)sender
{
  sociarium_project_algorithm_selector::set_graph_layout_algorithm(sociarium_project_algorithm_selector::LayoutAlgorithm::ARRAY);
}

- (IBAction)setAlgoritumOfGraphLayoutToRandom:(id)sender
{
  sociarium_project_algorithm_selector::set_graph_layout_algorithm(sociarium_project_algorithm_selector::LayoutAlgorithm::RANDOM);
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
  sociarium_project_view::set_community_edge_style(sociarium_project_view::CommunityEdgeView::Style::LINE);
}

- (IBAction)setStyleOfCommunityEdgeToTexture:(id)sender
{
  sociarium_project_view::set_community_edge_style(sociarium_project_view::CommunityEdgeView::Style::TEXTURE);
}

- (IBAction)setStyleOfCommunityToPolygon:(id)sender
{
  sociarium_project_view::set_community_style(sociarium_project_view::CommunityView::Style::POLYGON_CIRCLE);  
}

- (IBAction)setStyleOfCommunityToTexture:(id)sender
{  
  sociarium_project_view::set_community_style(sociarium_project_view::CommunityView::Style::TEXTURE);
}

- (IBAction)setStyleOfEdgeToLine:(id)sender
{
  sociarium_project_view::set_edge_style(sociarium_project_view::EdgeView::Style::LINE);  
}

- (IBAction)setStyleOfEdgeToTexture:(id)sender
{  
  sociarium_project_view::set_edge_style(sociarium_project_view::EdgeView::Style::TEXTURE);
}

- (IBAction)setStyleOfNodeToPolygon:(id)sender
{
  sociarium_project_view::set_node_style(sociarium_project_view::NodeView::Style::POLYGON);
}

- (IBAction)setStyleOfNodeToTexture:(id)sender
{
  sociarium_project_view::set_node_style(sociarium_project_view::NodeView::Style::TEXTURE);
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
  sociarium_project_view::set_show_edge_label(!sociarium_project_view::get_show_edge_label());
}

- (IBAction)toggleLabelVisibilityOfNode:(id)sender
{
  sociarium_project_view::set_show_node_label(!sociarium_project_view::get_show_node_label());
}

- (IBAction)toggleLabelVisibilityOfCommunity:(id)sender
{
  sociarium_project_view::set_show_community_label(!sociarium_project_view::get_show_community_label());
}

- (IBAction)toggleLabelVisibilityOfCommunityEdge:(id)sender
{
  sociarium_project_view::set_show_community_edge_label(!sociarium_project_view::get_show_community_edge_label());
}

- (IBAction)toggleVisibilityOfEdge:(id)sender
{
}

- (IBAction)toggleVisibilityOfNode:(id)sender
{
}

- (IBAction)toggleVisibilityOfCommunity:(id)sender
{
}

- (IBAction)toggleVisibilityOfCommunityEdge:(id)sender
{
}

#pragma mark NSDocument

- (NSData *)dataOfType:(NSString *)typeName error:(NSError **)outError
{
  if (outError)
  {
    *outError = [NSError errorWithDomain:NSOSStatusErrorDomain code:unimpErr userInfo:NULL];
  }
  return nil;
}

- (BOOL)readFromData:(NSData *)data ofType:(NSString *)typeName error:(NSError **)outError
{
  if (outError)
  {
    *outError = [NSError errorWithDomain:NSOSStatusErrorDomain code:unimpErr userInfo:NULL];
  }
  return YES;
}

- (NSString *)windowNibName
{
  return @"SociariumDocument";
}

- (void)windowControllerDidLoadNib:(NSWindowController *) aController
{
  [super windowControllerDidLoadNib:aController];
}

#pragma mark NSObject

- (id)init
{
  if (self = [super init])
  {
  }
  return self;
}

@end

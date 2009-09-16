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
  IBOutlet SociariumView *sociarium;
}

- (IBAction)cancelRunningThread:(id)sender;
- (IBAction)clearCommunityDetection:(id)sender;
- (IBAction)executeLayout:(id)sender;
- (IBAction)executeCommunityDetection:(id)sender;
- (IBAction)initializeView:(id)sender;
- (IBAction)initializeViewport:(id)sender;
- (IBAction)setAlgoritumOfCommunityDetectionToConnectedComponents:(id)sender;
- (IBAction)setAlgoritumOfCommunityDetectionToStronglyConnectedComponents:(id)sender;
- (IBAction)setAlgoritumOfCommunityDetectionToCliquePercolation3:(id)sender;
- (IBAction)setAlgoritumOfCommunityDetectionToCliquePercolation4:(id)sender;
- (IBAction)setAlgoritumOfCommunityDetectionToCliquePercolation5:(id)sender;
- (IBAction)setAlgoritumOfCommunityDetectionToModularityMaximizationUsingGreedyMethod:(id)sender;
- (IBAction)setAlgoritumOfCommunityDetectionToModularityMaximizationUsingTauEoMethod:(id)sender;
- (IBAction)setAlgoritumOfCommunityDetectionToBetweennessCentralityClustering:(id)sender;
- (IBAction)setAlgoritumOfGraphLayoutToKamadaKawaiMethod:(id)sender;
//- (IBAction)setAlgoritumOfGraphLayoutToHighDimensionalEmbedding:(id)sender;
- (IBAction)setAlgoritumOfGraphLayoutToCircle:(id)sender;
- (IBAction)setAlgoritumOfGraphLayoutToCircleInSizeOrder:(id)sender;
//- (IBAction)setAlgoritumOfGraphLayoutToArray:(id)sender;
- (IBAction)setAlgoritumOfGraphLayoutToRandom:(id)sender;
- (IBAction)setFontSizeOfCommunity:(id)sender;
- (IBAction)setFontSizeOfCommunityEdge:(id)sender;
- (IBAction)setFontSizeOfEdge:(id)sender;
- (IBAction)setFontSizeOfNode:(id)sender;
- (IBAction)setSizeOfNode:(id)sender;
- (IBAction)setStyleOfCommunityEdgeToLine:(id)sender;
- (IBAction)setStyleOfCommunityEdgeToPolygon:(id)sender;
- (IBAction)setStyleOfCommunityToPolygon:(id)sender;
- (IBAction)setStyleOfCommunityToTexture:(id)sender;
- (IBAction)setStyleOfEdgeToLine:(id)sender;
- (IBAction)setStyleOfEdgeToPolygon:(id)sender;
- (IBAction)setStyleOfNodeToPolygon:(id)sender;
- (IBAction)setStyleOfNodeToTexture:(id)sender;
- (IBAction)shiftStyle:(id)sender;
- (IBAction)toggleFullscreen:(id)sender;
- (IBAction)toggleLabelVisibilityOfEdge:(id)sender;
- (IBAction)toggleLabelVisibilityOfNode:(id)sender;
- (IBAction)toggleLabelVisibilityOfCommunity:(id)sender;
- (IBAction)toggleLabelVisibilityOfCommunityEdge:(id)sender;
- (IBAction)toggleUseOfWeightedModularity:(id)sender;
- (IBAction)toggleVisibilityOfEdge:(id)sender;
- (IBAction)toggleVisibilityOfNode:(id)sender;
- (IBAction)toggleVisibilityOfCommunity:(id)sender;
- (IBAction)toggleVisibilityOfCommunityEdge:(id)sender;

@end

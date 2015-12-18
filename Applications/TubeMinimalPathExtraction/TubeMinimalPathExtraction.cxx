/*=========================================================================

   Library:   TubeTK

   Copyright 2010 Kitware Inc. 28 Corporate Drive,
   Clifton Park, NY, 12065, USA.

   All rights reserved.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

=========================================================================*/

#include "tubeCLIProgressReporter.h"
#include "tubeMessage.h"
#include "tubeMacro.h"
#include "metaScene.h"

#include "itkTimeProbesCollectorBase.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkPolyLineParametricPath.h"
#include "itkSingleImageCostFunction.h"
#include "itkGradientDescentOptimizer.h"
#include "itkPathIterator.h"
#include "itkGroupSpatialObject.h"
#include "itkSpatialObjectWriter.h"
#include "itktubeRadiusExtractor2.h"

#include "itkSpeedFunctionToPathFilter.h"
#include "itkSpeedFunctionPathInformation.h"
#include "itkIterateNeighborhoodOptimizer.h"

#include <sstream>

#include "TubeMinimalPathExtractionCLP.h"

template< class TPixel, unsigned int DimensionT >
int DoIt( int argc, char * argv[] );

// Must follow include of "...CLP.h"
//   and forward declaration of int DoIt( ... ).
#include "tubeCLIHelperFunctions.h"

template< class TPixel, unsigned int DimensionT >
int DoIt( int argc, char * argv[] )
{
  PARSE_ARGS;
  // The timeCollector is used to perform basic profiling of the components
  //   of your algorithm.
  itk::TimeProbesCollectorBase timeCollector;

  // CLIProgressReporter is used to communicate progress with the Slicer GUI
  tube::CLIProgressReporter progressReporter( "Extract Minimal Path",
    CLPProcessInformation );
  progressReporter.Start();

  typedef TPixel                                    PixelType;
  typedef itk::Image< PixelType, DimensionT >       ImageType;
  typedef itk::ImageFileReader< ImageType >         ReaderType;

  typedef itk::GroupSpatialObject< DimensionT >           TubeGroupType;
  typedef itk::VesselTubeSpatialObject< DimensionT >      TubeType;
  typedef itk::VesselTubeSpatialObjectPoint< DimensionT > TubePointType;

  timeCollector.Start( "Load data" );
  typename ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName( inputImage.c_str() );
  try
    {
    reader->Update();
    }
  catch( itk::ExceptionObject & err )
    {
    std::stringstream out;
    out << "ExceptionObject caught !" << std::endl;
    out << err << std::endl;
    tube::ErrorMessage( out.str() );
    timeCollector.Stop( "Load data" );
    return EXIT_FAILURE;
    }

  typename ImageType::Pointer speed = reader->GetOutput();
  speed->DisconnectPipeline();

  timeCollector.Stop( "Load data" );
  progressReporter.Report( 0.1 );

  timeCollector.Start( "Set parameters" );
  typedef itk::PolyLineParametricPath<DimensionT>               PathType;
  typedef itk::SpeedFunctionToPathFilter< ImageType, PathType > PathFilterType;
  typedef itk::Point< double, DimensionT >                      PointType;

  typedef itk::LinearInterpolateImageFunction< ImageType, double >
    InterpolatorType;
  typename InterpolatorType::Pointer interpolator = InterpolatorType::New();

  typedef itk::SingleImageCostFunction< ImageType > CostFunctionType;
  typename CostFunctionType::Pointer costFunction = CostFunctionType::New();
  costFunction->SetInterpolator( interpolator );

  //Get Input image information
  typedef typename TubeType::TransformType TransformType;
  typename TransformType::InputVectorType scaleVector;
  typename TransformType::OffsetType offsetVector;
  typename ImageType::SpacingType spacing = speed->GetSpacing();
  typename ImageType::PointType origin = speed->GetOrigin();
  double tubeSpacing[DimensionT];

  for ( unsigned int k = 0; k < DimensionT; ++k )
    {
    scaleVector[k] = spacing[k];
    offsetVector[k] = origin[k];
    tubeSpacing[k] = spacing[k];
    }

  // Create path information
  typedef itk::SpeedFunctionPathInformation< PointType > PathInformationType;
  typename PathInformationType::Pointer pathInfo = PathInformationType::New();

  if( Path.size() >= 2 )
    {
    for( int k = 0; k<Path.size(); k++ )
      {
      PointType path;
      for( int i = 0; i<DimensionT; i++ )
        {
        path[i]=Path[k][i];
        }
      if( k == 0 )
        {
        pathInfo->SetStartPoint( path );
        }
      else if( k == Path.size() - 1 )
        {
        pathInfo->SetEndPoint( path );
        }
      else
        {
        pathInfo->AddWayPoint( path );
        }
      }
    }
  else
    {
    tubeErrorMacro(
      << "Error: Path should contain at least a Start and an End Point" );
    timeCollector.Stop( "Set parameters" );
    return EXIT_FAILURE;
    }

  // Create path filter
  typename PathFilterType::Pointer pathFilter = PathFilterType::New();
  pathFilter->SetInput( speed );
  pathFilter->SetCostFunction( costFunction );
  pathFilter->SetTerminationValue( TerminationValue );
  pathFilter->AddPathInformation( pathInfo );

  //Set Optimizer
  if( Optimizer == "Iterate Neighborhood" )
    {
    // Create IterateNeighborhoodOptimizer
    typedef itk::IterateNeighborhoodOptimizer OptimizerType;
    typename OptimizerType::Pointer optimizer = OptimizerType::New();
    optimizer->MinimizeOn();
    optimizer->FullyConnectedOn();
    typename OptimizerType::NeighborhoodSizeType size( DimensionT );
    for( unsigned int i = 0; i < DimensionT; i++ )
      {
      size[i] = speed->GetSpacing()[i] * StepLengthFactor;
      }
    optimizer->SetNeighborhoodSize( size );
    pathFilter->SetOptimizer( optimizer );
    }
  else if( Optimizer == "Gradient Descent" )
    {
    // Create GradientDescentOptimizer
    typedef itk::GradientDescentOptimizer OptimizerType;
    typename OptimizerType::Pointer optimizer = OptimizerType::New();
    optimizer->SetNumberOfIterations( NumberOfIterations );
    pathFilter->SetOptimizer( optimizer );
    }
  else if( Optimizer == "Regular Step Gradient Descent" )
    {
    // Compute the minimum spacing
    double minspacing = spacing[0];
    for( unsigned int dim = 0; dim < DimensionT; dim++ )
      {
      if ( spacing[dim] < minspacing )
        {
        minspacing = spacing[dim];
        }
      }
    // Create RegularStepGradientDescentOptimizer
    typedef itk::RegularStepGradientDescentOptimizer OptimizerType;
    typename OptimizerType::Pointer optimizer = OptimizerType::New();
    optimizer->SetNumberOfIterations( NumberOfIterations );
    optimizer->SetMaximumStepLength( 1.0 * StepLengthFactor*minspacing );
    optimizer->SetMinimumStepLength( 0.5 * StepLengthFactor*minspacing );
    optimizer->SetRelaxationFactor( StepLengthRelax );
    pathFilter->SetOptimizer( optimizer );
    }
  else
    {
    tubeErrorMacro(
      << "Error: Optimizer not known" );
    timeCollector.Stop( "Set parameters" );
    return EXIT_FAILURE;
    }

  timeCollector.Stop( "Set parameters" );
  progressReporter.Report( 0.2 );

  timeCollector.Start( "Extract minimal path" );

  try
    {
    pathFilter->Update();
    }
  catch( itk::ExceptionObject & err )
    {
    std::stringstream out;
    out << "ExceptionObject caught !" << std::endl;
    out << err << std::endl;
    tube::ErrorMessage( out.str() );
    timeCollector.Stop( "Extract minimal path" );
    return EXIT_FAILURE;
    }
  timeCollector.Stop( "Extract minimal path" );
  progressReporter.Report( 0.4 );

  // Create output TRE file
  typename TubeType::PointListType tubePointList;
  typename TubeGroupType::Pointer pTubeGroup = TubeGroupType::New();

  // Update tubes transform
  pTubeGroup->GetObjectToParentTransform()->SetScale(
    scaleVector );
  pTubeGroup->GetObjectToParentTransform()->SetOffset(
    offsetVector );
  pTubeGroup->GetObjectToParentTransform()->SetMatrix(
    speed->GetDirection() );
  pTubeGroup->ComputeObjectToWorldTransform();

  timeCollector.Start( "Rasterizing path" );

  for ( unsigned int i = 0; i<pathFilter->GetNumberOfOutputs(); i++ )
    {
    // Get the path
    typename PathType::Pointer path = pathFilter->GetOutput( i );
    // Check path is valid
    if ( path->GetVertexList()->Size() == 0 )
      {
      std::cout << "WARNING: Path " << ( i + 1 )
        << " contains no points!" << std::endl;
      continue;
      }

    //Output centerline in TRE file
    typename TubeType::PointListType tubePointList;
    typename PathType::VertexListType * vertexList = path->GetVertexList();

    for( unsigned int k = 0; k < vertexList->Size(); k++ )
      {
      PointType pathPoint;
      speed->TransformContinuousIndexToPhysicalPoint(
        vertexList->GetElement(k), pathPoint );
      for( int i = 0; i<DimensionT; i++ )
        {
        pathPoint[i]=(pathPoint[i]-origin[i])/spacing[i];
        }
      TubePointType tubePoint;
      tubePoint.SetPosition( pathPoint );
      tubePoint.SetID( k );
      tubePointList.push_back( tubePoint );
      }
    typename TubeType::Pointer pTube = TubeType::New();
    pTube->SetPoints( tubePointList );
    pTube->ComputeTangentAndNormals();
    pTube->SetSpacing( tubeSpacing );
    pTube->SetId( i );

    //Extract Radius
    if( ExtractRadius )
      {
      typedef itk::tube::RadiusExtractor2<ImageType> RadiusExtractorType;
      typename RadiusExtractorType::Pointer radiusExtractor
        = RadiusExtractorType::New();
      radiusExtractor->SetInputImage( speed );
      radiusExtractor->SetRadiusStart( 0.1 );
      radiusExtractor->SetRadiusMin( 0.1 );
      radiusExtractor->SetRadiusMax( 5.0 );
      radiusExtractor->SetRadiusStep( 0.5 );
      radiusExtractor->SetRadiusTolerance( 0.25 );
      radiusExtractor->SetDebug( false );
      radiusExtractor->ExtractRadii( pTube );
      }

    pTubeGroup->AddSpatialObject( pTube );
    pTubeGroup->ComputeObjectToWorldTransform();
    }

  timeCollector.Stop( "Rasterizing path" );
  progressReporter.Report( 0.9 );

  timeCollector.Start( "Write output data" );

  // Write output TRE file
  typedef itk::SpatialObjectWriter< DimensionT > TubeWriterType;
  typename TubeWriterType::Pointer tubeWriter = TubeWriterType::New();
  try
    {
    tubeWriter->SetFileName( outputTREFile.c_str() );
    tubeWriter->SetInput( pTubeGroup );
    tubeWriter->Update();
    }
  catch( itk::ExceptionObject & err )
    {
    tube::ErrorMessage( "Error writing TRE file: "
      + std::string( err.GetDescription() ) );
    timeCollector.Stop( "Write output data" );
    timeCollector.Report();
    return EXIT_FAILURE;
    }

  timeCollector.Stop( "Write output data" );
  progressReporter.Report( 1.0 );

  timeCollector.Report();
  return EXIT_SUCCESS;
}

// Main
int main( int argc, char * argv[] )
{
  PARSE_ARGS;
  // You may need to update this line if, in the project's .xml CLI file,
  //   you change the variable name for the inputImage.
  return tube::ParseArgsAndCallDoIt( inputImage, argc, argv );

}

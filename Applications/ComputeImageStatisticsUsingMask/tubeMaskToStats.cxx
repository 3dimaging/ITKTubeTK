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

#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#ifdef __BORLANDC__
#define ITK_LEAN_AND_MEAN
#endif

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

// The following three should be used in every CLI application
#include "tubeMessage.h"
#include "tubeCLIFilterWatcher.h"
#include "tubeCLIProgressReporter.h"
#include "itkTimeProbesCollectorBase.h"

// Includes specific to this CLI application
#include <map>
#include "itkArray.h"

// Must do a forward declaraction of DoIt before including
// tubeCLIHelperFunctions
template< class pixelT, unsigned int dimensionT >
int DoIt( int argc, char * argv[] );

// Must include CLP before including tubeCLIHelperFunctions
#include "tubeMaskToStatsCLP.h"

// Includes tube::ParseArgsAndCallDoIt function
#include "tubeCLIHelperFunctions.h"

// Your code should be within the DoIt function...
template< class pixelT, unsigned int dimensionT >
int DoIt( int argc, char * argv[] )
{
  PARSE_ARGS;

  // The timeCollector is used to perform basic profiling of the components
  //   of your algorithm.
  itk::TimeProbesCollectorBase timeCollector;

  // CLIProgressReporter is used to communicate progress with the Slicer GUI
  tube::CLIProgressReporter    progressReporter( "tubeMaskToStats",
                                                 CLPProcessInformation );
  progressReporter.Start();

  typedef itk::Image< pixelT,  dimensionT >        MaskType;
  typedef itk::Image< unsigned int,  dimensionT >  ConnCompType;
  typedef itk::Image< float,  dimensionT >         VolumeType;
  typedef itk::ImageFileReader< VolumeType >       VolumeReaderType;
  typedef itk::ImageFileReader< MaskType >         MaskReaderType;

  //
  //
  //
  timeCollector.Start("Load mask");
  typename MaskReaderType::Pointer maskReader = MaskReaderType::New();
  maskReader->SetFileName( inputMask.c_str() );
  try
    {
    maskReader->Update();
    }
  catch( itk::ExceptionObject & err )
    {
    tube::ErrorMessage( "Reading mask: Exception caught: "
                        + std::string(err.GetDescription()) );
    timeCollector.Report();
    return EXIT_FAILURE;
    }
  timeCollector.Stop("Load mask");
  double progress = 0.1;
  progressReporter.Report( progress );

  typename MaskType::Pointer curMask = maskReader->GetOutput();

  //
  //
  //
  timeCollector.Start("Load volume");
  typename VolumeReaderType::Pointer volumeReader = VolumeReaderType::New();
  volumeReader->SetFileName( inputVolume.c_str() );
  try
    {
    volumeReader->Update();
    }
  catch( itk::ExceptionObject & err )
    {
    tube::ErrorMessage( "Reading volume: Exception caught: "
                        + std::string(err.GetDescription()) );
    timeCollector.Report();
    return EXIT_FAILURE;
    }
  timeCollector.Stop("Load volume");
  progress = 0.2;
  progressReporter.Report( progress );

  typename VolumeType::Pointer curVolume = volumeReader->GetOutput();

  //
  //
  //
  typename ConnCompType::Pointer curConnComp = ConnCompType::New();
  curConnComp->CopyInformation( curVolume );
  curConnComp->SetRegions( curVolume->GetLargestPossibleRegion() );
  curConnComp->Allocate();

  timeCollector.Start("Connected Components");

  typedef std::map< pixelT, unsigned int > MapType;

  MapType maskMap;

  unsigned int maxNumComponents = curVolume->GetLargestPossibleRegion().
    GetNumberOfPixels();
  itk::Array< double > mean(maxNumComponents);
  mean.Fill( 0 );
  itk::Array< double > count(maxNumComponents);
  count.Fill( 0 );

  itk::ImageRegionIterator< MaskType > maskIter( curMask,
    curMask->GetLargestPossibleRegion() );
  itk::ImageRegionIterator< VolumeType > volumeIter( curVolume,
    curVolume->GetLargestPossibleRegion() );
  itk::ImageRegionIterator< ConnCompType > connCompIter( curConnComp,
    curConnComp->GetLargestPossibleRegion() );
  typename MapType::iterator mapIter;
  unsigned int numberOfComponents = 0;
  unsigned int id = 0;
  pixelT tf = 0;
  while( !connCompIter.IsAtEnd() )
    {
    tf = maskIter.Get();

    mapIter = maskMap.find( tf );
    if( mapIter != maskMap.end() )
      {
      id = mapIter->second;
      }
    else
      {
      id = numberOfComponents;
      maskMap[ tf ] = id;
      ++numberOfComponents;
      }
    mean[id] += volumeIter.Get();
    ++count[id];

    connCompIter.Set( id );

    ++maskIter;
    ++volumeIter;
    ++connCompIter;
    }
  std::cout << "Number of components = " << numberOfComponents << std::endl;

  for( unsigned int i=0; i<numberOfComponents; i++ )
    {
    if( count[i] > 0 )
      {
      mean[i] /= count[i];
      }
    }

  connCompIter.GoToBegin();
  volumeIter.GoToBegin();
  while( !connCompIter.IsAtEnd() )
    {
    id = connCompIter.Get();

    volumeIter.Set( mean[id] );

    ++connCompIter;
    ++volumeIter;
    }
  timeCollector.Stop("Connected Components");

  typedef itk::ImageFileWriter< VolumeType  >   ImageWriterType;

  timeCollector.Start("Save data");
  typename ImageWriterType::Pointer writer = ImageWriterType::New();
  writer->SetFileName( outputStatistics.c_str() );
  writer->SetInput( curVolume );
  writer->SetUseCompression( true );
  try
    {
    writer->Update();
    }
  catch( itk::ExceptionObject & err )
    {
    tube::ErrorMessage( "Writing volume: Exception caught: "
      + std::string(err.GetDescription()) );
    timeCollector.Report();
    return EXIT_FAILURE;
    }
  timeCollector.Stop("Save data");
  progress = 1.0;
  progressReporter.Report( progress );
  progressReporter.End( );

  timeCollector.Report();
  return EXIT_SUCCESS;
}

// Main
int main( int argc, char **argv )
{
  PARSE_ARGS;

  // You may need to update this line if, in the project's .xml CLI file,
  //   you change the variable name for the inputVolume.
  return tube::ParseArgsAndCallDoIt( inputMask, argc, argv );
}

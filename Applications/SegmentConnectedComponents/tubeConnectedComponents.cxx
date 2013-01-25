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
#include "itkArray.h"
#include "itkConnectedComponentImageFilter.h"

// Must do a forward declaraction of DoIt before including
// tubeCLIHelperFunctions
template< class pixelT, unsigned int dimensionT >
int DoIt( int argc, char * argv[] );

// Must include CLP before including tubeCLIHelperFunctions
#include "tubeConnectedComponentsCLP.h"

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
  tube::CLIProgressReporter    progressReporter( "ConnectedComponents",
                                                 CLPProcessInformation );
  progressReporter.Start();

  typedef itk::Image< pixelT,  dimensionT >        MaskType;
  typedef itk::ImageFileReader< MaskType >         MaskReaderType;

  typedef itk::Image< unsigned int,  dimensionT >  ConnCompType;

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
  timeCollector.Start("Connected Components");

  typedef itk::ConnectedComponentImageFilter< MaskType, ConnCompType >
    FilterType;
  typename FilterType::Pointer filter;

  filter = FilterType::New();
  filter->SetInput( curMask );

  tube::CLIFilterWatcher watcher( filter,
                                  "Connected Component Filter",
                                  CLPProcessInformation,
                                  0.6,
                                  progress,
                                  true );

  filter->Update();

  typename ConnCompType::Pointer curConnComp = filter->GetOutput();

  itk::ImageRegionIterator< MaskType > maskIter( curMask,
    curMask->GetLargestPossibleRegion() );
  maskIter.GoToBegin();

  itk::ImageRegionIterator< ConnCompType > iter( curConnComp,
    curConnComp->GetLargestPossibleRegion() );
  iter.GoToBegin();

  while( !iter.IsAtEnd() )
    {
    if( maskIter.Get() == 0 )
      {
      iter.Set( 0 );
      }
    else
      {
      unsigned int c = iter.Get();
      iter.Set( c+1 );
      }
    ++maskIter;
    ++iter;
    }

  if( minSize > 0 )
    {
    iter.GoToBegin();
    unsigned int numObjects = filter->GetObjectCount()+1;
    std::vector< unsigned int > cSize( numObjects, 0 );
    while( !iter.IsAtEnd() )
      {
      unsigned int c = iter.Get();
      if( c > 0 && c < numObjects )
        {
        ++cSize[ c ];
        }
      ++iter;
      }
    iter.GoToBegin();
    while( !iter.IsAtEnd() )
      {
      unsigned int c = iter.Get();
      if( c > 0 && c < numObjects )
        {
        if( cSize[c] < (unsigned int)minSize )
          {
          iter.Set( 0 );
          }
        }
      ++iter;
      }
    }

  //
  if( seedMask.size() > 0 )
    {
    timeCollector.Start("Load seed mask");
    typename MaskReaderType::Pointer seedMaskReader = MaskReaderType::New();
    seedMaskReader->SetFileName( seedMask.c_str() );
    try
      {
      seedMaskReader->Update();
      }
    catch( itk::ExceptionObject & err )
      {
      tube::ErrorMessage( "Reading seed mask: Exception caught: "
                          + std::string(err.GetDescription()) );
      timeCollector.Report();
      return EXIT_FAILURE;
      }
    timeCollector.Stop("Load seed mask");

    typename MaskType::Pointer curSeedMask = seedMaskReader->GetOutput();

    itk::ImageRegionIterator< MaskType > seedIter( curSeedMask,
      curSeedMask->GetLargestPossibleRegion() );
    seedIter.GoToBegin();

    iter.GoToBegin();

    unsigned int numObjects = filter->GetObjectCount()+1;
    std::vector< bool > cSeeded( numObjects, false );
    while( !iter.IsAtEnd() )
      {
      unsigned int c = iter.Get();
      if( c > 0 && c < numObjects )
        {
        if( ! cSeeded[c] )
          {
          if( seedIter.Get() != 0 )
            {
            cSeeded[ c ] = true;
            }
          }
        }
      ++iter;
      ++seedIter;
      }
    iter.GoToBegin();
    while( !iter.IsAtEnd() )
      {
      unsigned int c = iter.Get();
      if( c > 0 && c < numObjects+1 )
        {
        if( cSeeded[c] )
          {
          iter.Set( c );
          }
        else
          {
          iter.Set( 0 );
          }
        }
      ++iter;
      }
    }

  timeCollector.Stop("Connected Components");

  typedef itk::ImageFileWriter< ConnCompType  >   ImageWriterType;

  timeCollector.Start("Save data");
  typename ImageWriterType::Pointer writer = ImageWriterType::New();
  writer->SetFileName( outputMask.c_str() );
  writer->SetInput( curConnComp );
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

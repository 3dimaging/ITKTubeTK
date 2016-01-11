##############################################################################
#
# Library:   TubeTK
#
# Copyright 2010 Kitware Inc. 28 Corporate Drive,
# Clifton Park, NY, 12065, USA.
#
# All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
##############################################################################

set( proj Applications )

set( TubeTK_${proj}_MODULES
  AtlasBuilderUsingIntensity
  ClipTubes
  ComputeBinaryImageSimilarityMetrics
  ComputeImageSimilarityMetrics
  ComputeImageStatistics
  ComputeSegmentTubesParameters
  ComputeTubeFlyThroughImage
  ComputeTubeGraphProbability
  ComputeTubeMeasures
  ComputeTubeProbability
  ComputeTubeTortuosityMeasures
  ConvertImagesToCSV
  ConvertInnerOpticToPlus
  ConvertShrunkenSeedImageToList
  ConvertToMetaImage
  ConvertTRE
  ConvertTubeGraphToImage
  ConvertTubesToDensityImage
  ConvertTubesToImage
  ConvertTubesToTubeTree
  ConvertTubeToTubeGraph
  CropImage
  DeblendTomosynthesisSlicesUsingPrior
  EnhanceCoherenceAndEdgesUsingDiffusion
  EnhanceCoherenceUsingDiffusion
  EnhanceContrastUsingPrior
  EnhanceEdgesUsingDiffusion
  EnhanceTubesUsingDiffusion
  EnhanceTubesUsingDiscriminantAnalysis
  EnhanceUsingDiscriminantAnalysis
  EnhanceUsingNJetDiscriminantAnalysis
  ImageMath
  MergeAdjacentImages
  MergeTubeGraphs
  ResampleImage
  ResampleTubes
  RegisterImages
  # RegisterImageToTubesUsingRigidTransform
  RegisterUsingImageCenters
  SampleCLIApplication
  SegmentBinaryImageSkeleton
  SegmentConnectedComponents
  SegmentConnectedComponentsUsingParzenPDFs
  SegmentTubes
  SegmentTubeUsingMinimalPath
  SegmentUsingOtsuThreshold
  ShrinkImage
  SimulateAcquisitionArtifactsUsingPrior
  TubeMath
  VesselTrainMask )

set( TubeTK_${proj}_ImageViewer_MODULES "" )
if( TubeTK_BUILD_IMAGE_VIEWER )
  set( TubeTK_${proj}_ImageViewer_MODULES
     ImageEditor )
  list( APPEND TubeTK_${proj}_MODULES
    ${TubeTK_${proj}_ImageViewer_MODULES} )
endif( TubeTK_BUILD_IMAGE_VIEWER)

set( TubeTK_${proj}_Boost_MODULES )
if( TubeTK_USE_BOOST )
  set( TubeTK_${proj}_Boost_MODULES
    ComputeRegionSignatures
    ComputeTubeGraphSimilarityKernelMatrix
    SegmentUsingQuantileThreshold
    TransferLabelsToRegions )
  list( APPEND TubeTK_${proj}_MODULES
    ${TubeTK_${proj}_Boost_MODULES} )
endif( TubeTK_USE_BOOST )

set( TubeTK_${proj}_VTK_MODULES )
if( TubeTK_USE_VTK )
  set( TubeTK_${proj}_VTK_MODULES
    ConvertTubesToSurface
    RegisterUsingSlidingGeometries )
  list( APPEND TubeTK_${proj}_MODULES
    ${TubeTK_${proj}_VTK_MODULES} )
endif( TubeTK_USE_VTK )

if( NOT TubeTK_SOURCE_DIR )
  find_package( TubeTK REQUIRED )
  include( ${TubeTK_USE_FILE} )
endif( NOT TubeTK_SOURCE_DIR )

if( ${CMAKE_PROJECT_NAME} STREQUAL "Slicer" )
  include( ${CMAKE_CURRENT_SOURCE_DIR}/../CMake/TubeTKMacroAddModules.cmake )
else()
  include(TubeTKMacroAddModules)
endif( ${CMAKE_PROJECT_NAME} STREQUAL "Slicer" )
TubeTKMacroAddModules( MODULES ${TubeTK_${proj}_MODULES} )

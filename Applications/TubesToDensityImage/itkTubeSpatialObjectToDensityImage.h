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


#ifndef __TubeSpatialObjectToDensityImage_h
#define __TubeSpatialObjectToDensityImage_h

#include <itkObject.h>
#include <itkImage.h>
#include <itkGroupSpatialObject.h>
#include <itkDanielssonDistanceMapImageFilter.h>
#include "itkTubeSpatialObjectToImageFilter.h"
#include "itkInverseIntensityImageFilter.h"

#include <itkImageFileWriter.h>

namespace itk
{

namespace tube
{

template < class TDensityImageType,
           class TRadiusImageType = Image<float,3>,
           class TTangentImageType = Image<Vector<float,3>,3> >
class TubeSpatialObjectToDensityImage : public Object
{
  public:

    typedef TubeSpatialObjectToDensityImage         Self;
    typedef Object                                  Superclass;
    typedef SmartPointer<Self>                      Pointer;

    itkNewMacro(Self);
    itkTypeMacro(Self,Superclass);

    /** Typdefs */
    typedef TDensityImageType                              DensityImageType;
    typedef typename DensityImageType::PixelType           DensityPixelType;
    typedef typename DensityImageType::Pointer             DensityImagePointer;

    typedef TRadiusImageType                               RadiusImageType;
    typedef typename RadiusImageType::PixelType            RadiusPixelType;
    typedef typename RadiusImageType::Pointer              RadiusImagePointer;

    typedef TTangentImageType                              TangentImageType;
    typedef typename TangentImageType::PixelType           TangentPixelType;
    typedef typename TangentImageType::Pointer             TangentImagePointer;

    /** Define the Dimension variable */
    itkStaticConstMacro( TDimension, unsigned int,
                         DensityImageType::ImageDimension );

    typedef GroupSpatialObject<
      itkGetStaticConstMacro( TDimension ) > TubeGroupType;
    typedef typename TubeGroupType::Pointer TubeGroupPointer;

    typedef VesselTubeSpatialObject<
      itkGetStaticConstMacro( TDimension ) > TubeType;

    typedef typename DensityImageType::OffsetType VectorPixelType;
    typedef Image<
      VectorPixelType,
      itkGetStaticConstMacro(TDimension) >        VectorImageType;
    typedef typename VectorImageType::Pointer     VectorImagePointer;

    typedef typename DensityImageType::SizeType     SizeType;
    typedef typename DensityImageType::SpacingType  SpacingType;

    typedef TubeSpatialObjectToImageFilter<
      itkGetStaticConstMacro(TDimension),
      DensityImageType > TubetoImageFilterType;

    typedef DanielssonDistanceMapImageFilter<
      DensityImageType, DensityImageType > DanielssonFilterType;

    /** Retrieve Density map created by inverted Danielsson Distance Map */
    DensityImagePointer GetDensityMap() const { return m_DensityImage; }
    RadiusImagePointer  GetRadiusMap()  const { return m_RadiusImage;  }
    TangentImagePointer GetTangentMap() const { return m_TangentImage; }

    /** Use square distance instead of linear distance */
    inline void UseSquareDistance( bool v ) { m_UseSquareDistance = v; }

    /** Sets the input tubes */
    inline void SetTubes( TubeGroupPointer t ) { m_TubeGroup = t; }

    /** Sets the output size */
    void SetSize( SizeType s ) { m_Size = s; }

    /** Sets the element spacing */
    void SetSpacing( SpacingType );

    void SetMaxDensityIntensity( DensityPixelType max ) { m_max = max; }

    void Update();

  protected:

    TubeGroupPointer GetTubes() const
            { return m_TubeGroup; }
    void SetDensityMap( DensityImagePointer density )
            { m_DensityImage = density; }
    void SetRadiusMap( RadiusImagePointer radius )
            { m_RadiusImage = radius; }
    void SetTangentMap( TangentImagePointer tangent )
            { m_TangentImage = tangent; }

    TubeSpatialObjectToDensityImage();
    ~TubeSpatialObjectToDensityImage();

  private:

    TubeGroupPointer                  m_TubeGroup;
    DensityImagePointer               m_DensityImage;
    RadiusImagePointer                m_RadiusImage;
    TangentImagePointer               m_TangentImage;
    SizeType                          m_Size;
    SpacingType                       m_Spacing;

    /** Max value allowed for inverse intensity filter */
    DensityPixelType                  m_max;
    bool                              m_UseSquareDistance;
};

#ifndef ITK_MANUAL_INSTANTIATION
  #include "itkTubeSpatialObjectToDensityImage.txx"
#endif

} // End of namespace tube

} // End of namespace itk


#endif //__TubeSpatialObjectToDensityImage_h

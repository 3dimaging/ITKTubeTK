/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    itkTubeOtsuThresholdMaskedImageCalculator.txx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkTubeOtsuThresholdMaskedImageCalculator_txx
#define __itkTubeOtsuThresholdMaskedImageCalculator_txx

#include "itkTubeOtsuThresholdMaskedImageCalculator.h"
#include "itkImageRegionConstIteratorWithIndex.h"
#include "itkMinimumMaximumImageCalculator.h"

#include "vnl/vnl_math.h"

namespace itk {

namespace tube {

/**
 * Constructor
 */
template<class TInputImage>
OtsuThresholdMaskedImageCalculator<TInputImage>
::OtsuThresholdMaskedImageCalculator()
{
  m_Image = NULL;
  m_MaskImage = NULL;
  m_Threshold = NumericTraits<PixelType>::Zero;
  m_NumberOfHistogramBins = 128;
  m_RegionSetByUser = false;
}


/*
 * Compute the Otsu's threshold
 */
template<class TInputImage>
void
OtsuThresholdMaskedImageCalculator<TInputImage>
::Compute(void)
{

  unsigned int j;

  if ( !m_Image )
    {
    return;
    }
  if( !m_RegionSetByUser )
    {
    m_Region = m_Image->GetRequestedRegion();
    }

  double totalPixels = (double) m_Region.GetNumberOfPixels();
  if ( totalPixels == 0 )
    {
    return;
    }


  // compute image max and min
  typedef ImageRegionConstIteratorWithIndex<TInputImage> Iterator;
  Iterator iter( m_Image, m_Region );
  PixelType imageMin = iter.Get();
  PixelType imageMax = imageMin;
  if( m_MaskImage.IsNotNull() )
    {
    std::cout << "Using mask" << std::endl;
    Iterator maskIter( m_MaskImage, m_Region );
    bool first = true;
    while( !iter.IsAtEnd() )
      {
      if( maskIter.Get() != 0 )
        {
        PixelType tf = iter.Get();
        if( first || tf < imageMin )
          {
          imageMin = tf;
          first = false;
          }
        if( first || tf > imageMax )
          {
          imageMax = tf;
          first = false;
          }
        }
      ++iter;
      ++maskIter;
      }
    }
  else
    {
    bool first = true;
    while( !iter.IsAtEnd() )
      {
      PixelType tf = iter.Get();
      if( first || tf < imageMin )
        {
        imageMin = tf;
        first = false;
        }
      if( first || tf > imageMax )
        {
        imageMax = tf;
        first = false;
        }
      ++iter;
      }
    }

  if ( imageMin >= imageMax )
    {
    m_Threshold = imageMin;
    return;
    }

  // create a histogram
  std::vector<double> relativeFrequency;
  relativeFrequency.resize( m_NumberOfHistogramBins );
  for ( j = 0; j < m_NumberOfHistogramBins; j++ )
    {
    relativeFrequency[j] = 0.0;
    }

  double binMultiplier = (double) m_NumberOfHistogramBins /
    (double) ( imageMax - imageMin );

  if( m_MaskImage.IsNotNull() )
    {
    iter.GoToBegin();
    Iterator maskIter( m_MaskImage, m_Region );
    std::cout << "Using mask2" << std::endl;
    while( !iter.IsAtEnd() )
      {
      unsigned int binNumber;
      if( maskIter.Get() != 0 )
        {
        PixelType value = iter.Get();

        if ( value == imageMin )
          {
          binNumber = 0;
          }
        else
          {
          binNumber = (unsigned int) vcl_ceil((value - imageMin)
            * binMultiplier ) - 1;
          if ( binNumber == m_NumberOfHistogramBins )
            {
            binNumber -= 1;
            }
          }
        relativeFrequency[binNumber] += 1.0;
        }
      ++iter;
      ++maskIter;
      }
    }
  else
    {
    iter.GoToBegin();
    while( !iter.IsAtEnd() )
      {
      unsigned int binNumber;
      PixelType value = iter.Get();

      if ( value == imageMin )
        {
        binNumber = 0;
        }
      else
        {
        binNumber = (unsigned int) vcl_ceil((value - imageMin)
          * binMultiplier ) - 1;
        if ( binNumber == m_NumberOfHistogramBins )
          {
          binNumber -= 1;
          }
        }
      relativeFrequency[binNumber] += 1.0;

      ++iter;
      }
    }

  // normalize the frequencies
  double totalMean = 0.0;
  for ( j = 0; j < m_NumberOfHistogramBins; j++ )
    {
    relativeFrequency[j] /= totalPixels;
    totalMean += (j+1) * relativeFrequency[j];
    }


  // compute Otsu's threshold by maximizing the between-class
  // variance
  double freqLeft = relativeFrequency[0];
  double meanLeft = 1.0;
  double meanRight = ( totalMean - freqLeft ) / ( 1.0 - freqLeft );

  double maxVarBetween = freqLeft * ( 1.0 - freqLeft ) *
    vnl_math_sqr( meanLeft - meanRight );
  int maxBinNumber = 0;

  double freqLeftOld = freqLeft;
  double meanLeftOld = meanLeft;

  for ( j = 1; j < m_NumberOfHistogramBins; j++ )
    {
    freqLeft += relativeFrequency[j];
    meanLeft = ( meanLeftOld * freqLeftOld +
                 (j+1) * relativeFrequency[j] ) / freqLeft;
    if (freqLeft == 1.0)
      {
      meanRight = 0.0;
      }
    else
      {
      meanRight = ( totalMean - meanLeft * freqLeft ) /
        ( 1.0 - freqLeft );
      }
    double varBetween = freqLeft * ( 1.0 - freqLeft ) *
      vnl_math_sqr( meanLeft - meanRight );

    if ( varBetween > maxVarBetween )
      {
      maxVarBetween = varBetween;
      maxBinNumber = j;
      }

    // cache old values
    freqLeftOld = freqLeft;
    meanLeftOld = meanLeft;

    }

  m_Threshold = static_cast<PixelType>( imageMin +
                                        ( maxBinNumber + 1 ) / binMultiplier );
}

template<class TInputImage>
void
OtsuThresholdMaskedImageCalculator<TInputImage>
::SetRegion( const RegionType & region )
{
  m_Region = region;
  m_RegionSetByUser = true;
}


template<class TInputImage>
void
OtsuThresholdMaskedImageCalculator<TInputImage>
::PrintSelf( std::ostream& os, Indent indent ) const
{
  Superclass::PrintSelf(os,indent);

  if( m_MaskImage.IsNotNull() )
    {
    os << indent << "MaskImage: " << m_MaskImage << std::endl;
    }
  else
    {
    os << indent << "MaskImage: NULL" << std::endl;
    }
  os << indent << "Threshold: " << m_Threshold << std::endl;
  os << indent << "NumberOfHistogramBins: " << m_NumberOfHistogramBins << std::endl;
  os << indent << "Image: " << m_Image.GetPointer() << std::endl;
}

} // end namespace tube

} // end namespace itk

#endif

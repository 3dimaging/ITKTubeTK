/*=========================================================================
 *
 *  Copyright Insight Software Consortium
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
*=========================================================================*/
#ifndef __tubeConvertTubesToImage_hxx
#define __tubeConvertTubesToImage_hxx

#include "tubeConvertTubesToImage.h"

namespace tube
{

template< unsigned int Dimension, class TOutputPixel >
ConvertTubesToImage< Dimension, TOutputPixel >
::ConvertTubesToImage( void )
{
  m_Filter = FilterType::New();
  m_Filter->SetBuildRadiusImage( false );
  m_Filter->SetBuildTangentImage( false );

  m_TemplateImage = NULL;
}

template< unsigned int Dimension, class TOutputPixel >
void
ConvertTubesToImage< Dimension, TOutputPixel >
::SetTemplateImage( const typename ConvertTubesToImage< Dimension,
  TOutputPixel >::OutputImageType * pTemplateImage )
{
  if( this->m_TemplateImage != pTemplateImage )
    {
    this->m_TemplateImage = pTemplateImage;

    m_Filter->SetSize( pTemplateImage->GetLargestPossibleRegion().GetSize() );
    m_Filter->SetSpacing( pTemplateImage->GetSpacing() );

    this->Modified();
    }
}

template< unsigned int Dimension, class TOutputPixel >
void
ConvertTubesToImage< Dimension, TOutputPixel >
::PrintSelf(std::ostream & os, itk::Indent indent) const
{
  m_Filter->PrintSelf( os, indent );
}

}

#endif

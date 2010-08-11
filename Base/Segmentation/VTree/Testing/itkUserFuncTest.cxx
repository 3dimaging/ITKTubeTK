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
#include <cstdlib>
#include <iostream>

#include <vcl_cmath.h>

#include <vnl/vnl_vector.h> 

#include "itkMacro.h"

namespace itk {

/*! Derive this class to pass functions to Spline and Optimization Classes
 * \author Stephen R. Aylward
 * \date 11/22/99
 */
class UserFunc2
{
public :
        
  virtual ~UserFunc2() = 0;
     
  /** Derive this function */
  virtual const vnl_vector<double> & value( const vnl_vector<double> & x )
    = 0;

};

inline UserFunc2::~UserFunc2() 
{
}

}; // namespace itk

class MyFunc2:
public itk::UserFunc2 
  {
  public:

    MyFunc2( )
      {
      cVal.set_size(1);
      };
    const vnl_vector<double> & value( const vnl_vector<double> & x )
      {
      std::cout << "func:x = " << x[0] << ", " << x[1] << std::endl;
      cVal[0] = vcl_sin(x[0]) + vcl_cos(x[1]/2);
      std::cout << "  val = " << cVal << std::endl;
      return cVal;
      };
  private:
    vnl_vector<double> cVal;
  };

int itkUserFuncTest( int itkNotUsed(argc), char **itkNotUsed(argv) )
{
  MyFunc2 myFunc;

  vnl_vector<double> xTest(2);
  xTest[0] = 0.01;
  xTest[1] = 0.01;

  itk::UserFunc2 * op = &myFunc;
  std::cout << "test:func(0.01,0.01) = " << op->value( xTest )[0]
    << std::endl;

  std::cout << "test:func(0.01,0.01) = " << myFunc.value( xTest )[0] 
    << std::endl;

  return EXIT_SUCCESS;
}

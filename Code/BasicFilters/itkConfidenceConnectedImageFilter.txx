/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    itkConfidenceConnectedImageFilter.txx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkConfidenceConnectedImageFilter_txx_
#define __itkConfidenceConnectedImageFilter_txx_

#include "itkConfidenceConnectedImageFilter.h"
#include "itkExceptionObject.h"
#include "itkImageRegionIterator.h"
#include "itkMeanImageFunction.h"
#include "itkVarianceImageFunction.h"
#include "itkBinaryThresholdImageFunction.h"
#include "itkFloodFilledImageFunctionConditionalIterator.h"

namespace itk
{

/**
 * Constructor
 */
template <class TInputImage, class TOutputImage>
ConfidenceConnectedImageFilter<TInputImage, TOutputImage>
::ConfidenceConnectedImageFilter()
{
  m_Multiplier = 2.5;
  m_NumberOfIterations = 4;
  m_Seed = IndexType::ZeroIndex;
  m_ReplaceValue = NumericTraits<OutputImagePixelType>::One;
}

/**
 * Standard PrintSelf method.
 */
template <class TInputImage, class TOutputImage>
void
ConfidenceConnectedImageFilter<TInputImage, TOutputImage>
::PrintSelf(std::ostream& os, Indent indent) const
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Number of iterations: " << m_NumberOfIterations
     << std::endl;
  os << indent << "Multiplier for confidence interval: " << m_Multiplier
     << std::endl;
  os << indent << "ReplaceValue: " << m_ReplaceValue << std::endl;
}

template <class TInputImage, class TOutputImage>
void 
ConfidenceConnectedImageFilter<TInputImage,TOutputImage>
::GenerateInputRequestedRegion()
{
  Superclass::GenerateInputRequestedRegion();
  if ( this->GetInput() )
    {
    this->GetInput()->SetRequestedRegionToLargestPossibleRegion();
    }
}

template <class TInputImage, class TOutputImage>
void 
ConfidenceConnectedImageFilter<TInputImage,TOutputImage>
::EnlargeOutputRequestedRegion(DataObject *output)
{
  Superclass::EnlargeOutputRequestedRegion(output);
  output->SetRequestedRegionToLargestPossibleRegion();
}

template <class TInputImage, class TOutputImage>
void 
ConfidenceConnectedImageFilter<TInputImage,TOutputImage>
::GenerateData()
{
  typedef typename NumericTraits<ITK_TYPENAME InputImageType::PixelType>::RealType InputRealType;
  typedef BinaryThresholdImageFunction<InputImageType> FunctionType;
  typedef BinaryThresholdImageFunction<OutputImageType> SecondFunctionType;
  typedef FloodFilledImageFunctionConditionalIterator<OutputImageType, FunctionType> IteratorType;
  typedef FloodFilledImageFunctionConditionalIterator<InputImageType, SecondFunctionType> SecondIteratorType;

  unsigned int loop;
  unsigned long num;
  
  InputImagePointer inputImage = this->GetInput();
  OutputImagePointer outputImage = this->GetOutput();

  // Zero the output
  outputImage->SetBufferedRegion( outputImage->GetRequestedRegion() );
  outputImage->Allocate();
  outputImage->FillBuffer ( NumericTraits<OutputImagePixelType>::Zero );

  // Compute the statistics of the seed point
  MeanImageFunction<InputImageType>::Pointer meanFunction
    = MeanImageFunction<InputImageType>::New();
  meanFunction->SetInputImage( inputImage );
  VarianceImageFunction<InputImageType>::Pointer varianceFunction
    = VarianceImageFunction<InputImageType>::New();
  varianceFunction->SetInputImage( inputImage );
  
  // Set up the image function used for connectivity
  FunctionType::Pointer function = FunctionType::New();
  function->SetInputImage ( inputImage );

  InputRealType lower, upper;
  InputRealType mean, variance;
  mean = meanFunction->EvaluateAtIndex( m_Seed );
  variance = varianceFunction->EvaluateAtIndex( m_Seed );

  lower = mean - m_Multiplier * sqrt(variance);
  upper = mean + m_Multiplier * sqrt(variance);
  
  function->ThresholdBetween(lower, upper);

  itkDebugMacro(<< "Lower intensity = " << lower);
  itkDebugMacro(<< "Upper intensity = " << upper);
  
  IteratorType it = IteratorType ( outputImage, function, m_Seed );
  while( !it.IsAtEnd())
    {
    it.Set(m_ReplaceValue);
    ++it;
    }

  for (loop = 0; loop < m_NumberOfIterations; ++loop)
    {
    // Now that we have an initial segmentation, let's recalculate the
    // statistics Since we have already labelled the output, we walk the
    // output for candidate pixels and calculate the statistics from
    // the input image
    SecondFunctionType::Pointer secondFunction = SecondFunctionType::New();
    secondFunction->SetInputImage ( outputImage );
    secondFunction->ThresholdBetween( m_ReplaceValue, m_ReplaceValue );

    typename NumericTraits<ITK_TYPENAME InputImageType::PixelType>::RealType sum, sumOfSquares;
    sum = NumericTraits<InputRealType>::Zero;
    sumOfSquares = NumericTraits<InputRealType>::Zero;
    num = 0;
    
    SecondIteratorType sit
      = SecondIteratorType ( inputImage, secondFunction, m_Seed );
    while( !sit.IsAtEnd())
      {
      sum += static_cast<InputRealType>(sit.Get());
      sumOfSquares += (static_cast<InputRealType>(sit.Get())
                       * static_cast<InputRealType>(sit.Get()));
      ++num;
      ++sit;
      }
    mean = sum / double(num);
    variance = (sumOfSquares - (sum*sum / double(num))) / (double(num) - 1.0);
    
    lower = mean - m_Multiplier * sqrt(variance);
    upper = mean + m_Multiplier * sqrt(variance);
  
    function->ThresholdBetween(lower, upper);
    
    itkDebugMacro(<< "Lower intensity = " << lower);
    itkDebugMacro(<< "Upper intensity = " << upper);
    
    // Rerun the segmentation
    outputImage->FillBuffer ( NumericTraits<OutputImagePixelType>::Zero );
    IteratorType thirdIt = IteratorType ( outputImage, function, m_Seed );
    while( !thirdIt.IsAtEnd())
      {
      thirdIt.Set(m_ReplaceValue);
      ++thirdIt;
      }
    }

}


} // end namespace itk

#endif

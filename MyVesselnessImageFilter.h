#ifndef __MyVesselnessImageFilter_h
#define __MyVesselnessImageFilter_h

#include "itkImageToImageFilter.h"

namespace itk
{
	template< class TImage>
	class MyVesselnessImageFilter :public ImageToImageFilter< TImage, TImage >
	{
	public:
		/** Standard class typedefs. */
		typedef MyVesselnessImageFilter Self;
		typedef ImageToImageFilter<TImage, TImage> Superclass;
		typedef SmartPointer<Self> Pointer;

		/** Method for creation through the object factory. */
		itkNewMacro(Self);

		/** Run-time type information (and related methods). */
		itkTypeMacro(MyVesselnessImageFilter, ImageToImageFilter);
		//void PrintSelf(std::ostream& os, Indent indent) const;

		/** Typedefs **/
		typedef typename TImage::PixelType PixelType;

	protected:
		MyVesselnessImageFilter();
		~MyVesselnessImageFilter(){}

		/** Does the real work. */
		typedef MinimumMaximumImageCalculator<TImage> MinMaxCalcType;
		typedef InvertIntensityImageFilter<TImage> ImageInverterType;
		typedef HessianRecursiveGaussianImageFilter<TImage> HessianFilterType;
		typedef Hessian3DToVesselnessMeasureImageFilter<PixelType> VesselnessMeasureFilterType;
		virtual void GenerateData();

	private:
		MyVesselnessImageFilter(const Self &); //purposely not implemented
		void operator=(const Self &);  //purposely not implemented

		typename MinMaxCalcType::Pointer	m_mmcalc;
		typename ImageInverterType::Pointer m_inverter;
		typename HessianFilterType::Pointer m_hessianFilter;
		typename VesselnessMeasureFilterType::Pointer m_vesselnessFilter;

	};
} //namespace ITK


#ifndef ITK_MANUAL_INSTANTIATION
#include "MyVesselnessImageFilter.hxx"
#endif


#endif // __itkImageFilter_h
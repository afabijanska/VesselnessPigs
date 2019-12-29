#ifndef __MyGetSeedsForMoriFilter_h
#define __MyGetSeedsForMoriFilter_h

#include "itkImageToImageFilter.h"

namespace itk
{
	template< class TImage>
	class MyGetSeedsForMoriFilter :public ImageToImageFilter< TImage, TImage >
	{
	public:

		/** Standard class typedefs. */
		typedef MyGetSeedsForMoriFilter Self;
		typedef ImageToImageFilter<TImage, TImage> Superclass;
		typedef SmartPointer<Self> Pointer;

		/** Method for creation through the object factory. */
		itkNewMacro(Self);

		/** Run-time type information (and related methods). */
		itkTypeMacro(MyGetSeedsForMoriFilter, ImageToImageFilter);

		typedef typename TImage::IndexType IndexType;
		typedef typename TImage::PixelType PixelType;
		typedef typename TImage::RegionType RegionType;
		typedef typename TImage::SizeType SizeType;

		typedef typename std::vector<IndexType> SeedContainerType;


		virtual const SeedContainerType &GetSeeds() const;

	protected:
		MyGetSeedsForMoriFilter();
		~MyGetSeedsForMoriFilter(){}

		/** Does the real work. */
		virtual void GenerateData();

		SeedContainerType m_Seeds;
		PixelType m_Lower;
		PixelType m_Upper;
		PixelType m_InsideValue;
		PixelType m_OutsideValue;

	private:
		MyGetSeedsForMoriFilter(const Self &); //purposely not implemented
		void operator=(const Self &);  //purposely not implemented

	};
} //namespace ITK


#ifndef ITK_MANUAL_INSTANTIATION
#include "MyGetSeedsForMoriFilter.hxx"
#endif


#endif // __itkImageFilter_h

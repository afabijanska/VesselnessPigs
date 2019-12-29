#ifndef __MyRandomWalkerSegmentationFilter_h
#define __MyRandomWalkerSegmentationFilter_h

#include "itkImageToImageFilter.h"

namespace itk
{
	template< typename TImage>
	class MyRandomWalkerSegmentationFilter:public ImageToImageFilter< TImage, TImage >
	{
	public:
		/** Standard class typedefs. */
		typedef MyRandomWalkerSegmentationFilter Self;
		typedef ImageToImageFilter< TImage, TImage > Superclass;
		typedef SmartPointer< Self > Pointer;

		typedef itk::Image<typename TImage::PixelType, 2> SliceType;

		/** Method for creation through the object factory. */
		itkNewMacro(Self);

		/** Run-time type information (and related methods). */
		itkTypeMacro(MyRandomWalkerSegmentationFilter, ImageToImageFilter);

		void SetMoriImg(typename TImage::Pointer);
		void SetVesselnessImg(typename TImage::Pointer);
		void SaveAsMHD(std::string path);			// saves binary image as 8-bit mhd file

	protected:
		MyRandomWalkerSegmentationFilter();
		~MyRandomWalkerSegmentationFilter(){}

		/** Does the real work. */
		virtual void GenerateData();

	private:
		MyRandomWalkerSegmentationFilter(const Self &); //purposely not implemented
		void operator=(const Self &);					//purposely not implemented

		typename TImage::Pointer m_moriImg;			// mori segmentation result
		typename TImage::Pointer m_vessImg;			// vesselness image
		typename TImage::Pointer m_compositeImg;	// mori + vesselness for region growing

		typename TImage::PixelType m_VessTh;		// threshold on vesselness
		typename TImage::PixelType m_vessMax;		// max vesselness value
		typename TImage::PixelType m_IntTh;			// threshold on intensity
		typename TImage::PixelType m_replaceValue;	//
		unsigned char m_numIters;					// number of algorithm passess - one iteration = up & down
		float m_beta;								// beta parameter of region growing algorithm


		void GetAllInputsTogether();				// joins m_moriImg and m_vessImg and saves in m_compositeImg;


		void GoDown(typename SliceType::Pointer* binaryTree, 
					typename SliceType::Pointer* data3D,
					typename SliceType::Pointer* fusion,
					int numSlices);					// Walker goes down

		void GoUp(typename SliceType::Pointer* binaryTree,
			typename SliceType::Pointer* data3D,
			typename SliceType::Pointer* fusion,
			int numSlices);							// Walker goes up

		void image2buffer(typename TImage::PixelType*& buffer, int *dims, double *spac, double *orig, int ndim, const typename SliceType::Pointer image);
		typename itk::Image<typename TImage::PixelType, 2>::Pointer buffer2image(typename TImage::PixelType *buffer, int *dims, double *spacing, double *origin, const int ndim);

	};
} //namespace ITK


#ifndef ITK_MANUAL_INSTANTIATION
#include "MyRandomWalkerSegmentationFilter.hxx"
#endif


#endif // __itkImageFilter_h
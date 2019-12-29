#ifndef __itkMyMoriSegmentationFilter_h
#define __itkMyMoriSegmentationFilter_h

#include "itkImageToImageFilter.h"
#include "itkConnectedThresholdImageFilter.h"

namespace itk{

	template<typename TImageType>
	class MyMoriSegmentationFilter : public ImageToImageFilter<TImageType, TImageType>
	{

	public:

		/** Standard class typedefs. */
		typedef MyMoriSegmentationFilter Self;
		typedef ImageToImageFilter<TImageType, TImageType> Superclass;
		typedef SmartPointer<Self>  Pointer;

		/** Method for creation through the object factory. */
		itkNewMacro(Self);

		/** Run-time type information (and related methods). */
		itkTypeMacro(MyMoriSegmentationFilter, ImageToImageFilter);

		typedef typename TImageType::IndexType IndexType;
		typedef typename TImageType::PixelType PixelType;
		typedef typename TImageType::RegionType RegionType;
		typedef typename TImageType::SizeType SizeType;

		typedef typename std::vector<IndexType> SeedContainerType;
		SeedContainerType m_Seeds;
		
		void SetSeed(const typename IndexType &seed);
		void SetSeeds(const typename SeedContainerType &seeds);
		void SaveAsMHD(std::string path);

		void PrintSelf(std::ostream & os, Indent indent) const;

		void ClearSeeds(){};

	protected:

		typename TImageType::IndexType seed;							//seed for Mori region growing
		typename TImageType::PixelType initialIntensityThreshold;		//initial intensity threshold - used for the first run 
		typename TImageType::PixelType finalIntensityThreshold;			//final intensity threshold - when to finish growing the tree;
		typename TImageType::PixelType replaceValue;					//label assigned to airway tree;
		float fracThreshold;											//tree volume increase for indicating the leakage
		int step;														//step for intensity threshold increase during the consecutive runs of region growing

		MyMoriSegmentationFilter();
		~MyMoriSegmentationFilter(){};

		/** Does the real work. */
		virtual void GenerateData();

		/**  Create the Output */
		DataObject::Pointer MakeOutput();

	private:

		MyMoriSegmentationFilter(const Self&);
		void operator=(const Self&);

		typename TImageType::Pointer BAT_ConnectedThreshold(typename TImageType::PixelType lower, typename TImageType::PixelType upper);
		int BAT_volume(typename TImageType::Pointer BAT);											//determines volume (in pixels) of binary airway tree 

	};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "MyMoriSegmentationFilter.hxx"
#endif


#endif
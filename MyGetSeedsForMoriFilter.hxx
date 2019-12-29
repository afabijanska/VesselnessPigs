#ifndef __MyGetSeedsForMoriFilter_hxx
#define __MyGetSeedsForMoriFilter_hxx

#include "MyGetSeedsForMoriFilter.h"
#include "itkObjectFactory.h"

#include "itkBinaryThresholdImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkConnectedComponentImageFilter.h"
#include "itkExtractImageFilter.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkLabelShapeKeepNObjectsImageFilter.h"
#include "itkMedianImageFilter.h"

#include <QuickView.h>

namespace itk
{
/*** MyGetSeedsForMoriFilter() ***/

	template< class TImage>
	MyGetSeedsForMoriFilter<TImage>
		::MyGetSeedsForMoriFilter()
		{
			this->m_Seeds.clear();
			this->m_Lower = -1024;
			this->m_Upper = -850;
			this->m_InsideValue = 255;
			this->m_OutsideValue = 0;
		}

/***   void GenerateData()   ***/

	template< class TImage>
	void MyGetSeedsForMoriFilter<TImage>
		::GenerateData()
		{
			typename TImage::ConstPointer input = this->GetInput();
			typename TImage::Pointer output = this->GetOutput();

			TImage::SpacingType spacing = input->GetSpacing();
			
			int sliceId = 0;
			TImage::SpacingValueType offset = 0;

			while (offset < 65) //how deep is the slice of trachea containing round seed region 
			{
				offset += spacing[2];
				sliceId++;
			}
			
			/*extract signle slice for seeds*/
			typename TImage::RegionType inputRegion = input->GetLargestPossibleRegion();
			typename TImage::SizeType size = inputRegion.GetSize();
			sliceId = size[2] - sliceId;
			size[2] = 0;

			typename TImage::IndexType start = inputRegion.GetIndex();
			start[2] = sliceId;

			typename TImage::RegionType desiredRegion(start, size);

			typedef itk::Image<typename TImage::PixelType, 2> SliceType;
			typedef itk::ExtractImageFilter<TImage, SliceType> SliceExtractorType;

			SliceExtractorType::Pointer sliceExtractor = SliceExtractorType::New();
			sliceExtractor->SetExtractionRegion(desiredRegion);
			sliceExtractor->SetInput(input);

#if ITK_VERSION_MAJOR >= 4
			sliceExtractor->SetDirectionCollapseToIdentity(); // This is required.
#endif
			SliceType::Pointer slice;

			try{

				sliceExtractor->Update();
				slice = sliceExtractor->GetOutput();
			}
			catch (itk::ExceptionObject &ex){

				std::cout << ex << std::endl << "Problem z wydobyciem przekroju ze stosu" << std::endl;
				sliceExtractor = NULL;
			}

			QuickView viewer;
			viewer.AddImage(slice.GetPointer());
			viewer.Visualize();

			/*process the slice to find trachea*/
			/*thresholding*/

			typedef itk::BinaryThresholdImageFilter <SliceType, SliceType> ThresholderType;
			ThresholderType::Pointer thresholder = ThresholderType::New();
			thresholder->SetInput(slice);
			thresholder->SetLowerThreshold(this->m_Lower);
			thresholder->SetUpperThreshold(this->m_Upper);
			thresholder->SetInsideValue(this->m_InsideValue);
			thresholder->SetOutsideValue(this->m_OutsideValue);
			thresholder->Update();

			SliceType::Pointer temp = thresholder->GetOutput();
			
			viewer.AddImage(temp.GetPointer());
			viewer.Visualize();

			/*median filter to remove noise*/
			
			typedef itk::MedianImageFilter<SliceType, SliceType> MedianFilterType;
			MedianFilterType::Pointer medfilt = MedianFilterType::New();
			medfilt->SetInput(temp);
			medfilt->SetRadius(7);
			medfilt->Update();

			SliceType::Pointer temp2 = medfilt->GetOutput();

			viewer.AddImage(temp2.GetPointer());
			viewer.Visualize();

			/*select the most round object*/
			
			typedef itk::Image<unsigned short, 2> SliceTypeBW;
			typedef itk::CastImageFilter <SliceType, SliceTypeBW> ImageCasterType;

			ImageCasterType::Pointer caster = ImageCasterType::New();
			caster->SetInput(temp2);
			caster->Update();

			typedef itk::ConnectedComponentImageFilter <SliceTypeBW, SliceTypeBW> ConnectedComponentImageFilterType;
			ConnectedComponentImageFilterType::Pointer connected = ConnectedComponentImageFilterType::New();
			connected->SetInput(caster->GetOutput());
			connected->Update();

			std::cout << "Number of objects: " << connected->GetObjectCount() << std::endl;
	
			typedef itk::LabelShapeKeepNObjectsImageFilter<SliceTypeBW> LabelShapeKeepNObjectsImageFilterType;
			LabelShapeKeepNObjectsImageFilterType::Pointer labelShapeKeepNObjectsImageFilter = LabelShapeKeepNObjectsImageFilterType::New();
			labelShapeKeepNObjectsImageFilter->SetInput(connected->GetOutput());
			labelShapeKeepNObjectsImageFilter->SetBackgroundValue(0);
			labelShapeKeepNObjectsImageFilter->SetNumberOfObjects(1);
			labelShapeKeepNObjectsImageFilter->SetAttribute(LabelShapeKeepNObjectsImageFilterType::LabelObjectType::ROUNDNESS);
			labelShapeKeepNObjectsImageFilter->Update();

			SliceTypeBW::Pointer temp3 = labelShapeKeepNObjectsImageFilter->GetOutput();

			viewer.AddImage(temp3.GetPointer());
			viewer.Visualize();

			this->AllocateOutputs();
			output->FillBuffer(itk::NumericTraits<PixelType>::Zero);

			itk::ImageRegionIteratorWithIndex<SliceTypeBW> imageIterator(temp3, temp3->GetLargestPossibleRegion());

			while (!imageIterator.IsAtEnd())
			{
				if (imageIterator.Value() != 0)
				{
					SliceTypeBW::IndexType id2D = imageIterator.GetIndex();
					TImage::IndexType id3D;
					id3D[0] = id2D[0];
					id3D[1] = id2D[1];
					id3D[2] = sliceId;
					
					output->SetPixel(id3D, 255);
					this->m_Seeds.push_back(id3D);
				}

				++imageIterator;
			}

			std::cout << "Seeds done..." << std::endl;
			return;
		}

/***   SeedContainerType GetSeeds()   ***/

	template< class TImage>
	const typename MyGetSeedsForMoriFilter<TImage>::SeedContainerType &
		MyGetSeedsForMoriFilter<TImage>
		::GetSeeds() const
		{
			itkDebugMacro("returning Seeds");
			return this->m_Seeds;
		}

}// end namespace


#endif

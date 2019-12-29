#ifndef __itkMyMoriSegmentationFilter_hxx
#define __itkMyMoriSegmentationFilter_hxx

#include "MyMoriSegmentationFilter.h"
#include "itkObjectFactory.h"
#include "itkImageRegionIterator.h"
#include "itkCastImageFilter.h"
#include "itkImageFileWriter.h"

namespace itk{

	/*******************************************************/
	/*		MyMoriSegmentationFilter()			     	   */	
	/*******************************************************/

	template<typename TImageType>
	MyMoriSegmentationFilter<TImageType>
		::MyMoriSegmentationFilter()
		{
			this->SetNumberOfRequiredInputs(1);
			this->SetNumberOfRequiredOutputs(1);

			this->SetNthOutput(0, this->MakeOutput());

			this->initialIntensityThreshold = -900;				//initial intensity threshold - used for the first run 
			this->finalIntensityThreshold = -650;				//final intensity threshold - when to finish growing the tree if the leakage does not appear;
			this->replaceValue = 255;							//label assigned to airway tree;
			this->fracThreshold = 10;							//tree volume increase for indicating the leakage in %
			this->step = 10;									//step for intensity threshold increase during the consecutive runs of region growing
			this->m_Seeds.clear();
		}

	/*******************************************************/
	/*				void GenerateData()			     	   */
	/*******************************************************/

	template<typename TImageType>
	void MyMoriSegmentationFilter<TImageType>
		::GenerateData()
		{
			typename TImageType::ConstPointer	input = this->GetInput();
			typename TImageType::RegionType		region = input->GetLargestPossibleRegion();
			typename TImageType::SizeType		size = region.GetSize();
			typename TImageType::SpacingType	spacing = input->GetSpacing();
			typename TImageType::PointType		origin = input->GetOrigin();
			typename TImageType::DirectionType	direction = input->GetDirection();
			typename TImageType::IndexType		start = region.GetIndex();
			
			typename TImageType::Pointer output = this->GetOutput();
			output->SetRegions(region);
			output->SetSpacing(spacing);
			output->SetOrigin(origin);
			output->SetDirection(direction);

			typename TImageType::PixelType grayLevel = this->initialIntensityThreshold;
			typename TImageType::Pointer oldBAT = this->BAT_ConnectedThreshold(-1024, grayLevel);

			float oldNum = this->BAT_volume(oldBAT);

			typename TImageType::Pointer newBAT = TImageType::New();
			typename TImageType::Pointer toBeUsed = TImageType::New();

			float newNum = oldNum;

			float frac = 100 * (newNum - oldNum) / oldNum;

			while (frac < this->fracThreshold && grayLevel < this->finalIntensityThreshold){
				std::cout << "th: " << grayLevel << std::endl;
				toBeUsed = oldBAT;
				grayLevel += this->step;
				oldNum = this->BAT_volume(oldBAT);
				newBAT = this->BAT_ConnectedThreshold(-1024, grayLevel);
				newNum = this->BAT_volume(newBAT);

				frac = 100 * (newNum - oldNum) / oldNum;

				std::cout << grayLevel << " : " << newNum << "(" << frac << "%)" << std::endl;

				oldBAT = newBAT;
			}

			std::cout << "done" << std::endl;

			this->AllocateOutputs();
			ImageAlgorithm::Copy(toBeUsed.GetPointer(), output.GetPointer(), toBeUsed->GetRequestedRegion(), output->GetRequestedRegion());				
		}

	/*******************************************************/
	/*          DataObject::Pointer MakeOutput()     	   */
	/*******************************************************/

	template<typename TImageType>
	DataObject::Pointer MyMoriSegmentationFilter<TImageType>
		::MakeOutput()
		{
			DataObject::Pointer output;

			output = (TImageType::New()).GetPointer();

			return output.GetPointer();
		}

	/*******************************************************/
	/*	int BAT_volume (typename TImageType::Pointer BAT)  */
	/*******************************************************/

	template<typename TImageType>
	int MyMoriSegmentationFilter<TImageType>
		::BAT_volume(typename TImageType::Pointer BAT)
		{
			int num = 0;
			
			typedef itk::ImageRegionIterator<TImageType> IteratorType;
			IteratorType it(BAT, BAT->GetRequestedRegion());
			
			for (it.GoToBegin(); !it.IsAtEnd(); it++){

				if (it.Get() == this->replaceValue){
					num++;
				}
			}

			return num;
		}


	/*******************************************************/
	/*				   PrintSelf				     	   */
	/*******************************************************/

	template<typename TImageType>
	void MyMoriSegmentationFilter<TImageType>
		::PrintSelf(std::ostream & os, Indent indent) const
		{
			this->Superclass::PrintSelf(os, indent);
			os << indent << "Initial intensity threshold: "
				<< static_cast< typename NumericTraits< TImageType::PixelType >::PrintType >(this->initialIntensityThreshold)
				<< std::endl;
			os << indent << "Final intensity treshold: "
				<< static_cast< typename NumericTraits< TImageType::PixelType >::PrintType >(this->finalIntensityThreshold)
				<< std::endl;
			os << indent << "Seed coordinates: "
				<< static_cast< typename NumericTraits< TImageType::IndexType >::PrintType >(this->seed)
				<< std::endl;
			os << indent << "Intensity step: " << this->step << std::endl;
			os << indent << "Threshold for leakage (in %): " << this->fracThreshold << std::endl;
			os << indent << "Replace value: "
				<< static_cast< typename NumericTraits< TImageType::PixelType >::PrintType >(this->replaceValue)
				<< std::endl;
		}

	/*******************************************************/
	/*					 SetSeed				     	   */
	/*******************************************************/

	template<typename TImageType>
	void MyMoriSegmentationFilter<TImageType>
		::SetSeed(const typename TImageType::IndexType & seed)
		{
			this->seed = seed;
		}

	/*******************************************************/
	/*					 SetSeeds				     	   */
	/*******************************************************/

	template<typename TImageType>
	void MyMoriSegmentationFilter<TImageType>
		::SetSeeds(const typename MyMoriSegmentationFilter<TImageType>::SeedContainerType & seeds)
		{
			this->m_Seeds = seeds;
		}
	
	/*******************************************************/
	/*		         BAT_ConnectedThreshold		     	   */
	/*******************************************************/

	template<typename TImageType>
	typename TImageType::Pointer MyMoriSegmentationFilter<TImageType>
		::BAT_ConnectedThreshold(typename TImageType::PixelType lower, typename TImageType::PixelType upper)
		{
			typedef itk::ConnectedThresholdImageFilter<TImageType, TImageType> ConnectedFilterType;
			ConnectedFilterType::Pointer connectedThreshold = ConnectedFilterType::New();
			connectedThreshold->SetInput(this->GetInput());
			connectedThreshold->SetLower(lower);
			connectedThreshold->SetUpper(upper);
			connectedThreshold->SetReplaceValue(this->replaceValue);
			//connectedThreshold->SetSeed(this->seed);
			
			int k=0;

			for (SeedContainerType::iterator it = this->m_Seeds.begin(); it != this->m_Seeds.end(); ++it, k++)
			{
				if (k==0)
					connectedThreshold->SetSeed(*it);
				else
					connectedThreshold->AddSeed(*it);
				k++;
			}
				
			connectedThreshold->Update();
			
			typename TImageType::Pointer BinaryAirwayTree = connectedThreshold->GetOutput();
			return BinaryAirwayTree;
		}

	/*******************************************************/
	/*		    void SaveAsMHD(std::string path)     	   */
	/*******************************************************/

	template<typename TImageType>
	void MyMoriSegmentationFilter<TImageType>
		::SaveAsMHD(std::string path){

			typename TImageType::ConstPointer	input = this->GetInput();
			typename TImageType::RegionType		region = input->GetLargestPossibleRegion();
			typename TImageType::SpacingType	spacing = input->GetSpacing();
			typename TImageType::PointType		origin = input->GetOrigin();
			typename TImageType::DirectionType	direction = input->GetDirection();

			typedef unsigned char OutputPixelType;
			typedef itk::Image<OutputPixelType, 3> OutputImageType;
			OutputImageType::Pointer outImg;

			typedef itk::CastImageFilter<typename TImageType, OutputImageType> CasterType;
			CasterType::Pointer caster = CasterType::New();
			caster->SetInput(this->GetOutput());
			caster->Update();
			outImg = caster->GetOutput();
			outImg->SetRegions(region);
			outImg->SetSpacing(spacing);
			outImg->SetOrigin(origin);
			outImg->SetDirection(direction);

			typedef itk::ImageFileWriter<OutputImageType> WriterType;
			WriterType::Pointer writer = WriterType::New();
			writer->SetInput(outImg);
			writer->SetFileName(path);
			
			try{

				writer->Update();
			}
			catch (itk::ExceptionObject &excep){

				std::cerr << "Exception caught !" << std::endl;
				std::cerr << excep << std::endl;
			}

		}
}


#endif

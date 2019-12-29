#ifndef __MyVesselnessImageFilter_hxx
#define __MyVesselnessImageFilter_hxx

#include "MyVesselnessImageFilter.h"
#include "itkObjectFactory.h"

namespace itk
{
	template<typename TImage>
	MyVesselnessImageFilter< TImage>
		::MyVesselnessImageFilter()
	{
			this->m_mmcalc = MinMaxCalcType::New();
			this->m_inverter = ImageInverterType::New();
			
			this->m_hessianFilter = HessianFilterType::New();
			this->m_hessianFilter->SetSigma(static_cast< double >(0.5));
			
			this->m_vesselnessFilter = VesselnessMeasureFilterType::New();
			this->m_vesselnessFilter->SetAlpha1(static_cast< double >(0.5));
			this->m_vesselnessFilter->SetAlpha2(static_cast< double >(2));

	}

	template<typename TImage>
	void MyVesselnessImageFilter<TImage>
		::GenerateData()
	{
			typename TImage::Pointer input = TImage::New();
			input->Graft(const_cast<TImage *>(this->GetInput()));

			typename TImage::Pointer output = this->GetOutput();
			//output->SetBufferedRegion(output->GetRequestedRegion());
			//output->Allocate();

			this->m_mmcalc->SetImage(input);
			this->m_mmcalc->Compute();
			this->m_inverter->SetMaximum(m_mmcalc->GetMaximum());

			std::cout << "min: " << m_mmcalc->GetMinimum() << " max: " << m_mmcalc->GetMaximum() << std::endl;
			
			this->m_inverter->SetInput(input);

			//this->m_hessianFilter->GraftOutput(output);
			this->m_hessianFilter->SetInput(m_inverter->GetOutput());
			this->m_hessianFilter->Update();

			this->m_vesselnessFilter->SetInput(m_hessianFilter->GetOutput());
			this->m_vesselnessFilter->Update();
			//this->GraftOutput(m_vesselnessFilter->GetOutput());
			
			typename TImage::Pointer tmp = this->m_vesselnessFilter->GetOutput();
			this->AllocateOutputs();
			ImageAlgorithm::Copy(tmp.GetPointer(), output.GetPointer(), tmp->GetRequestedRegion(), output->GetRequestedRegion());
		}

}// end namespace

#endif
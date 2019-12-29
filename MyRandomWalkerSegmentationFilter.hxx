#ifndef __MyRandomWalkerSegmentationFilter_hxx
#define __MyRandomWalkerSegmentationFilter_hxx

#include "MyRandomWalkerSegmentationFilter.h"
#include "MyMoriSegmentationFilter.h"
#include "MyGetSeedsForMoriFilter.h"
#include "MyVesselnessImageFilter.h"

#include "itkObjectFactory.h"
#include "itkMaximumImageFilter.h"
#include "itkImageRegionIterator.h"
#include "itkImageRegionConstIterator.h"
#include "itkImageSliceIteratorWithIndex.h"
#include "itkMinimumMaximumImageCalculator.h"
#include "itkImageSliceConstIteratorWithIndex.h"
#include "itkJoinSeriesImageFilter.h"
#include "itkImageRegionIteratorWithIndex.h"

#include "itkExtractImageFilter.h"
#include "itkImportImageFilter.h"
#include "itkPasteImageFilter.h"

#include "random_walker.h"
#include "call_walker.h"

#pragma comment(lib,"random_walker.lib")

#include <QuickView.h>

extern int call_walker(PixelType *mask, double *probs, PixelType *image, int width, int height, int *seed_indexes,
	int no_seeds, int *seed_labels, int no_labels, double beta);

namespace itk
{

	/*********************************************************/
	/*					void GenerateData()					 */
	/*********************************************************/

	template< typename TImage>
	void MyRandomWalkerSegmentationFilter<TImage>
		::GenerateData()
	{
			typename TImage::ConstPointer	input = this->GetInput();
			typename TImage::RegionType		region = input->GetLargestPossibleRegion();
			typename TImage::SizeType		size = region.GetSize();
			typename TImage::SpacingType	spacing = input->GetSpacing();
			typename TImage::PointType		origin = input->GetOrigin();
			typename TImage::DirectionType	direction = input->GetDirection();
			typename TImage::IndexType		start = region.GetIndex();
			
			typename TImage::Pointer output = this->GetOutput();
			output->SetRegions(region);
			output->SetSpacing(spacing);
			output->SetOrigin(origin);
			output->SetDirection(direction);

			m_compositeImg->SetRegions(region);
			m_compositeImg->Allocate();
			m_compositeImg->FillBuffer(0);
			m_compositeImg->SetOrigin(origin);
			m_compositeImg->SetSpacing(spacing);

			this->GetAllInputsTogether();

			typedef itk::ExtractImageFilter<TImage, SliceType> ExtractFilterType;
			ExtractFilterType::Pointer sliceExtractorMori = ExtractFilterType::New();
			ExtractFilterType::Pointer sliceExtractorComp = ExtractFilterType::New();
			ExtractFilterType::Pointer sliceExtractorImg = ExtractFilterType::New();

			int i;
			int numSlices = size[2];
			size[2] = 0;
			std::cout << "Num slices: " << numSlices << std::endl;
			
			SliceType::Pointer* binaryTree = new SliceType::Pointer[numSlices];
			SliceType::Pointer* data3D = new SliceType::Pointer[numSlices];
			SliceType::Pointer* fusion = new SliceType::Pointer[numSlices];

			for (i = 0; i < numSlices; i++)
			{
				std::cout << "extracting slices (" << i << ")" << std::endl;
				start[2] = i;
				
				typename TImage::RegionType desiredRegion(start, size);
				
				sliceExtractorMori->SetExtractionRegion(desiredRegion);
				sliceExtractorMori->SetInput(m_moriImg);
				sliceExtractorMori->SetDirectionCollapseToIdentity(); // This is required.

				sliceExtractorComp->SetExtractionRegion(desiredRegion);
				sliceExtractorComp->SetInput(m_compositeImg);
				sliceExtractorComp->SetDirectionCollapseToIdentity(); // This is required.

				sliceExtractorImg->SetExtractionRegion(desiredRegion);
				sliceExtractorImg->SetInput(input);
				sliceExtractorImg->SetDirectionCollapseToIdentity(); // This is required.			

				try
				{
					sliceExtractorMori->Update();
					SliceType::Pointer sth = sliceExtractorMori->GetOutput();

					SliceType::RegionType	reg = sth->GetLargestPossibleRegion();
					SliceType::SizeType		siz = reg.GetSize();
					SliceType::SpacingType	spc = sth->GetSpacing();
					SliceType::PointType	ori = sth->GetOrigin();
					SliceType::IndexType	start = reg.GetIndex();

					binaryTree[i] = SliceType::New();
					binaryTree[i]->SetRegions(reg);
					binaryTree[i]->SetOrigin(ori);
					binaryTree[i]->SetSpacing(spc);
					binaryTree[i]->Allocate();
					
					ImageAlgorithm::Copy(sth.GetPointer(), binaryTree[i].GetPointer(), sth->GetRequestedRegion(), binaryTree[i]->GetRequestedRegion());

					sliceExtractorComp->Update();
					sth = sliceExtractorComp->GetOutput();
					reg = sth->GetLargestPossibleRegion();
					siz = reg.GetSize();
					spc = sth->GetSpacing();
					ori = sth->GetOrigin();
					start = reg.GetIndex();

					fusion[i] = SliceType::New();
					fusion[i]->SetRegions(reg);
					fusion[i]->SetOrigin(ori);
					fusion[i]->SetSpacing(spc);
					fusion[i]->Allocate();

					ImageAlgorithm::Copy(sth.GetPointer(), fusion[i].GetPointer(), sth->GetRequestedRegion(), fusion[i]->GetRequestedRegion());

					sliceExtractorImg->Update();
					sth = sliceExtractorImg->GetOutput();
					reg = sth->GetLargestPossibleRegion();
					siz = reg.GetSize();
					spc = sth->GetSpacing();
					ori = sth->GetOrigin();
					start = reg.GetIndex();

					data3D[i] = SliceType::New();
					data3D[i]->SetRegions(reg);
					data3D[i]->SetOrigin(ori);
					data3D[i]->SetSpacing(spc);
					data3D[i]->Allocate();

					ImageAlgorithm::Copy(sth.GetPointer(), data3D[i].GetPointer(), sth->GetRequestedRegion(), data3D[i]->GetRequestedRegion());

				}
				catch (itk::ExceptionObject &ex)
				{
					std::cout << ex << std::endl;
					std::cout << "Problem z wydobyciem przekroju ze stosu" << std::endl;
					exit(0);
				}
			}
			
			/***** WALKER GOES SLICE-BY-SLICE - FIRST RUN  *****/

			std::cout << " ----------------> THE WALKER GOES DOWN " << std::endl;
			this->GoDown(binaryTree, data3D, fusion, numSlices); 

			std::cout << " ----------------> THE WALKER GOES UP " << std::endl;
			this->GoUp(binaryTree, data3D, fusion, numSlices);

			/*if the lines below may be uncommented to get more  branches*/
			
			/***** WALKER GOES SLICE-BY-SLICE - SECOND RUN  *****/

			/*std::cout << " ----------------> THE WALKER GOES DOWN 2" << std::endl;
			this->GoDown(binaryTree, data3D, fusion, numSlices);

			std::cout << " ----------------> THE WALKER GOES UP 2" << std::endl;
			this->GoUp(binaryTree, data3D, fusion, numSlices);*/

			std::cout << "---------------- slice joiner goes here -----------------" << std::endl;
				
			typedef itk::JoinSeriesImageFilter<SliceType, typename TImage> VolumeJoinerType;
			VolumeJoinerType::Pointer joiner = VolumeJoinerType::New();
			
			for (i = 0; i < numSlices; i++)
			{
				joiner->SetInput(i, binaryTree[i]);

			}
			
			joiner->Update();

			typename TImage::Pointer f = joiner->GetOutput();

			std::cout << "---------------- slice joiner done -----------------" << std::endl;

			this->AllocateOutputs();
			ImageAlgorithm::Copy(f.GetPointer(), output.GetPointer(), f->GetRequestedRegion(), output->GetRequestedRegion());

			delete[] binaryTree;
			delete[] data3D;
			delete[] fusion;
			
		}

	/*********************************************************/
	/* MyRandomWalkerSegmentationFilter()					 */
	/*********************************************************/

	template< typename TImage>
	MyRandomWalkerSegmentationFilter<TImage>
		::MyRandomWalkerSegmentationFilter()
		{	
			this->m_moriImg = NULL;								// mori segmentation result
			this->m_vessImg = NULL;								// vesselness image
			this->m_compositeImg = typename TImage::New();		// mori + vesselness for region growing
			
			this->m_replaceValue = 255;
			this->m_VessTh = 5;									// threshold on vesselness in %
			this->m_IntTh = -650;								// threshold on intensity
			this->m_numIters = 2;								// number of algorithm passess - one iteration = up & down
			this->m_beta = 30;									// beta parameter of region growing algorithm
			this->m_vessMax = 0;
		}

	/*********************************************************/
	/* void SetMoriImg(const typename TImage::Pointer)       */
	/*********************************************************/

	template< typename TImage>
	void MyRandomWalkerSegmentationFilter<TImage>
		::SetMoriImg(typename TImage::Pointer moriImg)
		{
			this->m_moriImg = moriImg;
		}
	
	/*********************************************************/
	/* void SetVesselnessImg(const typename TImage::Pointer) */
	/*********************************************************/

	template< typename TImage>
	void MyRandomWalkerSegmentationFilter<TImage>
		::SetVesselnessImg(typename TImage::Pointer vessImg)
		{
			this->m_vessImg = vessImg;

			typedef MinimumMaximumImageCalculator<TImage> MinMaxCalcType;
			typename MinMaxCalcType::Pointer m_mmcalc = MinMaxCalcType::New();
			m_mmcalc->SetImage(vessImg);
			m_mmcalc->Compute();

			this->m_vessMax = m_mmcalc->GetMaximum();
			std::cout << "-----> Max Vess: " << this->m_vessMax << std::endl;
		}

	/*********************************************************/
	/* void GetAllInputsTogether()							 */
	/*********************************************************/

	template< typename TImage>
	void MyRandomWalkerSegmentationFilter<TImage>
		::GetAllInputsTogether()
		{
			if (m_moriImg.IsNull())
			{
				std::cout << "mori image not provided" << std::endl;
				exit(0);
			}

			if (m_vessImg.IsNull())
			{
				std::cout << "vesselness image not provided" << std::endl;
				exit(0);
			}

			if (m_compositeImg.IsNull())
			{
				std::cout << "composite image not allocated" << std::endl;
				exit(0);
			}

			ImageAlgorithm::Copy(m_vessImg.GetPointer(), m_compositeImg.GetPointer(), m_vessImg->GetRequestedRegion(), m_compositeImg->GetRequestedRegion());

			// get max vesselness

			typedef MinimumMaximumImageCalculator<TImage> MinMaxCalcType;
			MinMaxCalcType::Pointer mmcalc = MinMaxCalcType::New();
			mmcalc->SetImage(m_vessImg);
			mmcalc->Compute();

			std::cout << "** max vesselness: " << mmcalc->GetMaximum() << std::endl; 
			std::cout << "** min vesselness: " << mmcalc->GetMinimum() << std::endl;
			
			// replace coarse airway tree in the vesselness image with max vesselness value

			typedef itk::ImageRegionIteratorWithIndex<TImage> IteratorType;
			IteratorType it(m_moriImg, m_moriImg->GetRequestedRegion());

			for (it.GoToBegin(); !it.IsAtEnd(); ++it)
			{
				typename TImage::IndexType idx = it.GetIndex();
				typename TImage::PixelType val = m_moriImg->GetPixel(idx);

				if (val != 0)
				{
					m_compositeImg->SetPixel(idx, mmcalc->GetMaximum());
				}
			}

		}

	/*********************************************************/
	/* void image2buffer()									 */
	/*********************************************************/

	template< typename TImage>
	void MyRandomWalkerSegmentationFilter<TImage>
		::image2buffer(typename TImage::PixelType*& buffer, int *dims, double *spac, double *orig, int ndim, const typename SliceType::Pointer image)
		{
			int i, siz;

			typename SliceType::RegionType region = image->GetBufferedRegion();
			typename SliceType::SizeType size = region.GetSize();
			typename SliceType::IndexType start = region.GetIndex();

			siz = 1;
			for (i = 0; i<ndim; i++)
			{
				dims[i] = size[i];
				siz *= dims[i];
			}
			
			typename SliceType::SpacingType  spacing = image->GetSpacing();

			for (i = 0; i<ndim; i++)
				spac[i] = spacing[i];

			typename SliceType::PointType  origin = image->GetOrigin();

			for (i = 0; i<ndim; i++)
				orig[i] = origin[i];

			buffer = new PixelType[siz];

			typedef itk::ImageRegionConstIterator<SliceType> IteratorType;
			IteratorType it(image, region);
			it.GoToBegin();
			PixelType *data = buffer;
			while (!it.IsAtEnd())
			{
				*data = it.Get();
				++it;
				++data;
			}

		}
	
	/*********************************************************/
	/* void buffer2image()									 */
	/*********************************************************/
	
	template< typename TImage>
	typename itk::Image<typename TImage::PixelType, 2>::Pointer MyRandomWalkerSegmentationFilter<TImage>
		::buffer2image(typename TImage::PixelType *buffer, int *dims, double *spacing, double *origin, const int ndim)
		{
			typedef itk::Image<typename TImage::PixelType, 2> SliceType;

			int i, siz;

			typedef itk::ImportImageFilter< PixelType, 2> ImportFilterType;
			ImportFilterType::Pointer importer = ImportFilterType::New();
			typename SliceType::SizeType size;
			siz = 1;
			for (i = 0; i<ndim; i++)
			{
			size[i] = dims[i];
			siz *= dims[i];
			}

			typename SliceType::IndexType start;

			for (i = 0; i<ndim; i++)
			start[i] = 0;

			typename SliceType::RegionType region;
			region.SetSize(size);
			region.SetIndex(start);

			importer->SetRegion(region);
			importer->SetSpacing(spacing);
			importer->SetOrigin(origin);

			const bool importFilterWillDeleteTheInputBuffer = false;
			typename SliceType::PixelType* pixelData = static_cast<typename SliceType::PixelType*>(buffer);
			importer->SetImportPointer(pixelData, siz, importFilterWillDeleteTheInputBuffer);
			importer->Update();

			return importer->GetOutput();
		}

	/*********************************************************/
	/*						void GoDown()					 */
	/*********************************************************/

	template<typename TImage>
	void MyRandomWalkerSegmentationFilter<TImage>
		::GoDown(typename SliceType::Pointer* binaryTree,
		typename SliceType::Pointer* data3D,
		typename SliceType::Pointer* fusion,
		int numSlices){

			int z = -1;

			while (z < numSlices - 2)
			{
				z++;
				std::cout << "Slice: " << z << std::endl;

				SliceType::Pointer tmp = binaryTree[z];
				SliceType::Pointer next = binaryTree[z + 1];
				SliceType::Pointer data = data3D[z + 1];
				SliceType::Pointer vess = fusion[z + 1];

				SliceType::RegionType	sliceRegion = tmp->GetLargestPossibleRegion();
				SliceType::SizeType		size = sliceRegion.GetSize();
				SliceType::SpacingType	spacing = tmp->GetSpacing();
				SliceType::PointType	origin = tmp->GetOrigin();
				SliceType::IndexType	start = sliceRegion.GetIndex();

				SliceType::Pointer seedsObject = SliceType::New();
				seedsObject->SetRegions(sliceRegion);
				seedsObject->SetOrigin(origin);
				seedsObject->SetSpacing(spacing);
				seedsObject->Allocate();
				seedsObject->FillBuffer(0);

				SliceType::Pointer seedsBackground = SliceType::New();
				seedsBackground->SetRegions(sliceRegion);
				seedsBackground->SetOrigin(origin);
				seedsBackground->SetSpacing(spacing);
				seedsBackground->Allocate();
				seedsBackground->FillBuffer(0);

				typedef itk::ImageRegionIteratorWithIndex<SliceType> SliceIteratorType;
				SliceIteratorType itSliceMori(tmp, tmp->GetLargestPossibleRegion());

				int len = 0;
				int numO = 0;
				int numB = 0;

				itSliceMori.GoToBegin();

				while (!itSliceMori.IsAtEnd())
				{
					if (itSliceMori.Get() != 0)
						len++;

					typename TImage::PixelType vessVal = vess->GetPixel(itSliceMori.GetIndex());
					typename TImage::PixelType imgVal = data->GetPixel(itSliceMori.GetIndex());
					typename TImage::PixelType bw = tmp->GetPixel(itSliceMori.GetIndex());

					if (bw != 0 && vessVal > this->m_VessTh * this->m_vessMax / 100)
					{
						seedsObject->SetPixel(itSliceMori.GetIndex(), 1);
						numO++;
					}

					itSliceMori++;
				}

				if (len == 0)
				{
					std::cout << "Slice " << z << ": - brak pikseli zakwalifikowanych do drzewa" << std::endl;
					continue;
				}

				SliceIteratorType itSliceVess(vess, vess->GetLargestPossibleRegion());
				itSliceVess.GoToBegin();

				while (!itSliceVess.IsAtEnd())
				{
					if (itSliceVess.Get() < this->m_VessTh * this->m_vessMax / 100)
					{
						seedsBackground->SetPixel(itSliceVess.GetIndex(), 1);
						numB++;
					}
					itSliceVess++;

				}

				std::cout << "slice: " << z << "seeds obj: " << numO << " seeds bkg " << numB << std::endl;
				// przygotowania do Random Walkera

				typename TImage::PixelType  *bufferImg = 0;
				typename TImage::PixelType  *bufferSeedsObj = 0;
				typename TImage::PixelType  *bufferSeedsBkg = 0;

				int dims[2], dims2[2], dims3[2];
				double spac[2], orig[2], spac2[2], orig2[2], spac3[2], orig3[2];

				image2buffer(bufferImg, dims, spac, orig, 2, data);
				image2buffer(bufferSeedsObj, dims2, spac2, orig2, 2, seedsObject);
				image2buffer(bufferSeedsBkg, dims3, spac3, orig3, 2, seedsBackground);

				// TO DO HERE
				int width = dims[0];
				int height = dims[1];

				PixelType *mask = new PixelType[width*height];
				double *probs = new double[2 * width*height];

				int i, j, no_seeds = 0;

				for (i = 0; i < width*height; i++)

				if (bufferSeedsObj[i] == 1 || bufferSeedsBkg[i] == 1)
					no_seeds++;

				int *seed_indexes = new int[no_seeds];
				int *seed_labels = new int[no_seeds];

				for (j = 0, i = 0; i < width*height; i++)
				{
					if (bufferSeedsObj[i] == 1)
					{
						seed_indexes[j] = i + 1;//=i+1 (i)
						seed_labels[j] = this->m_replaceValue;
						j++;
					}
					else if (bufferSeedsBkg[i] == 1)
					{
						seed_indexes[j] = i + 1;//=i+1 (i)
						seed_labels[j] = 0;
						j++;
					}
				}

				call_walker(mask, probs, bufferImg, height, width, seed_indexes, no_seeds, seed_labels, 2, this->m_beta);

				delete[] bufferSeedsObj;
				delete[] bufferSeedsBkg;
				delete[] seed_indexes;
				delete[] seed_labels;

				SliceType::Pointer img2 = buffer2image(mask, dims, spac, orig, 2);

				SliceType::PixelType *probs_img = new PixelType[width*height];
				for (i = 0; i < width*height; i++)
					probs_img[i] = floor(255 * probs[i]);

				SliceType::Pointer img3 = buffer2image(probs_img, dims, spac, orig, 2);

				typedef itk::MaximumImageFilter<SliceType> MaxFilterType;
				MaxFilterType::Pointer maxFilter = MaxFilterType::New();
				maxFilter->SetInput(0, img2);
				maxFilter->SetInput(1, next);
				maxFilter->Update();

				SliceType::Pointer sth = maxFilter->GetOutput();
				ImageAlgorithm::Copy(sth.GetPointer(), binaryTree[z + 1].GetPointer(), sth->GetRequestedRegion(), binaryTree[z + 1]->GetRequestedRegion());

				delete[] probs;
			}
		}

	/*********************************************************/
	/*						void GoUp()						 */
	/*********************************************************/
	
	template< typename TImage>
	void MyRandomWalkerSegmentationFilter<TImage>
		::GoUp(typename SliceType::Pointer* binaryTree,
		typename SliceType::Pointer* data3D,
		typename SliceType::Pointer* fusion,
		int numSlices){

			int z = numSlices;

			while (z > 1)
			{
				z--;
				std::cout << "Slice: " << z << std::endl;

				SliceType::Pointer tmp = binaryTree[z];
				SliceType::Pointer next = binaryTree[z - 1];
				SliceType::Pointer data = data3D[z - 1];
				SliceType::Pointer vess = fusion[z - 1];

				SliceType::RegionType	sliceRegion = tmp->GetLargestPossibleRegion();
				SliceType::SizeType		size = sliceRegion.GetSize();
				SliceType::SpacingType	spacing = tmp->GetSpacing();
				SliceType::PointType	origin = tmp->GetOrigin();
				SliceType::IndexType	start = sliceRegion.GetIndex();

				SliceType::Pointer seedsObject = SliceType::New();
				seedsObject->SetRegions(sliceRegion);
				seedsObject->SetOrigin(origin);
				seedsObject->SetSpacing(spacing);
				seedsObject->Allocate();
				seedsObject->FillBuffer(0);

				SliceType::Pointer seedsBackground = SliceType::New();
				seedsBackground->SetRegions(sliceRegion);
				seedsBackground->SetOrigin(origin);
				seedsBackground->SetSpacing(spacing);
				seedsBackground->Allocate();
				seedsBackground->FillBuffer(0);

				typedef itk::ImageRegionIteratorWithIndex<SliceType> SliceIteratorType;
				SliceIteratorType itSliceMori(tmp, tmp->GetLargestPossibleRegion());

				int len = 0;
				int numO = 0;
				int numB = 0;

				itSliceMori.GoToBegin();

				while (!itSliceMori.IsAtEnd())
				{
					if (itSliceMori.Get() != 0)
						len++;

					typename TImage::PixelType vessVal = vess->GetPixel(itSliceMori.GetIndex());
					typename TImage::PixelType imgVal = data->GetPixel(itSliceMori.GetIndex());
					typename TImage::PixelType bw = tmp->GetPixel(itSliceMori.GetIndex());

					if (bw != 0 && vessVal > this->m_VessTh * this->m_vessMax / 100)
					{
						seedsObject->SetPixel(itSliceMori.GetIndex(), 1);
						numO++;
					}

					itSliceMori++;
				}

				if (len == 0)
				{
					std::cout << "Slice " << z << ": - brak pikseli zakwalifikowanych do drzewa" << std::endl;
					continue;
				}

				SliceIteratorType itSliceVess(vess, vess->GetLargestPossibleRegion());
				itSliceVess.GoToBegin();

				while (!itSliceVess.IsAtEnd())
				{
					if (itSliceVess.Get() < this->m_VessTh * this->m_vessMax / 100)
					{
						seedsBackground->SetPixel(itSliceVess.GetIndex(), 1);
						numB++;
					}
					itSliceVess++;

				}

				std::cout << "slice: " << z << "seeds obj: " << numO << " seeds bkg " << numB << std::endl;
				// przygotowania do Random Walkera

				typename TImage::PixelType  *bufferImg = 0;
				typename TImage::PixelType  *bufferSeedsObj = 0;
				typename TImage::PixelType  *bufferSeedsBkg = 0;

				int dims[2], dims2[2], dims3[2];
				double spac[2], orig[2], spac2[2], orig2[2], spac3[2], orig3[2];

				image2buffer(bufferImg, dims, spac, orig, 2, data);
				image2buffer(bufferSeedsObj, dims2, spac2, orig2, 2, seedsObject);
				image2buffer(bufferSeedsBkg, dims3, spac3, orig3, 2, seedsBackground);

				// TO DO HERE
				int width = dims[0];
				int height = dims[1];

				PixelType *mask = new PixelType[width*height];
				double *probs = new double[2 * width*height];

				int i, j, no_seeds = 0;

				for (i = 0; i < width*height; i++)

				if (bufferSeedsObj[i] == 1 || bufferSeedsBkg[i] == 1)
					no_seeds++;

				int *seed_indexes = new int[no_seeds];
				int *seed_labels = new int[no_seeds];

				for (j = 0, i = 0; i < width*height; i++)
				{
					if (bufferSeedsObj[i] == 1)
					{
						seed_indexes[j] = i + 1;//=i+1 (i)
						seed_labels[j] = this->m_replaceValue;
						j++;
					}
					else if (bufferSeedsBkg[i] == 1)
					{
						seed_indexes[j] = i + 1;//=i+1 (i)
						seed_labels[j] = 0;
						j++;
					}
				}

				call_walker(mask, probs, bufferImg, height, width, seed_indexes, no_seeds, seed_labels, 2, this->m_beta);

				delete[] bufferSeedsObj;
				delete[] bufferSeedsBkg;
				delete[] seed_indexes;
				delete[] seed_labels;

				SliceType::Pointer img2 = buffer2image(mask, dims, spac, orig, 2);

				SliceType::PixelType *probs_img = new PixelType[width*height];
				for (i = 0; i < width*height; i++)
					probs_img[i] = floor(255 * probs[i]);

				SliceType::Pointer img3 = buffer2image(probs_img, dims, spac, orig, 2);

				typedef itk::MaximumImageFilter<SliceType> MaxFilterType;
				MaxFilterType::Pointer maxFilter = MaxFilterType::New();
				maxFilter->SetInput(0, img2);
				maxFilter->SetInput(1, next);
				maxFilter->Update();

				SliceType::Pointer sth = maxFilter->GetOutput();
				ImageAlgorithm::Copy(sth.GetPointer(), binaryTree[z - 1].GetPointer(), sth->GetRequestedRegion(), binaryTree[z - 1]->GetRequestedRegion());

				delete[] probs;
			}
		}

		/*******************************************************/
		/*		    void SaveAsMHD(std::string path)     	   */
		/*******************************************************/

		template<typename TImage>
		void MyRandomWalkerSegmentationFilter<TImage>
			::SaveAsMHD(std::string path){

				typename TImage::ConstPointer	input = this->GetInput();
				typename TImage::RegionType		region = input->GetLargestPossibleRegion();
				typename TImage::SpacingType	spacing = input->GetSpacing();
				typename TImage::PointType		origin = input->GetOrigin();
				typename TImage::DirectionType	direction = input->GetDirection();

				typedef unsigned char OutputPixelType;
				typedef itk::Image<OutputPixelType, 3> OutputImageType;
				OutputImageType::Pointer outImg;

				typedef itk::CastImageFilter<typename TImage, OutputImageType> CasterType;
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

}// end namespace

#endif
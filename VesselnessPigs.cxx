#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImageToVTKImageFilter.h"
#include "itkConnectedThresholdImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkConfidenceConnectedImageFilter.h"
#include "itkImageRegionIterator.h"
#include "itkHessian3DToVesselnessMeasureImageFilter.h"
#include "itkHessianRecursiveGaussianImageFilter.h"
#include "itkInvertIntensityImageFilter.h"
#include "itkMinimumMaximumImageCalculator.h"
#include "itkImageRegionIterator.h"
#include "itkImageSliceIteratorWithIndex.h"
#include "itkFlipImageFilter.h"

#include "itkMetaImageIO.h"
#include "itkRawImageIO.h"

#include <QuickView.h>

#include "MyMoriSegmentationFilter.h"
#include "MyGetSeedsForMoriFilter.h"
#include "MyVesselnessImageFilter.h"
#include "MyRandomWalkerSegmentationFilter.h"

/******************************* TYPEDEFS *********************************/

const unsigned char Dimensions = 3;
typedef float PixelType;
typedef itk::Image<PixelType, Dimensions> ImageType;
typedef itk::ImageFileReader<ImageType> ReaderType;
typedef itk::ImageFileWriter<ImageType> WriterType2;

/********************** FUNCTION HEADERS **********************************/

ImageType::Pointer ReadMHD(std::string filePath);

void Show3D(ImageType::Pointer image);
void RunSimpleViewer(ImageType::Pointer image);

/****************************** MAIN **************************************/

int main(int argc, char* argv[]){

	/********************************** reading input dataset */

	std::vector<std::string> vecFiles;

	std::string dir("E:\\pigs\\pig_05");
	std::string dataset("expi2__1.0__B31f_46");
	std::string path_main = dir + "\\" + dataset + ".mhd";
	std::string path_mori = dir + "\\" + dataset + "_mori.mhd";
	std::string path_final = dir + "\\" + dataset + "_final.mhd";
	
	ImageType::Pointer image = ReadMHD(path_main);
	ImageType::Pointer tmp, mori, vessImg;
	
	/********************************** getting seeds */


	typedef itk::MyGetSeedsForMoriFilter <ImageType> GetSeedsFilterType;
	GetSeedsFilterType::Pointer seedsGetter = GetSeedsFilterType::New();
	seedsGetter->SetInput(image);
	seedsGetter->Update();
	tmp = seedsGetter->GetOutput();

	typedef itk::Index<3> IndexType;
	typedef std::vector<IndexType> SeedContainerType;
	SeedContainerType seeds = seedsGetter->GetSeeds();

	/********************************** Mori segmentation */

	typedef itk::MyMoriSegmentationFilter<ImageType> MyFilterType;
	MyFilterType::Pointer test = MyFilterType::New();
	test->SetInput(image);
	test->SetSeeds(seeds);
	
	/*Alternatively, instead of MyMoriSegmentationFilter::SetSeeds(), MyMoriSegmentationFilter::SetSeed() method can be called with a single user defined seed */
	
	test->Update();
	mori = test->GetOutput();

	test->SaveAsMHD(path_mori);

	/********************************** calculating vesselness */ 

	typedef itk::MyVesselnessImageFilter <ImageType> VesselnessFilterType;
	VesselnessFilterType::Pointer vesselnessFilter = VesselnessFilterType::New();
	vesselnessFilter->SetInput(image);
	vesselnessFilter->Update();
	vessImg = vesselnessFilter->GetOutput();

	/********************************** starting with random walker */

	typedef itk::MyRandomWalkerSegmentationFilter <ImageType> RWFilterType;
	RWFilterType::Pointer rwFilter = RWFilterType::New();
	
	/*all below inputs are required by my segmentation approach*/
	
	rwFilter->SetInput(image);
	rwFilter->SetMoriImg(mori);
	rwFilter->SetVesselnessImg(vessImg);
	rwFilter->Update();
	
	//tmp = rwFilter->GetOutput();

	rwFilter->SaveAsMHD(path_final);

	std::cout << "Press Enter to finish" << std::endl;
	getchar();
	return 0;

}

/********************** FUNCTION DEFINITIONS ******************************/

ImageType::Pointer ReadMHD(std::string filePath){

	ImageType::Pointer image;
	
	ReaderType::Pointer reader = ReaderType::New();
	reader->SetFileName(filePath);

	itk::MetaImageIO::Pointer MetaReader = itk::MetaImageIO::New();
	reader->SetImageIO(MetaReader);
	reader->GenerateOutputInformation();

	std::cout << "---------------Raw" << reader;
	
	try {  
		reader->Update();  
	}
	catch( std::exception & see )
	{
		std::cerr << see.what() << std::endl;
		exit(0);
	}
	catch( itk::ExceptionObject & eee )
	{
		std::cerr << eee << std::endl;
		exit(0);
	}
	
	image = reader->GetOutput();

	return image;
}
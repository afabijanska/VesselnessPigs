#include <cstdlib>
#include <windows.h>
#include <cmath>
#include "random_walker.h"
#include "call_walker.h"


using namespace std;


int call_walker(PixelType *mask, double *probs, PixelType *image, int width, int height, int *seed_indexes, 
				int no_seeds, int *seed_labels, int no_labels, double beta)
{
	
    mwSize dims[3] = {height, width, no_labels};
	int siz = width * height;
    int siz3 = height * width * no_labels;

	mclmcrInitialize();
	/*
	if (!mclInitializeApplication(NULL,0)) 
    {
        std::cerr << "could not initialize the application properly"
                   << std::endl;
    	return -1;
    } 
	*/
    if( !random_walkerInitialize() ) //grady_random_walker - nazwa biblioteki dll
    {
        std::cerr << "could not initialize the library properly"
                  << std::endl;
		return -1;
    } 
	//std::cout<<"ww="<<width<<"   hh="<<height<<std::endl;
	try
	{  
		
		mwArray msk_dat(height, width, mxDOUBLE_CLASS, mxREAL);
		msk_dat.SetData(mask,siz);
        
		mwArray prob_dat(3, dims, mxDOUBLE_CLASS, mxREAL);
		prob_dat.SetData(probs, siz3);
        

		mwArray img_dat(height, width, mxDOUBLE_CLASS, mxREAL);
		img_dat.SetData(image, siz);
       
		mwArray seed_dat(1, no_seeds, mxDOUBLE_CLASS, mxREAL);
		seed_dat.SetData(seed_indexes, no_seeds);
       
        
		mwArray label_dat(1, no_seeds, mxDOUBLE_CLASS, mxREAL);
		label_dat.SetData(seed_labels, no_seeds);
		
		mwArray beta_dat(beta);
		

        std::cout<<"start"<<std::endl;
		random_walker(2, msk_dat, prob_dat, img_dat, seed_dat, label_dat, beta_dat); //funkcja eksportowana
	    std::cout<<"stop"<<std::endl;


		msk_dat.GetData(mask, siz);
		prob_dat.GetData(probs, siz3);
		
	}
	 catch (const mwException& e)
    {
		 std::cerr << e.what() << std::endl;
		return -2;
	}
	catch(...)
	{
		std::cerr << "Unexpected error thrown" << std::endl;
		return -3;
	}
	
	random_walkerTerminate(); //grady_random_walker - nazwa biblioteki dll
    
	//mclTerminateApplication(); 

	return 0;
}


//
// MATLAB Compiler: 6.1 (R2015b)
// Date: Sun Mar 20 21:10:19 2016
// Arguments: "-B" "macro_default" "-W" "cpplib:random_walker" "-T" "link:lib"
// "-d"
// "C:\Users\Ania\Documents\MATLAB\random_walker\random_walker\for_testing"
// "-v" "C:\Users\Ania\Documents\MATLAB\random_walker\adjacency.m"
// "C:\Users\Ania\Documents\MATLAB\random_walker\dirichletboundary.m"
// "C:\Users\Ania\Documents\MATLAB\random_walker\laplacian.m"
// "C:\Users\Ania\Documents\MATLAB\random_walker\lattice.m"
// "C:\Users\Ania\Documents\MATLAB\random_walker\makeweights.m"
// "C:\Users\Ania\Documents\MATLAB\random_walker\normalize.m"
// "C:\Users\Ania\Documents\MATLAB\random_walker\random_walker.m" 
//

#ifndef __random_walker_h
#define __random_walker_h 1

#if defined(__cplusplus) && !defined(mclmcrrt_h) && defined(__linux__)
#  pragma implementation "mclmcrrt.h"
#endif
#include "mclmcrrt.h"
#include "mclcppclass.h"
#ifdef __cplusplus
extern "C" {
#endif

#if defined(__SUNPRO_CC)
/* Solaris shared libraries use __global, rather than mapfiles
 * to define the API exported from a shared library. __global is
 * only necessary when building the library -- files including
 * this header file to use the library do not need the __global
 * declaration; hence the EXPORTING_<library> logic.
 */

#ifdef EXPORTING_random_walker
#define PUBLIC_random_walker_C_API __global
#else
#define PUBLIC_random_walker_C_API /* No import statement needed. */
#endif

#define LIB_random_walker_C_API PUBLIC_random_walker_C_API

#elif defined(_HPUX_SOURCE)

#ifdef EXPORTING_random_walker
#define PUBLIC_random_walker_C_API __declspec(dllexport)
#else
#define PUBLIC_random_walker_C_API __declspec(dllimport)
#endif

#define LIB_random_walker_C_API PUBLIC_random_walker_C_API


#else

#define LIB_random_walker_C_API

#endif

/* This symbol is defined in shared libraries. Define it here
 * (to nothing) in case this isn't a shared library. 
 */
#ifndef LIB_random_walker_C_API 
#define LIB_random_walker_C_API /* No special import/export declaration */
#endif

extern LIB_random_walker_C_API 
bool MW_CALL_CONV random_walkerInitializeWithHandlers(
       mclOutputHandlerFcn error_handler, 
       mclOutputHandlerFcn print_handler);

extern LIB_random_walker_C_API 
bool MW_CALL_CONV random_walkerInitialize(void);

extern LIB_random_walker_C_API 
void MW_CALL_CONV random_walkerTerminate(void);



extern LIB_random_walker_C_API 
void MW_CALL_CONV random_walkerPrintStackTrace(void);

extern LIB_random_walker_C_API 
bool MW_CALL_CONV mlxAdjacency(int nlhs, mxArray *plhs[], int nrhs, mxArray *prhs[]);

extern LIB_random_walker_C_API 
bool MW_CALL_CONV mlxDirichletboundary(int nlhs, mxArray *plhs[], int nrhs, mxArray 
                                       *prhs[]);

extern LIB_random_walker_C_API 
bool MW_CALL_CONV mlxLaplacian(int nlhs, mxArray *plhs[], int nrhs, mxArray *prhs[]);

extern LIB_random_walker_C_API 
bool MW_CALL_CONV mlxLattice(int nlhs, mxArray *plhs[], int nrhs, mxArray *prhs[]);

extern LIB_random_walker_C_API 
bool MW_CALL_CONV mlxMakeweights(int nlhs, mxArray *plhs[], int nrhs, mxArray *prhs[]);

extern LIB_random_walker_C_API 
bool MW_CALL_CONV mlxNormalize(int nlhs, mxArray *plhs[], int nrhs, mxArray *prhs[]);

extern LIB_random_walker_C_API 
bool MW_CALL_CONV mlxRandom_walker(int nlhs, mxArray *plhs[], int nrhs, mxArray *prhs[]);


#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

/* On Windows, use __declspec to control the exported API */
#if defined(_MSC_VER) || defined(__BORLANDC__)

#ifdef EXPORTING_random_walker
#define PUBLIC_random_walker_CPP_API __declspec(dllexport)
#else
#define PUBLIC_random_walker_CPP_API __declspec(dllimport)
#endif

#define LIB_random_walker_CPP_API PUBLIC_random_walker_CPP_API

#else

#if !defined(LIB_random_walker_CPP_API)
#if defined(LIB_random_walker_C_API)
#define LIB_random_walker_CPP_API LIB_random_walker_C_API
#else
#define LIB_random_walker_CPP_API /* empty! */ 
#endif
#endif

#endif

extern LIB_random_walker_CPP_API void MW_CALL_CONV adjacency(int nargout, mwArray& W, const mwArray& edges, const mwArray& weights, const mwArray& N);

extern LIB_random_walker_CPP_API void MW_CALL_CONV dirichletboundary(int nargout, mwArray& newVals, const mwArray& L, const mwArray& index, const mwArray& vals);

extern LIB_random_walker_CPP_API void MW_CALL_CONV laplacian(int nargout, mwArray& L, const mwArray& edges, const mwArray& weights, const mwArray& N);

extern LIB_random_walker_CPP_API void MW_CALL_CONV lattice(int nargout, mwArray& points, mwArray& edges, const mwArray& X, const mwArray& Y, const mwArray& connect);

extern LIB_random_walker_CPP_API void MW_CALL_CONV makeweights(int nargout, mwArray& weights, const mwArray& edges, const mwArray& vals, const mwArray& valScale, const mwArray& points, const mwArray& geomScale, const mwArray& EPSILON);

extern LIB_random_walker_CPP_API void MW_CALL_CONV normalize(int nargout, mwArray& normVals, const mwArray& newVals, const mwArray& oldVals);

extern LIB_random_walker_CPP_API void MW_CALL_CONV random_walker(int nargout, mwArray& mask, mwArray& probabilities, const mwArray& img, const mwArray& seeds, const mwArray& labels, const mwArray& beta);

#endif
#endif

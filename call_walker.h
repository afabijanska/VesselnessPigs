#ifndef CALL_WALKER_H
#define CALL_WALKER_H


#ifdef __cplusplus
extern "C" {
#endif

int call_walker(double *mask, double *probs, double *image, int width, int height, double *seed_indexes, int no_seeds,
double *seed_labels, int no_labels, double beta);

#ifdef __cplusplus
}
#endif
typedef float PixelType;

#endif

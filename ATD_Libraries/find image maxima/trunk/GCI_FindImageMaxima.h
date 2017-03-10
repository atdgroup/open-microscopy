// Find maxima in an image
// PRB first written in Mar 2001
// PRB updated in Oct 2005

#ifndef _FIND_IMAGE_MAXIMA_
#define _FIND_IMAGE_MAXIMA_

#include <windows.h>
#include <userint.h>
#include "IMAQ_CVI.h"

typedef struct
{
  Point centre;
  float value;
} GlPeak;

// you can specify the top n to find with number
// maxima above the threshold are found and the actual number found is returned
// the find all above the threshold, send number=0
// if threshold<0 it will default to 10.0

int findImageMaxima_imageOut (IPIImageRef image, IPIImageRef mask,  int number, double threshold_in, int minSeparation, GlPeak **pCentresIn, IPIImageRef imageOut);
int findImageMaxima_Thresholded (IPIImageRef image, IPIImageRef mask,  int number, double threshold_in, int minSeparation, GlPeak **pCentres);
int findImageMaxima (IPIImageRef image, IPIImageRef mask,  int number, GlPeak *centres);

#endif


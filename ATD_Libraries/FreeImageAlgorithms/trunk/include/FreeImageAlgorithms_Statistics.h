/*
 * Copyright 2007-2010 Glenn Pierce, Paul Barber,
 * Oxford University (Gray Institute for Radiation Oncology and Biology) 
 *
 * This file is part of FreeImageAlgorithms.
 *
 * FreeImageAlgorithms is free software: you can redistribute it and/or modify
 * it under the terms of the Lesser GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FreeImageAlgorithms is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * Lesser GNU General Public License for more details.
 *
 * You should have received a copy of the Lesser GNU General Public License
 * along with FreeImageAlgorithms.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __FREEIMAGE_ALGORITHMS_STATISTICS__
#define __FREEIMAGE_ALGORITHMS_STATISTICS__

#include "FreeImageAlgorithms.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
   double  minValue;					// miminum pixel value found 
   double  maxValue;					// maximum pixel value found 
   double  mean;						// mean value				
   double  stdDeviation;				// standard deviation		
   double  skewness;	  			    // skewness
   double  kurtosis;					// kurtosis/peakedness
   float   percentage_overloaded;		// amount of overloaded pixels
   float   percentage_underloaded;	    // amount of underloaded pixels
   int	   area;						// number of pixel scanned	
   
} StatisticReport;

/*! \file 
	Provides various statistical methods for FIBITMAP's.
*/ 

/** \brief Equalise and image using histogram equalisation with random additions.
 *
 *  \param src FIBITMAP bitmap to perform the equalisation operation on.
 *  \return FIBITMAP on success or NULL on error.
*/
DLL_API FIBITMAP* DLL_CALLCONV
FIA_HistEq_Random_Additions(FIBITMAP *src);

/** \brief Equalise and image using histogram equalisation.
 *
 *  \param src FIBITMAP bitmap to perform the equalisation operation on.
 *  \return FIBITMAP on success or NULL on error.
*/
DLL_API FIBITMAP* DLL_CALLCONV
FIA_HistEq(FIBITMAP *src);

/** \brief Calculate the greylevel average of all the pixels.
 *
 *  \param src FIBITMAP bitmap to perform the average calculation operation on. 
 *	           Must be a greylevel image.
 *  \return double Average value on success or 0.0 on error.
*/
DLL_API double DLL_CALLCONV
FIA_GetGreyLevelAverage(FIBITMAP *src);

/** \brief Return the histogram for a greylevel image.
 *
 *	This function is different from the FreeImage_GetHist as you can specify how
 *  to bin values.
 *
 *  \param src FIBITMAP bitmap to perform the histogram operation on.
 *  \param min The minimum value where binning or histogram counting begins.
 *  \param max The maximum value where binning or histogram counting ends.
 *  \param number_of_bins How many bins you want between min and max.
 *  \param hist Long pointer to the histogram data.
 *  \return int FIA_SUCCESS on success or FIA_ERROR on error.
*/
DLL_API int DLL_CALLCONV
FIA_Histogram(FIBITMAP *src, double min, double max,
							  int number_of_bins, unsigned long *hist);
DLL_API int DLL_CALLCONV
FIA_HistogramWithMask(FIBITMAP *src, FIBITMAP * mask, double min, double max,
							  int number_of_bins, unsigned long *hist);
DLL_API int DLL_CALLCONV
FIA_2dHistogram(FIBITMAP * src_x, FIBITMAP * src_y,
                double min_x, double max_x, int number_of_bins_x,
                double min_y, double max_y, int number_of_bins_y,
                unsigned long **hist);

DLL_API int DLL_CALLCONV
FIA_2dHistogramWithMask(FIBITMAP * src_x, FIBITMAP * src_y,
                        FIBITMAP * mask_x, FIBITMAP * mask_y,
                        double min_x, double max_x, int number_of_bins_x,
                        double min_y, double max_y, int number_of_bins_y,
                        unsigned long **hist);

/** \brief Return the histogram for a rgb image.
 *
 *	This function is different from the FreeImage_GetHist as you can specify how
 *  to bin values.
 *
 *  \param src FIBITMAP bitmap to perform the histogram operation on.
 *  \param min The minimum value where binning or histogram counting begins.
 *  \param max The maximum value where binning or histogram counting ends.
 *  \param number_of_bins How many bins you want between min and max.
 *  \param rhist Long pointer to the red histogram data.
 *  \param ghist Long pointer to the green histogram data.
 *  \param bhist Long pointer to the blue histogram data.
 *  \return int FIA_SUCCESS on success or FIA_ERROR on error.
*/

DLL_API int DLL_CALLCONV
FIA_RGBHistogram(FIBITMAP *src,
			unsigned char  min, unsigned char  max, int number_of_bins,
			unsigned long *rhist, unsigned long *ghist, unsigned long *bhist);


/** \brief This function finds the the amount of white ie area in a monochrome image.
 *		   This works with 8 bit images by assuming everything above 1 is white.
 *  \param src FIBITMAP bitmap to perform the histogram operation on.
 *  \param white_area unsigned int * Counts of pixels above or equal to 1.
 */
DLL_API int DLL_CALLCONV
FIA_MonoImageFindWhiteArea(FIBITMAP *src, unsigned int *white_area);

/** \brief This function finds the the amount of white ie area in a monochrome image.
 *		   This works with 8 bit images by assuming everything above 1 is white.
 *  \param src FIBITMAP bitmap to perform the histogram operation on.
 *  \param white_area double * Fraction of pixels above or equal to 1.
 *  \param black_area double * Fraction of pixels below 1.
 *  \return int FIA_SUCCESS on success or FIA_ERROR on error.
 */
DLL_API int DLL_CALLCONV
FIA_MonoImageFindWhiteFraction(FIBITMAP *src, double *white_area, double *black_area);

/** \brief This function determines how a detail is present though two images.
 *
 *  \param src FIBITMAP bitmap to perform the comparison on.
 *  \param result FIBITMAP bitmap to perform the comparison on. This is the expected result image ie
 *						  gold standard.
 *  \param tp int * (True Positive) A detail present in src is also in result.
 *  \param tn int * (True Negative) A detail not in src is not in result ie two pixels that are 0.
 *  \param fp int * (False Positive) A detail not in src is in result.
 *  \param fn int * (False Negative) A detail in src is not in result.
 *  \return int FIA_SUCCESS on success or FIA_ERROR on error.
 */
DLL_API int DLL_CALLCONV
FIA_MonoTrueFalsePositiveComparison(FIBITMAP *src, FIBITMAP *result,
													int *tp, int *tn, int *fp, int *fn);

/** \brief This function measures statistics on an image.
 *
 *  \param src FIBITMAP bitmap to perform the computation on.
 *  \param report StatisticReport * Report describing the statistics of the image.
 *  \return int FIA_SUCCESS on success or FIA_ERROR on error.
 */
DLL_API int DLL_CALLCONV
FIA_StatisticReport(FIBITMAP *src, StatisticReport *report);

DLL_API int DLL_CALLCONV
FIA_StatisticReportWithMask (FIBITMAP * src, FIBITMAP * mask, StatisticReport * report);

/** \brief This function measures statistics on an image.
 *
 *  \param src FIBITMAP bitmap to perform the computation on.
 *  \param Rreport StatisticReport * Report describing the statistics of the Red plane.
 *  \param Greport StatisticReport * Report describing the statistics of the Green plane.
 *  \param Breport StatisticReport * Report describing the statistics of the Blue plane.
 *  \return int FIA_SUCCESS on success or FIA_ERROR on error.
 */
DLL_API int DLL_CALLCONV
FIA_StatisticReportColour (FIBITMAP * src, StatisticReport * Rreport, StatisticReport * Greport, StatisticReport * Breport);

DLL_API int DLL_CALLCONV
FIA_StatisticReportColourWithMask (FIBITMAP * src, FIBITMAP * mask, StatisticReport * Rreport, StatisticReport * Greport, StatisticReport * Breport);

/** \brief This function determines the center of pixel energy of an image.
 *
 *  \param src FIBITMAP bitmap to perform the computation on.
 *  \param x_centroid float * X centre.
 *  \param y_centroid float * Y centre.
 *  \return int FIA_SUCCESS on success or FIA_ERROR on error.
 */
DLL_API int DLL_CALLCONV
FIA_Centroid(FIBITMAP *src, float *x_centroid, float *y_centroid);


/** \brief This function determines the median value of all the pixels.
 *
 *  \param src FIBITMAP bitmap to perform the computation on.
 *  \return double The median of all the pixels in the image.
 */
DLL_API double DLL_CALLCONV
FIA_GetMedianFromImage(FIBITMAP* src);

#ifdef __cplusplus
}
#endif

#endif

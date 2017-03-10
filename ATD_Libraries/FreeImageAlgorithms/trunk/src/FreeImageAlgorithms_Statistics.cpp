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

#include "FreeImageAlgorithms.h"
#include "FreeImageAlgorithms_Statistics.h"
#include "FreeImageAlgorithms_Colour.h"
#include "FreeImageAlgorithms_Palettes.h"
#include "FreeImageAlgorithms_Utilities.h"
#include "FreeImageAlgorithms_Utils.h"

#include <assert.h>
#include <math.h>

template < class Tsrc > class Statistic
{
  public:
    int CalculateHistogram (FIBITMAP * src,  FIBITMAP * mask, double min, double max, int number_of_bins,
                            unsigned long *hist);

    int CalculateStatisticReport (FIBITMAP * src,  FIBITMAP * mask, StatisticReport * report);

    int Centroid (FIBITMAP * src, float *x_centroid, float *y_centroid);

    double CalculateGreyLevelAverage (FIBITMAP * src);

    int Calculate2dHistogram (FIBITMAP * src_x, FIBITMAP * src_y,
                              FIBITMAP * mask_x, FIBITMAP * mask_y,
                              double min_x, double max_x, int number_of_bins_x,
                              double min_y, double max_y, int number_of_bins_y,
                              unsigned long **hist);
};

template < class Tsrc > int Statistic < Tsrc >::CalculateHistogram (FIBITMAP * src, FIBITMAP * mask, double min,
                                                                    double max, int number_of_bins,
                                                                    unsigned long *hist)
{
    if (hist == NULL)
    {
        return FIA_ERROR;
    }

    if (mask != NULL)
    {
        // Mask has to be the same size
        if (FIA_CheckDimensions (src, mask) == FIA_ERROR)
        {
            FreeImage_OutputMessageProc (FIF_UNKNOWN,
                                         "Image source and mask have different dimensions");
            return FIA_ERROR;
        }

        // Mask has to be 8 bit 
        if (FreeImage_GetBPP (mask) != 8 || FreeImage_GetImageType (mask) != FIT_BITMAP)
        {
            FreeImage_OutputMessageProc (FIF_UNKNOWN, "Mask must be an 8bit FIT_BITMAP");
            return FIA_ERROR;
        }
    }

    // We need to find the min and max in the image.
    if (min == 0 && max == 0)
    {
        FIA_FindMinMax (src, &min, &max);
    }

    if (min > max)
    {
        FreeImage_OutputMessageProc (FIF_UNKNOWN,
                                     "Error minimum specified is greater than the maximum");
        return FIA_ERROR;
    }

    if (number_of_bins < 1)
    {
        FreeImage_OutputMessageProc (FIF_UNKNOWN, "Error number of bins less than 1");
        return FIA_ERROR;
    }

    // Clear histogram array
    memset (hist, 0, number_of_bins * sizeof (unsigned long));

//    Tsrc tmp_min = (Tsrc) min;
//    Tsrc tmp_max = (Tsrc) max;
//    Tsrc range = tmp_max - tmp_min;
    double tmp_min = min;
    double tmp_max = max;
    double range = tmp_max - tmp_min;

    // bins-1 as we need the histogram range to exceed the image range
    // by one extra bin to accomodate the pixels with max intensity
    double range_per_bin = (double) range / (double) (number_of_bins - 1);

    int width = FreeImage_GetWidth (src);
    int height = FreeImage_GetHeight (src);

    Tsrc *bits = NULL;
    Tsrc pixel;
    unsigned int bin;

    if (mask != NULL)
    {
		for(register int y = 0; y < height; y++)
		{

			bits = (Tsrc *) FreeImage_GetScanLine (src, y);
            BYTE *mask_ptr = (BYTE *) FreeImage_GetScanLine (mask, y);

            for(register int x = 0; x < width; x++)
            {
                if(mask_ptr[x] == 0)
                    continue;

				pixel = bits[x];

				if (pixel >= tmp_min && pixel <= tmp_max)
				{
					// If range_per_bin == 1 with dont need the divide. The divide is very slow.
					if (range_per_bin == 1)
					{
						bin = (int) (pixel - tmp_min);
					}
					else
					{
						bin = (int) ((pixel - tmp_min) / range_per_bin);
					}

					hist[bin]++;
				}
			}
		}
	} 
	else {
		for(register int y = 0; y < height; y++)
		{

			bits = (Tsrc *) FreeImage_GetScanLine (src, y);

			for(register int x = 0; x < width; x++)
			{
				pixel = bits[x];

				if (pixel >= tmp_min && pixel <= tmp_max)
				{
					// If range_per_bin == 1 with dont need the divide. The divide is very slow.
					if (range_per_bin == 1)
					{
						bin = (int) (pixel - tmp_min);
					}
					else
					{
						bin = (int) ((pixel - tmp_min) / range_per_bin);
					}

					hist[bin]++;
				}
			}
		}
	}
    return FIA_SUCCESS;
}

template < class Tsrc > int Statistic < Tsrc >::CalculateStatisticReport (FIBITMAP * src, FIBITMAP * mask,
                                                                          StatisticReport * report)
{
    memset(report, 0, sizeof(StatisticReport));

    if (report == NULL)
    {
        return FIA_ERROR;
    }

    if (mask != NULL)
    {
        // Mask has to be the same size
        if (FIA_CheckDimensions (src, mask) == FIA_ERROR)
        {
            FreeImage_OutputMessageProc (FIF_UNKNOWN,
                                         "Image source and mask have different dimensions");
            return FIA_ERROR;
        }

        // Mask has to be 8 bit 
        if (FreeImage_GetBPP (mask) != 8 || FreeImage_GetImageType (mask) != FIT_BITMAP)
        {
            FreeImage_OutputMessageProc (FIF_UNKNOWN, "Mask must be an 8bit FIT_BITMAP");
            return FIA_ERROR;
        }
    }

    int width = FreeImage_GetWidth (src);
    int height = FreeImage_GetHeight (src);

    double sum = 0.0, sd_sum = 0.0, skew_sum = 0.0, kurt_sum = 0.0;
    Tsrc *bits;
	int number_of_pixels = 0;
    int	amount_of_overloaded_pixels = 0;
    int	amount_of_underloaded_pixels = 0;

    double min_possible_for_type = 0.0;
    double max_possible_for_type = 0.0;

    FIA_GetMinPosibleValueForGreyScaleType (FreeImage_GetImageType(src), &min_possible_for_type);
    FIA_GetMaxPosibleValueForGreyScaleType (FreeImage_GetImageType(src), &max_possible_for_type);

    // start min at the highest val, max at the lowest.
	report->maxValue = min_possible_for_type;
	report->minValue = max_possible_for_type;

    if (mask != NULL)
    {
        for(register int y = 0; y < height; y++)
        {
            bits = (Tsrc *) FreeImage_GetScanLine (src, y);
            BYTE *mask_ptr = (BYTE *) FreeImage_GetScanLine (mask, y);

            for(register int x = 0; x < width; x++)
            {
                if(mask_ptr[x] == 0)
                    continue;

                if (bits[x] > report->maxValue)
                {
                    report->maxValue = (double) bits[x];
                }

                if (bits[x] < report->minValue)
                {
                    report->minValue = (double) bits[x];
                }

                if(bits[x] <= min_possible_for_type)
                {
                    amount_of_underloaded_pixels++;
                }

                if(bits[x] >= max_possible_for_type)
                {
                    amount_of_overloaded_pixels++;
                }

				number_of_pixels++;
                sum += bits[x];
            }
        }
    }
    else {

        for(register int y = 0; y < height; y++)
        {
            bits = (Tsrc *) FreeImage_GetScanLine (src, y);

            for(register int x = 0; x < width; x++)
            {
                if (bits[x] > report->maxValue)
                {
                    report->maxValue = (double) bits[x];
                }

                if (bits[x] < report->minValue)
                {
                    report->minValue = (double) bits[x];
                }

                if(bits[x] <= min_possible_for_type)
                {
                    amount_of_underloaded_pixels++;
                }

                if(bits[x] >= max_possible_for_type)
                {
                    amount_of_overloaded_pixels++;
                }

				number_of_pixels++;
                sum += bits[x];
            }
        }
    }

    report->area = number_of_pixels;
    report->mean = (double) sum / report->area;
    report->percentage_underloaded = (float) amount_of_underloaded_pixels / report->area;
    report->percentage_overloaded = (float) amount_of_overloaded_pixels / report->area;

	// Calc SD, skewness and kurtosis
    if (mask != NULL)
    {
        for(register int y = 0; y < height; y++)
        {
            bits = (Tsrc *) FreeImage_GetScanLine (src, y);
            BYTE *mask_ptr = (BYTE *) FreeImage_GetScanLine (mask, y);

            for(register int x = 0; x < width; x++)
            {
                if(mask_ptr[x] == 0)
                    continue;

                sd_sum += ((bits[x] - report->mean) * (bits[x] - report->mean));
                skew_sum += ((bits[x] - report->mean) * (bits[x] - report->mean) * (bits[x] - report->mean));
                kurt_sum += ((bits[x] - report->mean) * (bits[x] - report->mean) * (bits[x] - report->mean) * (bits[x] - report->mean));
            }
        }
    }
    else
    {
        for(register int y = 0; y < height; y++)
        {
            bits = (Tsrc *) FreeImage_GetScanLine (src, y);

            for(register int x = 0; x < width; x++)
            {
                sd_sum += ((bits[x] - report->mean) * (bits[x] - report->mean));
                skew_sum += ((bits[x] - report->mean) * (bits[x] - report->mean) * (bits[x] - report->mean));
                kurt_sum += ((bits[x] - report->mean) * (bits[x] - report->mean) * (bits[x] - report->mean) * (bits[x] - report->mean));
            }
        }
    }

    report->stdDeviation = sqrt ((double) sd_sum / (report->area - 1));
    report->skewness     = (double) skew_sum / (report->area) / pow(report->stdDeviation, 3.0);
    report->kurtosis     = (double) kurt_sum / (report->area) / pow(report->stdDeviation, 4.0) - 3.0;

    return FIA_SUCCESS;
}

template < class Tsrc > int Statistic < Tsrc >::Centroid (FIBITMAP * src, float *x_centroid,
                                                          float *y_centroid)
{
    *x_centroid = 0;
    *y_centroid = 0;

    int width = FreeImage_GetWidth (src);
    int height = FreeImage_GetHeight (src);

    Tsrc *bits;
    double sum_x = 0.0, sum_y = 0.0, sum_i = 0.0;
    int row;

    for(register int y = 0; y < height; y++)
    {
        bits = (Tsrc *) FreeImage_GetScanLine (src, y);

        row = height - y;

        for(register int x = 0; x < width; x++)
        {
            sum_i += (double) bits[x];
            sum_x += (double) ((x + 1) * bits[x]);
            sum_y += (double) (row * bits[x]);
        }
    }

    *x_centroid = (float) (sum_x / sum_i);
    *y_centroid = (float) (sum_y / sum_i);

    return FIA_SUCCESS;
}

template < class Tsrc > double Statistic < Tsrc >::CalculateGreyLevelAverage (FIBITMAP * src)
{
    int width = FreeImage_GetWidth (src);
    int height = FreeImage_GetHeight (src);
    int total = width * height;

    Tsrc *bits;
    double sum = 0.0;

    for(register int y = 0; y < height; y++)
    {
        bits = (Tsrc *) FreeImage_GetScanLine (src, y);

        for(register int x = 0; x < width; x++)
        {
            sum += (double) bits[x];
        }
    }

    return sum / total;
}

template < class Tsrc > int Statistic < Tsrc >::Calculate2dHistogram (FIBITMAP * src_x, FIBITMAP * src_y,
                                                                      FIBITMAP * mask_x, FIBITMAP * mask_y,
                                                                      double min_x, double max_x, int number_of_bins_x,
                                                                      double min_y, double max_y, int number_of_bins_y,
                                                                      unsigned long **hist)
{
    if (hist == NULL)
    {
        return FIA_ERROR;
    }

    FREE_IMAGE_TYPE src_type_x = FreeImage_GetImageType (src_x);
    FREE_IMAGE_TYPE src_type_y = FreeImage_GetImageType (src_y);

    if ((!src_type_x) || (!src_type_y))
    {
        FreeImage_OutputMessageProc (FIF_UNKNOWN, "Source images are of different types");
        return FIA_ERROR;
    }

    if ((FreeImage_HasPixels(src_x)==FALSE) || (FreeImage_HasPixels(src_y)==FALSE))
    {
        FreeImage_OutputMessageProc (FIF_UNKNOWN, "Source image has no pixels");
        return FIA_ERROR;
    }

    if (FIA_CheckDimensions (src_x, src_y) == FIA_ERROR)
    {
        // Source images must be of same size
        FreeImage_OutputMessageProc (FIF_UNKNOWN,
                                     "Images have different dimensions");
        return FIA_ERROR;
    }

    if (mask_x != NULL)
    {
        // Mask has to be the same size
        if (FIA_CheckDimensions (src_x, mask_x) == FIA_ERROR)
        {
            FreeImage_OutputMessageProc (FIF_UNKNOWN,
                                         "Image source and mask have different dimensions");
            return FIA_ERROR;
        }

        // Mask has to be 8 bit 
        if (FreeImage_GetBPP (mask_x) != 8 || FreeImage_GetImageType (mask_x) != FIT_BITMAP)
        {
            FreeImage_OutputMessageProc (FIF_UNKNOWN, "Mask must be an 8bit FIT_BITMAP");
            return FIA_ERROR;
        }
    }

    if (mask_y != NULL)
    {
        // Mask has to be the same size
        if (FIA_CheckDimensions (src_y, mask_y) == FIA_ERROR)
        {
            FreeImage_OutputMessageProc (FIF_UNKNOWN,
                                         "Image source and mask have different dimensions");
            return FIA_ERROR;
        }

        // Mask has to be 8 bit 
        if (FreeImage_GetBPP (mask_y) != 8 || FreeImage_GetImageType (mask_y) != FIT_BITMAP)
        {
            FreeImage_OutputMessageProc (FIF_UNKNOWN, "Mask must be an 8bit FIT_BITMAP");
            return FIA_ERROR;
        }
    }

    if ((number_of_bins_x < 1) || (number_of_bins_y < 1))
    {
        FreeImage_OutputMessageProc (FIF_UNKNOWN, "Error number of bins less than 1");
        return FIA_ERROR;
    }

    // We need to find the min and max in the images.
    if (min_x == 0 && max_x == 0)
    {
        FIA_FindMinMax (src_x, &min_x, &max_x);
    }

    if (min_y == 0 && max_y == 0)
    {
        FIA_FindMinMax (src_y, &min_y, &max_y);
    }

    if ((min_x > max_x) || (min_y >= max_y))
    {
        FreeImage_OutputMessageProc (FIF_UNKNOWN,
                                     "Error minimum specified is greater than the maximum");
        return FIA_ERROR;
    }

    // Clear histogram array
    for(register int i = 0; i < number_of_bins_x; i++)
        for(register int j = 0; j < number_of_bins_y; j++)
            hist[i][j] = 0;
//    memset (hist, 0, number_of_bins * sizeof (unsigned long));

    Tsrc tmp_min_x = (Tsrc) min_x;
    Tsrc tmp_max_x = (Tsrc) max_x;
    Tsrc range_x   = tmp_max_x - tmp_min_x;
    Tsrc tmp_min_y = (Tsrc) min_y;
    Tsrc tmp_max_y = (Tsrc) max_y;
    Tsrc range_y   = tmp_max_y - tmp_min_y;

    // bins-1 as we need the histogram range to exceed the image range
    // by one extra bin to accomodate the pixels with max intensity
    double range_per_bin_x = (double) range_x / (double) (number_of_bins_x - 1);
    double range_per_bin_y = (double) range_y / (double) (number_of_bins_y - 1);

    int width = FreeImage_GetWidth (src_x);
    int height = FreeImage_GetHeight (src_x);

    Tsrc *bits_x = NULL, *bits_y = NULL;
    Tsrc pixel_x, pixel_y;
    unsigned int bin_x, bin_y;

    if ((mask_x != NULL) || (mask_y != NULL))
    {
		for(register int j = 0; j < height; j++)
		{
			bits_x           = (Tsrc *) FreeImage_GetScanLine (src_x, j);
			bits_y           = (Tsrc *) FreeImage_GetScanLine (src_y, j);
            BYTE *mask_ptr_x, *mask_ptr_y;

            if (mask_x != NULL)
                mask_ptr_x = (BYTE *) FreeImage_GetScanLine (mask_x, j);
            else
                mask_ptr_x = NULL;

            if (mask_y != NULL)
                mask_ptr_y = (BYTE *) FreeImage_GetScanLine (mask_y, j);
            else
                mask_ptr_y = NULL;

            for(register int i = 0; i < width; i++)
            {
                if (mask_ptr_x != NULL)
                    if (mask_ptr_x[i] == 0)
                        continue;

                if (mask_ptr_y != NULL)
                    if (mask_ptr_y[i] == 0)
                        continue;

				pixel_x = bits_x[i];
                pixel_y = bits_y[i];

				if (pixel_x >= tmp_min_x && pixel_x <= tmp_max_x && pixel_y >= tmp_min_y && pixel_y <= tmp_max_y)
				{
					// If range_per_bin == 1 with dont need the divide. The divide is very slow.
					if (range_per_bin_x == 1)
					{
						bin_x = (int) (pixel_x - tmp_min_x);
					}
					else
					{
						bin_x = (int) ((pixel_x - tmp_min_x) / range_per_bin_x);
					}

                    if (range_per_bin_y == 1)
					{
						bin_y = (int) (pixel_y - tmp_min_y);
					}
					else
					{
						bin_y = (int) ((pixel_y - tmp_min_y) / range_per_bin_y);
					}

					hist[bin_x][bin_y]++;
				}
			}
		}
	} 
	else {
		for(register int j = 0; j < height; j++)
		{
			bits_x = (Tsrc *) FreeImage_GetScanLine (src_x, j);
            bits_y = (Tsrc *) FreeImage_GetScanLine (src_y, j);

			for(register int i = 0; i < width; i++)
			{
				pixel_x = bits_x[i];
                pixel_y = bits_y[i];

				if (pixel_x >= tmp_min_x && pixel_x <= tmp_max_x && pixel_y >= tmp_min_y && pixel_y <= tmp_max_y)
				{
					// If range_per_bin == 1 with dont need the divide. The divide is very slow.
					if (range_per_bin_x == 1)
					{
						bin_x = (int) (pixel_x - tmp_min_x);
					}
					else
					{
						bin_x = (int) ((pixel_x - tmp_min_x) / range_per_bin_x);
					}

                    if (range_per_bin_y == 1)
					{
						bin_y = (int) (pixel_y - tmp_min_y);
					}
					else
					{
						bin_y = (int) ((pixel_y - tmp_min_y) / range_per_bin_y);
					}

					hist[bin_x][bin_y]++;
				}
			}
		}
	}
    return FIA_SUCCESS;
}

Statistic < unsigned char >statisticUCharImage;
Statistic < unsigned short >statisticUShortImage;
Statistic < short >statisticShortImage;
Statistic < unsigned long >statisticULongImage;
Statistic < long >statisticLongImage;
Statistic < float >statisticFloatImage;
Statistic < double >statisticDoubleImage;

/** 
 * Calculate the histogram for the image.
 * Does not work with colour images.
 */
int DLL_CALLCONV
FIA_Histogram (FIBITMAP * src, double min, double max, int number_of_bins, unsigned long *hist)
{
    if (!src)
        return FIA_ERROR;

    FREE_IMAGE_TYPE src_type = FreeImage_GetImageType (src);

    switch (src_type)
    {
        case FIT_BITMAP:
        {                       // standard image: 1-, 4-, 8-, 16-, 24-, 32-bit
            if (FreeImage_GetBPP (src) == 8)
            {
                return statisticUCharImage.CalculateHistogram (src, NULL, min, max, number_of_bins, hist);
            }
            break;
        }
        case FIT_UINT16:
        {                       // array of unsigned short: unsigned 16-bit
            return statisticUShortImage.CalculateHistogram (src, NULL, min, max, number_of_bins, hist);
        }
        case FIT_INT16:
        {                       // array of short: signed 16-bit
            return statisticShortImage.CalculateHistogram (src, NULL, min, max, number_of_bins, hist);
        }
        case FIT_UINT32:
        {                       // array of unsigned long: unsigned 32-bit
            return statisticULongImage.CalculateHistogram (src, NULL, min, max, number_of_bins, hist);
        }
        case FIT_INT32:
        {                       // array of long: signed 32-bit
            return statisticLongImage.CalculateHistogram (src, NULL, min, max, number_of_bins, hist);
        }
        case FIT_FLOAT:
        {                       // array of float: 32-bit
            return statisticFloatImage.CalculateHistogram (src, NULL, min, max, number_of_bins, hist);
        }
        case FIT_DOUBLE:
        {                       // array of double: 64-bit
            return statisticDoubleImage.CalculateHistogram (src, NULL, min, max, number_of_bins, hist);
        }
        default:
        {
            break;
        }
    }

    return FIA_ERROR;
}
int DLL_CALLCONV
FIA_HistogramWithMask (FIBITMAP * src, FIBITMAP * mask, double min, double max, int number_of_bins, unsigned long *hist)
{
    if (!src)
        return FIA_ERROR;

    FREE_IMAGE_TYPE src_type = FreeImage_GetImageType (src);

    switch (src_type)
    {
        case FIT_BITMAP:
        {                       // standard image: 1-, 4-, 8-, 16-, 24-, 32-bit
            if (FreeImage_GetBPP (src) == 8)
            {
                return statisticUCharImage.CalculateHistogram (src, mask, min, max, number_of_bins, hist);
            }
            break;
        }
        case FIT_UINT16:
        {                       // array of unsigned short: unsigned 16-bit
            return statisticUShortImage.CalculateHistogram (src, mask, min, max, number_of_bins, hist);
        }
        case FIT_INT16:
        {                       // array of short: signed 16-bit
            return statisticShortImage.CalculateHistogram (src, mask, min, max, number_of_bins, hist);
        }
        case FIT_UINT32:
        {                       // array of unsigned long: unsigned 32-bit
            return statisticULongImage.CalculateHistogram (src, mask, min, max, number_of_bins, hist);
        }
        case FIT_INT32:
        {                       // array of long: signed 32-bit
            return statisticLongImage.CalculateHistogram (src, mask, min, max, number_of_bins, hist);
        }
        case FIT_FLOAT:
        {                       // array of float: 32-bit
            return statisticFloatImage.CalculateHistogram (src, mask, min, max, number_of_bins, hist);
        }
        case FIT_DOUBLE:
        {                       // array of double: 64-bit
            return statisticDoubleImage.CalculateHistogram (src, mask, min, max, number_of_bins, hist);
        }
        default:
        {
            break;
        }
    }

    return FIA_ERROR;
}
DLL_API int DLL_CALLCONV
FIA_2dHistogram(FIBITMAP * src_x, FIBITMAP * src_y,
                double min_x, double max_x, int number_of_bins_x,
                double min_y, double max_y, int number_of_bins_y,
                unsigned long **hist)
{
    if ((!src_x) || (!src_y))
        return FIA_ERROR;

    FREE_IMAGE_TYPE src_type_x = FreeImage_GetImageType (src_x);

    switch (src_type_x)
    {
        case FIT_BITMAP:
        {                       // standard image: 1-, 4-, 8-, 16-, 24-, 32-bit
            if (FreeImage_GetBPP (src_x) == 8)
            {
                return statisticUCharImage.Calculate2dHistogram (src_x, src_y, NULL, NULL,
                                                                 min_x, max_x, number_of_bins_x,
                                                                 min_y, max_y, number_of_bins_y,
                                                                 hist);
            }
            break;
        }
        case FIT_UINT16:
        {                       // array of unsigned short: unsigned 16-bit
            return statisticUShortImage.Calculate2dHistogram (src_x, src_y, NULL, NULL,
                                                              min_x, max_x, number_of_bins_x,
                                                              min_y, max_y, number_of_bins_y,
                                                              hist);
        }
        case FIT_INT16:
        {                       // array of short: signed 16-bit
            return statisticShortImage.Calculate2dHistogram (src_x, src_y, NULL, NULL,
                                                             min_x, max_x, number_of_bins_x,
                                                             min_y, max_y, number_of_bins_y,
                                                             hist);
        }
        case FIT_UINT32:
        {                       // array of unsigned long: unsigned 32-bit
            return statisticULongImage.Calculate2dHistogram (src_x, src_y, NULL, NULL,
                                                             min_x, max_x, number_of_bins_x,
                                                             min_y, max_y, number_of_bins_y,
                                                             hist);
        }
        case FIT_INT32:
        {                       // array of long: signed 32-bit
            return statisticLongImage.Calculate2dHistogram (src_x, src_y, NULL, NULL,
                                                            min_x, max_x, number_of_bins_x,
                                                            min_y, max_y, number_of_bins_y,
                                                            hist);
        }
        case FIT_FLOAT:
        {                       // array of float: 32-bit
            return statisticFloatImage.Calculate2dHistogram (src_x, src_y, NULL, NULL,
                                                             min_x, max_x, number_of_bins_x,
                                                             min_y, max_y, number_of_bins_y,
                                                             hist);
        }
        case FIT_DOUBLE:
        {                       // array of double: 64-bit
            return statisticDoubleImage.Calculate2dHistogram (src_x, src_y, NULL, NULL,
                                                              min_x, max_x, number_of_bins_x,
                                                              min_y, max_y, number_of_bins_y,
                                                              hist);
        }
        default:
        {
            break;
        }
    }

    return FIA_ERROR;
}
int DLL_CALLCONV
FIA_2dHistogramWithMask (FIBITMAP * src_x, FIBITMAP * src_y,
                         FIBITMAP * mask_x, FIBITMAP * mask_y,
                         double min_x, double max_x, int number_of_bins_x,
                         double min_y, double max_y, int number_of_bins_y,
                         unsigned long **hist)
{
    if ((!src_x) || (!src_y))
        return FIA_ERROR;

    FREE_IMAGE_TYPE src_type_x = FreeImage_GetImageType (src_x);

    switch (src_type_x)
    {
        case FIT_BITMAP:
        {                       // standard image: 1-, 4-, 8-, 16-, 24-, 32-bit
            if (FreeImage_GetBPP (src_x) == 8)
            {
                return statisticUCharImage.Calculate2dHistogram (src_x,src_y, mask_x, mask_y,
                                                                 min_x, max_x, number_of_bins_x,
                                                                 min_y, max_y, number_of_bins_y,
                                                                 hist);
            }
            break;
        }
        case FIT_UINT16:
        {                       // array of unsigned short: unsigned 16-bit
            return statisticUShortImage.Calculate2dHistogram (src_x,src_y, mask_x, mask_y,
                                                              min_x, max_x, number_of_bins_x,
                                                              min_y, max_y, number_of_bins_y,
                                                              hist);
        }
        case FIT_INT16:
        {                       // array of short: signed 16-bit
            return statisticShortImage.Calculate2dHistogram (src_x,src_y, mask_x, mask_y,
                                                             min_x, max_x, number_of_bins_x,
                                                             min_y, max_y, number_of_bins_y,
                                                             hist);
        }
        case FIT_UINT32:
        {                       // array of unsigned long: unsigned 32-bit
            return statisticULongImage.Calculate2dHistogram (src_x,src_y, mask_x, mask_y,
                                                             min_x, max_x, number_of_bins_x,
                                                             min_y, max_y, number_of_bins_y,
                                                             hist);
        }
        case FIT_INT32:
        {                       // array of long: signed 32-bit
            return statisticLongImage.Calculate2dHistogram (src_x,src_y, mask_x, mask_y,
                                                            min_x, max_x, number_of_bins_x,
                                                            min_y, max_y, number_of_bins_y,
                                                            hist);
        }
        case FIT_FLOAT:
        {                       // array of float: 32-bit
            return statisticFloatImage.Calculate2dHistogram (src_x,src_y, mask_x, mask_y,
                                                             min_x, max_x, number_of_bins_x,
                                                             min_y, max_y, number_of_bins_y,
                                                             hist);
        }
        case FIT_DOUBLE:
        {                       // array of double: 64-bit
            return statisticDoubleImage.Calculate2dHistogram (src_x,src_y, mask_x, mask_y,
                                                              min_x, max_x, number_of_bins_x,
                                                              min_y, max_y, number_of_bins_y,
                                                              hist);
        }
        default:
        {
            break;
        }
    }

    return FIA_ERROR;
}
int DLL_CALLCONV
FIA_StatisticReport (FIBITMAP * src, StatisticReport * report)
{
    if (!src)
        return FIA_ERROR;

    FREE_IMAGE_TYPE src_type = FreeImage_GetImageType (src);

    switch (src_type)
    {
        case FIT_BITMAP:
        {                       // standard image: 1-, 4-, 8-, 16-, 24-, 32-bit
            if (FreeImage_GetBPP (src) == 8)
            {
                return statisticUCharImage.CalculateStatisticReport (src, NULL, report);
            }
            break;
        }

        case FIT_UINT16:
        {                       // array of unsigned short: unsigned 16-bit
            return statisticUShortImage.CalculateStatisticReport (src, NULL, report);
        }

        case FIT_INT16:
        {                       // array of short: signed 16-bit
            return statisticShortImage.CalculateStatisticReport (src, NULL, report);
        }

        case FIT_UINT32:
        {                       // array of unsigned long: unsigned 32-bit
            return statisticULongImage.CalculateStatisticReport (src, NULL, report);
        }

        case FIT_INT32:
        {                       // array of long: signed 32-bit
            return statisticLongImage.CalculateStatisticReport (src, NULL, report);
        }

        case FIT_FLOAT:
        {                       // array of float: 32-bit
            return statisticFloatImage.CalculateStatisticReport (src, NULL, report);
        }

        case FIT_DOUBLE:
        {                       // array of double: 64-bit
            return statisticDoubleImage.CalculateStatisticReport (src, NULL, report);
        }

        default:
        {                       // array of FICOMPLEX: 2 x 64-bit
            break;
        }
    }

    return FIA_ERROR;
}

int DLL_CALLCONV
FIA_StatisticReportWithMask (FIBITMAP * src, FIBITMAP * mask, StatisticReport * report)
{
    if (!src)
        return FIA_ERROR;

    FREE_IMAGE_TYPE src_type = FreeImage_GetImageType (src);

    switch (src_type)
    {
        case FIT_BITMAP:
        {                       // standard image: 1-, 4-, 8-, 16-, 24-, 32-bit
            if (FreeImage_GetBPP (src) == 8)
            {
                return statisticUCharImage.CalculateStatisticReport (src, mask, report);
            }
            break;
        }

        case FIT_UINT16:
        {                       // array of unsigned short: unsigned 16-bit
            return statisticUShortImage.CalculateStatisticReport (src, mask, report);
        }

        case FIT_INT16:
        {                       // array of short: signed 16-bit
            return statisticShortImage.CalculateStatisticReport (src, mask, report);
        }

        case FIT_UINT32:
        {                       // array of unsigned long: unsigned 32-bit
            return statisticULongImage.CalculateStatisticReport (src, mask, report);
        }

        case FIT_INT32:
        {                       // array of long: signed 32-bit
            return statisticLongImage.CalculateStatisticReport (src, mask, report);
        }

        case FIT_FLOAT:
        {                       // array of float: 32-bit
            return statisticFloatImage.CalculateStatisticReport (src, mask, report);
        }

        case FIT_DOUBLE:
        {                       // array of double: 64-bit
            return statisticDoubleImage.CalculateStatisticReport (src, mask, report);
        }

        default:
        {                       // array of FICOMPLEX: 2 x 64-bit
            break;
        }
    }

    return FIA_ERROR;
}

int DLL_CALLCONV
FIA_StatisticReportColour (FIBITMAP * src, StatisticReport * Rreport, StatisticReport * Greport, StatisticReport * Breport)
{
    if (!src)
        return FIA_ERROR;

    FREE_IMAGE_TYPE src_type = FreeImage_GetImageType (src);

    switch (src_type)
    {
        case FIT_BITMAP:
        {                       // standard image: 1-, 4-, 8-, 16-, 24-, 32-bit
            if (FreeImage_GetBPP (src) > 8)
            {
				FIBITMAP *R, *G, *B;

				FIA_ExtractColourPlanes(src, &R, &G, &B);

				statisticUCharImage.CalculateStatisticReport (R, NULL, Rreport);
				statisticUCharImage.CalculateStatisticReport (G, NULL, Greport);
				statisticUCharImage.CalculateStatisticReport (B, NULL, Breport);

				FreeImage_Unload(R);
				FreeImage_Unload(G);
				FreeImage_Unload(B);

                return FIA_SUCCESS;
            }
            break;
        }

        default:
        {     
            break;
        }
    }

    return FIA_ERROR;
}

int DLL_CALLCONV
FIA_StatisticReportColourWithMask (FIBITMAP * src, FIBITMAP * mask, StatisticReport * Rreport, StatisticReport * Greport, StatisticReport * Breport)
{
    if (!src)
        return FIA_ERROR;

    FREE_IMAGE_TYPE src_type = FreeImage_GetImageType (src);

    switch (src_type)
    {
        case FIT_BITMAP:
        {                       // standard image: 1-, 4-, 8-, 16-, 24-, 32-bit
            if (FreeImage_GetBPP (src) > 8)
            {
				FIBITMAP *R, *G, *B;

				FIA_ExtractColourPlanes(src, &R, &G, &B);

				statisticUCharImage.CalculateStatisticReport (R, mask, Rreport);
				statisticUCharImage.CalculateStatisticReport (G, mask, Greport);
				statisticUCharImage.CalculateStatisticReport (B, mask, Breport);

				FreeImage_Unload(R);
				FreeImage_Unload(G);
				FreeImage_Unload(B);

                return FIA_SUCCESS;
            }
            break;
        }

        default:
        {     
            break;
        }
    }

    return FIA_ERROR;
}

int DLL_CALLCONV
FIA_Centroid (FIBITMAP * src, float *x_centroid, float *y_centroid)
{
    if (!src)
        return FIA_ERROR;

    FREE_IMAGE_TYPE src_type = FreeImage_GetImageType (src);

    switch (src_type)
    {
        case FIT_BITMAP:
        {                       // standard image: 1-, 4-, 8-, 16-, 24-, 32-bit
            if (FreeImage_GetBPP (src) == 8)
            {
                return statisticUCharImage.Centroid (src, x_centroid, y_centroid);
            }
            break;
        }

        case FIT_UINT16:
        {                       // array of unsigned short: unsigned 16-bit
            return statisticUShortImage.Centroid (src, x_centroid, y_centroid);
        }

        case FIT_INT16:
        {                       // array of short: signed 16-bit
            return statisticShortImage.Centroid (src, x_centroid, y_centroid);
        }

        case FIT_UINT32:
        {                       // array of unsigned long: unsigned 32-bit
            return statisticULongImage.Centroid (src, x_centroid, y_centroid);
        }

        case FIT_INT32:
        {                       // array of long: signed 32-bit
            return statisticLongImage.Centroid (src, x_centroid, y_centroid);
        }

        case FIT_FLOAT:
        {                       // array of float: 32-bit
            return statisticFloatImage.Centroid (src, x_centroid, y_centroid);
        }

        case FIT_DOUBLE:
        {                       // array of double: 64-bit
            return statisticDoubleImage.Centroid (src, x_centroid, y_centroid);
        }

        default:
        {
            break;
        }
    }

    return FIA_ERROR;
}

double DLL_CALLCONV
FIA_GetGreyLevelAverage (FIBITMAP * src)
{
    if (!src)
    {
        return 0.0;
    }

    FREE_IMAGE_TYPE src_type = FreeImage_GetImageType (src);

    switch (src_type)
    {
        case FIT_BITMAP:
        {                       // standard image: 1-, 4-, 8-, 16-, 24-, 32-bit
            if (FreeImage_GetBPP (src) == 8)
            {
                return statisticUCharImage.CalculateGreyLevelAverage (src);
            }
            break;
        }

        case FIT_UINT16:
        {                       // array of unsigned short: unsigned 16-bit
            return statisticUShortImage.CalculateGreyLevelAverage (src);
        }
        case FIT_INT16:
        {                       // array of short: signed 16-bit
            return statisticShortImage.CalculateGreyLevelAverage (src);
        }
        case FIT_UINT32:
        {                       // array of unsigned long: unsigned 32-bit
            return statisticULongImage.CalculateGreyLevelAverage (src);
        }

        case FIT_INT32:
        {                       // array of long: signed 32-bit
            return statisticLongImage.CalculateGreyLevelAverage (src);
        }

        case FIT_FLOAT:
        {                       // array of float: 32-bit
            return statisticFloatImage.CalculateGreyLevelAverage (src);
        }

        case FIT_DOUBLE:
        {                       // array of double: 64-bit
            return statisticDoubleImage.CalculateGreyLevelAverage (src);
        }
        default:
        {
            break;
        }
    }

    return 0.0;
}

int DLL_CALLCONV
FIA_RGBHistogram (FIBITMAP * src, unsigned char min, unsigned char max,
                  int number_of_bins, unsigned long *rhist, unsigned long *ghist,
                  unsigned long *bhist)
{
    if (rhist == NULL || ghist == NULL || bhist == NULL)
    {
        FreeImage_OutputMessageProc (FIF_UNKNOWN, "Error null passed for arrays");
        return FIA_ERROR;
    }

    if (min > max)
    {
        FreeImage_OutputMessageProc (FIF_UNKNOWN,
                                     "Error minimum specified is greater than the maximum");
        return FIA_ERROR;
    }

    // clear histogram arrays
    memset (rhist, 0, number_of_bins * sizeof (unsigned long));
    memset (ghist, 0, number_of_bins * sizeof (unsigned long));
    memset (bhist, 0, number_of_bins * sizeof (unsigned long));

    unsigned char range = max - min;
    unsigned char range_per_bin = range / (number_of_bins - 1);

    int width = FreeImage_GetWidth (src);
    int height = FreeImage_GetHeight (src);

    unsigned int *bits;
    unsigned char red_pixel, green_pixel, blue_pixel;
    unsigned int bin;

    for(int y = 0; y < height; y++)
    {
        bits = reinterpret_cast < unsigned int *>(FreeImage_GetScanLine (src, y));

        for(int x = 0; x < width; x++)
        {
            red_pixel = *bits >> FI_RGBA_RED_SHIFT;
            green_pixel = *bits >> FI_RGBA_GREEN_SHIFT;
            blue_pixel = *bits >> FI_RGBA_BLUE_SHIFT;

            if (red_pixel >= (unsigned int) min && red_pixel <= (unsigned int) max)
            {
                // If range_per_bin == 1 with dont need the divide. The divide is very slow.
                if (range_per_bin == 1)
                {
                    bin = (unsigned int) (red_pixel - min);
                }
                else
                {
                    bin = (unsigned int) ((red_pixel - min) / range_per_bin);
                }

                rhist[bin]++;
            }

            if (green_pixel >= (unsigned int) min && green_pixel <= (unsigned int) max)
            {
                if (range_per_bin == 1)
                {
                    bin = (unsigned int) (green_pixel - min);
                }
                else
                {
                    bin = (unsigned int) ((green_pixel - min) / range_per_bin);
                }

                ghist[bin]++;
            }

            if (blue_pixel >= (unsigned int) min && blue_pixel <= (unsigned int) max)
            {
                if (range_per_bin == 1)
                {
                    bin = (unsigned int) (blue_pixel - min);
                }
                else
                {
                    bin = (unsigned int) ((blue_pixel - min) / range_per_bin);
                }

                bhist[bin]++;
            }
        }
    }

    return FIA_SUCCESS;
}

FIBITMAP *DLL_CALLCONV
FIA_HistEq_Random_Additions (FIBITMAP * src)
{
    FIBITMAP *dib;
    int i, R;
    const int number_grey_levels = 256;
    int type, bpp, image_width, image_height, *left, *width;
    unsigned char *src_bits, *dib_bits;
    long total, Hsum, Havg, *histogram;

    bpp = FreeImage_GetBPP (src);
    type = FreeImage_GetImageType (src);

    // The image is not 8bit greyscale return.
    if (bpp != 8 || type != FIT_BITMAP)
    {
        return NULL;
    }

    image_width = FreeImage_GetWidth (src);
    image_height = FreeImage_GetHeight (src);

    // Allocate a 8-bit dib
    dib = FreeImage_AllocateT (FIT_BITMAP, image_width, image_height, 8, 0, 0, 0);
    FIA_CopyPalette (src, dib);

    total = (long) image_width *image_height;

    left = (int *) malloc (sizeof (int) * number_grey_levels);
    width = (int *) malloc (sizeof (int) * number_grey_levels);
    histogram = (long *) malloc (sizeof (long) * number_grey_levels);

    /*
     * Clear histogram 
     */
    memset (histogram, 0, number_grey_levels);

    FreeImage_GetHistogram (src, (DWORD *) histogram, FICC_BLACK);

    /*
     * right end of interval 
     */
    R = 0;

    /*
     * cumulative value for interval 
     */
    Hsum = 0;

    /*
     * interval value for uniform histogram     
     */
    Havg = total / number_grey_levels;

    /*
     * evaluate remapping of all input gray levels;
     * * Each input gray value maps to an interval of valid output values.
     * * The endpoints of the intervals are left[] and left[]+width[].
     */
    for(i = 0; i < number_grey_levels; i++)
    {
        left[i] = R;            /* left end of interval    */

        Hsum += histogram[i];

        while (Hsum > Havg && R < (number_grey_levels - 1))
        {                       /* make interval wider    */
            Hsum -= Havg;       /* adjust Hsum                */
            R++;                /* update right end    */
        }

        width[i] = R - left[i] + 1;     /* width of interval        */
    }

    src_bits = (unsigned char *) FreeImage_GetBits (src);
    dib_bits = (unsigned char *) FreeImage_GetBits (dib);

    /*
     * visit all input pixels and remap intensities 
     */
    for(i = 0; i < total; i++)
    {
        const unsigned char pixel = *src_bits++;

        if (width[pixel] == 1)
        {

            *dib_bits++ = left[pixel];
        }
        else
        {
            /*
             * pixel spills over into width[] possible values 
             */
            /*
             * randomly pick from 0 to width[i] 
             */

            /*
             * 0 <= R < width 
             */
            R = ((rand () & 0x7fff) * width[pixel]) >> 15;

            *dib_bits++ = left[pixel] + R;
        }
    }

    free (histogram);

    return dib;
}

FIBITMAP *DLL_CALLCONV
FIA_HistEq (FIBITMAP * src)
{
    FIBITMAP *dst;
    int i;
    const int number_grey_levels = 256;
    int type, bpp, image_width, image_height;
    unsigned char *src_bits, *dst_bits;
    long total, Hsum;
    DWORD *histogram;
    double Havg;
    long *mapping;

    bpp = FreeImage_GetBPP (src);
    type = FreeImage_GetImageType (src);

    // The image is not 8bit greyscale return.
    if (bpp != 8 || type != FIT_BITMAP)
    {
        return NULL;
    }

    image_width = FreeImage_GetWidth (src);
    image_height = FreeImage_GetHeight (src);

    // Allocate a 8-bit dib
    dst = FreeImage_AllocateT (FIT_BITMAP, image_width, image_height, 8, 0, 0, 0);

    FIA_SetGreyLevelPalette (dst);

    total = image_width * image_height;

    mapping = (long *) malloc (sizeof (long) * number_grey_levels);
    histogram = (DWORD *) malloc (sizeof (long) * number_grey_levels);

    /*
     * Clear histogram 
     */
    memset (histogram, 0, number_grey_levels);
    memset (mapping, 0, number_grey_levels);

    FreeImage_GetHistogram (src, histogram, FICC_BLACK);

    /*
     * cumulative value for interval 
     */
    Hsum = 0;

    /*
     * interval value for uniform histogram     
     */
    Havg = (double) total / (number_grey_levels - 1);

    for(i = 0; i < number_grey_levels; i++)
    {
        Hsum += histogram[i];

        mapping[i] = Hsum;
    }

    src_bits = (unsigned char *) FreeImage_GetBits (src);
    dst_bits = (unsigned char *) FreeImage_GetBits (dst);

    /*
     * visit all input pixels and remap intensities 
     */
    for(i = 0; i < total; i++)
    {
        const unsigned char pixel = *src_bits++;

        *dst_bits++ = (const unsigned char) (int) ((mapping[pixel] / Havg) + 0.5);
    }

    free (histogram);

    return dst;
}

int DLL_CALLCONV
FIA_MonoImageFindWhiteArea (FIBITMAP * src, unsigned int *white_area)
{
    int bpp = FreeImage_GetBPP (src);
    int width = FreeImage_GetWidth (src);
    int height = FreeImage_GetHeight (src);

    int whites = 0;

    FREE_IMAGE_TYPE type = FreeImage_GetImageType (src);

    if (type != FIT_BITMAP || bpp > 8)
        return FIA_ERROR;

    BYTE *src_bits;

    for(int y = 0; y < height; y++)
    {
        src_bits = reinterpret_cast < BYTE * >(FreeImage_GetScanLine (src, y));

        for(int x = 0; x < width; x++)
        {
            if (src_bits[x] >= 1)
            {
                whites++;
            }
        }
    }

    *white_area = whites;

    return FIA_SUCCESS;
}

int DLL_CALLCONV
FIA_MonoImageFindWhiteFraction (FIBITMAP * src, double *white_area, double *black_area)
{
    int width = FreeImage_GetWidth (src);
    int height = FreeImage_GetHeight (src);
    long size = width * height;

    unsigned int whites = 0;

    if (FIA_MonoImageFindWhiteArea (src, &whites) == FIA_ERROR)
    {
        return FIA_ERROR;
    }

    *white_area = (double) whites / size;
    *black_area = 1 - *white_area;

    return FIA_SUCCESS;
}

int DLL_CALLCONV
FIA_MonoTrueFalsePositiveComparison (FIBITMAP * src, FIBITMAP * result,
                                     int *tp, int *tn, int *fp, int *fn)
{
    *tp = 0;
    *tn = 0, *fp = 0, *fn = 0;

    if (!src || !result)
    {
        return FIA_ERROR;
    }

    int bpp = FreeImage_GetBPP (src);

    if (bpp > 8)
    {
        return FIA_ERROR;
    }

    if (FreeImage_GetImageType (src) != FreeImage_GetImageType (result))
    {
        return FIA_ERROR;
    }

    unsigned int width = FreeImage_GetWidth (src);
    unsigned int height = FreeImage_GetHeight (src);

    if (width != FreeImage_GetWidth (result))
    {
        return FIA_ERROR;
    }

    if (height != FreeImage_GetHeight (result))
    {
        return FIA_ERROR;
    }

    FIBITMAP *src_converted;

    if (bpp < 8)
    {
        src_converted = FreeImage_ConvertTo8Bits (src);
    }
    else
    {
        src_converted = FreeImage_Clone (src);
    }

    BYTE src_tmp, result_tmp;
    BYTE *src_bits, *result_bits;

    for(unsigned int y = 0; y < height; y++)
    {
        src_bits = reinterpret_cast < BYTE * >(FreeImage_GetScanLine (src_converted, y));
        result_bits = reinterpret_cast < BYTE * >(FreeImage_GetScanLine (result, y));

        for(unsigned int x = 0; x < width; x++)
        {
            src_tmp = src_bits[x];
            result_tmp = result_bits[x];

            if (src_tmp > 1)
            {
                src_tmp = 1;
            }

            if (result_tmp > 1)
            {
                result_tmp = 1;
            }

            // Pixels are the same and 1 increase true positive count.
            if (src_tmp == 1 && result_tmp == 1)
            {
                (*tp)++;
            }

            // Pixels are the same and 0 increase true negative count.
            if (src_tmp == 0 && result_tmp == 0)
            {
                (*tn)++;
            }

            // Pixel not in src but in result increase false positive.
            if (src_tmp == 0 && result_tmp == 1)
            {
                (*fp)++;
            }

            // Pixel in src image not in result increase false negative.
            if (src_tmp == 1 && result_tmp == 0)
            {
                (*fn)++;
            }
        }
    }

    return FIA_SUCCESS;
}

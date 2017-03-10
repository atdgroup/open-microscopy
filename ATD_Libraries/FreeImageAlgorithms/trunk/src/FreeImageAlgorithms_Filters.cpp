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
#include "FreeImageAlgorithms_Utils.h"
#include "FreeImageAlgorithms_IO.h"
#include "FreeImageAlgorithms_Filters.h"
#include "FreeImageAlgorithms_Utilities.h"
#include "FreeImageAlgorithms_Convolution.h"
#include "FreeImageAlgorithms_Colour.h"

#include <math.h>

FIBITMAP *DLL_CALLCONV
FIA_Sobel (FIBITMAP * src)
{
    FIBITMAP *magnitude_dib = NULL;

    int err = FIA_SobelAdvanced (src, NULL, NULL, &magnitude_dib);

    if (err != FIA_SUCCESS)
    {
        return NULL;
    }

    return magnitude_dib;
}

int DLL_CALLCONV
FIA_SobelAdvanced (FIBITMAP * src,
                   FIBITMAP ** vertical, FIBITMAP ** horizontal, FIBITMAP ** magnitude)
{
    double sobel_horizontal_kernel[] = { 1.0, 2.0, 1.0, 0.0, 0.0, 0.0, -1.0, -2.0, -1.0 };
    double sobel_vertical_kernel[] = { -1.0, 0.0, 1.0, -2.0, 0.0, 2.0, -1.0, 0.0, 1.0 };

    FIABITMAP *src_bordered = FIA_SetBorder (src, 1, 1, BorderType_Copy, 0.0);

    FIBITMAP *vertical_tmp = NULL, *horizontal_tmp = NULL;

    if (vertical != NULL || magnitude != NULL)
    {
        FilterKernel convolve_kernel_left = FIA_NewKernel (1, 1,
                                                           sobel_vertical_kernel, 1.0);

        vertical_tmp = FIA_Convolve (src_bordered, convolve_kernel_left);
    }

    if (horizontal != NULL || magnitude != NULL)
    {
        FilterKernel convolve_kernel_top = FIA_NewKernel (1, 1,
                                                          sobel_horizontal_kernel, 1.0);

        horizontal_tmp = FIA_Convolve (src_bordered, convolve_kernel_top);
    }

    FIA_Unload (src_bordered);

    // We need both vertical_tmp and horizontal_tmp to calculate the magnitude.
    if (magnitude != NULL)
    {
        if (vertical_tmp == NULL || horizontal_tmp == NULL)
        {
            return FIA_ERROR;
        }

        int dst_width = FreeImage_GetWidth (vertical_tmp);
        int dst_height = FreeImage_GetHeight (vertical_tmp);

        FIBITMAP *dst = FreeImage_AllocateT (FIT_DOUBLE, dst_width, dst_height,
                                             8, 0, 0, 0);

        const int dst_pitch_in_pixels = FreeImage_GetPitch (dst) / sizeof (double);
        register double *dst_ptr, *dib1_ptr, *dib2_ptr;

        double *dst_first_pixel_address_ptr = (double *) FreeImage_GetBits (dst);
        double *dib1_first_pixel_address_ptr = (double *) FreeImage_GetBits (vertical_tmp);
        double *dib2_first_pixel_address_ptr = (double *) FreeImage_GetBits (horizontal_tmp);

        for(register int y = 0; y < dst_height; y++)
        {
            dst_ptr = (dst_first_pixel_address_ptr + y * dst_pitch_in_pixels);
            dib1_ptr = (dib1_first_pixel_address_ptr + y * dst_pitch_in_pixels);
            dib2_ptr = (dib2_first_pixel_address_ptr + y * dst_pitch_in_pixels);

            for(register int x = 0; x < dst_width; x++)
            {
                *dst_ptr = sqrt ((*dib1_ptr * *dib1_ptr) + (*dib2_ptr * *dib2_ptr));
                ++dst_ptr;
				++dib1_ptr;
                ++dib2_ptr;
            }
        }

        *magnitude = dst;
    }

    if (vertical != NULL)
    {
        *vertical = vertical_tmp;
    }
    else
    {
        FreeImage_Unload (vertical_tmp);
    }

    if (horizontal != NULL)
    {
        *horizontal = horizontal_tmp;
    }
    else
    {
        FreeImage_Unload (horizontal_tmp);
    }

    return FIA_SUCCESS;
}

static void DLL_CALLCONV
FIA_MakeCircularKernel (int radius, double *kernel, int normalise)
{
    double  dsq, maxdsq;
	int x, y, width, abs_x, abs_y;
	double *kernel_ptr = kernel, sum = 0;

	width = 2*radius+1;
	maxdsq = radius*radius;

	for (x=0; x < width; x++) {

		abs_x = abs(x-radius);

		for (y=0; y < width; y++)
		{
			abs_y = abs(y-radius);

			dsq = (abs_x * abs_x) + (abs_y * abs_y);

			if (dsq<=maxdsq) {
				kernel_ptr[y*width+x]=1.0;
				sum += 1.0;
			}
			else 
				kernel_ptr[y*width+x]=0.0;
		}
	}

	if (normalise) {
		for (x=0; x < width; x++) {

			for (y=0; y < width; y++)
			{
				kernel_ptr[y*width+x] /= sum;
			}
		}
	}
}

static void DLL_CALLCONV
FIA_MakeGaussianKernel (int radius, double *kernel, int normalise)
{
	double  dsq, sigmasq;
	int x, y, width, abs_x, abs_y;
	double *kernel_ptr = kernel, val, sum = 0;

	width = 2*radius+1;
	sigmasq = pow(radius/2.0,2);

	for (x=0; x < width; x++) {

		abs_x = abs(x-radius);

		for (y=0; y < width; y++)
		{
			abs_y = abs(y-radius);

			dsq = (abs_x * abs_x) + (abs_y * abs_y);

			val = (double) exp(-(dsq/sigmasq));
			kernel_ptr[y*width+x] = val;
			sum += val;
		}
	}

	if (normalise) {
		for (x=0; x < width; x++) {

			for (y=0; y < width; y++)
			{
				kernel_ptr[y*width+x] /= sum;
			}
		}
	}
}

static void DLL_CALLCONV
FIA_MakeSquareKernel (int radius, double *kernel, int normalise)
{
	int x, y, width;
	double *kernel_ptr = kernel, sum = 0;

	width = 2*radius+1;
	
	for (x=0; x < width; x++) {

		for (y=0; y < width; y++)
		{
			kernel_ptr[y*width+x] = (double) 1.0;
			sum += (double) 1.0;
		}
	}

	if (normalise) {
		for (x=0; x < width; x++) {

			for (y=0; y < width; y++)
			{
				kernel_ptr[y*width+x] /= sum;
			}
		}
	}
}

FIBITMAP* DLL_CALLCONV
FIA_Binning (FIBITMAP * src, FIA_BINNING_TYPE type, int radius)
{
	int size = radius * 2 + 1;
	double *kernel = (double*) malloc ((size*size)*sizeof(double));
	
	if(type == FIA_BINNING_GAUSSIAN)
		FIA_MakeGaussianKernel (radius, kernel, 0);
	else if(type == FIA_BINNING_CIRCULAR)
		FIA_MakeCircularKernel (radius, kernel, 0);
	else 
		FIA_MakeSquareKernel (radius, kernel, 0);

    FIABITMAP *src_bordered = FIA_SetBorder (src, radius, radius, BorderType_Copy, 0.0);

    FilterKernel convolve_kernel = FIA_NewKernel (radius, radius, kernel, 1.0);

    FIBITMAP* binned_fib = FIA_Convolve (src_bordered, convolve_kernel);

    FIA_Unload (src_bordered);

	free(kernel);

    return binned_fib;
}

FIBITMAP* DLL_CALLCONV
FIA_Blur (FIBITMAP * src, FIA_KERNEL_TYPE type, int radius)
{
	int size = radius * 2 + 1;
	int is8bit=0, isColour=0;
	double *kernel = (double*) malloc ((size*size)*sizeof(double));
	FIBITMAP* new_fib=NULL;
	
	if (FIA_Is8Bit(src))
		is8bit = 1;
	else if (!FIA_IsGreyScale(src))
		isColour = 1;

	if(type == FIA_KERNEL_GAUSSIAN)
		FIA_MakeGaussianKernel (radius, kernel, 1);
	else if(type == FIA_KERNEL_CIRCULAR)
		FIA_MakeCircularKernel (radius, kernel, 1);
	else 
		FIA_MakeSquareKernel (radius, kernel, 1);

	if (isColour) {
		FIBITMAP *R, *G, *B;

		FIA_ExtractColourPlanes (src, &R, &G, &B);

		FIABITMAP *src_bordered = FIA_SetBorder (R, radius, radius, BorderType_Copy, 0.0);
		FilterKernel convolve_kernel = FIA_NewKernel (radius, radius, kernel, 1.0);
		R = FIA_Convolve (src_bordered, convolve_kernel);
		FIA_Unload (src_bordered);
		FIA_InPlaceConvertToStandardType(&R, 1);

		src_bordered = FIA_SetBorder (G, radius, radius, BorderType_Copy, 0.0);
		convolve_kernel = FIA_NewKernel (radius, radius, kernel, 1.0);
		G = FIA_Convolve (src_bordered, convolve_kernel);
		FIA_Unload (src_bordered);
		FIA_InPlaceConvertToStandardType(&G, 1);

		src_bordered = FIA_SetBorder (B, radius, radius, BorderType_Copy, 0.0);
		convolve_kernel = FIA_NewKernel (radius, radius, kernel, 1.0);
		B = FIA_Convolve (src_bordered, convolve_kernel);
		FIA_Unload (src_bordered);
		FIA_InPlaceConvertToStandardType(&B, 1);

		FIA_ReplaceColourPlanes(&new_fib, R, G, B);
	}
	else {

		FIABITMAP *src_bordered = FIA_SetBorder (src, radius, radius, BorderType_Copy, 0.0);

		FilterKernel convolve_kernel = FIA_NewKernel (radius, radius, kernel, 1.0);

		new_fib = FIA_Convolve (src_bordered, convolve_kernel);

		FIA_Unload (src_bordered);
	}

	if (is8bit)
		FIA_InPlaceConvertToStandardType(&new_fib, 1);

	free(kernel);

    return new_fib;
}


FIBITMAP* DLL_CALLCONV
GreyImageMerge (FIBITMAP *image, FIBITMAP *GBimage, double amount, double threshold, double minval, double maxval)
{
	float *bufin, *bufblur, *bufout;
	int x, y, i, b, bpp, w, h;
	double diff;
	FIBITMAP *dest_image;
	FREE_IMAGE_TYPE type;

	w = FreeImage_GetWidth (image);
	h = FreeImage_GetHeight(image);

	bufin = (float *)calloc(w * h, sizeof(float));
	bufblur = (float *)calloc(w * h, sizeof(float));
	bufout = (float *)calloc(w * h, sizeof(float));
	
	FIA_GreyImageToFloatArray (image, bufin, &x, &y, 0);
	FIA_GreyImageToFloatArray (GBimage, bufblur, &x, &y, 0);

	for (i=0; i<x*y; i++) {
		diff = bufin[i] - bufblur[i];
		if (fabs(diff*2) < threshold)
			diff = 0;
		bufout[i] = min(bufin[i] + diff*amount, maxval);
		bufout[i] = max(bufout[i],minval);
	}

	dest_image = FIA_LoadGreyScaleFIBFromArrayData ((BYTE*)bufout, 32, x, y, FIT_FLOAT, 0, 0);
	FreeImage_FlipVertical(dest_image);

	free(bufin);
	free(bufblur);
	free(bufout);

	return dest_image;
}

FIBITMAP* DLL_CALLCONV
ImageMerge (FIBITMAP *image, FIBITMAP *GBimage, double amount, double threshold)
{
	float *bufin, *bufblur, *bufout;
	BYTE *cbufin, *cbufblur, *cbufout;
	int x, y, i, b, bpp, w, h, B, pitch;
	double diff, minval = 0, maxval = 255;
	FIBITMAP *dest_image;
	FREE_IMAGE_TYPE type;

	bpp = FreeImage_GetBPP(image);
	type = FreeImage_GetImageType(image);

	if (FIA_IsGreyScale(image) && bpp==16)
		maxval = 32767;
	if (type==FIT_FLOAT || type==FIT_DOUBLE) {
		FIA_FindMinMax(image, &minval, &maxval);
	}
		
	dest_image = FreeImage_Clone(image);

	w = FreeImage_GetWidth (image);
	h = FreeImage_GetHeight(image);

	if (!FIA_IsGreyScale(image)) { // colour

		FIBITMAP *R, *G, *B;
		FIBITMAP *GR, *GG, *GB;
		FIBITMAP *RR, *RG, *RB;

		FIA_ExtractColourPlanes(image, &R, &G, &B);
		FIA_ExtractColourPlanes(GBimage, &GR, &GG, &GB);

		RR = GreyImageMerge (R, GR, amount, threshold, minval, maxval);
		RG = GreyImageMerge (G, GG, amount, threshold, minval, maxval);
		RB = GreyImageMerge (B, GB, amount, threshold, minval, maxval);

		FIA_InPlaceConvertToStandardType(&RR, 1);
		FIA_InPlaceConvertToStandardType(&RG, 1);
		FIA_InPlaceConvertToStandardType(&RB, 1);

		FIA_ReplaceColourPlanes(&dest_image, RR, RG, RB);

		FreeImage_Unload(R); FreeImage_Unload(G); FreeImage_Unload(B);
		FreeImage_Unload(GR); FreeImage_Unload(GG); FreeImage_Unload(GB);
		FreeImage_Unload(RR); FreeImage_Unload(RG); FreeImage_Unload(RB);

		return dest_image;
	}
	
	// Greyscale

	dest_image = GreyImageMerge (image, GBimage, amount, threshold, minval, maxval);

	return dest_image;	
}

FIBITMAP* DLL_CALLCONV
FIA_UnsharpMask (FIBITMAP *src, double radius, double amount, double threshold)
{
	FIBITMAP *GBimage=NULL, *dest_image=NULL;

	GBimage = FIA_Blur (src, FIA_KERNEL_GAUSSIAN, radius);

	if (GBimage==NULL)
		return NULL;

	dest_image = ImageMerge(src, GBimage, amount, threshold);

	FreeImage_Unload(GBimage);

	return dest_image;
}
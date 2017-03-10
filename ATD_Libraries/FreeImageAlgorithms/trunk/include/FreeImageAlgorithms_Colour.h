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

#ifndef __FREEIMAGE_ALGORITHMS_COLOUR__
#define __FREEIMAGE_ALGORITHMS_COLOUR__

#ifdef __cplusplus
extern "C" {
#endif

#include "FreeImageAlgorithms.h"

/*! \file 
*	Provides colour space functions.
*/

/** \brief Convert red, green and blue values to hue, satuation and value.
 *
 *  \param red Red value.
 *  \param green Green value.
 *  \param blue Blue value.
 *  \param hue Hue returned.
 *  \param satuation Satuation returned.
 *  \param value Value returned.
 *  \return int FIA_SUCCESS on success or FIA_ERROR on error.
*/
DLL_API int DLL_CALLCONV
FIA_RGBToHSV( unsigned char red, unsigned char green, unsigned char blue,
							double *hue, double *satuation, double *value);

/** \brief Convert hue, satuation and value values to red, green and blue.
 *
 *  \param hue Hue.
 *  \param satuation Satuation.
 *  \param value Value.
 *  \param red Red value returned.
 *  \param green Green value returned.
 *  \param blue Blue value returned.
 *  \return int FIA_SUCCESS on success or FIA_ERROR on error.
*/
DLL_API int DLL_CALLCONV
FIA_HSVToRGB( double hue, double satuation, double value,
							unsigned char *red, unsigned char *green, unsigned char *blue);

/** \brief Convert red, green and blue values to hue, satuation and luminosity.
 *
 *  \param red Red value.
 *  \param green Green value.
 *  \param blue Blue value.
 *  \param hue Hue returned.
 *  \param satuation Satuation returned.
 *  \param luminosity Value returned.
 *  \return int FIA_SUCCESS on success or FIA_ERROR on error.
*/
DLL_API int DLL_CALLCONV
FIA_RGBToHSL( unsigned char red, unsigned char green, unsigned char blue,
							double *hue, double *satuation, double *luminosity);

/** \brief Convert hue, satuation and luminosity values to red, green and blue.
 *
 *  \param hue Hue.
 *  \param satuation Satuation.
 *  \param luminosity Value.
 *  \param red Red value returned.
 *  \param green Green value returned.
 *  \param blue Blue value returned.
 *  \return int FIA_SUCCESS on success or FIA_ERROR on error.
*/
DLL_API int DLL_CALLCONV
FIA_HSLToRGB( double hue, double satuation, double luminosity,
							unsigned char *red, unsigned char *green, unsigned char *blue);

/** \brief Extract the colour plane from an image.
 *
 *  \param src The colour image
 *  \param R returned pointer to the red FIBITMAP
 *  \param G returned pointer to the green FIBITMAP
 *  \param B returned pointer to the blue FIBITMAP
 *  \return int FIA_SUCCESS on success or FIA_ERROR on error.
*/
DLL_API int DLL_CALLCONV
FIA_ExtractColourPlanes (FIBITMAP *src, FIBITMAP **R, FIBITMAP **G, FIBITMAP **B);

/** \brief Replace the colour planes in an image.
 *   Will allocate src if src is NULL. Otherwise it must be of the correct size and type.
 *   WARNING: This function will replace any palette on R, G and B with a standard grey palette.
 *
 *  \param src The colour image
 *  \param R returned pointer to the red FIBITMAP
 *  \param G returned pointer to the green FIBITMAP
 *  \param B returned pointer to the blue FIBITMAP
 *  \return int FIA_SUCCESS on success or FIA_ERROR on error.
*/
DLL_API int DLL_CALLCONV
FIA_ReplaceColourPlanes (FIBITMAP **src, FIBITMAP *R, FIBITMAP *G, FIBITMAP *B);

/** \brief Replace the colour planes in an image from HSV images.
 *   Will allocate src if src is NULL. Otherwise it must be of the correct size and type.
 *   WARNING: This function will replace any palette on R, G and B with a standard grey palette.
 *
 *  \param src The colour image
 *  \param H returned pointer to the hue FIBITMAP (val 0 = 0 deg, 255 = 360 deg)
 *  \param S returned pointer to the saturation FIBITMAP (val 0 = 0, 255 = 1.0)
 *  \param V returned pointer to the value FIBITMAP (val 0 = 0, 255 = 1.0)
 *  \return int FIA_SUCCESS on success or FIA_ERROR on error.
*/
DLL_API int DLL_CALLCONV
FIA_ReplaceColourPlanesHSV (FIBITMAP **src, FIBITMAP *H, FIBITMAP *S, FIBITMAP *V);

/** \brief Change the order of the colour bytes.
 *
 *  \param src The colour image
 *  \return FIBITMAP* Resulting image.
*/
DLL_API FIBITMAP* DLL_CALLCONV 
FIA_ColourConvertBGRtoRGB (FIBITMAP * src);

/** \brief Change the order of the colour bytes.
 *
 *  \param src The colour image
 *  \return int FIA_SUCCESS on success or FIA_ERROR on error.
*/
DLL_API int DLL_CALLCONV
FIA_InplaceColourConvertBGRtoRGB (FIBITMAP ** src);

#ifdef __cplusplus
}
#endif

#endif

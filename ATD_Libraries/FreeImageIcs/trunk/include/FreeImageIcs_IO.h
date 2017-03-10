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

#ifndef __FIA_ICS__
#define __FIA_ICS__

#ifdef __cplusplus
extern "C" {
#endif

#include "FreeImageAlgorithms.h"

#include "libics.h"

DLL_API int DLL_CALLCONV
FreeImageIcs_IsIcsFile (const char *filepath);

DLL_API int DLL_CALLCONV
FreeImageIcs_NumberOfDimensions (ICS *ics);

DLL_API int DLL_CALLCONV
FreeImageIcs_GetLabelForDimension (ICS *ics, int dimension, char *label);

DLL_API int DLL_CALLCONV
FreeImageIcs_GetDimensionDetails (ICS *ics, int dimension, char* order, char *label, int* size);

DLL_API int DLL_CALLCONV
FreeImageIcs_IsIcsFileColourFile(ICS *ics);

DLL_API FIBITMAP* DLL_CALLCONV
FreeImageIcs_LoadFIBFromIcsFile (ICS *ics);

DLL_API FIBITMAP* DLL_CALLCONV
FreeImageIcs_LoadFIBFromIcsFilePath (const char* filepath);

DLL_API FIBITMAP* DLL_CALLCONV
FreeImageIcs_LoadFIBFromColourIcsFile (ICS *ics);

DLL_API FIBITMAP* DLL_CALLCONV
FreeImageIcs_GetIcsImageDimensionalDataSlice(ICS *ics, int dimension, int slice);

DLL_API int DLL_CALLCONV
FreeImageIcs_SaveImage (FIBITMAP *dib, const char *filepath, int save_metadata);

DLL_API int DLL_CALLCONV
FreeImageIcs_SaveIcsDataToFile (const char *filepath, void *data, Ics_DataType dt, int ndims, int *dims);

DLL_API int DLL_CALLCONV
FreeImageIcs_SaveIcsImageDataToBinaryFile (const char *filepath, const char *output_filepath);

// Abstracts Ics files which have to be the __cdcel calling convention
DLL_API Ics_Error DLL_CALLCONV
FreeImageIcs_IcsOpen (ICS* *ics, char const* filename, char const* mode);

DLL_API Ics_Error DLL_CALLCONV
FreeImageIcs_IcsClose (ICS* ics);

DLL_API Ics_Error DLL_CALLCONV
FreeImageIcs_IcsSetNativeScale (ICS* ics, int dimension, double origin, double scale, const char *units);

DLL_API Ics_Error DLL_CALLCONV
FreeImageIcs_IcsSetNativeIntensityScale (ICS* ics, double origin, double scale, const char *units);

DLL_API Ics_Error DLL_CALLCONV
FreeImageIcs_IcsSetDimOrder (ICS* ics, int dimension, const char* order, const char* label);

DLL_API int DLL_CALLCONV
FreeImageIcs_SaveIcsFileWithDimensionsAs(ICS *ics, const char *filepath, size_t* order, int size);

DLL_API int DLL_CALLCONV
FreeImageIcs_SaveIcsFileWithFirstTwoDimensionsAs(ICS *ics, const char *filepath, int first, int second);

DLL_API FIBITMAP* DLL_CALLCONV
FreeImageIcs_SumIntensityProjection(ICS *ics, int dimension);

DLL_API FIBITMAP* DLL_CALLCONV
FreeImageIcs_MaximumIntensityProjection(ICS *ics, int dimension);

DLL_API void DLL_CALLCONV
FreeImageIcs_SetICSVersion (int version);  // 1 or 2, default=2

DLL_API void DLL_CALLCONV
FreeImageIcs_SetCompressionLevel (int level);

DLL_API size_t DLL_CALLCONV
FreeImageIcs_GetDataSize(ICS *ics);

DLL_API Ics_Error DLL_CALLCONV
FreeImageIcs_GetData (ICS* ics, void* dest, size_t n);

DLL_API Ics_Error DLL_CALLCONV
FreeImageIcs_SetData (ICS* ics, void const* src, size_t n);

DLL_API Ics_Error DLL_CALLCONV
FreeImageIcs_GetLayout (ICS *ics, Ics_DataType* dt, int* ndims, size_t* dims);

DLL_API Ics_Error DLL_CALLCONV
FreeImageIcs_SetLayout (ICS* ics, Ics_DataType dt, int ndims, size_t const* dims);

DLL_API Ics_Error DLL_CALLCONV
FreeImageIcs_SetCompression (ICS* ics, Ics_Compression compression, int level);

#ifdef __cplusplus
}
#endif

#endif
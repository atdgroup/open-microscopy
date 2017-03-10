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

#include "FreeImageIcs_IO.h"
#include "FreeImageIcs_Private.h"
#include "FreeImageIcs_MetaData.h"

#include "FreeImageAlgorithms_IO.h"
#include "FreeImageAlgorithms_Utilities.h"
#include "FreeImageAlgorithms_Palettes.h"
#include "FreeImageAlgorithms_Arithmetic.h"

#include "libics_ll.h"
#include "libics.h"

#include <vector>
#include <string>
#include <algorithm>

#include "profile.h"

#include <string.h>
#include <stdio.h>

// This is to allow the client to set a global compression level
// RGB images can take ages to save when using compression.
static int compression_level = 7;

// This allows the client to set the version of ICS file to use for writing
// 1 = version 1, 2 = version 2 (default and usual)
static int ics_version = 2;

// This gets the size in bytes between each element in a dimension
// The first dimension 0 always return the size of dataType.
static Ics_Error DLL_CALLCONV
GetSizeInBytesBetweenDimensions(size_t* dims, int ndims, Ics_DataType dataType, int dimension, size_t *size)
{
    if(dimension < 0 || dimension > ndims)
		return IcsErr_NotValidAction;

	size_t total_size = 1; 

	// Get the combined size of all the dimension after x,y.
	for (int i=1; i <= dimension; i++)
		total_size *= dims[i-1];

	*size = (total_size * GetIcsDataTypeBPP (dataType)) / 8;
	
	return IcsErr_Ok;
}

// This returns the size in bytes of one dimension slice.
static Ics_Error DLL_CALLCONV
GetSizeInBytesOfOneDimensionSlice(size_t* dims, int ndims, Ics_DataType dataType, int dimension, size_t *size)
{
    if(dimension < 0 || dimension > ndims)
		return IcsErr_NotValidAction;

	size_t total_size = dims[0]; 

	// Get the combined size of all the dimension after x,y.
	for (int i=0; i < dimension; i++)
		total_size *= dims[i];

	*size = (total_size * GetIcsDataTypeBPP (dataType)) / 8;
	
	return IcsErr_Ok;
}

static void DLL_CALLCONV
CopyBytesToFIBitmap(FIBITMAP *src, BYTE *data, int padded)
{
	register int y;

	int data_line_length, height = FreeImage_GetHeight(src);
	
	if(padded)
		data_line_length = FreeImage_GetPitch(src);
	else
		data_line_length = FreeImage_GetLine(src);
	
	BYTE *bits;
	BYTE *data_row;
			
	for( y = 0; y < height; y++) {

		bits = (BYTE *) FreeImage_GetScanLine(src, y);

		data_row = data + (height - y - 1) * data_line_length;

		memcpy( bits, data_row, data_line_length ); 
	}
}


static FIBITMAP* DLL_CALLCONV
FreeImageIcs_CreateFIB(BYTE *data, Ics_DataType icsType, int bpp, int width, int height, int colour, int padded)
{
	FIBITMAP 	*dib;
	BYTE 		*data_ptr;
	
	// Check the data type and convert to bpp
	if ( bpp < 0 )
		return NULL;

	data_ptr = data;

	FREE_IMAGE_TYPE fit = IcsTypeToFreeImageType (icsType);

	if(bpp == 8 && colour == 1)
		bpp = 24;

	dib = FreeImage_AllocateT(fit, width, height, bpp, 0, 0, 0);

	if(bpp == 8)
		FIA_SetGreyLevelPalette(dib);

	CopyBytesToFIBitmap(dib, data, padded);

	return dib;
}


static void swap_dims(size_t *dims, size_t dim1, size_t dim2)
{
    size_t tmp;

    tmp = dims[dim1];
    dims[dim1] = dims[dim2];
    dims[dim2] = tmp;
}

inline std::vector<std::string> split( const std::string& s, const std::string& f ) {

    std::vector<std::string> temp;

    if ( f.empty() ) {
        temp.push_back( s );
        return temp;
    }
    typedef std::string::const_iterator iter;
    const iter::difference_type f_size( distance( f.begin(), f.end() ) );
    iter i( s.begin() );
    for ( iter pos; ( pos = search( i , s.end(), f.begin(), f.end() ) ) != s.end(); ) {
        temp.push_back( std::string( i, pos ) );
        advance( pos, f_size );
        i = pos;
    }
    temp.push_back( std::string( i, s.end() ) );

    return temp;
}

static void trim(std::string& str)
{
	std::string::size_type pos = str.find_last_not_of(' ');
	
	if(pos != std::string::npos) {
		
		str.erase(pos + 1);	
		pos = str.find_first_not_of(' ');
		
		if(pos != std::string::npos)
			str.erase(0, pos);
	}
	else {
		str.erase(str.begin(), str.end());
	}
}

int DLL_CALLCONV
SwitchDimensionLabels (ICS *ics, char*out, size_t* order)
{
	char labels[ICS_LINE_LENGTH];
 
	if(FreeImageIcs_GetFirstIcsHistoryValueWithKey(ics, "labels", labels) == FIA_ERROR)
        return FIA_ERROR;

	std::string labels_str = std::string(labels);
	trim(labels_str);

    std::vector<std::string> words( split( labels_str, " " ) );
	std::vector<std::string> ordered_words(words.size());

	for(int i=0; i < words.size(); i++)
		ordered_words[i] = words[order[i]];

    std::string output;
    std::vector<std::string>::size_type i;

    for(i=0; i < ordered_words.size(); i++)
        output += ordered_words[i] + " ";

    strcpy(out, output.c_str());

	return FIA_SUCCESS;
}

// Returns strides for ics file.
// Passed in array must be large enough for data.
static void CalculateStrides(ICS *ics, size_t* dims, int ndims, size_t *strides)
{
	int val;

	// Calculate the strides of the new data layout
	strides[0] = 1;

	for(int s = 1; s < ndims; s++)
	{
		val = 1;

		for(int i = 0; i < s; i++)
			val *= dims[i];

		strides[s] = val;
	}
}

void DLL_CALLCONV
FreeImageIcs_SetICSVersion (int version)  // 1 or 2, default=2
{
	if (version == 1)
		ics_version = 1;
	else
		ics_version = 2;
}

void  DLL_CALLCONV
FreeImageIcs_SetCompressionLevel (int level)
{
	if(level < 0)
		level = 0;

	if(level > 9)
		level = 9;

	compression_level = level;
}

size_t DLL_CALLCONV
FreeImageIcs_GetDataSize(ICS *ics)
{
    return IcsGetDataSize (ics);
}

Ics_Error DLL_CALLCONV
FreeImageIcs_GetData (ICS* ics, void* dest, size_t n)
{
  return IcsGetData (ics, dest, n);
}

Ics_Error DLL_CALLCONV
FreeImageIcs_SetData (ICS* ics, void const* src, size_t n)
{
  return IcsSetData (ics, src, n);
}


Ics_Error DLL_CALLCONV
FreeImageIcs_GetLayout (ICS *ics, Ics_DataType* dt, int* ndims, size_t* dims)
{
    return IcsGetLayout(ics, dt, ndims, dims);
}

Ics_Error DLL_CALLCONV
FreeImageIcs_SetLayout (ICS* ics, Ics_DataType dt, int ndims, size_t const* dims)
{
    return IcsSetLayout(ics, dt, ndims, dims);
}

Ics_Error DLL_CALLCONV
FreeImageIcs_SetCompression (ICS* ics, Ics_Compression compression, int level)
{
	// if we want no compression set that explicitly
	if (level ==0)
		compression = IcsCompr_uncompressed;

	return IcsSetCompression (ics, compression, level);
}

int DLL_CALLCONV
FreeImageIcs_SaveIcsFileWithDimensionsAs(ICS *ics, const char *filepath, size_t* order, int size)
{
	Ics_Error err;
    ICS *new_ics;
	int ndims, i;
	size_t old_dims[10], dims[10];
	size_t bufsize;
    Ics_DataType dt;

	BYTE *bits = NULL;

	if(ics == NULL)
		goto Error;
  
    // Create new ics file that contails the swapped dimensions
	if (ics_version == 1)
		err = IcsOpen (&new_ics, filepath, "w1");
	else
		err = IcsOpen (&new_ics, filepath, "w2");
	
	if(err != IcsErr_Ok)
   		goto Error;

    // Get number of dimensions in old ics file
    if((err = IcsGetLayout(ics, &dt, &ndims, old_dims)) != IcsErr_Ok)
   		goto Error;
	
	// Check all dims given are valid
	for(i=0; i < ndims; i++) {

		if(order[i] >= ndims)
			goto Error;
	}

    bufsize = IcsGetDataSize (ics);

    bits = (BYTE*) malloc(bufsize);
	if (!bits) goto Error;

	size_t strides[10], new_strides[10];

	// Get dims of order
	for(i=0; i < size; i++)
		dims[i] = old_dims[order[i]];

	CalculateStrides(ics, dims, ndims, strides);

	// Put strides in order we want

	for(int i=0; i < size; i++)
		new_strides[order[i]] = strides[i];

	if((err = IcsGetDataWithStrides (ics, bits, bufsize, new_strides, ndims)) != IcsErr_Ok)
   		goto Error;

    FreeImageIcs_CopyHistoryText(ics, new_ics);

    char out[100];

    // Get number of dimensions in old ics file
    if( IcsSetLayout(new_ics, dt, ndims, (size_t *) dims) != IcsErr_Ok)
		goto Error;	

    if( (err = IcsSetData(new_ics, bits, bufsize)) != IcsErr_Ok)
		goto Error;

    // Change the order
	char order_str[100], label_str[100];

	for(int i=0; i < ndims; i++) {
        IcsGetOrder (ics, order[i], order_str, label_str);
	    IcsSetOrder (new_ics, i, order_str, label_str);
	}
   
	if(SwitchDimensionLabels (new_ics, out, order) == FIA_SUCCESS)
    {
	    // Change Labels History
        FreeImageIcs_ReplaceIcsHistoryValueForKey(new_ics, "labels", out);
    }

    if( (err = FreeImageIcs_SetCompression (new_ics, IcsCompr_gzip, compression_level)) != IcsErr_Ok)
	    goto Error;

    if( (err = FreeImageIcs_IcsClose (new_ics)) != IcsErr_Ok)
	    goto Error;

	free(bits);

	return FIA_SUCCESS;

	Error:

	if(bits)
		free(bits);

	return FIA_ERROR;
}


int DLL_CALLCONV
FreeImageIcs_SaveIcsFileWithFirstTwoDimensionsAs(ICS *ics, const char *filepath, int first, int second)
{
	int ndims, i;
	size_t dims[10];
    Ics_DataType dt;
	Ics_Error err;

    // Get number of dimensions in old ics file
    if((err = IcsGetLayout(ics, &dt, &ndims, dims)) != IcsErr_Ok)
   		return FIA_ERROR;

    dims[0] = first; 
	dims[1] = second; 
						
	int count = 2; 
			
    // If we have dimensions [1,2,3,4,5,6]
    // and we want 3 and 5 first and second the following code does this
    //[3, 5, 1, 2, 4, 6]

    // Find next value in old array not the first two
	for(i=0; i < ndims; i++) {
					
		if(i == dims[0] || i == dims[1])    
		    continue;
					
		dims[count++] = i;					
	}
			
	return FreeImageIcs_SaveIcsFileWithDimensionsAs(ics, filepath, dims, ndims);
}

FIBITMAP* DLL_CALLCONV
FreeImageIcs_GetIcsImageDimensionalDataSlice(ICS *ics, int dimension, int slice)
{
	Ics_DataType dataType;
	int ndims;
	size_t dims[ICS_MAXDIM];

    if(dimension < 2)
        return NULL;

	memset(dims, 0, sizeof(size_t) * ICS_MAXDIM);

	// Reset the pointer to the beginning of file.
	if(ics->BlockRead)
		IcsSetIdsBlock (ics, 0, SEEK_SET);

	IcsGetLayout (ics, &dataType, &ndims, dims);

	// Dimension must not be greater than the total dimensions
	if(dimension > ndims)
		return NULL;

	// If the slice is greater than the size for that dimension return error;
	if(slice != 0 && slice >= (int) dims[dimension])
		return NULL;

	size_t bufsize;

	GetSizeInBytesBetweenDimensions(dims, ndims, dataType, dimension, &bufsize);		
	
	if(IcsSkipDataBlock  (ics, bufsize * slice) != IcsErr_Ok)
		return NULL;

	BYTE *buf = (BYTE *) malloc (bufsize);

	if (buf == NULL)
   		return NULL;

	if(IcsGetDataBlock  (ics, buf, bufsize) != IcsErr_Ok) {
		free(buf);
		return NULL;
	}

	FIBITMAP *dib =
        FreeImageIcs_CreateFIB(buf, dataType, GetIcsDataTypeBPP (dataType), (int) dims[0], (int) dims[1], 0, 0);
	
	free(buf);

	return dib;
}


static FIBITMAP*
FreeImageIcs_GetIcsImageDimensionalDataSliceFromData(BYTE *data, Ics_DataType dataType, int ndims,  size_t *dims, int dimension, int slice)
{
	size_t dimension_size;

    if(dimension < 2)
        return NULL;

	// Dimension must not be greater than the total dimensions
	if(dimension > ndims)
		return NULL;

	// If the slice is greater than the size for that dimension return error;
	if(slice != 0 && slice >= (int) dims[dimension])
		return NULL;

	GetSizeInBytesBetweenDimensions(dims, ndims, dataType, dimension, &dimension_size);
		
	// Skip data block
	data += (dimension_size * slice);

	BYTE *buf = (BYTE *) malloc (dimension_size);

	if (buf == NULL)
   		return NULL;

	memcpy(buf, data, dimension_size);

	FIBITMAP *dib =
        FreeImageIcs_CreateFIB(buf, dataType, GetIcsDataTypeBPP (dataType), (int) dims[0], (int) dims[1], 0, 0);
	
	free(buf);

	return dib;
}

// This function get the sum intensity projection of each slice in the multidimensional image.
// The dimension must be specified.
// Each slice is made up off the first two dimensions.
FIBITMAP* DLL_CALLCONV
FreeImageIcs_SumIntensityProjection(ICS *ics, int dimension)
{
    Ics_DataType dataType;
	int ndims;
	size_t dims[ICS_MAXDIM], bufsize;
	BYTE *data = NULL;

    if(dimension < 2)
        return NULL;

	memset(dims, 0, sizeof(size_t) * ICS_MAXDIM);

	IcsGetLayout (ics, &dataType, &ndims, dims);

	// Dimension must not be greater than the total dimensions
	if(dimension > ndims)
		return NULL;

	int number_of_slices = (int) dims[dimension];
	
    int width = dims[0];
    int height = dims[1];

    FIBITMAP *tmp = NULL;
    FIBITMAP *sum = FreeImage_AllocateT(FIT_FLOAT, width, height, 32, 0, 0, 0);
    bufsize = IcsGetDataSize (ics);
	data = (BYTE*) malloc(bufsize);

	if (!data) goto Error;

	if(IcsGetData(ics, data, bufsize) != IcsErr_Ok) {
        FreeImage_OutputMessageProc(FIF_UNKNOWN, "Error calling IcsGetData buffer size");
		goto Error;
    }

    if(sum == NULL)
         goto Error;

    if(sum == NULL)
        return NULL;
    
    if(number_of_slices < 2)
        return sum;

    for(int i=1; i < number_of_slices; i++) {

        // Get slice image
		if((tmp = FreeImageIcs_GetIcsImageDimensionalDataSliceFromData(data, dataType, ndims,  dims, dimension, i)) == NULL)
			goto Error;

        if(FIA_AddGreyLevelImages(sum, tmp) == FIA_ERROR) {
            FreeImage_Unload(tmp);
            goto Error;
        }

        FreeImage_Unload(tmp);
    }

    return sum;

    Error:
    
		free(data);
        FreeImage_Unload(sum);
        return NULL;
}

FIBITMAP* DLL_CALLCONV
FreeImageIcs_MaximumIntensityProjection(ICS *ics, int dimension)
{
    Ics_DataType dataType;
	int ndims;
	size_t dims[ICS_MAXDIM], bufsize;
	BYTE *data = NULL;

    if(dimension < 2)
        return NULL;

	memset(dims, 0, sizeof(size_t) * ICS_MAXDIM);

	IcsGetLayout (ics, &dataType, &ndims, dims);
	
	// Dimension must not be greater than the total dimensions
	if(dimension > ndims)
		return NULL;

	int number_of_slices = (int) dims[dimension];
	
    int width = dims[0];
    int height = dims[1];

    FIBITMAP *sum = FreeImage_AllocateT(FIT_FLOAT, width, height, 32, 0, 0, 0);
    FIBITMAP *tmp = NULL;

	bufsize = IcsGetDataSize (ics);
	data = (BYTE*) malloc(bufsize);
	if (!data) goto Error;

	if(IcsGetData(ics, data, bufsize) != IcsErr_Ok) {
        FreeImage_OutputMessageProc(FIF_UNKNOWN, "Error calling IcsGetData buffer size");
		goto Error;
    }
        
    if(sum == NULL)
         goto Error;
    
    if(number_of_slices < 2)
        return sum;

    for(int i=1; i < number_of_slices; i++) {

		if((tmp = FreeImageIcs_GetIcsImageDimensionalDataSliceFromData(data, dataType, ndims,  dims, dimension, i)) == NULL)
			goto Error;

        if(FIA_GetMaxIntensityFromImages(sum, tmp) == FIA_ERROR)             {
            FreeImage_Unload(tmp);
            goto Error;
        }

        FreeImage_Unload(tmp);
    }

    return sum;

    Error:
    
		free(data);
        FreeImage_Unload(sum);
        return NULL;
}



static FIBITMAP* DLL_CALLCONV
LoadFIBFromColourIcsFile (ICS *ics, int width, int height)
{
	FIBITMAP *dib = FreeImage_AllocateT(FIT_BITMAP, width, height, 24, 0, 0, 0);
	 
	FIBITMAP *red_fib = FreeImageIcs_GetIcsImageDimensionalDataSlice(ics, 2, 0);
	FIBITMAP *green_fib = FreeImageIcs_GetIcsImageDimensionalDataSlice(ics, 2, 1);
	FIBITMAP *blue_fib = FreeImageIcs_GetIcsImageDimensionalDataSlice(ics, 2, 2);

	FreeImage_SetChannel(dib, red_fib, FICC_RED);
	FreeImage_SetChannel(dib, green_fib, FICC_GREEN);
	FreeImage_SetChannel(dib, blue_fib, FICC_BLUE);

	FreeImage_Unload(red_fib);
	FreeImage_Unload(green_fib);
	FreeImage_Unload(blue_fib);

	return dib;
}

int DLL_CALLCONV
FreeImageIcs_IsIcsFile (const char *filepath)
{
	return IcsVersion (filepath, 0) ? 1 : 0;
}


int DLL_CALLCONV
FreeImageIcs_IsIcsFileColourFile(ICS *ics)
{
	Ics_DataType dt;
	int ndims, channels = 1;
	int dims[ICS_MAXDIM];
	char order[10], label[50];

	// GCI haven't been storing c is the labels or ordering for colour images.
	// They have left it as z. However, they have added some history with key labels 
	// that does the same thing.
	
	if(FreeImageIcs_GetFirstIcsHistoryValueWithKey(ics, "labels", label) == FIA_SUCCESS) {

		if(strcmp(label, "x y c") == 0)
			return 1;
	}

	// More tradintional layout
	IcsGetLayout (ics, &dt, &ndims, (size_t *) dims);

	// If any of the labels or orders contain a c then it is some form of colour image.
	for(int i=0; i < ndims; i++) {
		
		if(IcsGetOrder (ics, i, order, label) != IcsErr_Ok)
			return 0;

		if(strcmp(order, "c") == 0)
			return 1;
	}

	return 0;
}


int DLL_CALLCONV
FreeImageIcs_NumberOfDimensions (ICS *ics)
{
	Ics_DataType dt;
	int ndims;
	int dims[ICS_MAXDIM];
	
	IcsGetLayout (ics, &dt, &ndims, (size_t *) dims);

	return ndims;
}


int DLL_CALLCONV
FreeImageIcs_GetDimensionDetails (ICS *ics, int dimension, char* order, char *label, int* size)
{	
	Ics_DataType dt;
	int ndims, channels = 1;
	int dims[ICS_MAXDIM];
	
    IcsGetLayout (ics, &dt, &ndims, (size_t *) dims);
    *size = dims[dimension];

    // If we have labels history then used those instead of order
    // Someone added Labels to history and we are now stuck with 
    // it for backward compatibility.
	if(FreeImageIcs_GetLabelForDimension (ics, dimension, label) == FIA_SUCCESS)
        return FIA_SUCCESS;
	
    // Hmm no labels ? Use the order specified in all ics files.
	IcsGetOrder  (ics, dimension, order, label);

	return FIA_SUCCESS;
}


FIBITMAP* DLL_CALLCONV
FreeImageIcs_LoadFIBFromIcsFile (ICS *ics)
{
	FIBITMAP *dib;
	Ics_DataType dt;
	int ndims, channels = 1;
	int dims[ICS_MAXDIM];

	if(IcsGetLayout (ics, &dt, &ndims, (size_t *) dims) != IcsErr_Ok)
        return NULL;

    if(FreeImageIcs_IsIcsFileColourFile(ics))
		dib = LoadFIBFromColourIcsFile (ics, dims[0], dims[1]);
    else
		dib = FreeImageIcs_GetIcsImageDimensionalDataSlice(ics, 2, 0);

	return dib;
}

FIBITMAP* DLL_CALLCONV
FreeImageIcs_LoadFIBFromColourIcsFile (ICS *ics)
{
	Ics_DataType dt;
	int ndims, channels = 1;
	int dims[ICS_MAXDIM];

	if(IcsGetLayout (ics, &dt, &ndims, (size_t *) dims) != IcsErr_Ok)
        return NULL;

	return LoadFIBFromColourIcsFile (ics, dims[0], dims[1]);
}

FIBITMAP* DLL_CALLCONV
FreeImageIcs_LoadFIBFromIcsFilePath (const char* filepath)
{
	ICS *ics;

	if(IcsOpen(&ics, filepath, "r") != IcsErr_Ok)
        return NULL;

	FIBITMAP *fib = FreeImageIcs_LoadFIBFromIcsFile (ics);

	IcsClose(ics);

	return fib;
}



// Ics Saving Code

static void
GetColourImageDataInIcsFormat(FIBITMAP *dib, void *data)
{
	register int channel, x, y;  
	int i=0;
	int line = FreeImage_GetLine(dib);
	int height = FreeImage_GetHeight(dib);

	BYTE *bits = FreeImage_GetBits(dib);
	BYTE *buffer = (BYTE *) data;		
	
	for(channel = 2; channel >= 0; channel-- )
	{
		for(y = 0; y < height; y++) {

			bits = (BYTE *) FreeImage_GetScanLine(dib, height - y - 1);

			for( x = channel; x < line; x+=3)
				*buffer++ = bits[x]; i++;
		}
	}
}


// This function get the bits bottom row first.
// It also ignores padding bytes as these are not stored
// in ics files.
static void
FreeImage_GetBitsVerticalFlip(FIBITMAP *dib, BYTE *bytes)
{
	register int y;
	BYTE *scanline_bytes;
	int height = FreeImage_GetHeight(dib);
	int pitch = FreeImage_GetPitch(dib);
	int line = FreeImage_GetLine(dib); 
	
	for( y = 0; y < height; y++) {

		scanline_bytes = (BYTE *) FreeImage_GetScanLine(dib, height - y - 1);

		memcpy( bytes, scanline_bytes, line ); 
				
		bytes += line;
	}
}

int
SaveGreyScaleImage (FIBITMAP *dib, const char *filepath, bool save_metadata)
{
	ICS* ics = NULL;
	Ics_Error err;
	int ndims;
	int dims[2];
	int bufsize;
	int bytes_per_pixel;

	if(dib == NULL)
		return FIA_ERROR;

	if (ics_version == 1)
		err = IcsOpen (&ics, filepath, "w1");
	else
		err = IcsOpen (&ics, filepath, "w2");

	if (err != IcsErr_Ok)
   		return FIA_ERROR;

	bytes_per_pixel = FreeImage_GetBPP(dib) / 8;

	ndims = 2;	
	dims[0] = FreeImage_GetWidth(dib); 
    dims[1] = FreeImage_GetHeight(dib);
  
	Ics_DataType dt = FreeImageTypeToIcsType (FreeImage_GetImageType(dib));

	bufsize = dims[0] * dims[1] * bytes_per_pixel;

	if(save_metadata) {
		IcsAddHistory (ics, "labels", "x y");
	}

	BYTE *bits = (BYTE*) malloc(bufsize);

    if(bits == NULL)
        goto Error;

	FreeImage_GetBitsVerticalFlip(dib, bits);

	if( IcsSetLayout(ics, dt, ndims, (size_t *) dims) != IcsErr_Ok)
		goto Error;

	if(IcsSetOrder  (ics, 0, "x", "x-position") != IcsErr_Ok)
		goto Error;

	if(IcsSetOrder  (ics, 1, "y", "y-position") != IcsErr_Ok)
		goto Error;

	if( (err = IcsSetData(ics, bits, bufsize)) != IcsErr_Ok)
		goto Error;
		
	if( FreeImageIcs_SetCompression (ics, IcsCompr_gzip, compression_level) != IcsErr_Ok)
		goto Error;
	
	if( IcsClose (ics) != IcsErr_Ok) {
		ics = NULL;
		goto Error;
	}

	free(bits);
	bits = NULL;

	return FIA_SUCCESS;

	Error:

	if(bits) {
		free(bits);
		bits = NULL;
	}

	if(ics != NULL) {
        IcsClose (ics);
		ics = NULL;
	}

	return FIA_ERROR;
}


static int
SaveColourImage (FIBITMAP *dib, const char *filepath, bool save_metadata)
{
	ICS* ics;
	Ics_Error err;
	Ics_DataType dt = Ics_uint8;
	FIBITMAP *standard_dib;
	int ndims;
	int dims[3];
	int bufsize;
	
	if(dib == NULL)
		return FIA_ERROR;

	if(FIA_IsGreyScale(dib))
		return FIA_ERROR;

	if (ics_version == 1)
		err = IcsOpen (&ics, filepath, "w1");
	else
		err = IcsOpen (&ics, filepath, "w2");

	if (err != IcsErr_Ok)
   		return FIA_ERROR;

	if(FreeImage_GetBPP(dib) != 24)
		standard_dib = FreeImage_ConvertTo24Bits(dib);
	else
		standard_dib = dib;

	ndims = 3;	
	dims[0] = FreeImage_GetWidth(standard_dib); 
    dims[1] = FreeImage_GetHeight(standard_dib);
	dims[2] = 3;
  
	bufsize = dims[0] * dims[1] * dims[2];	
	
	if(save_metadata) {
		IcsAddHistory (ics, "labels", "x y c");
	}

	BYTE *bits = (BYTE*) malloc(bufsize);
	if (!bits) goto Error;

	GetColourImageDataInIcsFormat(standard_dib, bits);

	if( IcsSetLayout(ics, dt, ndims, (size_t *) dims) != IcsErr_Ok)
		goto Error;

	if(IcsSetOrder  (ics, 0, "x", "x-position") != IcsErr_Ok)
		goto Error;

	if(IcsSetOrder  (ics, 1, "y", "y-position") != IcsErr_Ok)
		goto Error;

	if(IcsSetOrder	 (ics, 2, "c", "colour") != IcsErr_Ok)
		goto Error;

	if( (err = IcsSetData(ics, bits, bufsize)) != IcsErr_Ok)
		goto Error;
		
	if( FreeImageIcs_SetCompression (ics, IcsCompr_gzip, compression_level) != IcsErr_Ok)
		goto Error;
	
	if( IcsClose (ics) != IcsErr_Ok) {
		
		FreeImage_Unload(dib);
		goto Error;
	}

	free(bits);
	if(FreeImage_GetBPP(dib) != 24)   // only need to unload if this temporary image was created
		FreeImage_Unload(standard_dib);

	return FIA_SUCCESS;

Error:
	if (bits) free(bits);

	return FIA_ERROR;
}


int DLL_CALLCONV
FreeImageIcs_SaveImage (FIBITMAP *dib, const char *filepath, int save_metadata)
{
    int md_save = 0;

    if(save_metadata)
        md_save = 1;

	if(FIA_IsGreyScale(dib))
		return SaveGreyScaleImage (dib, filepath, md_save);
	else
		return SaveColourImage (dib, filepath, md_save);
}


int DLL_CALLCONV
FreeImageIcs_SaveIcsDataToFile (const char *filepath, void *data, Ics_DataType dt, int ndims, int *dims)
{
	ICS* ics;
	Ics_Error err;
	int bufsize = 1;
	
	if (ics_version == 1)
		err = IcsOpen (&ics, filepath, "w1");
	else
		err = IcsOpen (&ics, filepath, "w2");

    if (err != IcsErr_Ok) {
        FreeImage_OutputMessageProc(FIF_UNKNOWN, "Error opening filepath %s", filepath);
   		return FIA_ERROR;
    }

	for(int i=0; i < ndims; i++)
		bufsize *= dims[i];

    int bytes_per_pixel = GetIcsDataTypeBPP (dt) / 8;
    bufsize *= bytes_per_pixel;

    if( IcsSetLayout(ics, dt, ndims, (size_t *) dims) != IcsErr_Ok) {
        FreeImage_OutputMessageProc(FIF_UNKNOWN, "Error calling IcsSetLayout");
		return FIA_ERROR;
    }

    if( (err = IcsSetData(ics, data, bufsize)) != IcsErr_Ok) {
        FreeImage_OutputMessageProc(FIF_UNKNOWN, "Error calling IcsSetData buffer size %d bytes per pixel %d", bufsize, bytes_per_pixel);
		return FIA_ERROR;
    }
		
    if( FreeImageIcs_SetCompression (ics, IcsCompr_gzip, compression_level) != IcsErr_Ok) {
        FreeImage_OutputMessageProc(FIF_UNKNOWN, "Error calling IcsSetCompression");
		return FIA_ERROR;
    }
	
    if( (err = IcsClose (ics)) != IcsErr_Ok) {
		FreeImage_OutputMessageProc(FIF_UNKNOWN, "Error calling IcsClose: %d", err);
		return FIA_ERROR;
    }

	return FIA_SUCCESS;
}


int DLL_CALLCONV
FreeImageIcs_SaveIcsImageDataToBinaryFile (const char *filepath, const char *output_filepath)
{
	ICS* ics;
	Ics_Error err;
	size_t bufsize = 1;
	FILE *fp = NULL;
	BYTE *bits = NULL;

	err = IcsOpen (&ics, filepath, "r");

    if (err != IcsErr_Ok) {
        FreeImage_OutputMessageProc(FIF_UNKNOWN, "Error opening filepath %s", filepath);
   		return FIA_ERROR;
    }

	//int bytes_per_pixel = GetIcsDataTypeBPP (dt) / 8;

	bufsize = IcsGetDataSize (ics);
    bits = (BYTE*) malloc(bufsize);

    if( (err = IcsGetData(ics, bits, bufsize)) != IcsErr_Ok) {
        FreeImage_OutputMessageProc(FIF_UNKNOWN, "Error calling IcsGetData buffer size");
		goto Error;
    }

	if((fp = fopen(output_filepath, "wb")) == NULL) {
		FreeImage_OutputMessageProc(FIF_UNKNOWN, "Error opening filepath %s", filepath);
		goto Error;
	}

	 /* write the entire array in one step */
	if( fwrite(bits, bufsize, 1, fp) != 1) {
		FreeImage_OutputMessageProc(FIF_UNKNOWN, "Error saving filepath %s", filepath);
		goto Error;
	}

	fclose(fp);
		
	if( (err = FreeImageIcs_IcsClose (ics)) != IcsErr_Ok)
	    goto Error;

	free(bits);

	return FIA_SUCCESS;

Error:

	if(bits != NULL)
		free(bits);

	return FIA_ERROR;
}

Ics_Error DLL_CALLCONV
FreeImageIcs_IcsOpen (ICS* *ics, char const* filename, char const* mode)
{
    return IcsOpen (ics, filename, mode);
}

Ics_Error DLL_CALLCONV
FreeImageIcs_IcsClose (ICS* ics)
{
    return IcsClose (ics);
}

Ics_Error DLL_CALLCONV
FreeImageIcs_IcsAddHistoryString (ICS* ics, char const* key, char const* value)
{
    return IcsAddHistoryString (ics, key, value);
}

Ics_Error DLL_CALLCONV
FreeImageIcs_IcsSetNativeIntensityScale (ICS* ics, double origin, double scale, const char *units)
{
    return IcsSetImelUnits (ics, origin, scale, units);
}

Ics_Error DLL_CALLCONV
FreeImageIcs_IcsSetNativeScale (ICS* ics, int dimension, double origin, double scale, const char *units)
{
    return IcsSetPosition  (ics, dimension, origin, scale, units);
}

Ics_Error DLL_CALLCONV
FreeImageIcs_IcsSetDimOrder (ICS* ics, int dimension, const char* order, const char* label)
{
    return IcsSetOrder  (ics, dimension, order, label);
}

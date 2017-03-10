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
#include "FreeImageIcs_MetaData.h"

#include "FreeImageAlgorithms_IO.h"
#include "FreeImageAlgorithms_Utilities.h"
#include "FreeImageAlgorithms_Palettes.h"

#include "libics_ll.h"

extern const char* const Errors[] = 
{
        "No Error",
        "Non fatal error: unexpected data size",
        "Non fatal error: the output buffer could not be completely filled (meaning that your buffer was too large)",
        "Memory allocation error",
        "Image size conflicts with bits per element",
        "The buffer was too small to hold the given ROI",
        "Some error occurred during compression",
        "The compressed input stream is currupted",
        "Some error occurred during decompression",
        "The ICS data structure already contains incompatible stuff",
        "Empty field (intern error)",
        "All history lines have already been returned",
        "Unexpected end of stream",
        "Failed to write a line in .ics file",
        "File close error on .ics file",
        "File close error on .ids file",
        "Failed to copy image data from temporary file on .ics file opened for updating",
        "File open error on .ics file",
        "File open error on .ids file",
        "File read error on .ics file",
        "File read error on .ids file",
        "Failed to remane .ics file opened for updating",
        "File write error on .ics file",
        "File write error on .ids file",
        "The given ROI extends outside the image",
        "Illegal ICS token detected",
        "A function parameter has a value that is not legal or does not match with a value previously given",
        "Line overflow in ics file",
        "Missing bits element in .ics file",
        "Missing main category",
        "There is no Data defined",
        "Missing layout subcategory",
        "Missing parameter subcategory",
        "Missing representation subcategory",
        "Missing sensor subcategory",
        "Missing sensor subsubcategory",
        "Missing sub category",
        "Layout parameters missing or not defined",
        "There doesn't exist a SCIL_TYPE value for this image",
        "Not an ICS file",
        "The function won't work on the ICS given",
        "Too many channels specified",
        "Data has too many dimensions",
        "Unknown compression type",
        "The datatype is not recognized",
        "libics is linking to a different version of zlib than used during compilation"
};


#include <iostream>


FREE_IMAGE_TYPE DLL_CALLCONV
IcsTypeToFreeImageType (Ics_DataType icsType)
{
	switch (icsType)
	{
	 case Ics_sint8:
	 case Ics_uint8:
	 
		 return FIT_BITMAP;
		 
	 case Ics_sint16:
	 
		 return FIT_INT16;
		 
	 case Ics_uint16:
	 
		 return FIT_UINT16;

	 case Ics_sint32:
	 
		 return FIT_INT32;
		 
	 case Ics_uint32:
	 
		 return FIT_UINT32;
		 
	 case Ics_real32:
	 
		 return FIT_FLOAT;
		 
	 case Ics_real64:
	 
		 return FIT_DOUBLE;
	}
	
	return FIT_UNKNOWN;
}


Ics_DataType DLL_CALLCONV
FreeImageTypeToIcsType (FREE_IMAGE_TYPE fit)
{
	switch (fit)
	{
	 	case FIT_BITMAP:	
	
	 		return Ics_uint8;
	 
		case FIT_INT16:
		
			return Ics_sint16;
	
		case FIT_UINT16:
		
			return Ics_uint16;
			
		case FIT_INT32:
		
			return Ics_sint32;
			
		case FIT_UINT32:
		
			return Ics_uint32;
			
		case FIT_FLOAT:
		
			return Ics_real32;
			
		case FIT_DOUBLE:
		
			return Ics_real64;
	}		
	
	return Ics_unknown;
}


int DLL_CALLCONV
GetIcsDataTypeBPP (Ics_DataType icsType)
{
	switch (icsType)
	{
	 case Ics_sint8:
		 return 8;
		 
	 case Ics_uint8:
		 return 8;
		 
	 case Ics_sint16:
		 return 16;
		 
	 case Ics_uint16:
		 return 16;
		 
	 case Ics_sint32:
		 return 32;
		 
	 case Ics_uint32:
		 return 32;
		 
	 case Ics_real32:
		 return 32;
		 
	 case Ics_real64:
		 return 64;
	}
	
	return 0;
}

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

#ifndef __FIA_PRIVATE__
#define __FIA_PRIVATE__

#include "FreeImageIcs_IO.h"

extern const char* const Errors[];

FREE_IMAGE_TYPE DLL_CALLCONV
IcsTypeToFreeImageType (Ics_DataType icsType);

Ics_DataType DLL_CALLCONV
FreeImageTypeToIcsType (FREE_IMAGE_TYPE fit);

int DLL_CALLCONV
GetIcsDataTypeBPP (Ics_DataType icsType);

#endif


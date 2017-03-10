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

#ifndef __FIA_METADATA__
#define __FIA_METADATA__

#ifdef __cplusplus
extern "C" {
#endif

#include "FreeImageIcs_IO.h"

DLL_API int DLL_CALLCONV
FreeImageIcs_GetIcsHistoryStringCount(ICS *ics);

/*
Split a history string into key and value.
*/
DLL_API int DLL_CALLCONV
FreeImageIcs_SplitHistoryString(char *history_string, char *key, char *value);

/*
Join a key and value to construct a well formatted ics history string.
*/
DLL_API int DLL_CALLCONV
FreeImageIcs_JoinKeyValueIntoHistoryString(char *history_string, char *key, char *value);

/*
Delete all history in a file.
*/
DLL_API int DLL_CALLCONV
FreeImageIcs_IcsDeleteHistory(ICS *ics);

/*
Sets all the history string in one go.
The var argument must end in NULL.
Old history strings are removed.
*/
DLL_API int DLL_CALLCONV
FreeImageIcs_SetIcsHistoryStrings(ICS *ics, ...);

/*
Sets all the history keys and values in one go.
The var argument must end in NULL.
Old history strings are removed.
*/
DLL_API int DLL_CALLCONV
FreeImageIcs_SetIcsHistoryKeyValueStrings(ICS *ics, ...);


/*
Add more than one history strings in one go.
The var argument must end in NULL.
*/
DLL_API int DLL_CALLCONV
FreeImageIcs_AddIcsHistoryKeyValueStrings(ICS *ics, ...);

/*
Add history strings stored in an array.
*/
DLL_API int DLL_CALLCONV
FreeImageIcs_SetIcsHistoryKeyValueStringsFromArray(ICS *ics, char **history_strings, int number_of_items);

/*
Replace history string stored in an array.
*/
DLL_API int DLL_CALLCONV
FreeImageIcs_ReplaceIcsHistoryValueForKey(ICS *ics, const char *key, const char *value);


/*
Get the first history string value for the corresponding key.
*/
DLL_API int DLL_CALLCONV
FreeImageIcs_GetFirstIcsHistoryValueWithKey(ICS *ics, const char *key, char *value);


/*
Is the Iterator valid
*/
DLL_API int DLL_CALLCONV
IsIcsHistoryIteratorValid (ICS* ics, Ics_HistoryIterator* it);

DLL_API Ics_Error DLL_CALLCONV
FreeImageIcs_IcsNewHistoryIterator (ICS* ics, Ics_HistoryIterator* it, char const* key);

DLL_API Ics_Error DLL_CALLCONV
FreeImageIcs_IcsGetHistoryStringI (ICS* ics, Ics_HistoryIterator* it, char* string);

DLL_API int DLL_CALLCONV
FreeImageIcs_GetHistoryText(ICS *ics, char *text);

DLL_API int DLL_CALLCONV
FreeImageIcs_GetHistoryTextFromFile(const char *filepath, char *text);


DLL_API int DLL_CALLCONV
FreeImageIcs_CopyHistoryText(ICS *ics, ICS *dst_ics);

DLL_API Ics_Error DLL_CALLCONV
FreeImageIcs_IcsAddHistoryString (ICS* ics, char const* key, char const* value);

#ifdef __cplusplus
}
#endif

#endif

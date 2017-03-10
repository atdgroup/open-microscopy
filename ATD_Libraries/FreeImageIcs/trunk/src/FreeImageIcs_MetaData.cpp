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

#include "FreeImageIcs_MetaData.h"
#include "FreeImageIcs_Private.h"
#include "FreeImageIcs_IO.h"

#include "FreeImageAlgorithms.h"

#include "libics.h"

#include <stdio.h>
#include <string.h>

#include <iostream>

static char*
str_strlwr(char *s)
{
   if (s != NULL)
   {
      char *p;

      for (p = s; *p && p; p++)
         *p = tolower(*p);
   }
   
   return s;
}


int DLL_CALLCONV
FreeImageIcs_GetLabelForDimension (ICS *ics, int dimension, char *label)
{
	char labels[ICS_LINE_LENGTH];

    int no_of_dimensions = FreeImageIcs_NumberOfDimensions (ics);

	if(FreeImageIcs_GetFirstIcsHistoryValueWithKey(ics, "labels", labels) == FIA_ERROR)
        return FIA_ERROR;

	char *result = strtok(labels, " ");

	if(dimension < 0 || dimension >= no_of_dimensions)
		strcpy(label, result);

	for(int i = 1; i <= dimension; i++) {
		if((result = strtok(NULL, " ")) == NULL)
			return FIA_ERROR;
	}

	if(result != NULL)
		strcpy(label, result);
	else
		return FIA_ERROR;

	return FIA_SUCCESS;
}


int DLL_CALLCONV
FreeImageIcs_GetIcsHistoryStringCount(ICS *ics)
{
	int number_of_history_strings;
	
	/* Get ICS history */
	if (IcsGetNumHistoryStrings  (ics, &number_of_history_strings) != IcsErr_Ok)
		return 0;

	if(!number_of_history_strings)
		return 0;
	
	return number_of_history_strings;
}


int DLL_CALLCONV
FreeImageIcs_SplitHistoryString(char *history_string, char *key, char *value)
{
	char *s;

	if((s = strchr (history_string, 0x09)) == NULL) // Finds the splitting character
		return FIA_ERROR;
	
	strncpy (key, history_string, (s - history_string)); // Copy the key
	key[(s - history_string)] = '\0'; // Add NULL char
	
	strcpy(value, (s+1));  // copy the value

	return FIA_SUCCESS;
}


int DLL_CALLCONV
FreeImageIcs_JoinKeyValueIntoHistoryString(char *history_string, char *key, char *value)
{
	strcpy(history_string, key);

	size_t length = strlen(history_string);
	
	history_string[length] = 0x09;
	history_string[length + 1] = '\0';
	strcat(history_string, value);

	return FIA_SUCCESS;
}


int DLL_CALLCONV
FreeImageIcs_GetFirstIcsHistoryValueWithKey(ICS *ics, const char *key, char *value)
{
	Ics_HistoryIterator it;     
    Ics_Error err;

	if((err = IcsNewHistoryIterator (ics, &it, key)) != IcsErr_Ok)
		return FIA_ERROR;	

	if(IcsGetHistoryKeyValueI (ics, &it, NULL, value) != IcsErr_Ok)
		return FIA_ERROR;	

	return FIA_SUCCESS;
}


int DLL_CALLCONV
FreeImageIcs_ReplaceIcsHistoryValueForKey(ICS *ics, const char *key, const char *value)
{
	Ics_HistoryIterator it;     
    char temp[ICS_LINE_LENGTH];

	if(IcsNewHistoryIterator (ics, &it, key) != IcsErr_Ok)
		return FIA_ERROR;	
   
	if(IcsGetHistoryKeyValueI (ics, &it, NULL, temp) != IcsErr_Ok)
		return FIA_ERROR;

    if(IcsReplaceHistoryStringI (ics, &it, key, value) != IcsErr_Ok)
		return FIA_ERROR;	

	return FIA_SUCCESS;
}

int DLL_CALLCONV
FreeImageIcs_IcsDeleteHistory(ICS *ics)
{
	IcsDeleteHistory (ics, NULL);
    return FIA_SUCCESS; 
}

int DLL_CALLCONV
FreeImageIcs_SetIcsHistoryKeyValueStrings(ICS *ics, ...)
{
	IcsDeleteHistory (ics, NULL);

	char *history_key, *history_string;

	va_list ap;
	va_start(ap, ics);

	while((history_key = va_arg(ap, char*)) != NULL)
	{
		history_string = va_arg(ap, char*);
		IcsAddHistory (ics, history_key, history_string);
	}
			
	va_end(ap);

	return FIA_SUCCESS; 
}


int DLL_CALLCONV
FreeImageIcs_SetIcsHistoryStrings(ICS *ics, ...)
{
	IcsDeleteHistory (ics, NULL);

	char *history_string;

	va_list ap;
	va_start(ap, ics);

	while((history_string = va_arg(ap, char*)) != NULL)
		IcsAddHistory (ics, NULL, history_string);
			
	va_end(ap);

	return FIA_SUCCESS; 
}


int DLL_CALLCONV
FreeImageIcs_SetIcsHistoryKeyValueStringsFromArray(ICS *ics, char **history_strings, int number_of_items)
{
	IcsDeleteHistory (ics, NULL);

	for(int i=0; i < number_of_items; i++)
		IcsAddHistory (ics, NULL, history_strings[i]);	

	return FIA_SUCCESS; 
}


int DLL_CALLCONV
FreeImageIcs_AddIcsHistoryKeyValueStrings(ICS *ics, ...)
{
	char **history_strings = NULL;

	char *history_key, *history_string;

	va_list ap;
	va_start(ap, ics);

	while((history_key = va_arg(ap, char*)) != NULL)
	{
		history_string = va_arg(ap, char*);
		IcsAddHistory (ics, history_key, history_string);
	}
			
	va_end(ap);

	return FIA_SUCCESS;  
}


int DLL_CALLCONV
IsIcsHistoryIteratorValid (ICS* ics, Ics_HistoryIterator* it)
{
	return it->next;
}

Ics_Error DLL_CALLCONV
FreeImageIcs_IcsNewHistoryIterator (ICS* ics, Ics_HistoryIterator* it, char const* key)
{
    return IcsNewHistoryIterator (ics, it, key);
}

Ics_Error DLL_CALLCONV
FreeImageIcs_IcsGetHistoryStringI (ICS* ics, Ics_HistoryIterator* it, char* string)
{
    return IcsGetHistoryStringI (ics, it, string);
}

int DLL_CALLCONV
FreeImageIcs_GetHistoryText(ICS *ics, char *text)
{
	Ics_HistoryIterator it;     
	char buffer[ICS_LINE_LENGTH], key[ICS_LINE_LENGTH], value[ICS_LINE_LENGTH];

	memset(text, 0, 1);

	if(FreeImageIcs_GetIcsHistoryStringCount(ics) <= 0)
		return FIA_ERROR;	

	if(IcsNewHistoryIterator (ics, &it, NULL) != IcsErr_Ok)
		return FIA_ERROR;	

	while (it.next >= 0) { 
	   
		memset(buffer, 0, 100);
	    memset(key, 0, 1);
		memset(value, 0, 100);

    	if(IcsGetHistoryKeyValueI (ics, &it, key, value) != IcsErr_Ok)	   // get next string, update iterator 
			continue;    
	 
		if(strcmp(key, "") == 0)
			sprintf(buffer, "%s\n", value);
		else
			sprintf(buffer, "%s: %s\n", key, value);
			
		strcat(text, buffer);
	}	

	strcat(text, "\0");

	return FIA_SUCCESS;
}


int DLL_CALLCONV
FreeImageIcs_GetHistoryTextFromFile(const char *filepath, char *text)
{
	ICS *ics;
	int ret;

	// Reset text
	memset(text, 0, 1);

	if(!FreeImageIcs_IsIcsFile (filepath))
		return FIA_ERROR;	

	IcsOpen(&ics, filepath, "r");

	ret = FreeImageIcs_GetHistoryText(ics, text);

	IcsClose(ics);

	return ret;
}


int DLL_CALLCONV
FreeImageIcs_CopyHistoryText(ICS *ics, ICS *dst_ics)
{
	Ics_HistoryIterator it;     
	char buffer[ICS_LINE_LENGTH];

	if(FreeImageIcs_GetIcsHistoryStringCount(ics) <= 0)
		return FIA_ERROR;	

	if(IcsNewHistoryIterator (ics, &it, NULL) != IcsErr_Ok)
		return FIA_ERROR;	

	while (it.next >= 0) { 
	  
    	if(IcsGetHistoryStringI (ics, &it, buffer) != IcsErr_Ok)	   // get next string, update iterator 
			continue;    
	 
		IcsAddHistoryString (dst_ics, NULL, buffer);
	}	

	return FIA_SUCCESS;
}

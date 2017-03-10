#!/usr/bin/env python
# -*- coding: utf-8 -*-

# FreeImageIcs wrapper inherits types from the FreeImage wrapper.
# Written by Glenn Pierce glennpierce@gmail.com

import sys, string
import ctypes as C

lib = None
clib = None
    
"""
Wrappers
"""
def FreeImageIcs_Initialise():
    """ Initilaise the runtime and setup the ref to the dll """
    global lib
    global clib
    lib = C.windll.LoadLibrary("FreeImageIcs")
    clib = C.cdll.LoadLibrary("FreeImageIcs")

def GetFunction(function_string):   
        return getattr(lib, "FreeImageIcs_" + function_string)
    
def FreeImageIcs_LoadFIBFromIcsFile(ics):
    """ Load FI from file """
    return lib.FreeImageIcs_LoadFIBFromIcsFile(ics)
    
def FreeImageIcs_LoadFIBFromIcsFilePath(filepath):
    GetFunction("LoadFIBFromIcsFilePath").restype = C.c_void_p
    return GetFunction("LoadFIBFromIcsFilePath")(filepath)   
        
def FreeImageIcs_IsIcsFile(filepath):
    return GetFunction("IsIcsFile")(filepath)   
        
def FreeImageIcs_NumberOfDimensions(ics):
    return GetFunction("NumberOfDimensions")(ics)   

def FreeImageIcs_SaveIcsFileWithDimensionsAs(ics, filepath, order, size):
    return GetFunction("SaveIcsFileWithDimensionsAs")(ics, filepath, C.byref(order), size)

def FreeImageIcs_SaveIcsFileWithFirstTwoDimensionsAs(ics, filepath, first, second):
    return GetFunction("SaveIcsFileWithFirstTwoDimensionsAs")(ics, filepath, first, second)

def FreeImageIcs_SumIntensityProjection(ics, dimension):
    GetFunction("SumIntensityProjection").restype = C.c_void_p
    return GetFunction("SumIntensityProjection")(ics, dimension)

def FreeImageIcs_MaximumIntensityProjection(ics, dimension):
    GetFunction("SumIntensityProjection").restype = C.c_void_p
    return GetFunction("MaximumIntensityProjection")(ics, dimension)

def FreeImageIcs_GetIcsHistoryStringCount(ics):
    return GetFunction("GetIcsHistoryStringCount")(ics)

def FreeImageIcs_IsIcsFileColourFile(ics):
    return GetFunction("IsIcsFileColourFile")(ics)   

def FreeImageIcs_GetIcsImageDimensionalDataSlice(ics, dimension, slice):
    GetFunction("GetIcsImageDimensionalDataSlice").restype = C.c_void_p
    return GetFunction("GetIcsImageDimensionalDataSlice")(ics, dimension, slice)   

def FreeImageIcs_SaveImage (dib, filepath, save_metadata):
    GetFunction("SaveImage") (dib, filepath, save_metadata)

def FreeImageIcs_IcsClose(ics):
    return GetFunction("IcsClose")(ics)

def FreeImageIcs_IcsAddHistoryString(ics, key, value):
    return GetFunction("IcsAddHistoryString")(ics, key, value)

def FreeImageIcs_IcsOpen (filepath, mode):
    ics = C.c_void_p()
    GetFunction("IcsOpen") (byref(ics), filepath, mode)
    return ics
    
def FreeImageIcs_GetLabelForDimension(ics, dimension):
    retval = C.create_string_buffer(50)
    GetFunction("GetLabelForDimension")(ics, dimension, retval)
    return retval.value

def FreeImageIcs_GetDimensionDetails(ics, dimension, order_array):
    retlabel = C.create_string_buffer(50)
    size = C.C_int()
    GetFunction("GetDimensionDetails")(ics, dimension, order_array, retlabel, byref(size))
    return (label, size)

def FreeImageIcs_IcsNewHistoryIterator(ics, key):  
    iterator = C.c_void_p()
    GetFunction("IcsNewHistoryIterator")(ics, byref(iterator), key)
    return iterator

def FreeImageIcs_IcsGetHistoryStringI(ics, iterator):
    retval = C.create_string_buffer(50)
    GetFunction("IcsGetHistoryStringI")(ics, iterator, retval)
    return retval.value

def FreeImageIcs_SplitHistoryString(history_string):
    return string.split(history_string, 0x09)

def FreeImageIcs_JoinKeyValueIntoHistoryString(key, value):
    return string.join((key, value), 0x09)


#DLL_API int DLL_CALLCONV
#FreeImageIcs_SetIcsHistoryStrings(ICS *ics, ...);

#DLL_API int DLL_CALLCONV
#FreeImageIcs_SetIcsHistoryKeyValueStrings(ICS *ics, ...);

#DLL_API int DLL_CALLCONV
#FreeImageIcs_AddIcsHistoryKeyValueStrings(ICS *ics, ...);

#DLL_API int DLL_CALLCONV
#FreeImageIcs_SetIcsHistoryKeyValueStringsFromArray(ICS *ics, char **history_strings, int number_of_items);

#DLL_API int DLL_CALLCONV
#FreeImageIcs_ReplaceIcsHistoryValueForKey(ICS *ics, char *key, char *value);

#DLL_API int DLL_CALLCONV
#FreeImageIcs_GetFirstIcsHistoryValueWithKey(ICS *ics, char *key, char *value);


#DLL_API int DLL_CALLCONV
#IsIcsHistoryIteratorValid (ICS* ics, Ics_HistoryIterator* it);


#DLL_API int DLL_CALLCONV
#FreeImageIcs_GetHistoryText(ICS *ics, char *text);


#DLL_API int DLL_CALLCONV
#FreeImageIcs_GetHistoryTextFromFile(const char *filepath, char *text);


#DLL_API int DLL_CALLCONV
#FreeImageIcs_CopyHistoryText(ICS *ics, ICS *dst_ics);
#!/usr/bin/env python
# -*- coding: utf-8 -*-

# FreeImageAlgorithms wrapper inherits types from the FreeImage wrapper.
# Written by Glenn Pierce glennpierce@gmail.com

import sys
import ctypes as C
from warnings import warn

#Load the constants
from FreeImagePy.constants import *
import FreeImagePy.FreeImagePy as FI


class FIAImage(FI.Image):
    
    """ 
    FreeImageAlgorithms class wrapper
    
    The method with the first line uppercase are internal library methods
    the other one are internal method for help the user
        
    @author: Glenn Pierce
    """
    
    def __init__(self, f=None):
        """ Initilaise the runtime and setup the ref to the dll """
        self.lib = C.windll.LoadLibrary("FreeImageAlgorithms")
        self.clib = C.cdll.LoadLibrary("FreeImageAlgorithms")
        
        super(FIAImage, self).__init__(f, None)
    
    def GetFunction(self, function_string):   
        return getattr(self.lib, "_FreeImageAlgorithms_" + function_string)
          
    def SetRainBowPalette(self):
        """ Set a rainbow palette for the bitmap.
        """
        #return self.lib._SetRainBowPalette@4(self.getBitmap())
        return self.GetFunction("SetRainBowPalette@4")(self.getBitmap())  
                              
    def GetHistogram(self, min, max, bins):
        "Get the histogram of a greylevel image"
        DW_array = DWORD * bins # type
        histo = DW_array()
            
        self.lib.Histogram(self.getBitmap(), C.c_double(min), C.c_double(max), bins, C.byref(histo))
 
        return [int(x) for x in histo]
    
    
    
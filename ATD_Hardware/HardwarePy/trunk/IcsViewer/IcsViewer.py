# Project       IcsViewerPy
# file name:    IcsViewer.py
# Written by glennpierce

# Wrapper functions for icsviewer

import os, string
import ctypes as C
from ctypes.util import find_library


class IcsViewer:
    
    def __init__(self, title=None, left=None, top=None, width=None, height=None, override=None, monitor=None):
              
        libPath = find_library("IcsViewer.dll") 

        self.lib = C.windll.LoadLibrary(libPath)
        
        # Set the resource path to the location of the dll
        #self.SetResourceSearchPath(r"C:\Documents and Settings\Pierce\Desktop\Working Area\\")
        
        self.lib.GCI_ImagingWindow_Create.restype = C.c_void_p
        self.__window = self.lib.GCI_ImagingWindow_Create(title)
   
        if self.__window == None:
            raise ValueError("IcsViewer could not be created.")

    #def SetResourceSearchPath(self, path): 
    #    return self.lib.GCI_ImagingWindow_SetResourceSearchPath(path)

    def Show(self):
        return self.lib.GCI_ImagingWindow_Show(self.__window)
    
    def BringToFront(self):
        return self.lib.GCI_ImagingWindow_BringToFront(self.__window)
    
    def Hide(self):
        return self.lib.GCI_ImagingWindow_Hide(self.__window)
    
    def Close(self):
        return self.lib.GCI_ImagingWindow_Close(self.__window)
    
    def LoadImage(self, dib):
        return self.lib.GCI_ImagingWindow_LoadImage(self.__window, dib)

    def LoadImageFile(self, filepath):
        return self.lib.GCI_ImagingWindow_LoadImageFile(self.__window, filepath)

    def SetLiveStatus(self, status):
        return self.lib.GCI_ImagingWindow_SetLiveStatus(self.__window, status)  

    def GetLiveStatus(self):
        return self.lib.GCI_ImagingWindow_GetLiveStatus(self.__window)

    def SetBinningSize(self, size):
        return self.lib.GCI_ImagingWindow_SetBinningSize(self.__window, size)

    def SetFalseColourWavelength(self, wavelength):
        return self.lib.GCI_ImageWindow_SetFalseColourWavelength(self.__window, wavelength)

    def GetPanelID(self):
        return self.lib.GCI_ImagingWindow_GetPanelID(self.__window)

    def GetOriginalFib(self):
        return self.lib.GCI_ImagingWindow_GetOriginalFIB(self.__window)

    def GetDisplayedFib(self):
        return self.lib.GCI_ImagingWindow_GetDisplayedFIB(self.__window)

    def SetPixelToMicronFactor(self, factor):
        return self.lib.GCI_ImagingWindow_SetMicronsPerPixelFactor(self.__window, factor)

    def GetPixelToMicronFactor(self):
        return self.lib.GCI_ImagingWindow_GetMicronsPerPixelFactor(self.__window)
    
    # Marshall a point
    #def PlaceCrossHair(self):
    #    return self.lib.GCI_ImagingWindow_PlaceCrossHair(self.__window, factor)

    def SetDefaultDirectoryPath(self, path):
        return self.lib.GCI_ImagingWindow_SetDefaultDirectoryPath(self.__window, path)

    def SetWindowTitle(self, title):
        return self.lib.GCI_ImagingWindow_SetWindowTitle(self.__window, title)

    def ShowToolBar(self):
        return self.lib.GCI_ImagingWindow_ShowToolBar(self.__window)
    
    def HideToolBar(self):
        return self.lib.GCI_ImagingWindow_HideToolBar(self.__window)

    def SetZoomFactor(self, zoom):
        return self.lib.GCI_ImagingWindow_SetZoomFactor(self.__window, zoom)

    def SetResizeFitStyle(self):
        return self.lib.GCI_ImagingWindow_SetResizeFitStyle(self.__window)
        
    def EnableProfile(self):
        return self.lib.GCI_ImagingWindow_EnableProfile(self.__window)
    
    def EnableCrossHair(self):
        return self.lib.GCI_ImagingWindow_EnableCrossHair(self.__window)
    
    def EnableZoomTool(self):
        return self.lib.GCI_ImagingWindow_EnableZoomTool(self.__window)

    def EnableRoiTool(self):
        return self.lib.GCI_ImagingWindow_EnableRoiTool(self.__window)

    def DisableProfile(self):
        return self.lib.GCI_ImagingWindow_DisableProfile(self.__window)
    
    def DisableCrossHair(self):
        return self.lib.GCI_ImagingWindow_DisableCrossHair(self.__window)
    
    def DisableZoomTool(self):
        return self.lib.GCI_ImagingWindow_DisableZoomTool(self.__window)

    def DisableRoiTool(self):
        return self.lib.GCI_ImagingWindow_DisableRoiTool(self.__window)

    def LockProfileButton(self):
        return self.lib.GCI_ImagingWindow_LockProfileButton(self.__window)

    def LockCrossHairButton(self):
        return self.lib.GCI_ImagingWindow_LockCrossHairButton(self.__window)
    
    def LockRoiButton(self):
        return self.lib.GCI_ImagingWindow_LockRoiButton(self.__window)
    
    def UnLockProfileButton(self):
        return self.lib.GCI_ImagingWindow_UnLockProfileButton(self.__window)

    def UnLockCrossHairButton(self):
        return self.lib.GCI_ImagingWindow_UnLockCrossHairButton(self.__window)
    
    def UnLockRoiButton(self):
        return self.lib.GCI_ImagingWindow_UnLockRoiButton(self.__window)

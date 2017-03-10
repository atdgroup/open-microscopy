# Project       ImageViewerPy
# file name:    ImageViewer.py
# Written by glennpierce

# Wrapper functions for imageviewer

import ctypes as C
from ctypes.util import find_library

class ImageViewer:
    
    def __init__(self):     
        libPath = find_library("ImageViewer.dll")  
        self.lib = C.windll.LoadLibrary(libPath)
        
    def BasicWin32Window(self, title, left, top, width, height, dib):
        return self.lib.BasicWin32Window(title, left, top, width, height, dib);

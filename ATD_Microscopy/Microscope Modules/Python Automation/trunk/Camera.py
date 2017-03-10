import microscope
import FreeImagePy.FreeImagePy as FI
import FreeImageAlgorithmsPy.FreeImageAlgorithmsPy as FIA
import ctypes as C

from ctypes.util import find_library

class Camera:

  libName = find_library("ImageViewer.dll")
  lib = getattr(C.windll, libName)
    
  def __init__(self):
    pass

  def SnapImage(self):
    microscope.SnapImage()
    
  def SetExposure(self, exposure):
    microscope.SetExposure(exposure)
    
  def GetImage(self):    
    bitmap = microscope.GetImage()
    C.pythonapi.PyCObject_AsVoidPtr.restype = C.c_void_p
    C.pythonapi.PyCObject_AsVoidPtr.argtypes = [ C.py_object ]
    void_ptr = C.pythonapi.PyCObject_AsVoidPtr(bitmap)
    F = FIA.FIAImage()  
    tmp = F.Clone(void_ptr)
    F.loadFromBitmap(tmp)
    del tmp
    return F
    
  def SetLiveMode(self):
    microscope.SetCameraLiveMode()
    
  def DisplayBasicWindow(self, title, left, top, width, height, dib):
    bitmap = dib.getBitmap()
    Camera.lib.BasicWin32Window(title, left, top, width, height, bitmap)
    del bitmap
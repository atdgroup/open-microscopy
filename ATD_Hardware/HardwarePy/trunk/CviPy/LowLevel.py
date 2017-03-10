# Project       CviPy
# file name:    LowLevel.py

# Lowlevel wrapper functions that use ctypes

import os, string
import ctypes as C
from constants import *

"""
Custom exceptions
"""
class CviPy_Exception(Exception):
    """ 
        Exception raised then I don't found the library
    """
    def __init__(self, value):
        self.value = value
        
    def __str__(self):
        return repr(self.value)
    
class CviPy_ErrorCodeException(Exception):
    """ 
        Exception raised then I don't found the library
    """
    def __init__(self, code):
        self.code = code
        self.value = CviErros[code]
        
    def __str__(self):
        return repr(self.value)

lib = None
clib = None
    
"""
Wrappers
"""

def Initialise():
    """ Initilaise the runtime and setup the ref to the dll """
    global lib
    global clib
    lib = C.windll.LoadLibrary(r"C:\WINDOWS\system32\cvirt.dll")
    clib = C.cdll.LoadLibrary(r"C:\WINDOWS\system32\cvirt.dll")

def CheckValidResult(retval, function, arguments):
    """ Checks the reulsts from the cvi functions and if < 0 raises exception """
    if retval < 0:
         raise CviPy_ErrorCodeException(retval)
        
    return retval

def RunUserInterface():
    """ Load a uir file """
    return lib.RunUserInterface()

def QuitUserInterface(returnCode):
    """ Quit the user interface """
    return lib.QuitUserInterface(returnCode)

def ProcessSystemEvents():
    return lib.ProcessSystemEvents()

def LoadPanel(parentPanel, fileName, panelResourceId):
    """ Load a uir file """
    # if fileName is a full pathh just use that if it is a relative filename
    # Look in the py file directory

    if os.path.isabs(fileName):
        return lib.LoadPanel(parentPanel, fileName, panelResourceId)    

    ## Attempt to find relative uir file - Does not work if run from ide
    py_file_dir = os.path.split(os.path.abspath(__file__))[0]

    lib.LoadPanel.errcheck = CheckValidResult 
    return lib.LoadPanel(parentPanel, os.path.join(py_file_dir, fileName), panelResourceId)

def DisplayPanel(panel):
    """ Display a uir file """
    lib.DisplayPanel.errcheck = CheckValidResult 
    return lib.DisplayPanel(panel)

def HidePanel(panel):
    """ Hide a panel """
    lib.HidePanel.errcheck = CheckValidResult 
    return lib.HidePanel(panel)

def DiscardPanel(panel):
    """ Discards a panel """
    lib.DiscardPanel.errcheck = CheckValidResult 
    return lib.DiscardPanel(panel)

def GetPanelMenuBar (panel):
    """ Get panels menubar """
    lib.GetPanelMenuBar.errcheck = CheckValidResult 
    return lib.GetPanelMenuBar(panel)
    
def NewMenu (menubar_id, name,  before_menuid):
    """ Create a new menu """
    lib.NewMenu.errcheck = CheckValidResult 
    return lib.NewMenu (menubar_id, name, before_menuid);
    
def NewMenuItem (menubar_id, menu_id, item_name, before_menuid, shortcut_key, eventFunction, callback_data):
    """ Create a NewMenuItem """
    lib.NewMenuItem.errcheck = CheckValidResult 
    return lib.NewMenuItem (menubar_id, menu_id, item_name, before_menuid, shortcut_key, eventFunction, callback_data)
    
def GetPanelAttribute (panel, attribute):
    """ Gets a panel atrribute """

    if not CviAttributes.has_key(attribute):
        raise CviPy_NosuchAttribute("The CVI attribute \"%s\" does not exist" % attribute)
   
    # Use a return val of the correct type (depends upon the attribute)
    attr_val = CviAttributes[attribute][0]
    ret_val = CviAttributes[attribute][1]()
    lib.GetPanelAttribute.errcheck = CheckValidResult 
    lib.GetPanelAttribute(panel, attr_val, C.byref(ret_val))
    return ret_val.value

def SetPanelAttribute (panel, attribute, value):
    """ Gets a panel atrribute """

    if not CviAttributes.has_key(attribute):
        raise CviPy_NosuchAttribute("The CVI attribute \"%s\" does not exist" % attribute)

    attr_val = CviAttributes[attribute][0]
    val_type = CviAttributes[attribute][1]
    set_val = val_type()
    set_val.value = value

    clib.SetPanelAttribute.errcheck = CheckValidResult 
    clib.SetPanelAttribute.argtypes = (C.c_int, C.c_int, val_type)
    return clib.SetPanelAttribute (panel, attr_val, value)

def GetCtrlDataType(panel, ctrl):
    # We has a attribute that return a variale type
    # Ie can be int, float, double, string etc
    # We must find out which
    cvi_dt = C.c_int()
    lib.GetCtrlAttribute.errcheck = CheckValidResult 
    lib.GetCtrlAttribute(panel, ctrl, ATTR_DATA_TYPE, C.byref(cvi_dt))
    val_type = CviTypeToCType(cvi_dt.value)    
    return val_type

def GetCtrlAttribute (panel, ctrl, attribute):
    """ Gets a panel atrribute """

    if not CviAttributes.has_key(attribute):
        raise CviPy_NosuchAttribute("The CVI attribute \"%s\" does not exist" % attribute)

    # Use a return val of the correct type (depends upon the attribute)
    val_type = CviAttributes[attribute][1]
        
    # Handles the case of ATTR_VAL
    if val_type == "Variable":
        val_type = GetCtrlDataType(panel, ctrl)
  
    # Get the numerical attribute value
    attr_val = CviAttributes[attribute][0]
        
    lib.GetCtrlAttribute.errcheck = CheckValidResult 
        
    if val_type == C.c_char_p: # We want to get a return param string
        ret_val = C.create_string_buffer(50)  
        lib.GetCtrlAttribute(panel, ctrl, attr_val, ret_val)
        return ret_val.value
    else:
        ret_val = val_type()
        lib.GetCtrlAttribute(panel, ctrl, attr_val, C.byref(ret_val))
        return ret_val.value
        
    
def SetCtrlAttribute (panel, ctrl, attribute, value):
    """ Gets a panel atrribute """

    if not CviAttributes.has_key(attribute):
        raise CviPy_NosuchAttribute("The CVI attribute \"%s\" does not exist" % attribute)

    val_type = CviAttributes[attribute][1]

    # Handles the case of ATTR_VAL
    if val_type == "Variable":
        val_type = GetCtrlDataType(panel, ctrl)

    # Use a val of the correct type (depends upon the attribute
    attr_val = CviAttributes[attribute][0]
        
    set_val = val_type()
    set_val.value = value

    clib.SetCtrlAttribute.errcheck = CheckValidResult 
    clib.SetCtrlAttribute.argtypes = (C.c_int, C.c_int, C.c_int, val_type)
    return clib.SetCtrlAttribute (panel, ctrl, attr_val, set_val)
    
    
def GetMenuBarAttribute (menubar_id, menu_or_menuitem_id, attribute):
    """ Gets a menu atrribute """

    if not CviAttributes.has_key(attribute):
        raise CviPy_NosuchAttribute("The CVI attribute \"%s\" does not exist" % attribute)

    # Use a return val of the correct type (depends upon the attribute)
    val_type = CviAttributes[attribute][1]
        
    # Handles the case of ATTR_VAL
    if val_type == "Variable":
        val_type = GetCtrlDataType(panel, ctrl)
  
    # Get the numerical attribute value
    attr_val = CviAttributes[attribute][0]
        
    lib.GetMenuBarAttribute.errcheck = CheckValidResult 
        
    if val_type == C.c_char_p: # We want to get a return param string
        ret_val = C.create_string_buffer(50)  
        lib.GetMenuBarAttribute(menubar_id, menu_or_menuitem_id, attr_val, ret_val)
        return ret_val.value
    else:
        ret_val = val_type()
        lib.GetMenuBarAttribute(menubar_id, menu_or_menuitem_id, attr_val, C.byref(ret_val))
        return ret_val.value

def SetMenuBarAttribute (menubar_id, menu_or_menuitem_id, attribute, value):
    """ Gets a panel atrribute """

    if not CviAttributes.has_key(attribute):
        raise CviPy_NosuchAttribute("The CVI attribute \"%s\" does not exist" % attribute)

    val_type = CviAttributes[attribute][1]

    # Handles the case of ATTR_VAL
    if val_type == "Variable":
        val_type = GetCtrlDataType(panel, ctrl)

    # Use a val of the correct type (depends upon the attribute
    attr_val = CviAttributes[attribute][0]
        
    set_val = val_type()
    set_val.value = value

    clib.SetMenuBarAttribute.errcheck = CheckValidResult 
    clib.SetMenuBarAttribute.argtypes = (C.c_int, C.c_int, C.c_int, val_type)
    return clib.SetMenuBarAttribute (menubar_id, menu_or_menuitem_id, attr_val, set_val)
    

def MessagePopup (title, message):
    """ Pops up a dialog """
    return lib.MessagePopup(title, message)

def InstallPanelCallback (panel, eventFunction, callbackData):
    """ Installs a callback for panel events """
    lib.InstallPanelCallback.errcheck = CheckValidResult 
    return lib.InstallPanelCallback(panel, eventFunction, callbackData)  

def InstallCtrlCallback (panel, control, eventFunction, callbackData):
    """ Installs a callback for control events """
    lib.InstallCtrlCallback.errcheck = CheckValidResult 
    return lib.InstallCtrlCallback(panel, control, eventFunction, callbackData)  

def InstallMenuCallback (menubar_id, menuitem_id, eventFunction, callbackData):
    """ Installs a callback for menuitem """
    lib.InstallMenuCallback.errcheck = CheckValidResult 
    return lib.InstallMenuCallback(menubar_id, menuitem_id, eventFunction, callbackData)  

def NewPanel (parent, title, top, left, height, width):
    """ Create a new panel """
    lib.NewPanel.errcheck = CheckValidResult 
    return lib.NewPanel(parent, title, top, left, height, width)  
    
def NewCtrl(panel, type, label, top, left):
    lib.NewCtrl.errcheck = CheckValidResult
    return lib.NewCtrl(panel, CTRL_TIMER, label, top, left)
import sys
import gci
import ctypes as C

def CheckValidResult(retval, function, arguments):
    """ Checks the reulsts from the cvi functions and if < 0 raises exception """
    if retval < 0:
         raise CviException(retval)
        
    return retval
    
ATTR_LABEL_TEXT = 641
ATTR_TITLE = 573
ATTR_CTRL_VAL = 630
ATTR_DATA_TYPE = 710
ATTR_NEXT_PANEL = 609
ATTR_NUM_CTRLS = 582
ATTR_NUM_CHILDREN = 607
ATTR_FIRST_CHILD = 608
ATTR_NEXT_PANEL = 609
ATTR_MIN_VALUE = 731
ATTR_MAX_VALUE = 730
ATTR_VISIBLE = 530

lib = C.windll.LoadLibrary(r"C:\WINDOWS\system32\cvirt.dll")
clib = C.cdll.LoadLibrary(r"C:\WINDOWS\system32\cvirt.dll")

# Disable CVI Runtime checking
lib.SetBreakOnLibraryErrors (0);
   
# CVI ERRORS

CviErrors = { 0 : "Success",
            -1 : "The Interface Manager could not be opened.",
            -2 : "The system font could not be loaded.",
            -3 : "The operation attempted cannot be performed while a pop-up menu is active.",
            -4 : "Panel, pop-up, menu bar, or plot ID is invalid.",
            -5 : "Attempted to position panel at an invalid location ",
            -6 : "Attempted to make an inoperable control the active control.",
            -7 : "The operation requires that a panel be loaded.",
            -8 : "The operation requires that a pop-up menu be active.",
            -9 : "The operation requires that a menu bar be loaded.",
            -10 : "The control is not the type expected by the function.",
            -11 : "Invalid menu item ID.",
            -12 : "Out of memory! ",
            -13 : "Invalid control ID.",
            -14 : "Value is invalid or out of range.",
            -15 : "File is not a User Interface file or has been corrupted.",
            -16 : "File format is out-of-date.",
            -17 : "PCX image is corrupted or incompatible with current display type.",
            -18 : "No user event possible in current configuration.",
            -19 : "Unable to open UIR file.",
            -20 : "Error reading UIR file.",
            -21 : "Error writing UIR file.",
            -22 : "Error closing UIR file.",
            -23 : "Panel state file has invalid format.",
            -24 : "Invalid panel ID or menu bar ID in resource file.",
            -25 : "Error occurred during hardcopy output.",
            -26 : "Invalid default directory specified in FileSelectPopup function.",
            -27 : "Operation is invalid for specified object.",
            -28 : "Unable to find specified string in menu.",
            -29 : "Palette menu items can only be added to the end of the menu.",
            -30 : "Too many menus in the menu bar.",
            -31 : "Separators cannot have checkmarks.",
            -32 : "Separators cannot have submenus.",
            -33 : "The menu item must be a separator.",
            -34 : "The menu item cannot be a separator.",
            -35 : "The menu item already has a submenu.",
            -36 : "The menu item does not have a submenu.",
            -37 : "The control ID passed must be a menu ID, a menu item ID, or NULL.",
            -38 : "The control ID passed must be a menu ID, or a menu item ID.",
            -39 : "The control ID passed was not a submenu ID.",
            -40 : "The control ID passed was not a valid ID.",
            -41 : "The ID is not a menu bar ID.",
            -42 : "The ID is not a panel ID.",
            -43 : "This operation cannot be performed while this pop-up panel is active.",
            -44 : "This control/panel/menu does not have the specified attribute.",
            -45 : "The control type passed was not a valid type.",
            -46 : "The attribute passed is invalid.",
            -47 : "The fill option must be set to fill above or fill below to paint ring slide's fill color.",
            -48 : "The fill option must be set to fill above or fill below to paint numeric slide's fill color.",
            -49 : "The control passed is not a ring slide.",
            -50 : "The control passed is not a numeric slide.",
            -51 : "The control passed is not a ring slide with inc/dec arrows.",
            -52 : "The control passed is not a numeric slide with inc/dec arrows.",
            -53 : "The data type passed in is not a valid data type for the control.",
            -54 : "The attribute passed is not valid for the data type of the control.",
            -55 : "The index passed is out of range.",
            -56 : "There are no items in the list control.",
            -57 : "The buffer passed was to small for the operation.",
            -58 : "The control does not have a value.",
            -59 : "The value passed is not in the list control.",
            -60 : "The control passed must be a list control.",
            -61 : "The control passed must be a list control or a binary switch.",
            -62 : "The data type of the control passed must be set to a string.",
            -63 : "That attribute is not a settable attribute.",
            -64 : "The value passed is not a valid mode for this control.",
            -65 : "A NULL pointer was passed when a non-NULL pointer was expected.",
            -66 : "The text background color on a menu ring cannot be set or gotten.",
            -67 : "The ring control passed must be one of the menu ring styles.",
            -68 : "Text cannot be colored transparent.",
            -69 : "A value cannot be converted to the specified data type.",
            -70 : "Invalid tab order position for control.",
            -71 : "The tab order position of an indicator-only control cannot be set.",
            -72 : "Invalid number.",
            -73 : "There is no menu bar installed for the panel.",
            -74 : "The control passed is not a text box.",
            -75 : "Invalid scroll mode for chart.",
            -76 : "Invalid image type for picture.",
            -77 : "The attribute is valid for child panels only. Some attributes of top level panels are determined by the host operating system.",
            -78 : "The list control passed is not in check mode.",
            -79 : "The control values could not be completely loaded into the panel because the panel has changed.",
            -80 : "Maximum value must be greater than minimum value.",
            -81 : "Graph does not have that many cursors.",
            -82 : "Invalid plot.",
            -83 : "New cursor position is outside plot area.",
            -84 : "The length of the string exceeds the limit.",
            -85 : "The specified callback function does not have the required prototype.",
            -86 : "The specified callback function is not a known function. For external compilers, the UIR callbacks object file cannot be in the executable or DLL.",
            -87 : "Graph cannot be in this mode without cursors.",
            -88 : "Invalid axis scaling mode for chart.",
            -89 : "The font passed is not in font table.",
            -90 : "The attribute value passed is not valid.",
            -91 : "Too many files are open.",
            -92 : "Unexpectedly reached end of file.",
            -93 : "Input/Output error.",
            -94 : "File not found.",
            -95 : "File access permission denied.",
            -96 : "File access is not enabled.",
            -97 : "Disk is full.",
            -98 : "File already exists.",
            -99 : "File already open.",
            -100 : "Badly formed pathname.",
            -101 : "File is damaged.",
            -102 : "The format of the resource file is too old to read.",
            -103 : "File is corrupted.",
            -104 : "The operation could not be performed.",
            -105 : "The control passed is not a ring knob, dial, or gauge.",
            -106 : "The control passed is not a numeric knob, dial, or gauge.",
            -107 : "The count passed is out of range.",
            -108 : "The keycode is not valid.",
            -109 : "The control passed is not a ring slide with a frame.",
            -110 : "Panel background cannot be colored transparent.",
            -111 : "Title background cannot be colored transparent.",
            -112 : "Not enough memory for printing.",
            -113 : "The shortcut key passed is reserved.",
            -114 : "The format of the file is newer than this version of CVI.",
            -115 : "System printing error.",
            -116 : "Driver printing error.",
            -117 : "The deferred callback queue is full.",
            -118 : "The mouse cursor passed is invalid.",
            -119 : "Printing functions are not reentrant.",
            -120 : "Out of Windows GDI space.",
            -121 : "The panel must be visible.",
            -122 : "The control must be visible.",
            -123 : "The attribute not valid for the type of plot.",
            -124 : "Intensity plots cannot use transparent colors.",
            -125 : "Color is invalid.",
            -126 : "The specified callback function differs only by a leading underscore from another function or variable. Change one of the names for proper linking.",
            -127 : "Bitmap is invalid.",
            -128 : "There is no image in the control.",
            -129 : "The specified operation can be performed only in the thread in which the top-level panel was created.",
            -130 : "The specified panel was not found in the .tui file.",
            -131 : "The specified menu bar was not found in the .tui file.",
            -132 : "The specified control style was not found in the .tui file.",
            -133 : "A tag or value is missing in the .tui file.",
            -134 : "Error reading or parsing .sub file.",
            -135 : "There are no printers installed in the system.",
            -136 : "The beginning cell must be in the search range.",
            -137 : "The cell type passed is not valid for this operation ",
            -138 : "Cell type or data type is mismatched.",
            -139 : "Controls of the type passed do not have a menu.",
            -142 : "You must pass your callback function's eventData2 parameter to this function.",
            -143 : "ActiveX error.",
            -144 : "The specified object handle does not refer to an ActiveX control.",
            -145 : "ActiveX control not registered on this computer.",
            -146 : "ActiveX control does not support persistence.",
            -147 : "The id passed was not a valid menu button id.",
            -148 : "Cannot set or get the attributes of built-in control menu items.",
            -149 : "DataSocket Error.",
            -150 : "Control already has an active data binding.",
            -151 : "Control must have an active data binding.",
            -152 : "The panel to be attached must be a direct child of the panel containing the splitter control.",
            -153 : "Item is already attached to splitter control.",
            -154 : "Item is not attached to splitter control.",
            -155 : "Attached control cannot be sized in this direction.",
            -156 : "Splitter control cannot be attached to itself.",
            -157 : "Operation cannot be performed on a bitmap with a transparency mask.",
            -158 : "Operation cannot be performed on a bitmap with an alpha channel.",
            -159 : "Operation can be performed only on a bitmap with a transparency mask.",
            -160 : "Operation can be performed only on a bitmap with an alpha channel.",
            -161 : "Graph does not have that many annotations.",
            -162 : "Operation cannot be performed on a tab panel.",
            -163 : "The attribute passed is only valid for menu bars.",
            -164 : "The attribute passed is only valid for menu items.",
            -165 : "The attribute passed is only valid for menus and submenus."
            }
     
# Events

class Events:

    EVENT_NONE                       = 0
    EVENT_COMMIT                     = 1   
    EVENT_VAL_CHANGED                = 2 
    EVENT_LEFT_CLICK                 = 3   
    EVENT_LEFT_DOUBLE_CLICK          = 4
    EVENT_KEYPRESS                   = 7   


# Data Types

class DataTypes:

    VAL_CHAR                         = 0
    VAL_INTEGER                      = 1
    VAL_SHORT_INTEGER                = 2
    VAL_FLOAT                        = 3
    VAL_DOUBLE                       = 4
    VAL_STRING                       = 5
    VAL_UNSIGNED_SHORT_INTEGER       = 6
    VAL_UNSIGNED_INTEGER             = 7
    VAL_UNSIGNED_CHAR                = 8
    VAL_NO_TYPE                      = 9    


ctype_types = [C.c_char,
               C.c_int,
               C.c_short,
               C.c_float,
               C.c_double,
               C.c_char_p,
               C.c_ushort,
               C.c_uint,
               C.c_ubyte]

def CviTypeToCType(cvi_dt):
    return ctype_types[cvi_dt]
    
"""
Custom exceptions
"""
class CviException(Exception):
    """ 
        Exception raised then I don't found the library
    """
    def __init__(self, code):
        self.code = code
        self.value = CviErrors[code]
        
    def __str__(self):
        return repr(self.value)


def GetControlAttributeValue (panel_id, ctrl_id, attribute_val):
    """ Gets a controls value """

    val_type = GetCtrlDataType(panel_id, ctrl_id)
    
    lib.GetCtrlAttribute.errcheck = CheckValidResult 
        
    if val_type == C.c_char_p: # We want to get a return param string
        ret_val = C.create_string_buffer(256)  
        lib.GetCtrlAttribute(panel_id, ctrl_id, attribute_val, ret_val)
        return ret_val.value
    else:
        ret_val = val_type()
        lib.GetCtrlAttribute(panel_id, ctrl_id, attribute_val, C.byref(ret_val))
        return ret_val.value
        
        
def SetControlIndex(panel_id, ctrl_id, index):
    lib.SetCtrlIndex.errcheck = CheckValidResult
    return lib.SetCtrlIndex(panel_id, ctrl_id, index)
        
def SetControlVal (panel_id, ctrl_id, value):
    """ Sets a contols value """    
    val_type = GetCtrlDataType(panel_id, ctrl_id)
    set_val = val_type()
    set_val.value = value

    clib.SetCtrlVal.errcheck = CheckValidResult 
    clib.SetCtrlVal.argtypes = (C.c_int, C.c_int, val_type)
    return clib.SetCtrlVal (panel_id, ctrl_id, set_val)
            

def GetControlVal (panel_id, ctrl_id, control_label):
    """ Gets a controls value """
    return GetControlAttributeValue (panel_id, ctrl_id, ATTR_CTRL_VAL)
        
        
def GetLabelFromIndex(panel_id, ctrl_id, index):
    ret_val = C.create_string_buffer(256)  
    lib.GetLabelFromIndex.errcheck = CheckValidResult 
    lib.GetLabelFromIndex(panel_id, ctrl_id, index, C.byref(ret_val))
    return ret_val.value      
 
def GetRowsInList(panel_id, ctrl_id):
    ret_val = C.c_int()  
    lib.GetNumListItems.errcheck = CheckValidResult 
    lib.GetNumListItems(panel_id, ctrl_id, C.byref(ret_val))
    return ret_val.value    
 
def SetFocus(panel_id):
#    return lib.SetActivePanel (panel_id)
    return gci.set_focus(panel_id)

def GetNumberOfControlsOnPanel(panel_id):
    int_obj = C.c_int()
    lib.GetPanelAttribute.errcheck = CheckValidResult 
    lib.GetPanelAttribute(panel_id, ATTR_NUM_CTRLS, C.byref(int_obj))
    return int_obj.value    

def GetNumberOfChildPanels(panel_id):
    int_obj = C.c_int()
    lib.GetPanelAttribute.errcheck = CheckValidResult 
    lib.GetPanelAttribute(panel_id, ATTR_NUM_CHILDREN, C.byref(int_obj))
    return int_obj.value  

def GetMenubarId(panel_id):
    lib.GetPanelMenuBar.errcheck = CheckValidResult 
    return lib.GetPanelMenuBar (panel_id)
        
def GetFirstChildPanel(panel_id):
    int_obj = C.c_int()
    lib.GetPanelAttribute.errcheck = CheckValidResult 
    lib.GetPanelAttribute(panel_id, ATTR_FIRST_CHILD, C.byref(int_obj))
    return int_obj.value     

def GetNextChildPanel(child_panel_id):
    int_obj = C.c_int()
    lib.GetPanelAttribute.errcheck = CheckValidResult 
    lib.GetPanelAttribute(child_panel_id, ATTR_NEXT_PANEL, C.byref(int_obj))
    return int_obj.value          
    
def GetControlLabel (panel_id, control):
    """ Gets a panel atrribute """
    ret_val = C.create_string_buffer(256)  
    lib.GetCtrlAttribute.errcheck = CheckValidResult 
    lib.GetCtrlAttribute(panel_id, control, ATTR_LABEL_TEXT, ret_val)
        
    val = ret_val.value
       
    if val[0:2] == "__": # Remove __ from labels with underscore
        return val[2:].lower()
          
    return val.lower()
    
def GetCallbackData(panel_id, control):
    void_ptr_obj = C.c_void_p()
    lib.GetPanelAttribute.errcheck = CheckValidResult 
    lib.GetPanelAttribute(panel_id, ATTR_NEXT_PANEL, C.byref(void_ptr_obj))
    return void_ptr_obj.value      
   
def CallControlCallback(panel_id, control, event, eventdata1, eventdata2):
    return gci.callcallback(panel_id, control, event, eventdata1, eventdata2)
    
def GetNumericMinMax(panel_id, control):
        min = GetControlAttributeValue (panel_id, control, ATTR_MIN_VALUE)
        max = GetControlAttributeValue (panel_id, control, ATTR_MAX_VALUE)
        return (min, max)
        
        
def GetCtrlDataType(panel_id, ctrl):
    # We have an attribute that returns a value of unknown type
    # It can be int, float, double, string etc
    # We must find out which
    cvi_dt = C.c_int()
    lib.GetCtrlAttribute.errcheck = CheckValidResult 
    lib.GetCtrlAttribute(panel_id, ctrl, ATTR_DATA_TYPE, C.byref(cvi_dt))
    val_type = CviTypeToCType(cvi_dt.value)    
    return val_type
        
        
def SelectMenuItem (panel_id, menu_string):
    """ Selects a menuitem from a string. The string can be something like "File\\Open" """    
    menubar_id = GetMenubarId(panel_id)
    menuitem_id = gci.get_menu_id_from_path(menubar_id, menu_string)
    gci.call_menu_callback(panel_id, menuitem_id)
    
    
def GetPanelTitle (panel_id):
    """ Gets a panel atrribute """
    ret_val = C.create_string_buffer(256)  
    lib.GetPanelAttribute.errcheck = CheckValidResult 
    lib.GetPanelAttribute(panel_id, ATTR_TITLE, ret_val)  
    return ret_val.value.lower()

def MessagePopup (title, msg):
    """Displays a popup window and waits for confirmation from user."""
    return lib.MessagePopup(title, msg)

def ConfirmPopup (title, msg):
    """Displays a popup window and waits for yes(1)/no(0) confirmation from user."""
    return lib.ConfirmPopup(title, msg)

def FakeKeystroke(key_value):
    """ Fakes a keystroke"""
    lib.FakeKeystroke(key_value)
    
def PanelVisible(panel_id):
    """ Returns 1 if the panel is visible, 0 if hidden"""
    int_obj = C.c_int()
#    lib.GetPanelAttribute.errcheck = CheckValidResult 
    lib.GetPanelAttribute(panel_id, ATTR_VISIBLE, C.byref(int_obj))
    return int_obj.value          

    
#def GetPanelList():
    
#    panels = [1]
#    int_obj = C.c_int()
#    lib.GetPanelAttribute.errcheck = CheckValidResult 
         
#    next_panel = lib.GetPanelAttribute(1, ATTR_NEXT_PANEL, C.byref(int_obj))
        
#    while next_panel != 0:
#        panels.append(next_panel);
#        next_panel = lib.GetPanelAttribute(next_panel, ATTR_NEXT_PANEL, C.byref(int_obj))
 
#    return panels
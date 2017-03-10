#!/usr/bin/env python
import os, string
from UserDict import UserDict
from LowLevel import *
from constants import *
     
class PanelAttributes(UserDict):

    def __init__(self, panel):
        self.__panel_id = panel
        UserDict.__init__(self)
    
    def __getitem__(self, name):
        """ Allow easily getting of CVI panel attributes """   
        return GetPanelAttribute(self.__panel_id, name)
        
    def __setitem__(self, name, value):
        """ Allow easily setting of CVI panel attributes """
        return SetPanelAttribute (self.__panel_id, name, value)


class ControlAttributes(UserDict):

    def __init__(self, panel, ctrl):
        self.__panel_id = panel
        self.__ctrl_id = ctrl
        UserDict.__init__(self)
    
    def __getitem__(self, name):
        """ Allow easily getting of CVI panel attributes """
        return GetCtrlAttribute(self.__panel_id, self.__ctrl_id, name)
      
    def __setitem__(self, name, value):
        """ Allow easily setting of CVI panel attributes """
        return SetCtrlAttribute(self.__panel_id, self.__ctrl_id, name, value)


class MenuAttributes(UserDict):

    def __init__(self, menubar_id, menu_or_menuitem_id):
        self.__menubar_id = menubar_id
        self.__menu_or_menuitem_id = menu_or_menuitem_id
        UserDict.__init__(self)
    
    def __getitem__(self, name):
        """ Allow easily getting of CVI panel attributes """
        return GetMenuBarAttribute(self.__menubar_id, self.__menu_or_menuitem_id, name)
      
    def __setitem__(self, name, value):
        """ Allow easily setting of CVI panel attributes """
        return SetMenuBarAttribute(self.__menubar_id, self.__menu_or_menuitem_id, name, value)
    
    
    
class Control(object):

    # UIR_CALLBACK_FUNC is a class instance as it has to be prevented from being garbage collected
    # ctypes soes not hold references. See the ctypes tutorial
    # int return
    # int panelHandle
    # int controlID
    # int event
    # void *callbackData
    # int eventdata1
    # int eventdata2
    UIR_CALLBACK_FUNC = C.CFUNCTYPE(C.c_int, C.c_int, C.c_int, C.c_int, C.c_void_p, C.c_int, C.c_int)

    def __init__(self, panel, ctrl):
        self.panel_id = panel
        self.ctrl_id = ctrl
        self.callback_data = None
        self.callback_function = None
        self.Attributes = ControlAttributes(self.panel_id, self.ctrl_id)

    def on_ctrl_callback(self, panel, control, event, callback_data, event_data1, event_data2):
        # Get the object from the panel and control id's
        return self.callback_function(self, self.panel_id, event, self.callback_data, event_data1, event_data2)
 
    def InstallCtrlCallback (self, eventFunction, callbackData):
        """ Install a callback function """
        callback = Control.UIR_CALLBACK_FUNC(self.on_ctrl_callback)
        self.callback_data = callbackData
        self.callback_function = eventFunction
        InstallCtrlCallback (self.panel_id, self.ctrl_id, callback, None)

    def GetValue(self):
        return self.Attributes['ATTR_CTRL_VAL']
    
    def SetValue(self, value):
        self.Attributes['ATTR_CTRL_VAL'] = value
     
     


class MenuBase(object):
    
    def __init__(self, menubar_id, menu_or_menuitem_id):
        self.Attributes = MenuAttributes(menubar_id, menu_or_menuitem_id)


class MenuItem(MenuBase):
    
    # MENU_CALLBACK_FUNC is a class instance as it has to be prevented from being garbage collected
    # ctypes soes not hold references. See the ctypes tutorial
    # void CVICALLBACK EventFunctionName (int menuBarHandle, int menuItemID, void *callbackPtr, int panelHandle);
    # return void
    # int menBarHandle
    # int menuItemID
    # void *callbackData
    # int panelHandle
    MENU_CALLBACK_FUNC = C.CFUNCTYPE(C.c_void_p, C.c_int, C.c_int, C.c_void_p, C.c_int)

    def __init__(self, menubar_id, menu_id, menuitem_id):
        self.menubar_id = menubar_id
        self.menu_id = menu_id
        self.menuitem_id = menuitem_id
        self.callback_data = None
        self.callback_function = None
        MenuBase.__init__(self, self.menubar_id, self.menuitem_id)
        
    def on_menu_callback(self, menubar_id, menuitem_id, callback_data, panel_id):
        # Get the object from the panel and control id's
        return self.callback_function(self, self.callback_data)
 
    def InstallMenuCallback (self, eventFunction, callbackData):
        callback = MenuItem.MENU_CALLBACK_FUNC(self.on_menu_callback)
        self.callback_data = callbackData
        self.callback_function = eventFunction
        return InstallMenuCallback (self.menubar_id, self.menuitem_id, callback, None)
        

class Menu(MenuBase):
    
    def __init__(self, menubar_id, menu_id):
        self.menubar_id = menubar_id
        self.menu_id = menu_id
        self.menu_items = {}
        MenuBase.__init__(self, self.menubar_id, self.menu_id)
        
    def NewMenuItem(self, name, before_menuid, shortcut_key=0):
        id = NewMenuItem (self.menubar_id, self.menu_id, name, before_menuid, shortcut_key, None, None)
        self.menu_items[name] = MenuItem(self.menubar_id, self.menu_id, id)
        return self.menu_items[name]


class Menubar(object):
    
    def __init__(self, panel_id):
        self.menubar_id = GetPanelMenuBar(panel_id)
        self.menus = {}
                                    
        if self.menubar_id == 0:
            raise CviPy_Exception("Panel does not has menubar")

    def NewMenu(self, name, before_menuid):
        id = NewMenu (self.menubar_id, name, before_menuid);
        self.menus[name] = Menu(self.menubar_id, id)
        return self.menus[name]

 #   def GetMenuItem(self, constant):

        
        #first_menu_val = GetMenuBarAttribute(self.menubar_id, 0, 'ATTR_FIRST_MENU_ID')
        #number_of_menus = GetMenuBarAttribute(self.menubar_id, 0, 'ATTR_NUM_MENUS')
        #print first_menu_val, number_of_menus
        
        #print range(first_menu_val, number_of_menus)
        
        
#        for menu_key, menu_val in self.menus.items():

            # Check all menuitems in each menu
#            for menuitem_key, menuitem_val in menu_val.menu_items.items(): 
#                print menuitem_val, menuitem_val.menuitem_id
#                print "ATTR_MENU_NAME ", GetMenuBarAttribute(self.menubar_id, menuitem_val.menuitem_id, 'ATTR_ITEM_NAME')

#                ctrl_constant = GetMenuBarAttribute(self.menubar_id, menuitem_val.menuitem_id, 'ATTR_CONSTANT_NAME')

#                print "CTRL_CONSTANT", ctrl_constant

            #if ctrl_constant == constant:
            #    return Control(self.panel_id, i)

#        return None 
 
class Panel(object):

    def __init__(self, uir, resource_id, parent=0):
        self.panel_id = LoadPanel(parent, uir, resource_id)
        self.Attributes = PanelAttributes(self.panel_id)
        number_of_controls = self.Attributes['ATTR_NUM_CTRLS']
        self.controls = range(2, number_of_controls + 2)
            
    def Display(self):
        DisplayPanel(self.panel_id)

    def Discard(self):
        DiscardPanel(self.panel_id)

    def InstallPanelCallback (self, eventFunction, callbackData):
        InstallPanelCallback (self.panel_id, eventFunction, callbackData)

    def NewMenubar(self):
        NewMenuBar(self.panel_id)
  
    def GetMenubar(self):
        return Menubar(self.panel_id)
    
    def GetControl(self, constant):
        
        for i in self.controls:

            ctrl_constant = GetCtrlAttribute(self.panel_id, i, 'ATTR_CONSTANT_NAME')

            if ctrl_constant == constant:
                return Control(self.panel_id, i)

        return None 

    def GetControlByTitle(self, title):

        for i in self.controls:

            ctrl_title = GetCtrlAttribute(self.panel_id, i, 'ATTR_LABEL_TEXT')
         
            if ctrl_title == title:
                return Control(self.panel_id, i)

        return None  

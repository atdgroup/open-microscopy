#!/usr/bin/env python

from CviPy import *
import ctypes as C
import LowLevel as Cvi

def OnCloseButtonClicked(self, panel_id, event, callback_data, event_data1, event_data2):

    if event == EVENT_COMMIT:
        print self
        print callback_data
        Cvi.DiscardPanel(panel_id)
        Cvi.QuitUserInterface(0)
        Cvi.MessagePopup("Exit", "Press OK to Exit")
        
    return 0


def OnRingControlClicked(self, panel_id, event, callback_data, event_data1, event_data2):

    if event == EVENT_COMMIT:
        print "Ring Changed - Value, ", self.GetValue()
        
    return 0


def OnClickMeMenuClicked(self, callback_data):
    print "ClickMe menu clicked"


Initialise()

panel = Panel(r"test_uir.uir", 1)

# We change attributes like so
#panel.Attributes['ATTR_WIDTH'] = 600
#panel.Attributes['ATTR_HEIGHT'] = 600
#panel.Attributes['ATTR_DIMMED'] = 0
#print panel.Attributes['ATTR_HEIGHT']

# Access control from it's label like so
control = panel.GetControlByTitle("Close")
#control.Attributes['ATTR_DIMMED'] = 1
control.InstallCtrlCallback (OnCloseButtonClicked, None)

# Access a control from it's uir define.
#Note you dont need the full define which includes the Panel define
text_control = panel.GetControl("TEXTBOX")
text_control.Attributes['ATTR_TEXT_BGCOLOR'] = VAL_GREEN 
text_control.SetValue("This is Cool")
print text_control.GetValue()
#text_control.SetVal("Written by Glenn Pierce")

wavelength = panel.GetControl("WAVELENGTH")
wavelength.InstallCtrlCallback(OnRingControlClicked, None)
print "Wavelength: ", wavelength.GetValue()


menubar = panel.GetMenubar()
cool_menu = menubar.NewMenu("Cool", 0)
#cool_menu.Attributes['ATTR_DIMMED'] = True
#cool_menu.Attributes['ATTR_CHECKED'] = 1

clickme_item = cool_menu.NewMenuItem("ClickMe", 0)
#clickme_item.Attributes['ATTR_DIMMED'] = True
clickme_item.Attributes['ATTR_CHECKED'] = True
clickme_item.InstallMenuCallback (OnClickMeMenuClicked, None)

#menubar.GetMenuItem("OPEN")

panel.Display()

Cvi.RunUserInterface()    

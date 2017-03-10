import sys
import LowLevel
import Output

class Panel:

    def __init__(self, id, execute=1, debug=None):
        self.__id = id
        self.__debug = debug
        self.__ctrl_ids = {}
        self.__subpanels = self.__GetChildPanels()
        self.__execute_mode = execute
		
    def __GetChildPanels(self):
        panels = []
        
        try:
            if LowLevel.GetNumberOfChildPanels(self.__id) == 0:
                return None
                
        except CviException:
            return None
        
        next_panel = LowLevel.GetFirstChildPanel(self.__id)
        
        panels.append(next_panel)
                 
        while 1:
            next_panel = LowLevel.GetNextChildPanel(next_panel)
                          
            if next_panel == 0:
                break
  
            panels.append(next_panel)
            
        return panels
         

    def __GetControlOnPanel(self, panel_id, label):
    
        llabel = label.lower()
        
        if self.__debug:
            print "\n\nSearching for control with label: ", llabel
    
        # Already found before
        if self.__ctrl_ids.has_key(llabel):
            if self.__debug:
                print "Returning from cache control with label ", label
            return self.__ctrl_ids[llabel]
            
        found = None

        number_of_ctrls = LowLevel.GetNumberOfControlsOnPanel(panel_id)
        
        if self.__debug:
            print "Number of controls on panel %d is %d" % (panel_id, number_of_ctrls)
            
        for i in range(2, number_of_ctrls + 2):

            try:
                control_label = LowLevel.GetControlLabel (panel_id, i)
                
                if self.__debug:
                    print "Found ctrl with label", control_label
                
#                if control_label == llabel:
                if control_label[0:len(llabel)] == llabel:
                    found = 1
                    break;
            except:
                pass
                
        if found:
            self.__ctrl_ids[llabel] = (panel_id, i)
            return (panel_id, i)
            
        return (None, None)
        
        
    def __GetControlByLabel(self, label):

        # search main panel
        panel_id, ctrl_id = self.__GetControlOnPanel(self.__id, label)
                
        if not ctrl_id:  
            "Hmm we have not found a control on the main panel. Search sub panels"
            if self.__subpanels:
                for s in self.__subpanels:
                    panel_id, ctrl_id = self.__GetControlOnPanel(s, label)
                    if ctrl_id:
                        break
                        
        if not ctrl_id:
            print "GetControlByLabel: No control with found with that label ", label, "\n"
            return (None, None)
      
        return (panel_id, ctrl_id)  
   
                     
    def __GetControlIndexFromLabel (self, control_label, label):
        """ Sets a contols value """

        if self.__id == None:
            return
            
        panel_id, ctrl_id = self.__GetControlByLabel(control_label)
        
        if ctrl_id == None:
            return
        
        number_of_items = LowLevel.GetRowsInList(panel_id, ctrl_id)
      
        for i in range(0, number_of_items):
            lab = LowLevel.GetLabelFromIndex(panel_id, ctrl_id, i)
            
#            if lab.lower() == label.lower():
            try:
                if lab.lower()[0:len(label.lower())] == label.lower():
                    return i
            except AttributeError:
                print "Warning: GetControlIndexFromLabel received an int not a label,"
                print "used int as the index: control ", control_label, ", index ", label
                return label
                
        return None
        
    def OperateControl(self, control_label, value=None, event=LowLevel.Events.EVENT_COMMIT):
        """ Calls the callback method that has been attached to the control. 
            EVENT_COMMIT is passed by default.
        """
        if self.__id == None:
            return
    
        panel_id, control_id = self.__GetControlByLabel(control_label)    
     
        if control_id == None:
            return
        
        if self.__execute_mode and value!=None:
            LowLevel.SetControlVal (panel_id, control_id, value)
                    
        if self.__execute_mode:
            LowLevel.CallControlCallback(panel_id, control_id, event, 0, 0)
            if self.__debug:
                print "Calling Callback"
   
    def PressButton(self, name):
        """ Presses a button on a cvi panel """
        self.OperateControl(name)
        
    def SetNumeric(self, name, value):
        """ Set the value of a numeric control """
        self.OperateControl(name, value)
        
    def SetValue(self, name, value):
        """ Set the value of a control """
        self.OperateControl(name, value)
        
    def ToggleButtonOn(self, name):
        """ Sets a toggle button control to on """
        self.OperateControl(name, 1)
        
    def ToggleButtonOff(self, name):
        """ Sets a toggle button control to off """
        self.OperateControl(name, 0)
   
    def CheckButtonOn(self, name):
        """ Sets a control to on """
        self.OperateControl(name, 1)
   
    def CheckButtonOff(self, name):
        """ Sets a control to off """
        self.OperateControl(name, 0)
   
    def SetListToIndex(self, control_label, index, event=LowLevel.Events.EVENT_COMMIT):
        """ Sets a list/ring contol to a value with the specified index"""

        if self.__id == None:
            return
            
        panel_id, control_id = self.__GetControlByLabel(control_label)
        
        if control_id == None:
            return
            
        if self.__execute_mode:
            LowLevel.SetControlIndex(panel_id, control_id, index)

        if self.__execute_mode:
            LowLevel.CallControlCallback(panel_id, control_id, event, 0, 0)
            if self.__debug:
                print "Calling Callback"

    def SetListToLabel(self, control_label, label, event=LowLevel.Events.EVENT_COMMIT):
        """ Sets a list/ring contol to a value with the specified label"""

        index = self.__GetControlIndexFromLabel (control_label, label)
        
        if index == None:
        
            if self.__debug:
                print "No such label in list/ring control found by the name ", label
        
            return
  
        return self.SetListToIndex(control_label, index, event)
    
    def SetSliderToValue(self, name, value):
        """ Sets a slider with the specified label to an absolute value"""
        return self.OperateControl(name, value)
        
    
    def SetSliderToPercentage(self, name, value):
        """ Sets a slider with the specified label to an percentage value. Can be negative"""
        
        panel_id, ctrl_id = self.__GetControlByLabel(name)
        
        if ctrl_id == None:
            return
            
        min, max = LowLevel.GetNumericMinMax(panel_id, ctrl_id)
            
        # negative case
        if value < 0:
            val = min * -value / 100
        else:
            val = max * value / 100

        return self.OperateControl(name, val)
        
    def SelectMenuItem (self, menu_string):
        """ Selects and executes an item from the menu
          menu_string should be in the form \'__File//__Open\', where \'__\' = underlined"""
        if self.__execute_mode:
            LowLevel.SelectMenuItem (self.__id, menu_string)
            
    def SetFocus (self, title=None):
        """ Sets the focus of the input to the panel or child panel (if specified)"""
        if (title==None): # we want the parent panel
            return LowLevel.SetFocus(self.__id)

        else: # search the child panels
            found = None
            
            self.__subpanels = self.__GetChildPanels()   # make sure child panel list is up to date
            
            if self.__subpanels:
                ltitle = title.lower()
    
                if self.__debug:
                    print "Searching for ", ltitle

                for s in self.__subpanels:
                    try:
                        panel_title = LowLevel.GetPanelTitle (s)
                        if self.__debug:
                            print "Trying ", panel_title
                        if panel_title[0:len(ltitle)] == ltitle:
                            found = 1
                            break;
                    except:
                        pass
 
            if not found:    
                print "SetFocus: No panel found for: " + title
                return None
                
            return LowLevel.SetFocus(s)
            
class UIController:

    # Keypress modifiers
    SHIFT = 0x010000
    ALT = 0x020000
    CTRL = 0x040000
    SHIFT_CTRL = 0x050000
    
    # virtual keys
    DELETE = 0x0100
    BACKSPACE = 0x0200
    ESC = 0x0300
    TAB = 0x0400
    ENTER = 0x0500
    UP_ARROW = 0x0600
    DOWN_ARROW = 0x0700
    LEFT_ARROW = 0x0800
    RIGHT_ARROW = 0x0900
    INSERT = 0x0A00
    HOME = 0x0B00
    END = 0x0C00
    PAGE_UP = 0x0D00
    PAGE_DOWN = 0x0E00
    F1 = 0x0F00
    F2 = 0x1000
    F3 = 0x1100
    F4 = 0x1200
    F5 = 0x1300
    F6 = 0x1400
    F7 = 0x1500
    F8 = 0x1600
    F9 = 0x1700
    F10 = 0x1800
    F11 = 0x1900
    F12 = 0x1A00

    __panel_ids = {}

    def __init__(self, debug=None):
        self.__debug = debug
        self.execute_mode = 1
    
    def KeyPress(self, key):
        """ Fakes a keypress 
            Use ASCII code (i.e. ord("a")) or char (i.e. "a") or virtual key code.
            The ASCII or virtual key can be OR'd with a modifier (use |).
            e.g. ui.KeyPress("g") or ui.KeyPress(ord("g")) for g
                 ui.KeyPress(ui.ALT | ord("f")) for alt-f
                 ui.KeyPress(ui.ENTER) for the enter key
                 See help(UIController) for a list of modifiers (i.e. shift etc.) and virtual keys (i.e. enter, delete etc.)."""
        if type(key) is str and len(key)==1:
            print "is str ", ord(key)
            LowLevel.FakeKeystroke(ord(key))
        elif type(key) is int: 
            print "is int ", key
            LowLevel.FakeKeystroke(key)
    
    def GetPanelByTitle(self, title):
        """ Retrieves the panel starting with the specified title"""
        ltitle = title.lower()
    
        if self.__debug:
            print "Searching for ", ltitle
    
        # Already found before
        if self.__panel_ids.has_key(ltitle):
            if not LowLevel.PanelVisible(self.__panel_ids[ltitle]):
                print "GetPanelByTitle: Warning, the panel \'", ltitle, "\' is hidden"
            return Panel(self.__panel_ids[ltitle], self.execute_mode, self.__debug)
    
        found = None

        panel_range = range(1, 1000)
    
        if self.__debug:
            print "Possible panels"
    
        for i in panel_range:

            try:
            
                if self.__debug:
                    print "Trying panel id ", i
            
                panel_title = LowLevel.GetPanelTitle (i)
            
                if self.__debug:
                    print panel_title
            
                if panel_title[0:len(ltitle)] == ltitle:
                    found = 1
                    break;
            except:
                pass
 
        if not found:    
            print "GetPanelByTitle: No panel with that title found"
            return None
        
        if self.__debug:
            print "Setting panel dictionary for", ltitle
        
        self.__panel_ids[ltitle] = i

        if not LowLevel.PanelVisible(self.__panel_ids[ltitle]):
            print "GetPanelByTitle: Warning, the panel \'", panel_title, "\' is hidden"
        
        return Panel(i, self.execute_mode, self.__debug)
 
    def MessagePopup(self, title, msg):
        """Displays a popup window and waits for confirmation from user."""
        return LowLevel.MessagePopup (title, msg)
 
    def ConfirmPopup(self, title, msg):
        """Displays a popup window and waits for yes(1)/no(0) confirmation from user."""
        return LowLevel.ConfirmPopup (title, msg)
 
   
        
        
if __name__ == '__main__':
    controller = UIController()
    controller.debug = 1
  
    panel = Panel(1, 1)
    panel.SetControlVal ("string", "Hello")

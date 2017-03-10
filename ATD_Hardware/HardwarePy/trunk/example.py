#!/usr/bin/env python

from CviPy.CviPy import Panel, Control
import CviPy.constants as CviConstants
import CviPy.LowLevel as Cvi
import IcsViewer.IcsViewer as IcsViewer
import FreeImagePy as FI
import FreeImageAlgorithmsPy.FreeImageAlgorithmsPy as FIA
import FreeImageIcsPy.FreeImageIcsPy as FI_ICS 
import ImageViewer.ImageViewer as ImageViewer

try:  
    import pygtk  
    pygtk.require("2.0")  
except:  
    pass

try:
    import gtk
    import gobject
    import gtk.glade
except:
    sys.exit(1)
    
class TestPanel:  

    def __init__(self):
        gladefile = "test.glade"  
        self.windowname = "window"  
        self.wTree = gtk.glade.XML(gladefile, self.windowname)  
        self.window = self.wTree.get_widget(self.windowname)
        self.ok_button = self.wTree.get_widget("ok_button")
        
        # When the window is given the "delete_event" signal (this is given
        # by the window manager, usually by the "close" option, or on the
        # titlebar), we ask it to call the delete_event () function
        # as defined above. The data passed to the callback
        # function is NULL and is ignored in the callback function.
        self.window.connect("delete_event", self.delete_event)

        # Here we connect the "destroy" event to a signal handler.
        # This event occurs when we call gtk_widget_destroy() on the window,
        # or if we return FALSE in the "delete_event" callback.
        self.window.connect("destroy", self.destroy)
        
        # This will cause the window to be destroyed by calling
        # gtk_widget_destroy(window) when "clicked".  Again, the destroy
        # signal could come from here, or the window manager.
        self.ok_button.connect_object("clicked", gtk.Widget.destroy, self.window)
   
        self.window.show_all()
    
    def delete_event(self, widget, event, data=None):
        # If you return FALSE in the "delete_event" signal handler,
        # GTK will emit the "destroy" signal. Returning TRUE means
        # you don't want the window to be destroyed.
        # This is useful for popping up 'are you sure you want to quit?'
        # type dialogs.
        print "delete event occurred"

        # Change FALSE to TRUE and the main window will not be destroyed
        # with a "delete_event".
        return False

    def destroy(self, widget, data=None):
        print "Destroying Panel"
        gtk.main_quit()
        Cvi.QuitUserInterface(0)
   

def on_idle():
    #print "Idle"
    Cvi.ProcessSystemEvents()
    return True
    
def OnTimerTick(self, panel_id, event, callback_data, event_data1, event_data2):

    if event == CviConstants.EVENT_TIMER_TICK:
        # All PyGTK applications must have a gtk.main(). Control ends here
        # and waits for an event to occur (like a key press or mouse event).
        print "Running Gtk Main"
        idle_tag = gobject.idle_add(on_idle) 
        gtk.main()  # Never exits callback
        print "Disabling Timer"
        self.Attributes['ATTR_ENABLED'] = 0 # Disable time after first tick
        
    
    return 0
    
def ShowIcsViewer():
    icsViewer = IcsViewer.IcsViewer("Test Viewer")
    F = FIA.FIAImage()
    F.load(r"C:\Documents and Settings\Pierce\Desktop\Working Area\Test Images\wallpaper_river.jpg")
    icsViewer.LoadImage(F.getBitmap())
    icsViewer.Show()
    

imageViewer = ImageViewer.ImageViewer()

fib = FI_ICS.FreeImageIcs_LoadFIBFromIcsFilePath(r"C:\Documents and Settings\Pierce\Desktop\Working Area\Test Images\cells.ics")

F = FIA.FIAImage()
F.loadFromBitmap(fib)
F.SetRainBowPalette()

imageViewer.BasicWin32Window("Test", 100, 100, 500, 400, F.getBitmap())

Cvi.RunUserInterface()   
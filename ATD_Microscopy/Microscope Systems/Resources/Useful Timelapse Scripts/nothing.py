# nothing



# OnStart is called once when the user presses start to begin the timelapse.
def OnStart():
    pass
        
def OnAbort():
    print "OnAbort()" 
    
# Called once at the start of each cycle of points. Should contain at least one call to  microscope.MicroscopeVisitPoints() to start the cycle.
def OnCycleStart():
    pass
    
# Called each time the stage moves to a new point.
def OnNewPoint(x, y, z, position):
    pass
    
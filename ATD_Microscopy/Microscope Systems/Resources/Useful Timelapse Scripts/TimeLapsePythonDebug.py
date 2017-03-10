# Python RefCount Debug

import sys
import microscope

# OnStart is called once when the user presses start to begin the timelapse.
def OnStart():
    print "PyNone Ref Count: ", sys.getrefcount(None)
     
def OnAbort():
    print "OnAbort()"
     
# Called once at the start of each cycle of points. Should contain at least one call to  microscope.MicroscopeVisitPoints() to start the cycle.
def OnCycleStart():
    print "OnCycleStart"
    print "PyNone Recount before visit points ", sys.getrefcount(None)
    microscope.VisitPoints()
    print "PyNone Recount after visit points ", sys.getrefcount(None)

    
# Called each time the stage moves to a new point.
def OnNewPoint(x, y, z, position):
    print "NewPoint()"
    

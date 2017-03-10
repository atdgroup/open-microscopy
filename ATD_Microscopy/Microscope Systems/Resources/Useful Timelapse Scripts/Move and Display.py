# Move and display
import microscope, time

# OnStart is called once when the user presses start to begin the timelapse.
def OnStart():
    print "Stage Position, ", microscope.GetStagePosition()
    print microscope.GetTimeLapsePoints()
        
def OnAbort():
    print "OnAbort()" 
    
# Called once at the start of each cycle of points. Should contain at least one call to  microscope.MicroscopeVisitPoints() to start the cycle.
def OnCycleStart():
    microscope.VisitPoints()   
    
# Called each time the stage moves to a new point.
def OnNewPoint(x, y, z, position):
    microscope.SetStagePosition(x, y, z)

    # Wait for stage to finish moving (additional delay=0.1 seconds, time out = 1.0 seconds)
    microscope.WaitForStage(0.0, 1.0)

    microscope.SnapImage()

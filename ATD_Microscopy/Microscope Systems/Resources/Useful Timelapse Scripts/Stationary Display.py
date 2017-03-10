# Stationary display

import microscope, time

# OnStart is called once when the user presses start to begin the timelapse.
def OnStart():

    print "Stage Position, ", MicroscopeGetStagePosition()
    print microscope.GetTimeLapsePoints()
        
# Called once at the start of each cycle of points. Should contain at least one call to  microscope.MicroscopeVisitPoints() to start the cycle.
def OnCycleStart():
    print "Back to start."
    print "Cube 2."
    microscope.MoveCubeToPosition(2)
    microscope.WaitForCube(0.0, 10.0)
    microscope.VisitPoints()
    print "Cube 3."
    microscope.MoveCubeToPosition(3)
    microscope.WaitForCube(0.0, 10.0)
    microscope.VisitPoints()
    
# Called each time the stage moves to a new point.
def OnNewPoint(x, y, z, position):

    #    print "Moving Stage ", (x, y, z)
    microscope.SetStagePosition(x, y, z)
	
    #print "Moved Stage to ", microscope.MicroscopeGetStagePosition()
    
    # wait for stage to finish moving (additional delay=0.1 seconds, time out = 1.0 seconds)
    microscope.WaitForStage(0.0, 1.0)

    #print "Snapping Image"
    microscope.SnapImage()

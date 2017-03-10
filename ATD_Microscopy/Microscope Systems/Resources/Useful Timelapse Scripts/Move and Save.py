# Move and Save

import microscope, time

# OnStart is called once when the user presses start to begin the timelapse.
def OnStart():
    global output
    
    data_dir = microscope.GetUserDataDirectory()
    output = microscope.ShowFileSequenceSaveDialog(data_dir)
    
    if output != None:
        print "Saving to directory ", output[0]
        print "Filename Format ", output[1]
    else:
        print "Aborting Timelapse"
        microscope.AbortTimeLapseVisitPoints()
    
def OnAbort():
    pass
    
# Called once at the start of each cycle of points. Should contain at least one call to  microscope.MicroscopeVisitPoints() to start the cycle.
def OnCycleStart():
        microscope.VisitPoints()
 
# Called each time the stage moves to a new point.
def OnNewPoint(x, y, z, position):
    microscope.SetStagePosition(x, y, z)

    # Wait for stage to finish moving (additional delay=0.1 seconds, time out = 1.0 seconds)
    microscope.WaitForStage(0.0, 1.0)
    
    filename = microscope.ParseSequenceFilename(output[1], position) 
    path = output[0] + filename
    microscope.SnapAndSaveImage(path) 
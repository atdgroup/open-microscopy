# Move Focus and Save

import microscope, time

output = ()

# OnStart is called once when the user presses start to begin the timelapse.
def OnStart():

    global output
    
    print "Stage Position, ", microscope.GetStagePosition()
    print microscope.GetTimeLapsePoints()
    
    data_dir = microscope.GetUserDataDirectory()
    output = microscope.ShowFileSequenceSaveDialog(data_dir)
        
    print "Saving to directory ", output[0]
    print "Filename Format ", output[1]
    
def OnAbort():
    print "OnAbort()"
    
        
# Called once at the start of each cycle of points. Should contain at least one call to  microscope.MicroscopeVisitPoints() to start the cycle.
def OnCycleStart():
    print "Back to start."
    microscope.VisitPoints()

# Called each time the stage moves to a new point.
def OnNewPoint(x, y, z, position):

#    print "Moving Stage ", (x, y, z)
    microscope.SetStagePosition(x, y, z)
    # wait for stage to finish moving (additional delay=0.1 seconds, time out = 1.0 seconds)
    microscope.WaitForStage(0.0, 1.0)
    filename = microscope.ParseSequenceFilename(output[1], position) 
    path = output[0] + filename
    microscope.PerformSoftwareAutoFocus(path)
    microscope.SnapAndSaveImage(path)
          
    #mrowley - 081209 - recording the software auto focus determined focal point for the given well
    xf, yf, zf = microscope.GetStagePosition()
    filenameautopts = 'AutoFocusedPlane.pts'
    path = output[0] + filenameautopts
    fileautopts = open(path, 'a')
    fileautopts.write('%.2f' %xf + '\t' + '%.2f' %yf + '\t' + '%.2f' %zf + '\n')
    fileautopts.close()
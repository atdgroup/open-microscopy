# Name: Move Perform spc and Save
# Description: Move Perform spc and Save
# Author: Default
# Category: SPC

import MicroscopeModules, microscope, time

output = ()
spc = MicroscopeModules.Spc()
aborted = False

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
    global aborted
    aborted = True
    print "aborted" 
        
# Called once at the start of each cycle of points. Should contain at least one call to  microscope.MicroscopeVisitPoints() to start the cycle.
def OnCycleStart():
    print "Back to start."
    microscope.VisitPoints()
 
# Called each time the stage moves to a new point.
def OnNewPoint(x, y, z, position):
    global spc
    global aborted
    
    if aborted:
        pass

    microscope.SetStagePosition(x, y, z)
    # microscope.SetStagePosition(x, y, (z+11.50))
    # wait for stage to finish moving (additional delay=0.1 seconds, time out = 1.0 seconds)
    microscope.WaitForStage(0.0, 1.0)
    filename = microscope.ParseSequenceFilename(output[1], position) 
    path = output[0] + filename
    print "Performing SPC"
    microscope.MicroscopeSetMode(2)   # Laser Scanning
    spc.ClearBoardMemory()
    spc.AcquireAndSaveToFileUsingUIValues(1, path)
    print "Performing FL"
    microscope.MicroscopeSetMode(0)   # FLuorescence
    filename = microscope.ParseSequenceFilename(output[1], position) 
    path = output[0] + filename
    microscope.SnapAndSaveImage(path)
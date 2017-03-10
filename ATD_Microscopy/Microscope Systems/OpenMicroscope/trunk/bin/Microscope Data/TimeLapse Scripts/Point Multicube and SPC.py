# Name: Point Multicube and SPC
# Description: Aquire points with all cubes and SPC FLIM, uses UI cube options for exposure and Z offset
# Author: Fluorescence Acq
# Category: Points

import microscope, MicroscopeModules, time

aborted = False
cube_data = {}
spc = MicroscopeModules.Spc()

# OnStart is called once when the user presses start to begin the timelapse.
def OnStart():
    print "Start: Point Multicube and SPC"
    global output, cube_data

    data_dir = microscope.GetUserDataDirectory()
    output = microscope.ShowFileSequenceSaveDialog(data_dir)
        
    if output != None:
        print "Saving to directory ", output[0]
        print "Filename Format ", output[1]
    else:
        print "Aborting Timelapse"
        microscope.AbortTimeLapseVisitPoints()

    cube_data = microscope.GetCubes();

 
def OnAbort():
    global aborted, spc
    
    print "aborted" 
    aborted = True
    spc.Stop()
    
# Called once at the start of each cycle of points. Should contain at least one call to  microscope.MicroscopeVisitPoints() to start the cycle.
def OnCycleStart():
    microscope.VisitPoints()   
  
    
# Called each time the stage moves to a new point.
def OnNewPoint(x, y, z, position):
    global aborted, spc

    print "Point: ", position

    if aborted:
        break

    print "Set Fluorescence Mode"
    microscope.MicroscopeSetMode(0)   

    for cube in cube_data:

        print "Selecting Cube ", cube['Position'], ": ", cube['Name']
        microscope.MoveCubeToPosition(cube['Position'])
        
        options = microscope.GetCubeOptions(cube['Position'])  # camera exposure, gain and focus offset
        print "Setting camera exposure to ", options[0]
        microscope.SetExposure(options[0])
        print "Setting camera gain to ", options[1]
        microscope.SetGain(options[1])
        
        # store the focus offset
        offset = options[2]

        if aborted:
            break
        
        print "Moving to (%0.1f, %0.1f, %0.1f)" % (x, y, z+offset)
        microscope.SetStagePosition(x, y, z+offset)

        # Wait for stage to finish moving (additional delay seconds, time out seconds)
        microscope.WaitForStage(0.0, 1.0)

        output_directory = output[0]
        filename = output[1]

        filename = microscope.ParseSequenceFilename(filename, position) 
        filename = microscope.InsertCubeIntoFilename(filename, cube['Position'])
        path = output_directory + filename
        print "Acquire with FL cube: ", path
        microscope.SnapAndSaveImage(path)     
          
    if aborted:
        pass
    
    print "Set Laser Scanning Mode"
    microscope.MicroscopeSetMode(2)   # Laser Scanning
    spc.ClearBoardMemory()
    filename = microscope.ParseSequenceFilename(output[1], position)   
    filename = microscope.InsertTextIntoFilename(filename, "spc")
    path = output[0] + filename
    print "Acquire with SPC: ", path
    spc.AcquireAndSaveToFileUsingUIValues(1, path)
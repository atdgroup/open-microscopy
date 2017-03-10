# spc and 3 cubes

import MicroscopeModules, microscope, time

# Cubes that we will cycle through
# You can extend this list with any valid cube positions
# Here we have two cube positions 2 and 3 
cubes = [1, 2, 3] 

# This is the exposure setting for each cube in the cubes list 
# For example the cubes in position 2 and 3 above both should have exposure
# set to 15 ms
exposures = [100.0, 100.0, 100.0]

# These are the offsets that will be applied to the zdrive for each cube + 1 for spc
focus_offsets = [0.0, 0.0, 0.0, 0.0]

###################################################################################
###################################################################################
###################################################################################
# Do not edit below this line

output = ()
current_cube_index = 0
spc = MicroscopeModules.Spc()
cameraAqc = 1

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
    global current_cube_index, camera_acq
    current_cube_index = 0
    
    print "Fluorescence Mode"
    microscope.MicroscopeSetMode(0)   # FLuorescence
    camera_acq = 1  # do the widefield camera images now
    for cube in cubes:
        print "Moving to cube position ", cube
        microscope.MoveCubeToPosition(cube)
        print "Setting camera exposure to ", exposures[current_cube_index]
        microscope.SetExposure(exposures[current_cube_index])
        microscope.VisitPoints()
        current_cube_index = current_cube_index + 1
    
    print "Laser Scanning Mode"
    camera_acq = 0  # do the flim images now
    microscope.MicroscopeSetMode(2)   # Laser Scanning
    microscope.VisitPoints()

 
# Called each time the stage moves to a new point.
def OnNewPoint(x, y, z, position):

    global current_cube_index, camera_acq
    microscope.SetStagePosition(x, y, z + focus_offsets[current_cube_index])

    # Wait for stage to finish moving (additional delay=0.1 seconds, time out = 1.0 seconds)
    microscope.WaitForStage(0.0, 1.0)

    filename = microscope.ParseSequenceFilename(output[1], position) 
    path = output[0] + filename
    
    if camera_acq == 1:
        print "Camera image: ", position
        microscope.SnapAndSaveImage(path)
    else:
        print "Performing SPC: ", position
        spc.ClearBoardMemory()
        spc.AcquireAndSaveToFileUsingUIValues(1, path)

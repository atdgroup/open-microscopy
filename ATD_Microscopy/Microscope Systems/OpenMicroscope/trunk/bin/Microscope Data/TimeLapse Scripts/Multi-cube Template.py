# Name: Multi-cube Template
# Description: For each cube, visit the points and get ans save an image
# Author: Default
# Category: Basic

import microscope, time

# Cubes that we will cycle through
# You can extend this list with any valid cube positions
# Here we have three cube positions 1, 2 and 3 
cubes = [1, 2, 3] 

# This is the exposure setting for each cube in the cubes list 
# For example the cubes in position 2 and 3 above both should have exposure
# set to 150 ms
exposures = [100.0, 150.0, 150.0]

# These are the offsets that will be applied to the zdrive for each cube.
focus_offsets = [0.0, 1.6, -3.2]

###################################################################################
###################################################################################
###################################################################################
# Do not edit below this line

aborted = False
output = ()
current_cube_index = 0

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
    global aborted
    aborted = True
    print "aborted" 
    
# Called once at the start of each cycle of points. Should contain at least one call to  microscope.MicroscopeVisitPoints() to start the cycle.
def OnCycleStart():
    global current_cube_index
    current_cube_index = 0
    
    for cube in cubes:
        print "Moving to cube position ", cube
        microscope.MoveCubeToPosition(cube)
        print "Setting camera exposure to ", exposures[current_cube_index]
        microscope.SetExposure(exposures[current_cube_index])
        print "Revisit points"
        microscope.VisitPoints()
        current_cube_index = current_cube_index + 1
 
# Called each time the stage moves to a new point.
def OnNewPoint(x, y, z, position):

    global current_cube_index
    global aborted
    
    if aborted:
        pass

    microscope.SetStagePosition(x, y, z + focus_offsets[current_cube_index])

    # Wait for stage to finish moving (additional delay=0.1 seconds, time out = 1.0 seconds)
    microscope.WaitForStage(0.0, 1.0)
    
    filename = microscope.ParseSequenceFilename(output[1], position) 
    filename = microscope.InsertCubeIntoFilename(filename, current_cube_index+1)
    path = output[0] + filename
    microscope.SnapAndSaveImage(path) 
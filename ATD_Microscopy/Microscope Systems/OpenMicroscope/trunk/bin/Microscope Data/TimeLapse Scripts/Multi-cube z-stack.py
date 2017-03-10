# Name: Multi-cube z-stack
# Description: For each cube, perform a z-stack at each point
# Author: Default
# Category: Basic

import microscope, time, os

# Z stack setup
# The set z focus position will be the starting point of the z range
# z_images_above + z_images_below + 1 images will be acquired
# i.e. z_step=10, z_images_above=2, z_images_below=2 will aquire at -20, -10, 0, 10, 20 relative to initial z
# all values in microns
z_step  = 10
z_images_above = 5
z_images_below = 5

# Cubes that we will cycle through
# You can extend this list with any valid cube positions
# Here we have three cube positions 1, 2 and 3 
cubes = [1, 2, 3] 

# This is the exposure setting for each cube in the cubes list 
# For example the cubes in position 2 and 3 above both should have exposure
# set to 150 ms
exposures = [100.0, 150.0, 150.0]

# These are the offsets that will be applied to the zdrive for each cube, before the z stack is started.
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
    
# Called once at the start of each cycle of points. Should contain at least one call to  microscope.MicroscopeVisitPoints() to start the cycle.
def OnCycleStart():
    global current_cube_index
    global aborted
    current_cube_index = 0
    
    for cube in cubes:
        if aborted:
            break
            
        print "Moving to cube position ", cube
        microscope.MoveCubeToPosition(cube)
        print "Setting camera exposure to ", exposures[current_cube_index]
        microscope.SetExposure(exposures[current_cube_index])

        microscope.VisitPoints()
        current_cube_index = current_cube_index + 1
 
# Called each time the stage moves to a new point.
def OnNewPoint(x, y, z, position):

    global current_cube_index
    global aborted
    
    z_centre = z + focus_offsets[current_cube_index]

    dir = output[0] + 'Point' + str(position)    

    if not os.path.exists(dir):
        os.mkdir(dir)
        
    for z_current_step in range(-z_images_below, z_images_above+1): # +1 ensures the last point is aquired
    
        if aborted:
            break
            
        z_current = z_centre + z_step * z_current_step
    
        print "Capturing image at (z = " , str(z_current) , ")"
        microscope.SetStagePosition(x, y, z_current)
        microscope.WaitForStage(0.0, 1.0)
        
        filename = microscope.ParseSequenceFilename(output[1], position) 

        zpos = '%(#)02d' % {"#":(z_current_step+z_images_below)}
        filename = microscope.InsertTextIntoFilename(filename, "z" + zpos)
        filename = microscope.InsertCubeIntoFilename(filename, cubes[current_cube_index])
        path = dir + os.sep + filename
        microscope.SnapAndSaveImage(path)

	

# Gregory Colocalisation

import microscope, MicroscopeModules, os, time

# Specify the whether the region scan saves images or just displays them
action = MicroscopeModules.RegionScan.SAVE_DISPLAY

# Cubes that we will cycle through
# You can extend this list with any valid cube positions
# Here we have two cube positions 2 and 3 
cubes = [1, 2, 3]

# This is the exposure setting for each cube in the cubes list 
# For example the cubes in position 2 and 3 above both should have exposure
# set to 15 ms
exposures = [10.0, 15.0, 15.0]

# Set the gain of the camera for each cube position 
gains = [0.0, 0.0, 0.0]

# These are the offsets that will be applied to the zdrive for each cube.
focus_offsets = [2.0, 0.0, 1.0]

###################################################################################
###################################################################################
###################################################################################
# Do not edit below this line

output = ()
core = 1
cube_data = {}
aborted = False

def OnAbort():
    print "Aborted"
    global aborted
    aborted = True
    microscope.AbortTimeLapseVisitPoints()
    
# OnStart is called once when the user presses start to begin the timelapse.
def OnStart():
    global output
    global cube_data
    global core
    
    core = 1
    
    data_dir = microscope.GetUserDataDirectory()
    output = microscope.ShowSimpleFileSequenceSaveDialog(data_dir)
    cube_data = microscope.GetCubes();
    
    if output != None:
        print "Saving to directory ", output[0]
        print "Filename Format ", output[1]
    else:
        print "Aborting Timelapse"
        microscope.AbortTimeLapseVisitPoints()

# Called once at the start of each cycle of points. Should contain at least one call to  microscope.MicroscopeVisitPoints() to start the cycle.
def OnCycleStart():
    microscope.VisitPoints()
    
    
# Called each time the stage moves to a new point.
# For this script we calculate 8 surrounding points 
# to also take an image of.
def OnNewPoint(x, y, z, position):

    global core
    global output
    global cube_data
    
    output_directory = output[0]
    filename = output[1]
    filename_ext = output[2]
    
    # Move to first cube to perform autofocus.
    microscope.MoveCubeToPosition(1)
        
    # AutoFocus Once for each new point (centre point)
    print "Performing AutoFocus"
    microscope.PerformSoftwareAutoFocus()
     
    for cube in cubes:
    
        if aborted:
            return;
            
        print "Setting camera exposure to ", exposures[cube - 1]
        microscope.SetExposure(exposures[cube - 1])
        print "Setting camera gain to ", gains[cube - 1]
        microscope.SetGain(gains[cube - 1])
    
        cube_details = cube_data[cube - 1]
        
        print "Moving to cube position ", cube
        microscope.MoveCubeToPosition(cube)
          
        print "Moving to (%d,%d)" % (x, y)
        # Apply focus offset
        microscope.SetStagePosition(x, y, z + focus_offsets[cube - 1])

        # Wait for stage to finish moving (additional delay=0.1 seconds, time out = 1.0 seconds)
        microscope.WaitForStage(0.0, 5.0)
    
        filepath = '%s\\%s_Core%d_%s%s' % (output_directory, filename, core, cube_details["Name"], filename_ext)
        print "Saving file: ", filepath
        microscope.SnapAndSaveImage(filepath) 
        
    core = core + 1
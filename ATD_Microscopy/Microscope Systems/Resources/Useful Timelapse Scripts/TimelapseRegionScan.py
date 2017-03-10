# Timelapse Regionscan

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

# These are the offsets that will be applied to the zdrive for each cube.
focus_offsets = [2.0, 0.0, 1.0]

###################################################################################
###################################################################################
###################################################################################
# Do not edit below this line

current_cube_index = 0
output = ()
count = 0

rs = MicroscopeModules.RegionScan()

def OnAbort():
    global rs
    rs.Stop()

# OnStart is called once when the user presses start to begin the timelapse.
def OnStart():
    global output
    data_dir = microscope.GetUserDataDirectory()
    output = microscope.ShowSimpleFileSequenceSaveDialog(data_dir)
        
    if output != None:
        print "Saving to directory ", output[0]
        print "Filename Format ", output[1]
    else:
        print "Aborting Timelapse"
        microscope.AbortTimeLapseVisitPoints()

# Called once at the start of each cycle of points. Should contain at least one call to  microscope.MicroscopeVisitPoints() to start the cycle.
def OnCycleStart():
    global count
    global current_cube_index
    current_cube_index = 0
    
    for cube in cubes:
        print "Moving to cube position ", cube
        count = 0
        microscope.MoveCubeToPosition(cube)
        print "Setting camera exposure to ", exposures[current_cube_index]
        microscope.SetExposure(exposures[current_cube_index])
        microscope.VisitPoints()
        current_cube_index = current_cube_index + 1
    
# Called each time the stage moves to a new point.
def OnNewPoint(x, y, z, position):
    global rs
    global count
    global output
    global action
    global current_cube_index
    output_directory = output[0]
    filename = output[1]
    filename_ext = output[2]
    subdir = '%s\\CubePosition%d\\Core%d\\' % (output_directory, cubes[current_cube_index], count)

    try:
        os.makedirs(subdir)
    except (OSError):
        print 'os.makedirs(' + subdir + ') FAILED'
 
    print z
    print focus_offsets[current_cube_index]
    microscope.SetStagePosition(x, y, z + focus_offsets[current_cube_index])

    # Wait for stage to finish moving (additional delay=0.1 seconds, time out = 1.0 seconds)
    microscope.WaitForStage(0.0, 5.0)
    rs.SetRelativeRoi(2000, 2000)
    rs.Start(action, subdir, filename, filename_ext)
    rs.Hide()
    count = count + 1
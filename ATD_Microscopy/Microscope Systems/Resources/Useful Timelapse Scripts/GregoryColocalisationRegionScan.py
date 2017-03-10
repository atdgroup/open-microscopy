# Gregory Colocalisation Region Scan

import microscope, MicroscopeModules, os, time

# Specify the whether the region scan saves images or just displays them
action = MicroscopeModules.RegionScan.SAVE_DISPLAY

# Cubes that we will cycle through
# You can extend this list with any valid cube positions
# Here we have two cube positions 2 and 3 
cubes = [1, 2, 3]
cube_names = ["Cy3", "FITC", "UV"]

# This is the exposure setting for each cube in the cubes list 
# For example the cubes in position 2 and 3 above both should have exposure
# set to 15 ms
exposures = [150.0, 1000.0, 400.0]

# Set the gain of the camera for each cube position 
gains = [0.0, 0.0, 0.0]

# These are the offsets that will be applied to the zdrive for each cube.
focus_offsets = [0.0, 1.8, 1.0]

microns_per_pixel = microscope.GetMicronsPerPixel()
pixel_xoffset = 950
pixel_yoffset = 950

xoffset = pixel_xoffset * microns_per_pixel
yoffset = pixel_yoffset * microns_per_pixel
    
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
    print "Microns per pixel: ", microns_per_pixel
    print "X Offset: ", xoffset
    print "Y Offset: ", yoffset
    
    microscope.VisitPoints()
    
    
def SetExposureAndGain(cube_index):
    print "Setting camera exposure to ", exposures[cube_index]
    microscope.SetExposure(exposures[cube_index])
    print "Setting camera gain to ", gains[cube_index]
    microscope.SetGain(gains[cube_index])
       

def Move(x, y, z, cube_index):
    print "Moving to (%d,%d)" % (x, y)
    # Apply focus offset
    microscope.SetStagePosition(x, y, z + focus_offsets[cube_index])

    # Wait for stage to finish moving (additional delay=0.1 seconds, time out = 1.0 seconds)
    microscope.WaitForStage(0.0, 5.0)

    
def SnapAndSave(cube_index, rs_point_count):

    global core
    global output
    global cube_data
    
    output_directory = output[0]
    filename = output[1]
    filename_ext = output[2]
    cube_details = cube_data[cube_index]
    
    filepath = '%s\\%s_Core%d_Point%d_%s%s' % (output_directory, filename, core, rs_point_count, cube_names[cube_index], filename_ext)
    print "Saving file: ", filepath
    microscope.SnapAndSaveImage(filepath) 
    
    
def DoRegionScan(x, y, z, position):
    global aborted
    global output
    global cube_data
    global action
 
    if aborted:
        return;
            
    rs_points = []
    
    rs_points.append((x-xoffset, y-yoffset))
    rs_points.append((x, y-yoffset))
    rs_points.append((x+xoffset, y-yoffset))
    rs_points.append((x+xoffset, y))
    rs_points.append((x, y))
    rs_points.append((x-xoffset, y))
    rs_points.append((x-xoffset, y+yoffset))
    rs_points.append((x, y+yoffset))
    rs_points.append((x+xoffset, y+yoffset))
    
    rs_point_count = 1
    # Loop region scan points
    for rs_point in rs_points:
    
        if aborted:
            return;

        for cube in cubes:
    
            if aborted:
                return;
            
            microscope.MoveCubeToPosition(cube)
            SetExposureAndGain(cube-1);
    
            if cube == 1:
                Move(rs_point[0], rs_point[1], z, cube-1)
                
                # We are on the first cube so we do an autofocus
                print "Performing AutoFocus"
                microscope.PerformSoftwareAutoFocus()
    
            SnapAndSave(cube-1, rs_point_count)

        rs_point_count = rs_point_count  + 1
        
    
# Called each time the stage moves to a new point.
# For this script we calculate 8 surrounding points 
# to also take an image of.
def OnNewPoint(x, y, z, position):

    global core
    
    # Move to first cube to perform autofocus.
    microscope.MoveCubeToPosition(1)
    SetExposureAndGain(0);
        
    # AutoFocus Once for each new point (centre point)
    #print "Performing AutoFocus"
    #microscope.PerformSoftwareAutoFocus()
    
    DoRegionScan(x, y, z, position)
        
    core = core + 1
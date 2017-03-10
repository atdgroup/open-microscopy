# Name: Region Move and save AF2
# Description: Move stage, get image(s) and save, with autofocus at start of each region, seeded from the last time point
# Author: Default
# Category: Regions

import microscope, MicroscopeModules, os, time

# Specify the whether the region scan saves images, SAVE_DISPLAY, or just displays them, DISPLAY_ONLY
action = MicroscopeModules.RegionScan.SAVE_DISPLAY

aborted = False
rs = MicroscopeModules.RegionScan()

def GetTimeLapsePointList():
    """Get a list of lists of timelapse points rather than tuples which are non-mutable (unchangable)"""
    point_tuples = list(microscope.GetTimeLapsePoints())
    point_list = list()
    
    for point in point_tuples:
        point_list.append(list(point))
        
    return point_list

# OnStart is called once when the user presses start to begin the timelapse.
def OnStart():
    print "Start: Region Move and Save"
    global output, point_list
    data_dir = microscope.GetUserDataDirectory()
    output = microscope.ShowSimpleFileSequenceSaveDialog(data_dir) # This is really geared to regions and does not have the %. options
        
    if output != None:
        print "Saving to directory ", output[0]
        print "Filename Format ", output[1]
    else:
        print "Aborting Timelapse"
        microscope.AbortTimeLapseVisitPoints()
        
    point_list = GetTimeLapsePointList()
    print point_list
        
def OnAbort():
    global rs
    global aborted
    
    rs.Stop()
    aborted = True
    print "aborted" 
    
# Called once at the start of each cycle of points. Should contain at least one call to  microscope.MicroscopeVisitPoints() to start the cycle.
def OnCycleStart():
    microscope.VisitPoints()   
    
# Called each time the stage moves to a new point.
def OnNewPoint(x, y, z, position):
    global aborted, point_list

    print "Point: ", position
    print ("input: x %f, y %f, z %f" % (x, y, z))
    print "list:  ", point_list[position-1][2]

    # Adjust z to last autofocus position, first time round these two will be the same
    z = point_list[position-1][2]
    print "Adjust z to last autofocus position: z=", z
	
    if aborted:
        pass

    microscope.SetStagePosition(x, y, z)
    # Wait for stage to finish moving (additional delay seconds, time out seconds)
    microscope.WaitForStage(0.0, 1.0)
    
    output_directory = output[0]
    filename = output[1]
    filename_ext = output[2]

    #filename = microscope.ParseSequenceFilename(filename, position) 
    #filename = microscope.InsertCubeIntoFilename(filename, current_cube_index+1)

    #subdir = '%s\\Cube%d\\Region%d\\' % (output_directory, cubes[current_cube_index], position)
    #subdir = '%s\\Region%d\\' % (output_directory, position)
    subdir = '%s\\Region%d_%s\\' % (output_directory, position, time.strftime("%d%b%Y_%Hh%Mm%Ss", time.localtime(time.time())))

    try:
        os.makedirs(subdir)
    except (OSError):
        print 'os.makedirs(' + subdir + ') FAILED'

    print "Autofocus"
    microscope.PerformSoftwareAutoFocus()   
    autoFocusedZPosition = microscope.GetStagePosition()[2] 
    point_list[position-1][2] = autoFocusedZPosition
    print "Auto focused position stored:", point_list[position-1][2]

    # get the focal plane options for this region and transfer to the region scan module, correct for the autofocus
    rs.SetFocalPlaneOptions(microscope.GetFocalPlaneOptions())
    rs.SetFocalPlaneOffsetFromXYZ(x, y, autoFocusedZPosition)

    # Set region scan with the region width and height
    rs.SetRelativeRoiFromTimelapse(microscope.GetTimeLapseRegion())
    rs.Start(action, subdir, filename, filename_ext)
    rs.Hide()
    
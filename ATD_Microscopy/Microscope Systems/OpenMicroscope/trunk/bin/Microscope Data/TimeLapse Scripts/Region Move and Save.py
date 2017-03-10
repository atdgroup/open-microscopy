# Name: Region Move and save
# Description: Move stage, get image(s) and save
# Author: Default
# Category: Regions

import microscope, MicroscopeModules, os, time

# Specify the whether the region scan saves images, SAVE_DISPLAY, or just displays them, DISPLAY_ONLY
action = MicroscopeModules.RegionScan.SAVE_DISPLAY

aborted = False
rs = MicroscopeModules.RegionScan()

# OnStart is called once when the user presses start to begin the timelapse.
def OnStart():
    print "Start: Region Move and Save"
    global output
    data_dir = microscope.GetUserDataDirectory()
    output = microscope.ShowSimpleFileSequenceSaveDialog(data_dir) # This is really geared to regions and does not have the %. options
        
    if output != None:
        print "Saving to directory ", output[0]
        print "Filename Format ", output[1]
    else:
        print "Aborting Timelapse"
        microscope.AbortTimeLapseVisitPoints()
        
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
    global aborted

    print "Point: ", position

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

    # get the focal plane options for this region and transfer to the region scan module
    rs.SetFocalPlaneOptions(microscope.GetFocalPlaneOptions())

    # Set region scan with the region width and height
    rs.SetRelativeRoiFromTimelapse(microscope.GetTimeLapseRegion())
    rs.Start(action, subdir, filename, filename_ext)
    rs.Hide()
    
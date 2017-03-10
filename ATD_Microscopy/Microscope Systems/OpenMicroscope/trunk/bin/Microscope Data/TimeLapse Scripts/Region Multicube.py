# Name: Region Multicube
# Description: Aquire regions with all cubes, uses UI cube options for exposure and Z offset
# Author: Fluorescence Acq
# Category: Regions

import microscope, MicroscopeModules, os, time

# Specify the whether the region scan saves images, SAVE_DISPLAY, or just displays them, DISPLAY_ONLY
action = MicroscopeModules.RegionScan.SAVE_DISPLAY

aborted = False
rs = MicroscopeModules.RegionScan()
cube_data = {}

# OnStart is called once when the user presses start to begin the timelapse.
def OnStart():
    print "Start: Region Multicube"
    global output, cube_data

    data_dir = microscope.GetUserDataDirectory()
    output = microscope.ShowSimpleFileSequenceSaveDialog(data_dir) # This is really geared to regions and does not have the %. options
        
    if output != None:
        print "Saving to directory ", output[0]
        print "Filename Format ", output[1]
    else:
        print "Aborting Timelapse"
        microscope.AbortTimeLapseVisitPoints()

    cube_data = microscope.GetCubes();

        
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
    global aborted, cube_data

    print "Point: ", position

    if aborted:
        pass
        
    output_directory = output[0]
    filename_ext = output[2]
    subdir = '%sRegion%d\\' % (output_directory, position)   # all cubes in one folder for Mosaic to make a composite RGB mosaic
    #subdir = '%s\\Region%d_%s\\' % (output_directory, position, time.strftime("%d%b%Y_%Hh%Mm%Ss", time.localtime(time.time())))  # use this to time stamp each region mosaic

    try:
        os.makedirs(subdir)
    except (OSError):
        print 'os.makedirs(' + subdir + ') FAILED - folder may exist already.'

    for cube in cube_data:

        if aborted:
            break

        print "Moving to region centre (%d,%d)" % (x, y)
        microscope.SetStagePosition(x, y, z)  # no need to apply z offset here, use it below

        # Wait for stage to finish moving (additional delay seconds, time out seconds)
        microscope.WaitForStage(0.0, 1.0)

        print "Selecting Cube ", cube['Position'], ": ", cube['Name']
        microscope.MoveCubeToPosition(cube['Position'])
        
        options = microscope.GetCubeOptions(cube['Position'])  # camera exposure, gain and focus offset
        print "Setting camera exposure to ", options[0]
        microscope.SetExposure(options[0])
        print "Setting camera gain to ", options[1]
        microscope.SetGain(options[1])
    
        #filename = output[1]
        #filename = microscope.InsertCubeIntoFilename(filename, cube['Position']) # do not use this here as filename from the simple select dialog does not have an extension, safer is the following
        filename = output[1] + "_" + cube['Name']
        print filename
         
        # individual folders for cubes
        #subdir = '%sCube%d\\Region%d\\' % (output_directory, cube['Position'], position) 
        #try:
        #    os.makedirs(subdir)
        #except (OSError):
        #    print 'os.makedirs(' + subdir + ') FAILED'

        # get the focal plane options for this region and transfer to the region scan module
        print "Offset focal plane by ", options[2]
        rs.SetFocalPlaneOptions(microscope.GetFocalPlaneOptions()) # timelapse based focal plane/options
        rs.SetFocalPlaneOffset(options[2]) # cube based offset

        # Set region scan with the region width and height
        print "Scanning Region"
        rs.SetRelativeRoiFromTimelapse(microscope.GetTimeLapseRegion())
        rs.Start(action, subdir, filename, filename_ext)
        rs.Hide()
    
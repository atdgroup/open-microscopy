# Name: Region Move and display
# Description: Move stage, get image and display
# Author: Default
# Category: Regions

import microscope, MicroscopeModules, time

# Specify the whether the region scan saves images, SAVE_DISPLAY, or just displays them, DISPLAY_ONLY
action = MicroscopeModules.RegionScan.DISPLAY_ONLY

aborted = False
rs = MicroscopeModules.RegionScan()

# OnStart is called once when the user presses start to begin the timelapse.
def OnStart():
    print "Start: Region Move and display"
        
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

    if microscope.GetTimeLapseHasRegion():
        # PROCESS A REGION
        # get the focal plane options for this region and transfer to the region scan module
        rs.SetFocalPlaneOptions(microscope.GetFocalPlaneOptions())

        # Set region scan with the region width and height
        rs.SetRelativeRoiFromTimelapse(microscope.GetTimeLapseRegion())
        rs.Start(action, "", "", "")
        rs.Hide()
    else:
        # PROCESS A POINT, we do this here just to show we can
        microscope.SnapImage()
